#include "headers.h"

//! syntax modification when merging with others for process variable data members

/**
 * updateCurrentProcess - update the current process data
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param current_process: Pointer to the current running process by the cpu.
 * just for code simplification.
 */

void updateCurrentProcess(pqueue_t **head, rprocess_t *current_process)
{
    if (current_process == NULL)
        current_process = (rprocess_t *)(sizeof(rprocess_t));
    current_process->process = *head->process;
    current_process->priority = *head->priority;
    pop(head);
}

/**
 * RR - Runs the Round Robin algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param current_process: Pointer to the current running process by the cpu.
 * checks for null value for queue or the head of the queue
 * check if their is a running process to push it back and take a new one or take a new one immediately
 * update the current_process data
 * checks for the remaining time to send a termination signal to the schedular
 */
void RR(pqueue_t **head, rprocess_t *current_process)
{
    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }

    if (*head == NULL)
        return;

    if (current_process != NULL)
        push(head, current_process->process, current_process->priority);

    updateCurrentProcess(head, current_process);

    if (--current_process->process->remaining_time == 0)
    {
        current_process = NULL;
        // send termination signal.
    }
}

/**
 * SRTN - Runs the Round Robin algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param current_process: Pointer to the current running process by the cpu.
 * checks for null value for queue or the head of the queue.
 * if their is no running process it gets one from the queue.
 * else if a new process came but with less remaining time "which is the priority here" we switch between them and push the current to the queue again.
 * update the priority of the current process as it represent the remaining time of the process.
 * checks for the remaining time to send a termination signal to the schedular.
 */

void SRTN(pqueue_t **head, rprocess_t *current_process)
{
    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }

    if (*head == NULL)
        return;

    if (current_process == NULL || *head->priority < current_process->priority)
    {
        updateCurrentProcess(head, current_process);
        if (*head->priority < current_process->priority)
            push(head, current_process->process, current_process->priority);
    }
    current_process->priority--;
    if (--current_process->process->remaining_time == 0)
    {
        current_process = NULL;
        // send termination signal.
    }
}