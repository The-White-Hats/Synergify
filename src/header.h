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
#include <errno.h>
#include <string.h>
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
 * struct msgbuf_s - Structure for the message sent through the message queue
 * from the process_generator to the scheduler.
 *
 * @mytype: message header.
 * @message: the actual message sent.
 */
typedef struct msgbuf_s {
  long mytype;
  process_info_t message;
} msgbuf_t;

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
 * process_state - Enumeration representing different state of a process
 * @RUNNING: process currently running of CPU
 * @READY: in the ready queue, waiting to get the CPU
 * @BLOCKED: waiting for IO or some event
 *
 * Description: Enumeration representing different state a process could be in
 */
typedef enum
{
    RUNNING = 1,
    READY,
    BLOCKED
} process_state;

/**
 * PCB - Process control block
 * @file_id: Unique identifier for the process, taken from the input file
 * @fork_id: Unique identifier for the process, given by the system when it is forked
 * @state: Current state of the process in the system
 * @arrival: Arrival time of the process
 * @runtime: Runtime of the process - CPU time
 * @priority: Priority of the process
 *
 * Description: Structure representing process information including its ID, arrival time,
 *              runtime, and priority.
 */
typedef struct PCB_s
{
    int file_id;    
    pid_t fork_id;
    process_state state;
    int arrival;
    int runtime;
    int priority;
    int turn_around_time;
} PCB;

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
 * Description: On a received signal from the generator, initializes processes, forks them, 
 *              and adds them to the ready queue.
 */
void initializeProcesses(int signum);

/**
 * terminateRunningProcess - Terminates the currently running process
 *
 * @param signum: The signal number that triggered the termination.
 *
 * Description: Waits for the currently running process to terminate and then removes
 *              it from the ready queue.
 */
void terminateRunningProcess(int signum);

/**
 * addToReadyQueue - Adds a process to the ready queue based on the scheduling algorithm.
 *
 * @param process: Pointer to the process to be added.
 *
 * Description: Adds a process to the ready queue based on the selected scheduling algorithm.
 */
void addToReadyQueue(process_info_t *process);

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

/**
 * contentSwitch - Switches context to the next process
 *
 * @param new_front: PID of the new front process
 * @param old_front: PID of the old front process
 *
 * Description: Stops the old front process and continues the new front process.
 *              If the new front process is -1, it means there's no new front process to switch to.
 */
void contentSwitch(pid_t new_front, pid_t old_front);
