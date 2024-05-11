#pragma once
#include <stdbool.h>

/**
 * struct queue_node_s - Node structure for a process information queue
 * @data: Pointer to the process information stored in the node
 * @next: Pointer to the next node in the queue
 *
 * Description: Node structure for a singly linked list representing a queue of process information.
 */
typedef struct queue_node_s queue_node_t;
typedef struct queue_node_s
{
	void *data;
	queue_node_t *next;
} queue_node_t;

/**
 * struct queue_s - Structure for managing a queue of process information
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
} queue_t;

/**
 * create_queue - function that returns a pointer to an initialized queue
 *
 * @return A pointer to the newly created process information queue, or NULL if memory allocation fails
*/
queue_t* create_queue();

/**
 * enqueue - function to insert a new element into the queue
 * @param my_queue: the queue to insert into 
 * @param data: the process to insert at the end fo the queue
 *
 * @return true if the process is enqueued successfully, otherwise false
*/
bool enqueue(queue_t* my_queue, void* data);

/**
 * dequeue - function to remove the element at the head of the queue
 * @param my_queue: the queue from which the process would be removed
 * 
 * Return: the process that has been dequeued
*/
void* dequeue(queue_t* my_queue);

/**
 * front - function that returns a copy of the first element of the queue without removing it
 * @param my_queue: the queue from which the first element will be copied
 * 
 * @return a pointer of the copy of the first element
*/
const void* front(queue_t* my_queue);

/**
 * is_queue_empty - check if the queue is empty
 * @param my_queue: the queue on which the check would be performed
 * 
 * @return a boolean that indicated the result of the check
*/
bool is_queue_empty(queue_t* my_queue);

/**
 * queue_copy - copy my_queue to a dist queue
 * @param my_queue: the queue to be copied.
 * @param dist: the queue on which I copy the nodes.
*/
void queue_copy(queue_t *my_queue, queue_t *dist);

/**
 * queue_free - Frees all memory allocated for the queue elements
 * @param my_queue: A pointer to the queue
 * @param deleteData: Determine if the data is freed or not
 *
 * Description: This function iteratively dequeues each element from the queue and frees its memory.
 */
void queue_free(queue_t *my_queue, bool deleteData);