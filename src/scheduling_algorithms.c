#include "headers.h"

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
    current_process->process = (*head)->process;
    current_process->priority = (*head)->priority;
    pop(head);
}

/**
 * scheduleRR - Runs the Round Robin algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param current_process: Pointer to the current running process by the cpu.
 * @param schedulerConfig: Pointer to the schedular configuration.
 * Description:
 * checks for null value for queue or the head of the queue
 * check if their is a running process to push it back and take a new one or take a new one immediately
 * update the current_process data
 * checks for the remaining time to send a termination signal to the schedular
 */
void scheduleRR(pqueue_t **head, rprocess_t *current_process, SchedulerConfig *schedulerConfig)
{

    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }

    if (*head == NULL)
        return;

    if (current_process == NULL)
    {
        updateCurrentProcess(head, current_process);
        resetCurrentQuantum(schedulerConfig);
    }
    else if (schedulerConfig->curr_quantum == 0)
    {
        push(head, current_process->process, current_process->priority);
        updateCurrentProcess(head, current_process);
        resetCurrentQuantum(schedulerConfig);
    }

    decrementCurrentQuantum(schedulerConfig);
    decrementRemainingCPUTime(current_process->process);

    if (current_process->process->runtime == 0)
    {
        current_process = NULL;
        // TODO: send termination signal.
    }
}

/**
 * scheduleSRTF - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param current_process: Pointer to the current running process by the cpu.
 *
 * Description: checks for null value for queue or the head of the queue.
 *              if their is no running process it gets one from the queue.
 *              else if a new process came but with less remaining time "which is the priority here" we switch between them and push the current to the queue again.
 *              update the priority of the current process as it represent the remaining time of the process.
 *              checks for the remaining time to send a termination signal to the schedular.
 */
void scheduleSRTN(pqueue_t **head, rprocess_t *current_process)
{
    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }

    if (*head == NULL)
        return;

    if (current_process == NULL)
        updateCurrentProcess(head, current_process);
    else if ((*head)->priority < current_process->priority)
    {
        push(head, current_process->process, current_process->priority);
        updateCurrentProcess(head, current_process);
    }

    current_process->priority--;
    decrementRemainingCPUTime(current_process->process);

    if (current_process->process->runtime == 0)
    {
        current_process = NULL;
        // TODO: send termination signal.
    }
}

/**
 * scheduleHPF - Schedule a process using the Highest Priority First (HPF) algorithm (non-preemptive).
 * @param head: Pointer to the head of the priority queue.
 * @param current_process: Pointer to the current process being scheduled.
 *
 * Description: Implements scheduling logic for HPF algorithm (non-preemptive).
 *              Decrements current process's remaining CPU time, updates if finished, and handles edge cases.
 */
void scheduleHPF(pqueue_t **head, rprocess_t *current_process)
{
    if (head == NULL)
    {
        fprintf(stderr, "Can't pop an empty queue\n");
        exit(EXIT_FAILURE);
    }

    if (*head == NULL)
        return;

    if (current_process == NULL)
        updateCurrentProcess(head, current_process);

    decrementRemainingCPUTime(current_process->process);
    if (current_process->process->runtime == 0)
    {
        current_process = NULL;
        // TODO: send termination signal.
    }
}