/**
 * @file osdp_message_printer_json.h
 * @brief Public interface for decoded OSDP message printing functions in JSON format.
 *
 * This module exposes functions used to display OSDP commands and replies on structured JSON objects suitable. The provided APIs generate a human-readable
 * representation of OSDP message contents and can be used by various logging and rendering backends.
 */

#ifndef OSDP_MESSAGE_PRINTER_JSON_H
#define OSDP_MESSAGE_PRINTER_JSON_H

#include <stdio.h>
#include "../moduleOSDP/osdp_common.h"

void print_osdp_cmd_json(FILE *out, const osdp_cmd *cmd, size_t len);
void print_osdp_reply_json(FILE *out, const osdp_reply *reply, size_t len);

#endif // OSDP_MESSAGE_PRINTER_JSON_H