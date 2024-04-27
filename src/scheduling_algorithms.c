#include "header.h"
#include "ds/fib_heap.h"
#include "ds/queue.h"

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
 * scheduleSRTN - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the ready_queue.
 *
 * Description: Checks if their is a running process and decrease its priority
 *              as it indicates its remaining running time.
 */
void scheduleRR(void *head)
{
    queue_t *queue = (queue_t *)head;
    PCB *running_process = (PCB *)front(queue);
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();

    // Return if there is no running process
    if (running_process == NULL)
        return;

    decrementCurrentQuantum(schedulerConfig);
    if (schedulerConfig->curr_quantum == 0)
    {
        // 1. Push Process To The Back Of The Queue.
        enqueue(queue, running_process);
        dequeue(queue);
        // 2. Reset Quantum.
        resetCurrentQuantum(schedulerConfig);
    }
}

/**
 * scheduleSRTN - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the ready_queue.
 *
 * Description: Checks if their is a running process and decrements its key
 *              as it indicates its remaining running time.
 */
void scheduleSRTN(void *head)
{
    fib_heap_t *heap = (fib_heap_t *) head;
    heap_node_t *min = heap->min;

    if (min == NULL)
        return;

    // Decrease The Running Process Runtime.
    if (min->key > 0)
        min->key--;
}

/**
 * scheduleHPF - Schedule a process using the Highest Priority First (HPF) algorithm (non-preemptive).
 * @param head: Pointer to the ready_queue.
 *
 * Description: Checks if their is a running process and decrease its key
 *              as it indicates its priority.
 */
void scheduleHPF(void *head)
{
    fib_heap_t *heap = (fib_heap_t *) head;
    PCB *running_process = fib_heap_min(heap);

    if (running_process == NULL)
        return;

    // Decrease The Running Process Key value to make sure it is never replaced.
    fib_heap_decrease_min_key(heap, 0);
}

/**
 * addLog - add new log line to the log file
 */

void addLog(FILE *file, int currentTime, int processId, char *state, int arrivalTime, int totalRuntime, int remainingTime, int waitingTime)
{
    fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d\n", currentTime, processId, state, arrivalTime, totalRuntime, remainingTime, waitingTime);
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
        int remaining_time = old_front->runtime - (currentTime - old_front->arrival - old_front->waiting_time);
        addLog(file,
               currentTime,
               old_front->file_id,
               "stopped",
               old_front->arrival,
               old_front->runtime,
               remaining_time,
               old_front->waiting_time);
        old_front->state = READY;
        kill(old_front->fork_id, SIGUSR1);
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
        int remaining_time = new_front->runtime - (currentTime - new_front->arrival - new_front->waiting_time);
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
