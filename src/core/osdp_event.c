/**
 * @file osdp_event.c
 * @brief Lightweight event dispatcher for OSDP protocol events.
 *
 * @details This module implements a minimal event notification system used to distribute OSDP protocol events across the application.
 *
 * The dispatcher allows independent modules to subscribe to protocol activity through callback registration. Registered listeners are
 * notified whenever an OSDP event is emitted by the protocol layer.
 *
 * The event system is primarily used to:
 *   - decouple protocol processing from output rendering
 *   - support multiple logging backends
 *   - centralize protocol event propagation
 *   - simplify extensibility of monitoring components
 *
 * Listeners are stored in a fixed-size internal registry and are invoked sequentially in the order they were registered.
 *
 * This implementation is intentionally lightweight and designed for single-threaded execution within the osdp-cli runtime.
 */
#include "osdp_event.h"


#define MAX_LISTENERS 8


static osdp_event_cb listeners[MAX_LISTENERS];
static void *listeners_data[MAX_LISTENERS];
static int listener_count = 0;


 /**
 * @brief Register a new OSDP event listener.
 *
 * Adds a callback function to the internal listener registry.
 * Registered listeners will receive all subsequently emitted OSDP events.
 *
 * If the listener registry is full, the registration request is silently ignored.
 *
 * @param cb         Listener callback function.
 * @param user_data  User-defined pointer passed back during event dispatching. This allows the user to associate additional information or context with the event listener.
 */
void osdp_register_listener(osdp_event_cb cb, void *user_data)
{
    if (listener_count >= MAX_LISTENERS)
        return;

    listeners[listener_count] = cb;
    listeners_data[listener_count] = user_data;
    listener_count++;
}


/**
 * @brief Emit an OSDP event to all registered listeners.
 *
 * Dispatches the provided event structure to every registered
 * callback in registration order.
 *
 * All listeners receive the same immutable event instance along
 * with their associated user-defined context pointer.
 *
 * @param ev Pointer to the OSDP event to dispatch.
 */ 
void osdp_emit_event(const struct osdp_event *ev)
{
    for (int i = 0; i < listener_count; i++) {
        listeners[i](ev, listeners_data[i]);
    }
}