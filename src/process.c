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

    // printf("Process %d awakened\n", getpid());
    printf("process id: %s\n", argv[1]);
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
        printf("Process.c #%s decremented to %d at %d\n", argv[1], remaining_time, prev_time);
        
        if (remaining_time > 0){
            kill(getppid(), SIGPWR);            //sends a signal to scheduler to make it decrement its runtime.
        }
    }

    // Send a signal to the scheduler to inform it that this process did finish
    printf("Almost finished process.c %d with remaining %d at %d\n", getpid(), remaining_time, getClk());
    destroyClk(false);
    printf("process.c #%s finished\n", argv[1]);
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
    printf("Stopped process.c with remaining %d at %d\n", remaining_time, getClk());
    pause();
}
