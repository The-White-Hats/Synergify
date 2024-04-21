#include "clk.h"
#include "header.h"

/* Modify this file as needed*/
int remainingtime;

int main(int argc, char * argv[])
{
    if (argc != 5) {
        perror("Use: ./process <id> <arrival_time> <running_time> <priority>");
        exit(EXIT_FAILURE);
    }

    remainingtime = atoi(argv[3]);
    initClk();
    
    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    size_t prev_time = 0;
    printf("Process %d awakened\n", getpid());
    while (remainingtime > 0)
    {
        if (prev_time != getClk()) {
            remainingtime--;
            prev_time = getClk();
        }
        // remainingtime = ??;
    }
    
    destroyClk(false);
    kill(getppid(), SIGUSR2);
    
    return 0;
}
