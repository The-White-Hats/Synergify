#include "header.h"
#include "clk.h"
#include "./ds/queue.h"
#include <stdarg.h>

///==============================
// functions
void clearResources(int);
void read_input_file(queue *);
void get_scheduling_algo(int *algorithm_choosen, int *quantum_time);
void start_program(const char *const file_name, int n, ...);
void append_to_path(char *const absolute_path, const char *const file_name);
///==============================

///==============================
/// global variables for process_generator
int msgq_id;
///==============================

int main(int argc, char *argv[])
{
    ///==============================
    // data
    queue *processes_queue = create_queue();
    ///==============================

    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    read_input_file(processes_queue);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algorithm_choosen = -1, quantum_time = 0;
    get_scheduling_algo(&algorithm_choosen, &quantum_time);

    // 3. Initiate and create the scheduler and clock processes.

    start_program("scheduler.out", 2, algorithm_choosen, quantum_time);
    start_program("clk.out", 0);

    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    // int x = getClk();
    // printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    
    msgq_id = msgget(SHKEY, 0666 | IPC_CREAT);
    msgbuf_t msgbuf;

    while (!is_queue_empty(processes_queue))
    {
        process_info_t *process_data = (process_info_t *)front(processes_queue);
        while (process_data && process_data->arrival == getClk())
        {
            msgbuf.mytype = getClk();
            msgbuf.message = (*process_data);

            msgsnd(msgq_id, &msgbuf, sizeof(msgbuf.message), IPC_NOWAIT);
            
            dequeue(processes_queue);
            process_data = (process_info_t *)front(processes_queue);
        }
    }

    sleep(5);
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    printf("\nIPC resources for process generator was cleared.\n");
    printf("\nProcess Generator is terminating\n");
    exit(0);
}

/**
 * read_input_file - to read the processes and their parameters from a text file
 */
void read_input_file(queue *processes_queue)
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
        enqueue(processes_queue, process);
    }
    fclose(input_file);
}

/**
 * get_scheduling_algo - read the scheduling algo and its parameters
 * @algorithm_choosen: a pointer to store the chosen algo
 * @quantum_time: a pointer to store the round robin quantum if it was choosen
*/
void get_scheduling_algo(int *algorithm_choosen, int *quantum_time)
{
    printf("\n");
read_algo:
    printf("Enter your choice for the Scheduling Algorithm\n");
    printf("1 for Non-preemptive Highest Priority First\n");
    printf("2 for Shortest Remaining time Next\n");
    printf("3 for Round Robin\n");
    printf("Your choice: ");

    scanf("%d", algorithm_choosen);
    if ((*algorithm_choosen) < 1 || (*algorithm_choosen) > 3)
    {
        printf("\nInvalid input, try again\n\n");
        goto read_algo;
    }

read_quantum:
    if ((*algorithm_choosen) == 3)
    {
        printf("\nEnter the quantum size of Round Robin: ");
        scanf("%d", quantum_time);
    }

    if ((*quantum_time) < 0)
    {
        printf("\nInvalid input, try again\n");
        goto read_quantum;
    }
}

/**
 * start_program - creates a new process and executes the passed file_name.
 *
 * @file_name: the name of the file (program) to execute.
 * @n: number of variadic arguments.
 *
 * Description: If the program to execute is the scheduler, then take another 2 arguments and convert
 * them from an int to a string and pass them to it as an input, else if its any other program then
 * execute it normally.
 */
void start_program(const char *const file_name, int n, ...)
{
    pid_t process_id = -1;
    va_list ptr;

    va_start(ptr, n);

    process_id = fork();
    if (process_id == 0)
    {
        char absolute_path[256];
        int error;

        // get the current working directory.
        getcwd(absolute_path, sizeof(absolute_path));
        append_to_path(absolute_path, file_name);

        // if we would execute scheduler, then pass to it some arguments.
        if (strcmp(file_name, "scheduler.out") == 0)
        {
            char argv[2][10];

            sprintf(argv[0], "%d", va_arg(ptr, int));
            sprintf(argv[1], "%d", va_arg(ptr, int));

            va_end(ptr);

            error = execl(absolute_path, file_name, argv[0], argv[1], NULL);
        }
        else
        {
            va_end(ptr);

            error = execl(absolute_path, file_name, NULL);
        }

        if (error == -1)
        {
            if (errno == ENOENT)
            {
                printf("\nNo such file or directory with the name %s\n", file_name);
            }
            else if (errno == EACCES)
            {
                printf("\nPermission denied for file %s\n", file_name);
            }
        }
        exit(-1);
    }
    else if (process_id < 0)
    {
        printf("\nAn error occured while forking a new process\n");
        exit(-1);
    }
}

/**
 * append_to_path - takes a file_name and appends it to an absolute_path.
 *
 * @file_name: name of the file to append.
*/
void append_to_path(char *const absolute_path, const char *const file_name)
{
    int i = 0, j = 0;

    while (absolute_path[i] != '\0')
        i++;

    // append /bin/ to the path.
    absolute_path[i++] = '/';
    absolute_path[i++] = 'b';
    absolute_path[i++] = 'i';
    absolute_path[i++] = 'n';
    absolute_path[i++] = '/';

    while (file_name[j] != '\0')
        absolute_path[i++] = file_name[j++];
}
