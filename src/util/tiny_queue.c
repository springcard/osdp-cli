/**
 * @file tiny_queue.c
 * @brief Implements a lightweight thread-safe FIFO queue.
 *
 * This module provides a minimal generic queue implementation based on a singly linked list and POSIX synchronization primitives.
 *
 * The queue is designed for inter-thread communication and supports:
 *   - thread-safe push operations,
 *   - blocking pop operations,
 *   - synchronization using mutexes and condition variables.
 *
 * The implementation is intentionally lightweight and stores generic pointers (`void *`) to allow usage with any application-specific data
 * structure.
 *
 * Typical use cases include:
 *   - producer/consumer pipelines,
 *   - asynchronous logging,
 *   - packet processing workers,
 *   - serial frame buffering.
 *
 * Synchronization behavior:
 *   - `tiny_queue_push()` wakes a waiting consumer thread.
 *   - `tiny_queue_pop()` blocks until data becomes available.
 *
 * Internally, the queue uses:
 *   - a linked list for dynamic storage,
 *   - a mutex for exclusive access,
 *   - a condition variable for blocking wakeup notifications.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include "tiny_queue.h"


/**
 * @brief Creates and initializes a new queue instance.
 *
 * This function allocates memory for the queue object and initializes its synchronization primitives.
 *
 * The queue starts empty with both head and tail pointers set to NULL.
 *
 * @return
 * Pointer to the newly created queue on success, or NULL if allocation or initialization fails.
 */
tiny_queue_t* tiny_queue_create() {
	struct tiny_queue_t* queue = (struct tiny_queue_t*)malloc(sizeof(struct tiny_queue_t));

	if (queue == NULL) {
		return NULL;
	}

	queue->head = NULL;
	queue->tail = NULL;

#ifdef _WIN32
	InitializeCriticalSection(&queue->mutex);
	InitializeConditionVariable(&queue->wakeup);
	return queue;
#else
	if(pthread_mutex_init(&queue->mutex, NULL) == 0 &&
		pthread_cond_init(&queue->wakeup, NULL) == 0) {
		return queue;
	}

	free(queue);
	return NULL;
#endif
}


/**
 * @brief Pushes an element into the queue.
 *
 * This function inserts a new element at the end of the queue.
 *
 * The operation is thread-safe and wakes one blocked consumer thread waiting in `tiny_queue_pop()`.
 *
 * @param queue
 * Pointer to the queue instance.
 *
 * @param x
 * Generic pointer to store inside the queue.
 *
 * @return
 *  - 0 on success
 *  - -1 on allocation failure
 */
int tiny_queue_push(tiny_queue_t *queue, void *x) {
	struct tiny_linked_list_t* new_node = (struct tiny_linked_list_t*)malloc(sizeof(struct tiny_linked_list_t));

	if (new_node == NULL) {
		return -1;
	}

	new_node->data = x;
	new_node->next = NULL;

#ifdef _WIN32
	EnterCriticalSection(&queue->mutex);
#else
	pthread_mutex_lock(&queue->mutex);
#endif

	if(queue->head == NULL && queue->tail == NULL){
		queue->head = queue->tail = new_node;
	} else {
		queue->tail->next = new_node;
		queue->tail = new_node;
	}

#ifdef _WIN32
	LeaveCriticalSection(&queue->mutex);
	WakeConditionVariable(&queue->wakeup);
#else
	pthread_mutex_unlock(&queue->mutex);
	pthread_cond_signal(&queue->wakeup);
#endif

	return 0;
}


/**
 * @brief Pops an element from the queue.
 *
 * This function retrieves and removes the oldest element stored in the queue.
 *
 * If the queue is empty, the function blocks until a producer pushes a new element.
 *
 * The operation is thread-safe.
 *
 * @param queue
 * Pointer to the queue instance.
 *
 * @return
 * Pointer previously inserted with `tiny_queue_push()`.
 */
void *tiny_queue_pop(tiny_queue_t *queue) {
	
#ifdef _WIN32
	EnterCriticalSection(&queue->mutex);
#else
	pthread_mutex_lock(&queue->mutex);
#endif
	
	while(queue->head == NULL) { // block if buffer is empty
#ifdef _WIN32
		SleepConditionVariableCS(&queue->wakeup, &queue->mutex, INFINITE);
#else
		pthread_cond_wait(&queue->wakeup, &queue->mutex);
#endif
	}

	struct tiny_linked_list_t* current_head = queue->head;
	void *data = current_head->data;
	
	if(queue->head == queue->tail) {
		queue->head = queue->tail = NULL;
	}
	else {
		queue->head = queue->head->next;
	}

	free(current_head);

#ifdef _WIN32
	LeaveCriticalSection(&queue->mutex);
#else
	pthread_mutex_unlock(&queue->mutex);
#endif

	return data;
}



/**
 * @brief Destroys the queue and releases associated resources.
 *
 * This function:
 *   - destroys synchronization primitives,
 *   - frees all remaining queue nodes,
 *   - releases the queue object itself.
 *
 * Stored user data pointers are NOT freed automatically.
 * The caller remains responsible for freeing queued objects.
 *
 * @param queue
 * Pointer to the queue instance.
 *
 * @return
 *  - 0 on success
 *  - -1 if mutex or condition destruction fails
 */
int tiny_queue_destroy(tiny_queue_t *queue) {
#ifdef _WIN32
	DeleteCriticalSection(&queue->mutex);
#else
	if (pthread_mutex_destroy(&queue->mutex) != 0 ||
		pthread_cond_destroy(&queue->wakeup) != 0)
		return -1;
#endif

	struct tiny_linked_list_t *node, *next;
	node = queue->head;

	if (node != NULL) {
		while (node->next != NULL) {
		next = node->next;
		free(node);
		node = next;
		}
	}

	free(queue);

	return 0;
}
