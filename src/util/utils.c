#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
/*#else
#include <time.h>*/
#endif

#include "utils.h"


/**
 * @brief Map OSDP command opcode to human-readable name.
 *
 * Provides a textual representation of known OSDP command codes for inclusion in JSON log entries.
 *
 * @param code OSDP command opcode.
 * @return Human-readable command name or "UNKNOWN".
 */
const char *cmd_name(uint8_t code)
{
    switch (code) {
        case 0x60: return "POLL";
        case 0x61: return "ID";
        case 0x62: return "CAP";
        case 0x64: return "LSTAT";
        case 0x65: return "ISTAT";
        case 0x66: return "OSTAT";
        case 0x67: return "RSTAT";	
        case 0x68: return "OUT";
        case 0x69: return "LED";    
        case 0x6A: return "BUZ";     
        case 0x6B: return "TEXT";    
        case 0x6E: return "COMSET";
        case 0x73: return "BIOREAD"; 
        case 0x74: return "BIOMATCH"; 
        case 0x75: return "KEYSET";
        case 0x76: return "CHLNG";
        case 0x77: return "SSCRYPT";
        case 0x7C: return "FILETRANSFER";
        case 0x80: return "MFG";
        default:   return "UNKNOWN";
    }
}


/**
 * @brief Map OSDP reply opcode to human-readable name.
 *
 * Provides a textual representation of known OSDP reply codes for inclusion in JSON log entries.
 *
 * @param code OSDP reply opcode.
 * @return Human-readable reply name or "UNKNOWN".
 */
const char *reply_name(uint8_t code)
{
    switch (code) {
    case 0x40: return "ACK";
    case 0x41: return "NAK";
    case 0x45: return "PDID";
    case 0x46: return "PDCAP";
    case 0x48: return "LSTATR";
    case 0x49: return "ISTATR";
    case 0x4A: return "OSTATR";
    case 0x4B: return "RSTATR";
    case 0x50: return "RAW";
    case 0x53: return "KEYPAD";
    case 0x54: return "COM";    
    case 0x57: return "BIOREADR";    
    case 0x58: return "BIOMATCR";    
    case 0x76: return "CCRYPT";    
    case 0x79: return "BUZY";    
    case 0x78: return "RMAC_I";    
    case 0x7A: return "FTSTAT";    
    case 0x90: return "MFGREP";   
    default:   return "UNKNOWN";
    }
}


/**
 * @brief Get timestamp
 * 
 * The function `get_timestamp_ns` returns the current timestamp in nanoseconds using the
 * CLOCK_MONOTONIC clock source.
 * 
 * @return a 64-bit unsigned integer representing the current
 * timestamp in nanoseconds since some unspecified starting point.
 */
uint64_t get_timestamp_ns(void)
{
#ifdef _WIN32
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);

    return (uint64_t)(counter.QuadPart / frequency.QuadPart) * 1000000000ULL +
           (uint64_t)(counter.QuadPart % frequency.QuadPart) * 1000000000ULL / frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}


/**
 * @brief introduce a delay in milliseconds
 * 
 * The `delay_ms` function is used to introduce a delay in milliseconds, utilizing different
 * implementations based on the operating system.
 * 
 * @param ms The `ms` parameter represents the number of milliseconds for which the delay should occur before the function returns.
 */
void delay_ms(unsigned int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}


void secure_zero(void *buffer, size_t size) {
    volatile unsigned char *p = buffer;
    while (size--) *p++ = 0;
}

void print_hex(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    for (size_t i = 0; i < size; ++i) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");
}

int validate_input(const char *input) {
    // TODO: Implement input validation
    if (input == NULL || *input == '\0') {
        return 0;
    }
    return 1; // Stub
}



uint8_t* hexaString2ByteArray(const char* hexString, size_t byteArraySize) {
    // Vérifie si la chaîne est valide
    if (!hexString || strlen(hexString) % 2 != 0) {
        return NULL; // Retourne NULL si la chaîne n'est pas valide
    }

    // Longueur de la chaîne
    size_t hexLength = strlen(hexString);
	    // Vérifie si longueur de la chaîne est valide
    if (byteArraySize != (hexLength / 2)) {
        return NULL; // Retourne NULL si la chaîne n'est pas valide
    }

    // Alloue la mémoire pour le tableau de bytes
    uint8_t* byteArray = malloc(byteArraySize);
    if (!byteArray) {
        return NULL; // Retourne NULL en cas d'erreur d'allocation
    }

    // Convertit chaque paire de caractères en un octet
    for (size_t i = 0; i < byteArraySize; i++) {
        char byteString[3] = { hexString[2 * i], hexString[2 * i + 1], '\0' };

        // Vérifie si les caractères sont bien hexadécimaux
        if (!isxdigit(byteString[0]) || !isxdigit(byteString[1])) {
            free(byteArray); // Libère la mémoire allouée en cas d'erreur
            return NULL;
        }

        // Convertit la paire de caractères en un octet
        byteArray[i] = (uint8_t) strtol(byteString, NULL, 16);
    }

    return byteArray;
}


char* byteArrayToHexaString(const uint8_t* byteArray, size_t byteArraySize) {
    if (!byteArray || byteArraySize == 0) {
        return NULL; // Retourne NULL si les arguments sont invalides
    }

    // Alloue la mémoire pour la chaîne hexadécimale (2 caractères par octet + 1 pour '\0')
    size_t hexStringLength = byteArraySize * 2 + 1;
    char* hexString = malloc(hexStringLength);
    if (!hexString) {
        return NULL; // Retourne NULL en cas d'erreur d'allocation
    }

    // Remplit la chaîne hexadécimale
    for (size_t i = 0; i < byteArraySize; i++) {
        sprintf(&hexString[i * 2], "%02X", byteArray[i]);
    }

    // Ajoute le caractère nul de fin de chaîne
    hexString[hexStringLength - 1] = '\0';

    return hexString;
}
