#ifndef CLI_COMMAND_RESULT_H
#define CLI_COMMAND_RESULT_H

#include <stdio.h>

/* One process executes one command. Diagnostics are retained until finalization. */
void cli_set_error(const char *format, ...);
void cli_diagnostic(FILE *stream, const char *format, ...);
void cli_error_code(const char *code);
int cli_reply_result(int size, unsigned char code, int valid);
int cli_finish(int status, int machine_output);

#endif
