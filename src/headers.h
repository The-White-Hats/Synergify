#include <stdio.h>      //if you don't use scanf/printf change this include
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
//don't mess with this variable//
int * shmaddr;                 //
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
typedef struct process_info_s {
    int id;
    int arrival;
    int runtime;
    int priority;
} process_info_t ;

/**
 * scheduling_algo - Enumeration representing different scheduling algorithms
 * @HPF: Highest Priority First
 * @SRTN: Shortest Remaining Time Next
 * @RR: Round Robin
 *
 * Description: Enumeration representing different scheduling algorithms including
 *              Highest Priority First, Shortest Remaining Time Next, and Round Robin.
*/
typedef enum {
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
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
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
