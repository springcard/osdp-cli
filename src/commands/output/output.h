#ifndef OUTPUT_H
#define OUTPUT_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../../moduleOSDP/osdp_common.h"
struct cli_context;
enum output_mode { OUTPUT_OFF, OUTPUT_ON, OUTPUT_UNSET = -1 };
struct output_options {
    const char *port, *baud;
    int addr, json, timeout, retries, timeout_set, retries_set;
    int scbk_d, scbk_set;
    uint8_t scbk[16];
    bool status_only;
    int output, mode, duration, duration_set;
};
struct output_result {
    const char *code;
    bool authenticated, acknowledged, uncertain, has_status;
    int reply_code, nak_code;
    size_t count;
    uint8_t states[MAX_SIZE_MSG_PACKET];
};
int output_parse(int argc, char **argv, struct output_options *opts, bool status_only);
void output_execute(struct output_options *opts, struct output_result *result);
void output_emit(const struct output_options *opts, const struct output_result *result);
int output_run(struct cli_context *ctx);
#endif
