#include "headers.h"
#include "./ds/process_info_queue.h"

///==============================
// functions
void clearResources(int);
void read_input_file(process_info_queue_t *);
void get_scheduling_algo(scheduling_algo*, int*);
///==============================

int main(int argc, char *argv[])
{
    ///==============================
    // data
    process_info_queue_t *queue = malloc(sizeof(process_info_queue_t));
    scheduling_algo algo;
    int quantum = 0;
    ///==============================

    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    read_input_file(queue);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    get_scheduling_algo(&algo, &quantum);

    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    // initClk();
    // To get time use this
    // int x = getClk();
    // printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    // destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
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

/**
 * get_scheduling_algo - read the scheduling algo and its parameters
 * @algo: a pointer to store the chosen algo
 * @quantum: a pointer to store the round robin quantum if it was choosen
*/
void get_scheduling_algo(scheduling_algo* algo, int* quantum)
{
    int choice = -1;
    printf("\n");
read_algo:
    printf("Enter your choice for the Scheduling Algorithm\n");
    printf("1 for Non-preemptive Highest Priority First\n");
    printf("2 for Shortest Remaining time Next\n");
    printf("3 for Round Robin\n");
    printf("Your choice: ");

    scanf("%d", &choice);
    if (choice < 1 || choice > 3)
    {
        printf("\nInvalid input, try again\n\n");
        goto read_algo;
    }
    *algo = (scheduling_algo)choice;

read_quantum:
    if (choice == 3)
    {
        printf("\nEnter the quantum size of Round Robin: ");
        scanf("%d", quantum);
    }

    if (*quantum < 0)
    {
        printf("\nInvalid input, try again\n");
        goto read_quantum;
    }
}