/**
 * @file osdp_message_printer.h
 * @brief Public interface for decoded OSDP message printing functions.
 *
 * This module exposes functions used to display decoded OSDP commands and replies. The provided APIs generate a human-readable
 * representation of OSDP message contents and can be used by various logging and rendering backends.
 */

#ifndef OSDP_MESSAGE_PRINTER_H
#define OSDP_MESSAGE_PRINTER_H

#include <stdio.h>
#include "../moduleOSDP/osdp_common.h"

void print_osdp_cmd(FILE *out, const osdp_cmd *cmd, size_t len);
void print_osdp_reply(FILE *out, const osdp_reply *reply, size_t len);

#endif // OSDP_MESSAGE_PRINTER_H