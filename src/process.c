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
void pauseProcess(int);
///==============================

int main(int argc, char *argv[])
{
    ///==============================
    // bind signal handlers
    signal(SIGCONT, allocateCPU);
    signal(SIGUSR1, pauseProcess);
    ///==============================

    // Sleep till the scheduler wakes me up
    pause();

    if (argc != 5)
    {
        perror("Use: ./process <id> <arrival_time> <running_time> <priority>");
        exit(EXIT_FAILURE);
    }

    // initialize the clk and set the previous time step
    initClk();
    prev_time = getClk();

    // TODO it needs to get the remaining time from somewhere
    remaining_time = atoi(argv[3]);

    while (remaining_time > 0)
    {
        if (getClk() == prev_time)
            continue;
        if (getClk() - prev_time > 1)
        {
            prev_time = getClk();
            continue;
        }
        remaining_time -= 1;
        prev_time = getClk();
        
        if (remaining_time > 0){
            kill(getppid(), SIGPWR);            //sends a signal to scheduler to make it decrement its runtime.
        }
    }

    // Send a signal to the scheduler to inform it that this process did finish
    destroyClk(false);
    kill(getppid(), SIGALRM);
    return 0;
}

void allocateCPU(int sig_num)
{
    // allocate the CPU
    prev_time -= 1;
    return;
}

void pauseProcess(int sig_num)
{
    pause();
}
