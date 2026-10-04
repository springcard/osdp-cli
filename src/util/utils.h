
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

void secure_zero(void *buffer, size_t size);

// Utility functions header
void print_hex(const void *data, size_t size);
int validate_input(const char *input);
char* byteArrayToHexaString(const uint8_t* byteArray, size_t byteArraySize);
uint8_t* hexaString2ByteArray(const char* hexString, size_t byteArraySize);
void delay_ms(unsigned int ms);
uint64_t get_timestamp_ns(void);

/**
 * @brief Map OSDP command opcode to human-readable name.
 *
 * Provides a textual representation of known OSDP command codes for inclusion in JSON log entries.
 *
 * @param code OSDP command opcode.
 * @return Human-readable command name or "UNKNOWN".
 */
const char *cmd_name(uint8_t code);


/**
 * @brief Map OSDP reply opcode to human-readable name.
 *
 * Provides a textual representation of known OSDP reply codes for inclusion in JSON log entries.
 *
 * @param code OSDP reply opcode.
 * @return Human-readable reply name or "UNKNOWN".
 */
const char *reply_name(uint8_t code);

#endif // UTILS_H
