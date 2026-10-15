#include "command_result.h"
#include <stdarg.h>
#include <string.h>

static char error[512];

static void remember(const char *format, va_list args)
{
    vsnprintf(error, sizeof(error), format, args);
    /* A diagnostic must always fit on one terminal line. */
    for (char *p = error; *p; ++p)
        if (*p == '\n' || *p == '\r' || *p == '\t') *p = ' ';
    size_t length = strlen(error);
    while (length && error[length - 1] == ' ') error[--length] = '\0';
}

void cli_set_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    remember(format, args);
    va_end(args);
}

void cli_diagnostic(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
    va_start(args, format);
    remember(format, args);
    va_end(args);
}

void cli_error_code(const char *code)
{
    static const struct { const char *code; const char *message; } messages[] = {
        {"no_response", "No response from device (timeout)"},
        {"authentication_failed", "Secure Channel authentication failed"},
        {"port_unavailable", "Serial port unavailable"},
        {"unsupported_device", "Device does not support the requested operation"},
        {"entropy_error", "Secure random generation failed"},
        {"communication_error", "Communication failed (invalid frame or transport error)"},
        {"invalid_request", "Invalid or missing command options"},
        {"invalid_argument", "Invalid or missing command options"},
        {"unexpected_reply", "Unexpected device reply"},
        {"device_busy", "Device is busy"},
        {"cancelled", "Operation cancelled"},
        {"verification_failed", "Verification failed"},
        {"verification_timeout", "Verification timed out; outcome unknown"},
        {"outcome_unknown", "Operation outcome unknown; verify device before retrying"},
        {"partial_scan", "Device scan incomplete"}
    };
    if (!strcmp(code, "ok")) return;
    for (size_t i = 0; i < sizeof(messages) / sizeof(messages[0]); ++i) {
        if (!strcmp(code, messages[i].code)) {
            cli_set_error("%s", messages[i].message);
            return;
        }
    }
    cli_set_error("Operation failed (%s)", code);
}

int cli_reply_result(int size, unsigned char code, int valid)
{
    if (size >= 0 && valid) return 0;
    if (size == -1) cli_error_code("no_response");
    else if (size < 0) cli_error_code("communication_error");
    else if (code == 0x41) cli_set_error("Device rejected command (NAK)");
    else cli_set_error("Unexpected or malformed device reply (0x%02X)", code);
    return 1;
}

int cli_finish(int status, int machine_output)
{
    if (fflush(stdout) != 0 || ferror(stdout)) {
        status = status ? status : 1;
        cli_set_error("Failed to write command output");
        machine_output = 1;
    }
    fflush(stderr);
    FILE *stream = machine_output ? stderr : stdout;
    int written = status == 0 ? fprintf(stream, "SUCCESS\n") :
        fprintf(stream, "ERROR: %s\n", error[0] ? error : "Command failed");
    if (written < 0 || fflush(stream) != 0) return status ? status : 1;
    return status < 0 ? 1 : status;
}
