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
#include <stdarg.h>
#include <pthread.h>

#define PATH_SIZE 256

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
    int memsize;
} process_info_t;

/**
 * struct msgbuf_s - Structure for the message sent through the message queue
 * from the process_generator to the scheduler.
 *
 * @mytype: message header.
 * @message: the actual message sent.
 */
typedef struct msgbuf_s
{
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
 * @FINISHED: finished its execution
 * @NEWBIE: didn't start yet
 * Description: Enumeration representing different state a process could be in
 */
typedef enum
{
    RUNNING = 1,
    READY,
    NEWBIE,
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
    int start_time;
    int last_stop_time;
    int waiting_time;
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

//============================================Utils==============================================//

/**
 * getAbsolutePath - takes a file_name and appends it to an absolute_path.
 *
 * @absolute_path: pointer to the buffer where the absolute path will be stored.
 * @file_name: name of the file to append.
 */
void getAbsolutePath(char *const absolute_path, const char *const file_name);

//===========================================Process=============================================//

//==========================================Scheduler============================================//

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
 * scheduleSRTN - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the ready_queue.
 *
 * Description: Checks if their is a running process and decrease its priority
 *              as it indicates its remaining running time.
 */
void scheduleRR(void *head);

/**
 * scheduleSRTN - Runs the shortest remaining time first algorithm
 *
 * @param head: Pointer to the ready_queue.
 *
 * Description: Checks if their is a running process and decrements its key
 *              as it indicates its remaining running time.
 */
void scheduleSRTN(void *head);

/**
 * scheduleHPF - Schedule a process using the Highest Priority First (HPF) algorithm (non-preemptive).
 * @param head: Pointer to the ready_queue.
 *
 * Description: Checks if their is a running process and decrease its key
 *              as it indicates its priority.
 */
void scheduleHPF(void *head);

/**
 * contentSwitch - Switches context to the next process
 *
 * @param new_front: PID of the new front process
 * @param old_front: PID of the old front process
 * @param file: file which the log will be written to
 * @param currentTime: current system time
 * Description: Stops the old front process and continues the new front process.
 *              If the new front process is -1, it means there's no new front process to switch to.
 */
void contentSwitch(PCB *new_front, PCB *old_front, int currentTime, FILE *file);
// void printQueue(pqueue_t** head);