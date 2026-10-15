#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Serial communication header
bool serial_open(const char *pszPortCom, const char *pszBaudRate, int32_t timeout);
void serial_close();
int serial_read(void *buffer, size_t size);
int serial_write(const void *buffer, size_t size);
void serial_set_diagnostics(bool enabled);
/* Optional Linux sysfs direction GPIO, active high. -1 disables control. */
bool serial_set_tx_control(int gpio);

#endif // SERIAL_H
