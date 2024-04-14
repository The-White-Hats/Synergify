/**
 * process_info_queue_t - queue for the process_info_t
*/
typedef struct process_info_queue_s process_info_queue_t;

/**
 * create_queue - function that returns a pointer to an initialized process_info_queue_t
 *
 * Return: A pointer to the newly created process information queue, or NULL if memory allocation fails
*/
process_info_queue_t* create_queue();

/**
 * enqueue - function to insert a new element into the process_info_queue_t
 * @queue: the queue to insert into 
 * @process: the process to insert at the end fo the queue
 *
 * Return: true if the process is enqueued successfully, otherwise false
*/
bool enqueue(process_info_queue_t* queue, process_info_t* process);

/**
 * dequeue - function to remove the element at the head of the queue
 * @queue: the queue from which the process would be removed
 * 
 * Return: the process that has been dequeued
*/
process_info_t* dequeue(process_info_queue_t* queue);

/**
 * front - function that returns a copy of the first element of the queue without removing it
 * @queue: the queue from which the first element will be copied
 * 
 * Return: a pointer of the copy of the first element
*/
const process_info_t* front(process_info_queue_t* queue);

/**
 * is_queue_empty - check if the queue is empty
 * @queue: the queue on which the check would be performed
 * 
 * Return: a boolean that indicated the result of the check
*/
bool is_queue_empty(process_info_queue_t* queue);

#include "process_info_queue.c"
