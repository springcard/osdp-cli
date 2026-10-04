#ifndef COMMISSIONING_CONTROL_H
#define COMMISSIONING_CONTROL_H
#include <stdbool.h>
void commissioning_begin(int timeout_ms);
void commissioning_end(void);
bool commissioning_active(void);
bool commissioning_cancelled(void);
int commissioning_remaining_ms(void);
#endif
