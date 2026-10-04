#ifndef OSDP_COMMISSIONING_H
#define OSDP_COMMISSIONING_H
#include <stdbool.h>
#include "../probe/probe_result.h"
enum commissioning_operation { COMMISSIONING_PROBE, COMMISSIONING_AUTHENTICATE, COMMISSIONING_SET_KEY };
enum commissioning_code {
    COMM_OK, COMM_INVALID_REQUEST, COMM_PORT_UNAVAILABLE, COMM_NO_RESPONSE,
    COMM_COMMUNICATION_ERROR, COMM_AUTHENTICATION_FAILED, COMM_UNSUPPORTED_DEVICE,
    COMM_VERIFICATION_FAILED, COMM_VERIFICATION_TIMEOUT, COMM_OUTCOME_UNKNOWN, COMM_CANCELLED
};
struct commissioning_request {
    enum commissioning_operation operation;
    struct probe_options device;
    uint8_t target_key[16];
    int timeout_ms;
};
struct commissioning_result {
    enum commissioning_code code;
    bool unknown;
    bool authenticated;
    bool verified_target_key;
    struct probe_result device;
};
bool commissioning_parse(int argc, char **argv, struct commissioning_request *request);
void commissioning_execute(struct commissioning_request *request, struct commissioning_result *result);
int commissioning_main(int argc, char **argv);
#endif
