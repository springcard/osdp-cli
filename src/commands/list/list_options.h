#ifndef LIST_OPTIONS_H
#define LIST_OPTIONS_H

struct cli_context;

struct list_options {
    const char *port;
    const char *baud;
    int first_addr;
    int last_addr;
    int timeout;
    int retries;
    int json;
    int progress;
    int deadline;
    int tx_control;
};

int list_parse(int argc, char **argv, struct list_options *opts);
int list_run(struct cli_context *ctx);

#endif
