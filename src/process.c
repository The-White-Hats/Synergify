#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    // Sleep till the scheduler wakes me up
    if (argc != 5) {
        perror("Use: ./process <id> <arrival_time> <running_time> <priority>");
        exit(EXIT_FAILURE);
    }

    // initialize the clk and set the previous time step
    initClk();
    
    //TODO it needs to get the remaining time from somewhere
    remaining_time = atoi(argv[3]);

    printf("process id: %s\n", argv[1]);
    //printf("Process %d awakened\n", getpid());
  
    while (remaining_time > 0)
    {
        // remainingtime = ??;
    }
    
    destroyClk(false);
    
    return 0;
}
