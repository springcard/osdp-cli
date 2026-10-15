#ifndef SERIAL_TX_CONTROL_H
#define SERIAL_TX_CONTROL_H
#include <stdbool.h>
bool serial_tx_control_enabled(void);
bool serial_tx_control_open(void);
bool serial_tx_control_set(bool transmit);
void serial_tx_control_close(void);
#endif
