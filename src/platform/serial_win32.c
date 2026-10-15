#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>

#include "serial.h"
#include "../core/commissioning_control.h"
#include "../util/utils.h"

static HANDLE handle /*hSerial*/ = INVALID_HANDLE_VALUE;
static bool diagnostics_enabled = false;
static int read_timeout_ms = 100;
void serial_set_diagnostics(bool enabled) { diagnostics_enabled = enabled; }



/**
 * @brief configure a serial port
 * 
 * The function `configure_serial_port` sets up the communication parameters and timeouts for a serial
 * port based on the specified baud rate.
 * 
 * @param pszBaudRate is a pointer to a character array that represents the baud rate at which the serial port should operate. 
 * The function converts to int and uses this parameter to set the baud rate in the configuration of the serial port
 * communication parameters. 
 * @param timeout Frame read timeout.
 * 
 * @return The function `configure_serial_port` is returning a `HANDLE` which represents a handle to the configured serial port.
 */
static HANDLE configure_serial_port(const char * pszBaudRate, int32_t timeout) 
{
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    // retrieving the current state of the serial port
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(handle, &dcbSerialParams)) {
        if (diagnostics_enabled) fprintf(stderr, "Erreur lors de la recuperation de l'etat du port serie\n");
        CloseHandle(handle);
        return INVALID_HANDLE_VALUE;
    }

    // set the communication parameters for the serial port, including the baud rate, byte size, stop bits, and parity.
    dcbSerialParams.BaudRate = strtoul(pszBaudRate, NULL, 10);
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    
    // attempting to configure the serial port parameters by setting the communication state
    if (!SetCommState(handle, &dcbSerialParams)) {
        if (diagnostics_enabled) fprintf(stderr, "Erreur lors de la configuration du port serie\n");
        CloseHandle(handle);
        return INVALID_HANDLE_VALUE;
    }

    // Configuring the timeouts for reading and writing operations on the serial port
    timeouts.ReadIntervalTimeout = 50;  // Timeout en millisecondes
    timeouts.ReadTotalTimeoutConstant = timeout;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10; 

    if (!SetCommTimeouts(handle, &timeouts)) {
        if (diagnostics_enabled) fprintf(stderr, "Erreur lors de la configuration des timeouts\n");
        CloseHandle(handle);
        return INVALID_HANDLE_VALUE;
    }

    // clear the input and output buffers of the specified serial port handle
    PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return handle;
}


/**
 * @brief open a serial port on Windows
 *
 * The function `serial_open` attempts to open a serial port on Windows using the provided port name
 * and baud rate.
 * 
 * @param pszPortCom The `pszPortCom` parameter in the `serial_open` function is a pointer to a
 * character array that represents the name of the serial port to be opened. It typically corresponds
 * to the COM port on Windows systems, such as "COM1", "COM2", etc.
 * @param pszBaudRate The `pszBaudRate` parameter in the `serial_open` function is a pointer to a
 * character array that represents the baud rate at which the serial port should operate. Baud rate is
 * the rate at which data is transferred in a communication channel, typically measured in bits per
 * second (bps).
 * @param timeout Frame read timeout.
 * 
 * @returns  The `serial_open` function returns a boolean value indicating whether the serial port was successfully opened and configured.
 * It returns `true` if the port was opened and configured successfully, and `false` if there was an error during the opening or configuration process.
 */
bool serial_open(const char *pszPortCom, const char *pszBaudRate, int32_t timeout) 
{
    read_timeout_ms = timeout > 0 ? timeout : 100;
    char full_port_name[32];

    // Formats full port name by prepending the special prefix "\\.\" to the provided port name (pszPortCom). 
    // Necessary for COM ports numbered 10 and above on Windows systems. 
    snprintf(full_port_name, sizeof(full_port_name), "\\\\.\\%s", pszPortCom);   
    if (diagnostics_enabled) fprintf(stderr, "Opening serial port (Win32)\n");

    // Attempts to open the specified serial port with the full port name
    handle = CreateFile(full_port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) 
    {
        if (diagnostics_enabled) fprintf(stderr, "Serial port unavailable\n");
        return false;
    }

    // Configures the opened serial port with the specified baud rate and other communication parameters
    handle =  configure_serial_port(pszBaudRate, timeout);

    if (handle == INVALID_HANDLE_VALUE) 
    {  
        if (diagnostics_enabled) fprintf(stderr, "Erreur lors de la configuration du port RS485\n\n");
        return false;  // Erreur lors de l'ouverture du port sÃƒÆ’Ã‚Â©rie
    }

    return true;
}


/**
 * @brief close a serial port on Windows
 *
 * The function serial_close() closes a serial port in a Win32 environment and handles any errors
 * related to timeouts.
 */
void serial_close() 
{
    if (handle != INVALID_HANDLE_VALUE) 
    {
        if (diagnostics_enabled) fprintf(stderr, "Closing serial port (Win32)\n\n");
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
    }
}


/**
 * @brief read data from a serial port on Windows
 * 
 * The function `serial_read` reads data from a serial port and prints the received data in hexadecimal
 * format.
 * 
 * @param buffer is a pointer to a memory location where the data read from the serial port will be stored.
 * 
 * @param size parameter represents the number of bytes to read from the serial port into the `buffer`. 
 * It specifies the maximum number of bytes that can be read at a time from the serial port.
 * 
 * @return The `serial_read` function returns the number of bytes read from the serial port.
 */
int serial_read(void *buffer, size_t size) 
{
    if (commissioning_active()) {
        int started_remaining = commissioning_remaining_ms();
        while (commissioning_remaining_ms() > 0) {
            COMMTIMEOUTS limits = {0};
            limits.ReadIntervalTimeout = 50;
            int remaining = commissioning_remaining_ms();
            int reply_remaining = read_timeout_ms - (started_remaining - remaining);
            if (reply_remaining <= 0) break;
            if (remaining > reply_remaining) remaining = reply_remaining;
            limits.ReadTotalTimeoutConstant = remaining > 50 ? 50 : remaining;
            limits.WriteTotalTimeoutConstant = remaining > 50 ? 50 : remaining;
            if (!SetCommTimeouts(handle, &limits)) return -1;
            DWORD count = 0;
            if (!ReadFile(handle, buffer, size, &count, NULL)) return -1;
            if (count) return (int)count;
        }
        return 0;
    }
    DWORD bytes_read = 0;

    if (!ReadFile(handle, buffer, size, &bytes_read, NULL)) 
    {
        if (diagnostics_enabled) fprintf(stderr, "Serial read failed\n");
        return -1;
    }

    return (int) bytes_read;    
}


/**
 * @brief write data to a serial port on Windows
 * 
 * The function `serial_write` writes data from a buffer to a serial port and prints the data sent in
 * hexadecimal format.
 * 
 * @param buffer is a pointer to the data to write to a serial port. It is of type `void*`, which means it is a pointer to an unspecified type. In this context, it
 * is expected to point to the data that you want to send over the serial
 * @param size The `size` parameter in the `serial_write` function represents the size in bytes of the
 * data to be written from the `buffer` to the serial port. It indicates the number of bytes that
 * should be sent over the serial connection.
 * 
 * @return The `serial_write` function returns the number of bytes that were successfully written to
 * the serial port.
 */
int serial_write(const void *buffer, size_t size) 
{
    if (commissioning_active()) {
        int remaining = commissioning_remaining_ms();
        if (remaining <= 0) return -1;
        COMMTIMEOUTS limits = {0};
        limits.ReadIntervalTimeout = 50;
        limits.ReadTotalTimeoutConstant = remaining > 50 ? 50 : remaining;
        limits.WriteTotalTimeoutConstant = remaining > 50 ? 50 : remaining;
        if (!SetCommTimeouts(handle, &limits)) return -1;
    }
    DWORD bytes_written = 0;

    if (!WriteFile(handle, buffer, size, &bytes_written, NULL)) 
    {
        if (diagnostics_enabled) fprintf(stderr, "Serial write failed\n");
        return -1;
    } 

    return (int) bytes_written;
}
