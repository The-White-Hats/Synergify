#include "headers.h"
#include "./ds/process_info_queue.h"

void clearResources(int);
void read_input_file(process_info_queue_t *);

int main(int argc, char * argv[])
{
    process_info_queue_t *queue = malloc(sizeof(process_info_queue_t));
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    read_input_file(queue);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}

/**
 * read_input_file - to read the processes and their parameters from a text file
 */
void read_input_file(process_info_queue_t *queue)
{
    FILE *input_file = fopen("./processes.txt", "r");
    if (!input_file)
    {
        printf("\nCould not open file processes.txt!!\n");
        exit(-1);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), input_file))
    {
        if (buffer[0] == '#')
            continue;
        process_info_t *process = malloc(sizeof(process_info_t));
        sscanf(buffer, "%d %d %d %d", &(process->id),
               &(process->arrival), &(process->runtime), &(process->priority));
        enqueue(queue, process);
    }
    fclose(input_file);
}
