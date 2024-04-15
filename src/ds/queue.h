#pragma once
/**
 * queue - queue for the process_info_t
*/
typedef struct queue_s queue;

/**
 * create_queue - function that returns a pointer to an initialized queue
 *
 * Return: A pointer to the newly created process information queue, or NULL if memory allocation fails
*/
queue* create_queue();

/**
 * enqueue - function to insert a new element into the queue
 * @queue: the queue to insert into 
 * @process: the process to insert at the end fo the queue
 *
 * Return: true if the process is enqueued successfully, otherwise false
*/
bool enqueue(queue* my_queue, void* data);

/**
 * dequeue - function to remove the element at the head of the queue
 * @queue: the queue from which the process would be removed
 * 
 * Return: the process that has been dequeued
*/
void* dequeue(queue* my_queue);

/**
 * front - function that returns a copy of the first element of the queue without removing it
 * @queue: the queue from which the first element will be copied
 * 
 * Return: a pointer of the copy of the first element
*/
const void* front(queue* my_queue);

/**
 * is_queue_empty - check if the queue is empty
 * @queue: the queue on which the check would be performed
 * 
 * Return: a boolean that indicated the result of the check
*/
bool is_queue_empty(queue* my_queue);
