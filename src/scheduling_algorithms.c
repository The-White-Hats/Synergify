#include "header.h"

//! syntax modification when merging with others for process variable data members

/**
 * decrementCurrentQuantum - Decrement the current quantum value in the scheduler configuration.
 * @param schedulerConfig: Pointer to the scheduler configuration struct.
 */
void decrementCurrentQuantum(SchedulerConfig *schedulerConfig)
{
    schedulerConfig->curr_quantum--;
}
/**
 * resetCurrentQuantum - Reset the current quantum value to the initial quantum value in the scheduler configuration.
 * @param schedulerConfig: Pointer to the scheduler configuration struct.
 */
void resetCurrentQuantum(SchedulerConfig *schedulerConfig)
{
    schedulerConfig->curr_quantum = schedulerConfig->quantum;
}

/**
 * scheduleRR - Runs the Round Robin algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 *
 * Description: Checks if their is a running process and decrease the quantum.
 *              If the set quantum ends, we return the process to the back of
 *              the queue and reset the quantum.
 */
void scheduleRR(pqueue_t **head)
{
    pqueue_t *running_process;
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }
    running_process = *head;

    // Return if there is no running process
    if (running_process == NULL)
        return;

    decrementCurrentQuantum(schedulerConfig);
    if (schedulerConfig->curr_quantum == 0)
    {
        // TODO:
        // 1. Send Sleep Signal To Running Process.

        // 2. Push Process To The Back Of The Queue.
        push(head, running_process->process, running_process->priority);
        pop(head);
        // 3. Reset Quantum.
        resetCurrentQuantum(schedulerConfig);
    }
}

/**
 * scheduleSRTN - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 *
 * Description: Checks if their is a running process and decrease its priority
 *              as it indicates its remaining running time.
 */
void scheduleSRTN(pqueue_t **head)
{
    pqueue_t *running_process;
    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }
    running_process = *head;
    if (running_process == NULL)
        return;

    // Decrease The Running Process Runtime.
    running_process->priority--;
}

/**
 * scheduleHPF - Schedule a process using the Highest Priority First (HPF) algorithm (non-preemptive).
 * @param head: Pointer to the head of the priority queue.
 *
 * Description: Useless for now
 */
void scheduleHPF(pqueue_t **head)
{
    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }
}
