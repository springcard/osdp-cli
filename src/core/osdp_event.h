/**
 * @file osdp_event.h
 * @brief Public event system interface for OSDP protocol notifications.
 *
 * This module defines the core event structures and callback interfaces used by osdp-cli to propagate OSDP protocol activity between internal
 * components.
 *
 * The event system provides a lightweight observer mechanism allowing independent modules to subscribe to protocol events without creating
 * direct dependencies between the protocol layer and higher-level application components.
 *
 * Typical consumers include:
 *   - console loggers
 *   - JSON loggers
 *   - packet sniffers
 *   - monitoring utilities
 *   - future telemetry or tracing backends
 *
 * Each emitted event contains metadata describing a captured OSDP frame, including:
 *   - raw frame buffer
 *   - payload information
 *   - communication direction
 *   - secure channel status
 *   - event timestamp
 *
 * The dispatcher implementation is intentionally minimal and optimized for lightweight single-process event propagation.
 */

#ifndef OSDP_EVENT_H
#define OSDP_EVENT_H

#pragma once

#include <stdint.h>
#include <stddef.h>

// Communication direction: Control Panel to Peripheral Device
#define CP_2_PD 0
// Communication direction: Peripheral Device to Control Panel
#define PD_2_CP 1 // PD->CP


// Represents a captured OSDP protocol event.
// This structure encapsulates the information associated with a single OSDP frame exchange between a Control Panel (CP) and a Peripheral
// Device (PD).
struct osdp_event
{
    size_t frame_len; // Total length of the raw OSDP frame
    uint8_t *frame;   // Pointer to the raw OSDP frame buffer

    size_t payload_len;  // Length of the extracted payload data
    const void *payload; // Pointer to the decoded or extracted payload

    int direction;     // Communication direction identifier (CP->PD, PD->CP)
    int is_secure;     // Indicates whether Secure Channel mode was active

    uint64_t timestamp; // Event timestamp in implementation-defined units
};


/**
 * @brief Callback prototype for OSDP event listeners.
 *
 * Registered listener callbacks receive emitted OSDP events together with an optional user-defined context pointer.
 *
 * @param ev         Pointer to the emitted event structure.
 * @param user_data  User-defined context pointer associated with the listener registration.
 */
typedef void (*osdp_event_cb)(
        const struct osdp_event *ev,
        void *user_data);


/**
 * @brief Register a new OSDP event listener.
 *
 * Adds a callback function to the internal dispatcher registry.
 * Registered listeners are notified whenever an OSDP event is emitted.
 *
 * @param cb         Listener callback function.
 * @param user_data  Optional user-defined context pointer.
 */
void osdp_register_listener(osdp_event_cb cb, void *user_data);


/**
 * @brief Emit an OSDP event to all registered listeners.
 *
 * Dispatches the specified event to every registered callback.
 *
 * @param ev Pointer to the event structure to emit.
 */
void osdp_emit_event(const struct osdp_event *ev);

#endif //OSDP_EVENT_H