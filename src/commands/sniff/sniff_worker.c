/**
 * @file sniff_worker.c
 * @brief Background worker responsible for OSDP frame processing.
 *
 * This module implements a dedicated worker thread that consumes captured RS485 data from the sniff queue and forwards it to the
 * packet extraction and event pipeline.
 *
 * The worker thread isolates protocol decoding and logging from the real-time acquisition loop in order to minimize frame loss under
 * heavy console or file logging activity.
 */

#include "sniff_worker.h"
#include "sniff_options.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../util/tiny_queue.h"

#include <stdlib.h>
#include <stdio.h> //! a supprimer, juste pour les tests

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

extern tiny_queue_t *inFrameQueue;

#ifdef _WIN32
static HANDLE worker;
#else
static pthread_t worker;
#endif

static volatile int running = 0;


/**
 * @brief Worker thread entry point.
 *
 * Continuously consumes captured chunks from the sniff queue and forwards them to the packet parser.
 *
 * @param arg Unused thread argument.
 *
 * @return NULL.
 */
#ifdef _WIN32
static DWORD WINAPI worker_thread(LPVOID arg)
#else
static void *worker_thread(void *arg)
#endif
{
    (void)arg;

    while (running)
    {
        sniff_frame_t *chunk;

        if((chunk = (sniff_frame_t *)tiny_queue_pop(inFrameQueue)) != NULL)
        {
            if (chunk->len > 0)
            {
                decryptMsgPacket(0, chunk->data, chunk->len);
            }
            free(chunk);
        }
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}


/**
 * @brief Start the sniff worker thread.
 *
 * @return 0 on success, non-zero on failure.
 */
int sniff_worker_start(void)
{
    running = 1;

#ifdef _WIN32
    worker = CreateThread(NULL, 0, worker_thread, NULL, 0, NULL);
    if (worker == NULL)
    {
        running = 0;
        return -1;
    }
#else
    if (pthread_create(&worker,
                       NULL,
                       worker_thread,
                       NULL) != 0)
    {
        running = 0;
        return -1;
    }
#endif

    return 0;
}


/**
 * @brief Stop the sniff worker thread.
 */
void sniff_worker_stop(void)
{
    running = 0;

    /*
     * Wakeup strategy:
     * push an empty frame to unblock waiting consumer.
     */
    tiny_queue_push(inFrameQueue, NULL);

#ifdef _WIN32
    WaitForSingleObject(worker, INFINITE);
    CloseHandle(worker);
    worker = NULL;
#else
    pthread_join(worker, NULL);
#endif
}
