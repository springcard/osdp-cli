/* Generic optional Linux sysfs GPIO direction, independent of product boards. */
#include "serial.h"
#include "serial_tx_control.h"
#ifdef _WIN32
bool serial_set_tx_control(int gpio) { return gpio == -1; }
bool serial_tx_control_enabled(void) { return false; }
bool serial_tx_control_open(void) { return true; }
bool serial_tx_control_set(bool transmit) { (void)transmit; return true; }
void serial_tx_control_close(void) {}
#else
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>
static int selected_gpio = -1, value_fd = -1;
bool serial_set_tx_control(int gpio)
{
    if (gpio < -1 || gpio > 65535 || value_fd >= 0) { errno = EINVAL; return false; }
    selected_gpio = gpio;
    return true;
}
bool serial_tx_control_enabled(void) { return selected_gpio >= 0; }
static bool write_attribute(const char *path, const char *value, size_t size)
{
    int attribute = open(path, O_WRONLY | O_CLOEXEC);
    if (attribute < 0) return false;
    ssize_t written;
    do { written = write(attribute, value, size); } while (written < 0 && errno == EINTR);
    int saved = errno;
    close(attribute);
    errno = saved;
    return written == (ssize_t)size;
}
bool serial_tx_control_set(bool transmit)
{
    if (!serial_tx_control_enabled()) return true;
    if (value_fd < 0 || lseek(value_fd, 0, SEEK_SET) < 0) return false;
    ssize_t written;
    do { written = write(value_fd, transmit ? "1" : "0", 1); } while (written < 0 && errno == EINTR);
    return written == 1;
}
void serial_tx_control_close(void)
{
    if (value_fd >= 0) {
        (void)serial_tx_control_set(false);
        close(value_fd);
        value_fd = -1;
    }
}
bool serial_tx_control_open(void)
{
    char direction[96], value[96], number[16];
    if (!serial_tx_control_enabled()) return true;
    snprintf(direction, sizeof(direction), "/sys/class/gpio/gpio%d/direction", selected_gpio);
    snprintf(value, sizeof(value), "/sys/class/gpio/gpio%d/value", selected_gpio);
    if (access(direction, F_OK) != 0) {
        int length = snprintf(number, sizeof(number), "%d", selected_gpio);
        if (!write_attribute("/sys/class/gpio/export", number, (size_t)length) && errno != EBUSY) return false;
        unsigned waited = 0;
        while (access(direction, F_OK) != 0 && waited++ < 100) (void)poll(NULL, 0, 10);
    }
    /* Atomic direction + initial low level avoids a transmit glitch. */
    if (!write_attribute(direction, "low", 3)) return false;
    value_fd = open(value, O_WRONLY | O_CLOEXEC);
    if (value_fd < 0) return false;
    if (!serial_tx_control_set(false)) { serial_tx_control_close(); return false; }
    return true;
}
#endif
