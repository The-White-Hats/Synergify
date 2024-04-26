#pragma once
#include <stdbool.h>

/**
 * queue_t - data structure that represent "first in, first out (FIFO)"   
*/
typedef struct queue_s queue_t;

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
 * queue_free - Frees all memory allocated for the queue elements
 * @param my_queue: A pointer to the queue
 *
 * Description: This function iteratively dequeues each element from the queue and frees its memory.
 */
void queue_free(queue_t *my_queue);