#pragma once

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
#include "ds/priority_queue.h"

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
    pid_t fork_id;
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
    int quantum;
    int curr_quantum;
} SchedulerConfig;

//===========================================Process=============================================//

//=======================================Scheduler=========================================//

/**
 * getSchedulerConfigInstance - Function to get the singleton instance of SchedulerConfig.
 * @return Pointer to the instance.
 *
 * Description: This function returns a pointer to the singleton instance of the SchedulerConfig
 *              structure, ensuring that only one instance exists throughout the program.
 */ 
SchedulerConfig *getSchedulerConfigInstance();

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
 * initializeProcesses - Initializes processes based on the number of processes to be created.
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param num_of_processes: Number of processes to be initialized.
 *
 * Description: Initializes processes, forks them, and adds them to the ready queue.
 */
void initializeProcesses(pqueue_t **head, int num_of_processes);

/**
 * addToReadyQueue - Adds a process to the ready queue based on the scheduling algorithm.
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param process: Pointer to the process to be added.
 *
 * Description: Adds a process to the ready queue based on the selected scheduling algorithm.
 */
void addToReadyQueue(pqueue_t **head, process_info_t *process);

/**
 * scheduleSRTN - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 *
 * Description: Checks if their is a running process and decrease its priority
 *              as it indicates its remaining running time.
 */
void scheduleRR(pqueue_t **head);

/**
 * scheduleSRTN - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 *
 * Description: Checks if their is a running process and decrease its priority
 *              as it indicates its remaining running time.
 */
void scheduleSRTN(pqueue_t **head);

/**
 * scheduleHPF - Schedule a process using the Highest Priority First (HPF) algorithm (non-preemptive).
 * @param head: Pointer to the head of the priority queue.
 *
 * Description: Useless for now
 */
void scheduleHPF(pqueue_t **head);
