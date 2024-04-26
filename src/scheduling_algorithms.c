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

/**
 * addLog - add new log line to the log file
 */

void addLog(FILE *file, int currentTime, int processId, char *state, int arrivalTime, int totalRuntime, int remainingTime, int waitingTime)
{
    fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d\n", currentTime, processId, state, arrivalTime, totalRuntime, totalRuntime, currentTime - arrivalTime);
}

/**
 * contentSwitch - Switches context to the next process
 *
 * @param new_front: PID of the new front process
 * @param old_front: PID of the old front process
 *
 * Description: Stops the old front process and continues the new front process.
 *              If the new front process is -1, it means there's no new front process to switch to.
 */
void contentSwitch(PCB *new_front, PCB *old_front, int currentTime, FILE *file)
{
    if (new_front == NULL)
        return;
    if (old_front != NULL)
    {
        old_front->last_stop_time = currentTime;
        int remaining_time = old_front->runtime - (currentTime - old_front->start_time - old_front->waiting_time);
        addLog(file,
               currentTime,
               old_front->file_id,
               "stopped",
               old_front->arrival,
               old_front->runtime,
               remaining_time,
               old_front->waiting_time);
        old_front->state = READY;
        kill(old_front->fork_id, SIGSTOP);
    }
    // started or resumed

    new_front->waiting_time += currentTime - new_front->last_stop_time;

    if (new_front->state == NEWBIE)
    {
        new_front->start_time = currentTime;
        addLog(file,
               currentTime,
               new_front->file_id,
               "started",
               new_front->arrival,
               new_front->runtime,
               new_front->runtime,
               new_front->waiting_time);
    }
    else if (new_front->state == READY)
    {
        int remaining_time = new_front->runtime - (currentTime - new_front->start_time - new_front->waiting_time);
        addLog(file,
               currentTime,
               new_front->file_id,
               "resumed",
               new_front->arrival,
               new_front->runtime,
               remaining_time,
               new_front->waiting_time);
    }

    printf("Current running process: %d , current time: %d\n", new_front->fork_id, currentTime);
    new_front->state = RUNNING;
    kill(new_front->fork_id, SIGCONT);
}
