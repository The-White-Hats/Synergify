#ifndef SAMPLE_HEADER_H
#define SAMPLE_HEADER_H

#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

///==============================
// Structs & Enums

/**
 * struct process_info_s - Structure for holding process information
 * @id: Unique identifier for the process
 * @arrival: Arrival time of the process
 * @runtime: Runtime of the process
 * @priority: Priority of the process
 *
 * Description: Structure representing process information including its ID, arrival time,
 *              runtime, and priority.
 */
typedef struct process_info_s
{

    int id;
    int arrival;
    int runtime;
    int priority;
} process_info_t;

/**
 * scheduling_algo - Enumeration representing different scheduling algorithms
 * @HPF: Highest Priority First
 * @SRTN: Shortest Remaining Time Next
 * @RR: Round Robin
 *
 * Description: Enumeration representing different scheduling algorithms including
 *              Highest Priority First, Shortest Remaining Time Next, and Round Robin.
 */
typedef enum
{
    HPF = 1,
    SRTN,
    RR
} scheduling_algo;
///==============================

int getClk()
{
    return *shmaddr;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
 */
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        // Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

//===========================================Process=============================================//

/**
 * decrementRemainingCPUTime - Decrement the remaining CPU time of a process.
 * @param process: Pointer to the process information structure.
 */
void decrementRemainingCPUTime(process_info_t *process);

//=======================================Priority Queue=========================================//

/**
 * struct queue_s - linked list representation of a priority queue
 * @process: Pointer to the process
 * @priority: Value of the node priority
 * @next: Pointer to the next element of the stack (or queue)
 *
 * Description: linked list node structure
 */
typedef struct pqueue_s
{
    void *process;
    int priority;
    struct stack_s *next;
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

//=======================================Scheduler=========================================//

/**
 * SchedulerConfig - Structure for scheduler configuration settings.
 * @selected_algorithm: The selected scheduling algorithm.
 * @quantum: Quantum for time slice (if applicable).
 *
 * Description: Structure representing the configuration settings for the scheduler,
 *              including the selected scheduling algorithm and quantum for time slice
 *              (if applicable).
 */
typedef struct
{
    scheduling_algo selected_algorithm;
    int quantum = 0;
    int curr_quantum = 0;
} SchedulerConfig;

/**
 * getSchedulerConfigInstance - Function to get the singleton instance of SchedulerConfig.
 * @return Pointer to the instance.
 *
 * Description: This function returns a pointer to the singleton instance of the SchedulerConfig
 *              structure, ensuring that only one instance exists throughout the program.
 */
SchedulerConfig *getSchedulerConfigInstance()
{
    // Declare the static instance of the singleton
    static SchedulerConfig instance;

    // Return a pointer to the instance
    return &instance;
}

/**
 * struct rprocess_s - current running process
 * @process: Pointer to the process
 * @priority: Value of the node priority
 *
 * Description: save the info of the current running process
 */
typedef struct rprocess_s
{
    process_info_t *process;
    int priority;
} rprocess_t;

/**
 * scheduleRR - Runs the Round Robin algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param current_process: Pointer to the current running process by the cpu.
 * 
 * Description: checks for null value for queue or the head of the queue
 *              check if their is a running process to push it back and take a new one or take a new one immediately
 *              update the current_process data
 *              checks for the remaining time to send a termination signal to the schedular
 */
void scheduleRR(pqueue_t **head, rprocess_t *current_process);

/**
 * scheduleSRTN - Runs the shortest remaining time first algorithm
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
void scheduleSRTN(pqueue_t **head, rprocess_t *current_process);

/**
 * scheduleHPF - Schedule a process using the Highest Priority First (HPF) algorithm (non-preemptive).
 * @param head: Pointer to the head of the priority queue.
 * @param current_process: Pointer to the current process being scheduled.
 *
 * Description: Implements scheduling logic for HPF algorithm (non-preemptive). 
 *              Decrements current process's remaining CPU time, updates if finished, and handles edge cases.
 */
void scheduleHPF(pqueue_t **head, rprocess_t *current_process);
#endif /* SAMPLE_HEADER_H */
