/**
 * @file sniff_worker.h
 * @brief Declares the asynchronous worker responsible for OSDP frame processing.
 *
 * This module defines the interface of the sniff worker thread used by the `sniff` command.
 *
 * The worker operates asynchronously from the serial reception loop and is
 * responsible for:
 *   - retrieving captured frames from the sniff queue,
 *   - decoding OSDP packets,
 *   - extracting commands and replies,
 *   - generating OSDP events,
 *   - dispatching events to the logging/rendering pipeline.
 *
 * This design prevents heavy frame decoding and console/file rendering
 * operations from blocking the RS-485 reception loop, improving reliability
 * and reducing the risk of frame loss during high traffic activity.
 *
 * The worker typically runs in its own dedicated thread and consumes
 * complete frames previously pushed into the sniff queue by the serial
 * acquisition loop.
 */

#ifndef SNIFF_WORKER_H
#define SNIFF_WORKER_H

/**
 * @brief Starts the asynchronous sniff worker thread.
 *
 * This function initializes and launches the worker responsible for processing captured OSDP frames from the sniff queue.
 *
 * Once started, the worker continuously waits for incoming frames, decodes them, and forwards generated events to registered listeners.
 *
 * @return
 *  - 0 on success
 *  - negative value on failure
 */
int sniff_worker_start(void);

/**
 * @brief Stops the asynchronous sniff worker thread.
 *
 * This function requests termination of the worker thread and performs any required cleanup before returning.
 *
 * It should be called before application shutdown or when stopping the sniff command.
 */
void sniff_worker_stop(void);

#endif