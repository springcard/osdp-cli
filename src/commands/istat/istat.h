#ifndef ISTAT_H
#define ISTAT_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../../moduleOSDP/osdp_common.h"
struct cli_context;
struct istat_options {
    const char *port;
    const char *baud;
    int addr, json, timeout, retries, timeout_set, retries_set;
    int scbk_d, scbk_set;
    uint8_t scbk[16];
};
struct istat_result {
    const char *code;
    bool authenticated;
    int reply_code, nak_code;
    size_t count;
    uint8_t inputs[MAX_SIZE_MSG_PACKET];
};
int istat_parse(int argc, char **argv, struct istat_options *opts);
void istat_read(struct istat_options *opts, struct istat_result *result);
void istat_emit(const struct istat_options *opts, const struct istat_result *result);
int istat_run(struct cli_context *ctx);
#endif
