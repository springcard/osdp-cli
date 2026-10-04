#ifndef OSDP_RANDOM_H
#define OSDP_RANDOM_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Fills the entire buffer or erases it and returns false. No weak fallback. */
bool osdp_random_bytes(uint8_t *buffer, size_t size);
#endif
