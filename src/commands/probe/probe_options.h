#ifndef PROBE_OPTIONS_H
#define PROBE_OPTIONS_H
#include <stdint.h>
struct cli_context;
struct probe_options {
    const char *port;
    const char *baud;
    int addr;
    uint8_t key[16];
    int key_set;
};
int probe_parse(int argc, char **argv, struct probe_options *opts);
int probe_run(struct cli_context *ctx);
#endif
