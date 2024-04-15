#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

/**
 * struct process_info_queue_node_s - Node structure for a process information queue
 * @data: Pointer to the process information stored in the node
 * @next: Pointer to the next node in the queue
 *
 * Description: Node structure for a singly linked list representing a queue of process information.
 */
typedef struct queue_node_s queue_node_t;
typedef struct queue_node_s
{
	void* data;
	queue_node_t* next;
} queue_node_t;

/**
 * struct process_info_queue_s - Structure for managing a queue of process information
 * @head: Pointer to the first node in the queue
 * @tail: Pointer to the last node in the queue
 * @size: Number of elements in the queue
 *
 * Description: Structure for managing a queue of process information using a singly linked list implementation.
 */
typedef struct queue_s
{
	queue_node_t *head;
	queue_node_t *tail;
	int size;
} queue;

/**
 * create_queue - Creates a new empty process information queue
 *
 * Return: A pointer to the newly created process information queue, or NULL if memory allocation fails
 */
queue *create_queue()
{
	queue *my_queue = malloc(sizeof(queue));
	if (!my_queue)
	{
		exit(-1);
	}

	my_queue->head = NULL;
	my_queue->tail = NULL;
	my_queue->size = 0;

	return my_queue;
}

/**
 * enqueue - Enqueues a process into the queue
 * @queue: A pointer to the process information queue
 * @process: A pointer to the process information to be enqueued
 *
 * Return: true if the process is enqueued successfully, otherwise false
 */
bool enqueue(queue *my_queue, void *data)
{
	queue_node_t* node = malloc(sizeof(queue_node_t));
	if (!node)
	{
		return false;
	}

	node->data = data;
	node->next = NULL;

	if (my_queue->tail == NULL) {
		my_queue->head = node;
	} else my_queue->tail->next = node;
	my_queue->tail = node;
	my_queue->size += 1;

	return true;
}

/**
 * dequeue - Dequeues a process from the queue
 * @queue: A pointer to the process information queue
 *
 * Return: A pointer to the dequeued process information, or NULL if the queue is empty
 */
void *dequeue(queue *my_queue)
{
	if (!my_queue->head) return NULL;
	void* data = my_queue->head->data;
	queue_node_t* next = my_queue->head->next;

	free(my_queue->head);
	my_queue->head = next;
	my_queue->size -= 1;
	return data;
}

/**
 * front - Retrieves the process at the front of the queue without dequeuing it
 * @queue: A pointer to the process information queue
 *
 * Return: A pointer to the process information at the front of the queue, or NULL if the queue is empty
 */
const void* front(queue* my_queue)
{
	if (!my_queue->head) return NULL;

	return (const void*)(my_queue->head->data);
}

/**
 * is_queue_empty - Checks if the queue is empty
 * @queue: A pointer to the process information queue
 *
 * Return: true if the queue is empty, otherwise false
 */
bool is_queue_empty(queue *my_queue)
{
	return my_queue->size == 0;
}
