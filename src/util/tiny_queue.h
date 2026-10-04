/**
 * @file tiny_queue.h
 * @brief Declares a lightweight thread-safe FIFO queue API.
 *
 * This module provides a generic queue implementation intended for producer/consumer communication between threads.
 *
 * The queue stores generic pointers (`void *`) and internally uses:
 *   - a singly linked list for dynamic storage,
 *   - a POSIX mutex for thread synchronization,
 *   - a POSIX condition variable for blocking wakeup signaling.
 *
 * Main features:
 *   - thread-safe push operation,
 *   - blocking pop operation,
 *   - dynamic queue growth,
 *   - lightweight implementation.
 *
 * Typical use cases include:
 *   - asynchronous packet processing,
 *   - event dispatching,
 *   - serial communication workers,
 *   - logging pipelines.
 */

#ifndef __TINY_QUEUE__
#define __TINY_QUEUE__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

/**
 * @brief Internal linked-list node used by the queue.
 *
 * Each node stores:
 *   - a generic user pointer,
 *   - a pointer to the next node.
 */
typedef struct tiny_linked_list_t
{
    // User-provided data pointer
    void *data;

    // Pointer to the next queue node
    struct tiny_linked_list_t *next;

} tiny_linked_list_t;


/**
 * @brief Thread-safe FIFO queue structure.
 *
 * The queue maintains:
 *   - a head pointer for pop operations,
 *   - a tail pointer for push operations,
 *   - a mutex protecting concurrent access,
 *   - a condition variable used to wake waiting consumers.
 */
typedef struct tiny_queue_t {
  // Pointer to the first queue node
  struct tiny_linked_list_t* head;

  // Pointer to the last queue node
  struct tiny_linked_list_t* tail;
  
#ifdef _WIN32
  // Critical section protecting queue access
  CRITICAL_SECTION mutex;

  // Condition variable used for blocking wakeup
  CONDITION_VARIABLE wakeup;
#else
  // Mutex protecting queue access
  pthread_mutex_t mutex;

  // Condition variable used for blocking wakeup
  pthread_cond_t wakeup;
#endif

} tiny_queue_t;

/**
 * @brief Creates and initializes a new queue.
 *
 * @return
 * Pointer to a newly allocated queue on success, or NULL on failure.
 */
tiny_queue_t* tiny_queue_create();

/**
 * @brief Pushes an element into the queue.
 *
 * This operation is thread-safe.
 *
 * @param queue
 * Pointer to the queue instance.
 *
 * @param x
 * Generic pointer to insert into the queue.
 *
 * @return
 *  - 0 on success
 *  - -1 on allocation failure
 */
int tiny_queue_push(tiny_queue_t *queue, void *x);

/**
 * @brief Pops an element from the queue.
 *
 * If the queue is empty, the function blocks until a new element becomes available.
 *
 * This operation is thread-safe.
 *
 * @param queue
 * Pointer to the queue instance.
 *
 * @return
 * Pointer previously inserted with `tiny_queue_push()`.
 */
void *tiny_queue_pop(tiny_queue_t *queue);

/**
 * @brief Destroys a queue and releases its resources.
 *
 * Remaining queue nodes are freed, but stored user data pointers are NOT automatically released.
 *
 * @param queue
 * Pointer to the queue instance.
 *
 * @return
 *  - 0 on success
 *  - -1 on synchronization cleanup failure
 */
int tiny_queue_destroy(tiny_queue_t *queue);

#endif
