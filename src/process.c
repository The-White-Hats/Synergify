#include "clk.h"
#include "header.h"

///==============================
// global variables
int remaining_time;
int prev_time;
///==============================

///==============================
// functions
void allocateCPU(int);
///==============================

int main(int argc, char * argv[])
{
    ///==============================
    // bind signal handlers
    signal(SIGCONT, allocateCPU);
    ///==============================

    // Sleep till the scheduler wakes me up
    pause();
  
    if (argc != 5) {
        perror("Use: ./process <id> <arrival_time> <running_time> <priority>");
        exit(EXIT_FAILURE);
    }

    // initialize the clk and set the previous time step
    initClk();
    prev_time = getClk();

    //TODO it needs to get the remaining time from somewhere
    remaining_time = atoi(argv[3]);

    //printf("Process %d awakened\n", getpid());
    printf("process id: %s\n", argv[1]);
    while (remaining_time > 0)
    {
        if (getClk() == prev_time) continue;
        remaining_time -= 1;
        printf("Process.c #%s decremented to %d\n", argv[1], remaining_time);
        prev_time = getClk();
    }

    // Send a signal to the scheduler to inform it that this process did finish
    kill(getppid(),SIGRTMIN + 1);
    destroyClk(false);
    printf("process.c #%s finished\n", argv[1]);
    return 0;
}

void allocateCPU(int sig_num)
{
    // allocate the CPU
    return;
}
