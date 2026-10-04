#include "serial.h"
#include "../core/commissioning_control.h"
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/serial.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/file.h>

#include "../util/utils.h"


struct baud_map {
    unsigned long baud;
    speed_t speed;
};

static const struct baud_map baud_table[] = {
    { 1200,   B1200 },
    { 1800,   B1800 },
    { 2400,   B2400 },
    { 4800,   B4800 },
    { 9600,   B9600 },
    { 19200,  B19200 },
    { 38400,  B38400 },
    { 57600,  B57600 },
    { 115200, B115200 },
    { 230400, B230400 },
    { 460800, B460800 },
    { 500000, B500000 },
    { 576000, B576000 },
    { 921600, B921600 }
};


static int fd = -1;
static bool exclusive;
static bool diagnostics_enabled = true;
void serial_set_diagnostics(bool enabled) { diagnostics_enabled = enabled; }



/** 
 * @brief converts a baud rate to a termios speed value
 * 
 * The function `baudrate_to_speed` takes an unsigned long integer representing a baud rate and a pointer to a `speed_t` variable. 
 * It iterates through a predefined table of baud rates and their corresponding termios speed values. 
 * If it finds a match for the provided baud rate, it assigns the corresponding speed value to the variable pointed to by `speed` and returns `true`. 
 * If no match is found, it returns `false`.
 * 
 * @param baud The `baud` parameter is an unsigned long integer that represents the baud rate you want to convert to a termios speed value.
 * @param speed The `speed` parameter is a pointer to a `speed_t` variable where the corresponding termios speed value will be stored if a match is found for the provided baud rate.
 * 
 * @return The `baudrate_to_speed` function returns a boolean value indicating whether the conversion was successful. 
 *         It returns `true` if a matching baud rate was found in the table and the corresponding speed value was assigned to the variable pointed to by `speed`. 
 *         If no match is found, it returns `false`.
 */
static bool baudrate_to_speed(unsigned long baud, speed_t *speed)
{
    for (size_t i = 0; i < sizeof(baud_table)/sizeof(baud_table[0]); i++) {
        if (baud_table[i].baud == baud) {
            *speed = baud_table[i].speed;
            return true;
        }
    }
    return false;
}


/** 
 * @brief configures the serial port 
 * 
 * The function `configure_serial_port` takes a string representing the baud rate and configures the serial port accordingly. 
 * It converts the baud rate string to an unsigned long integer and then uses the `baudrate_to_speed` function to convert it to a termios speed value. 
 * If the conversion is successful, it sets the terminal attributes for the serial port.
 * 
 * @param pszBaudRate The `pszBaudRate` parameter is a string representing the desired baud rate.
 * 
 * @return It returns `true` if the serial port was configured successfully, and `false` otherwise.
 */
static bool configure_serial_port(const char *pszBaudRate, int32_t timeout) {
    struct termios tty;
    speed_t baud_rate;

    // Convert the baud rate string to an unsigned long integer
    unsigned long baud = strtoul(pszBaudRate, NULL, 10);

    // Get current terminal attributes
    if (tcgetattr(fd, &tty) != 0) {
        if (diagnostics_enabled) fprintf(stderr, "Cannot read serial attributes\n");
        return false;
    }
 
    if(!baudrate_to_speed(baud, &baud_rate)) {
        if (diagnostics_enabled) fprintf(stderr, "Invalid serial baud rate\n");
        return false;
    }

    // Set input and output baud rates
    cfsetispeed(&tty, baud_rate);
    cfsetospeed(&tty, baud_rate);

    // Configure 8N1, no flow control
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8 data bits
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines

    // Disable canonical mode, echo, etc.
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;

    // Disable input processing
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // Disable output processing
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;

    // Set timeouts
    tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = timeout <= 0 ? 1 : (timeout > 25500 ? 255 : (timeout + 99) / 100);

    // Apply the settings
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        if (diagnostics_enabled) fprintf(stderr, "Cannot set serial attributes\n");
        return false;
    }

    // Clear the O_NDELAY flag so that read() blocks
    if (fcntl(fd, F_SETFL, commissioning_active() ? O_NONBLOCK : 0) < 0) return false;

    return true;
}


/** 
 * @brief opens the serial port
 * 
 * The function `serial_open` takes a string representing the port name and the desired baud rate, and opens the serial port accordingly.
 * It calls the `configure_serial_port` function to set up the serial port configuration.
 * 
 * @param port The `port` parameter is a string representing the serial port device file (e.g., "/dev/ttyUSB0").
 * @param pszBaudRate The `pszBaudRate` parameter is a string representing the desired baud rate.
 * 
 * @return It returns `true` if the serial port was opened successfully, and `false` otherwise.
 */
bool serial_open(const char *port, const char *pszBaudRate, int32_t timeout) {
    // Open the serial port
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        if (diagnostics_enabled) fprintf(stderr, "Serial port unavailable\n");
        return false;
    }

    if (commissioning_active()) {
        if (flock(fd, LOCK_EX | LOCK_NB) < 0 || ioctl(fd, TIOCEXCL) < 0) {
            close(fd); fd = -1;
            return false;
        }
        exclusive = true;
    }

    if (!configure_serial_port(pszBaudRate, timeout)) {
        if (exclusive) { ioctl(fd, TIOCNXCL); exclusive = false; }
        close(fd);
        fd = -1;
        return false;
    }

    if (diagnostics_enabled) fprintf(stderr, "Serial port opened\n");
    return true;
}


/** 
 * @brief closes the serial port
 * 
 * The function `serial_close` closes the serial port and sets the file descriptor to -1.
 */
void serial_close(void) {
    if (fd != -1) {
        if (exclusive) { ioctl(fd, TIOCNXCL); exclusive = false; }
        close(fd);
        fd = -1;
        if (diagnostics_enabled) fprintf(stderr, "Serial port closed\n");
    }
}


/** 
 * @brief reads data from the serial port
 * 
 * The function `serial_read` reads data from the serial port into a buffer and returns the number of bytes read. 
 * If there is an error during reading, it prints an error message and returns -1.
 * 
 * @param buffer The `buffer` parameter is a pointer to a memory location where the data read from the serial port will be stored.
 * @param size The `size` parameter represents the number of bytes to read from the serial port into the `buffer`. It specifies the maximum number of bytes that can be read at a time from the serial port.
 * 
 * @return It returns the number of bytes read from the serial port, or -1 if there was an error during reading.
 */
int serial_read(void *buffer, size_t size) {
    if (commissioning_active()) {
        while (commissioning_remaining_ms() > 0) {
            int remaining = commissioning_remaining_ms();
            struct pollfd poll_fd = {fd, POLLIN, 0};
            int ready = poll(&poll_fd, 1, remaining > 50 ? 50 : remaining);
            if (ready < 0 && errno == EINTR) continue;
            if (ready < 0 || (poll_fd.revents & (POLLERR | POLLNVAL | POLLHUP))) return -1;
            if (ready > 0) {
                ssize_t count = read(fd, buffer, size);
                if (count < 0 && (errno == EINTR || errno == EAGAIN)) continue;
                return count == 0 ? -1 : (int)count; /* EOF is not a timeout. */
            }
        }
        return 0;
    }
	ssize_t bytes_read = 0;
	int total_bytes_read = 0;
	
    if (fd == -1) {
        return -1;
    }

	while (total_bytes_read < size) {
		bytes_read = read(fd, (unsigned char *)buffer + total_bytes_read, size - total_bytes_read);

		if (bytes_read > 0) {
			total_bytes_read += bytes_read;
		} else if (bytes_read == 0) {
			// timeout Ã¢â€ â€™ fin de trame
			break;
		} else {
			if (diagnostics_enabled) fprintf(stderr, "Serial read failed\n");
            return -1;
		}
	}

	return (int)total_bytes_read;
}


/** 
 * @brief writes data to the serial port
 * 
 * The function `serial_write` writes data from a buffer to the serial port and returns the number of bytes written. 
 * If there is an error during writing, it prints an error message and returns -1.
 * 
 * @param buffer The `buffer` parameter is a pointer to the data to write to the serial port.
 * @param size The `size` parameter represents the number of bytes to write from the `buffer` to the serial port.
 * 
 * @return It returns the number of bytes written to the serial port, or -1 if there was an error during writing.
 */
int serial_write(const void *buffer, size_t size) {
    if (commissioning_active() && commissioning_remaining_ms() <= 0) return -1;
    if (fd == -1) {
        return -1;
    }
    if (commissioning_active()) {
        size_t sent = 0;
        while (sent < size && commissioning_remaining_ms() > 0) {
            int remaining = commissioning_remaining_ms();
            struct pollfd poll_fd = {fd, POLLOUT, 0};
            int ready = poll(&poll_fd, 1, remaining > 50 ? 50 : remaining);
            if (ready < 0 && errno == EINTR) continue;
            if (ready < 0 || (poll_fd.revents & (POLLERR | POLLNVAL | POLLHUP))) return -1;
            if (ready == 0) continue;
            ssize_t count = write(fd, (const unsigned char *)buffer + sent, size - sent);
            if (count < 0 && (errno == EINTR || errno == EAGAIN)) continue;
            if (count <= 0) return -1;
            sent += (size_t)count;
        }
        return (int)sent;
    }
    ssize_t bytes_written = write(fd, buffer, size);
    if (bytes_written == -1) {
        if (diagnostics_enabled) fprintf(stderr, "Serial write failed\n");
    }

    return (int)bytes_written;
}
