#pragma once
#include <stdio.h>
#include <stdlib.h>
/**
 * struct queue_s - linked list representation of a priority queue
 * @process: Pointer to the process
 * @priority: Value of the node priority
 * @next: Pointer to the next element of the stack (or queue)
 *
 * Description: linked list node structure
 */
typedef struct pqueue_s pqueue_t;
typedef struct pqueue_s
{
    void *process;
    int priority;
    pqueue_t *next;
} pqueue_t;

/**
 * createNode - Creates a new node for the priority queue.
 *
 * @param process: Pointer to the process data to be stored in the node.
 * @param priority: Priority associated with the process.
 * @return Pointer to the newly created node.
 *
 * Allocates memory for a new node in the priority queue and initializes its data fields.
 * If memory allocation fails, an error message is printed, and the program exits.
 */
pqueue_t *createNode(void *process, int priority);

/**
 * push - Inserts a new process into the priority queue based on its priority.
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param process: Pointer to the process data to be inserted.
 * @param priority: Priority associated with the process.
 *
 * Description: Inserts a new process into the priority queue in ascending order of priority.
 *               If the priority queue is empty, the new process becomes the head of the queue.
 *               If the priority queue is not empty, the new process is inserted at the appropriate position
 *               to maintain the ascending order of priority.
 */
void push(pqueue_t **head, void *process, int priority);

/**
 * pop - Removes and frees the node at the front of the priority queue.
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 *
 * Description: Removes and frees the node at the front of the priority queue.
 *               If the priority queue is empty, an error message is printed to stderr,
 *               and the program exits with failure status.
 */
void pop(pqueue_t **head);

/**
 * isEmpty - Checks if the priority queue is empty.
 * 
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @return 1 if the priority queue is empty, 0 otherwise.
 */
int isEmpty(pqueue_t **head);