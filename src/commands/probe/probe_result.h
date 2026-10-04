#ifndef PROBE_RESULT_H
#define PROBE_RESULT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "probe_options.h"
#include "../../moduleOSDP/osdp_common.h"

/* Internal result, independent of the unattended interface. */
enum probe_status {
    PROBE_OK, PROBE_NO_RESPONSE, PROBE_COMMUNICATION_ERROR,
    PROBE_AUTHENTICATION_FAILED, PROBE_PORT_UNAVAILABLE, PROBE_UNSUPPORTED_DEVICE,
    PROBE_INVALID_ARGUMENT, PROBE_ENTROPY_ERROR
};
struct probe_result {
    enum probe_status status;
    bool has_id;
    bool has_capabilities;
    bool authenticated;
    uint8_t id[REPLY_PDID_DATA_LEN];
    uint8_t capabilities[MAX_SIZE_MSG_PACKET];
    size_t capabilities_size;
};
const char *probe_status_name(enum probe_status status);
void probe_device(struct probe_options *opts, int timeout, int attempts, struct probe_result *result);
enum probe_status probe_read_information(struct probe_options *opts, int attempts, struct probe_result *result);
#endif
