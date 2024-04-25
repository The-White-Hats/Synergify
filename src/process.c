#include "clk.h"
#include "header.h"

/* Modify this file as needed*/
int remaining_time;
int prev_time;

int main(int argc, char * argv[])
{
    // Sleep till the scheduler wakes me up
    if (argc != 5) {
        perror("Use: ./process <id> <arrival_time> <running_time> <priority>");
        exit(EXIT_FAILURE);
    }

    // initialize the clk and set the previous time step
    initClk();
    prev_time = getClk();

    //TODO it needs to get the remaining time from somewhere
    remaining_time = atoi(argv[3]);

    printf("process id: %s\n", argv[1]);
    //printf("Process %d awakened\n", getpid());
  
    while (remaining_time > 0)
    {
        if (getClk() == prev_time) continue;
        remaining_time -= 1;
        prev_time = getClk();
    }

    // Send a signal to the scheduler to inform it that this process did finish
    kill(getppid(), SIGCHLD);

    destroyClk(false);
    return 0;
}
