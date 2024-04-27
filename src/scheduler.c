#include "clk.h"
#include "header.h"
#include "ds/fib_heap.h"
#include "ds/queue.h"
#include <math.h>
/**
 * getSchedulerConfigInstance - Function to get the singleton instance of SchedulerConfig.
 * @return Pointer to the instance.
 *
 * Description: This function returns a pointer to the singleton instance of the SchedulerConfig
 *              structure, ensuring that only one instance exists throughout the program.
 */
SchedulerConfig *getSchedulerConfigInstance()
{
    // Declare the static instance of the singleton
    static SchedulerConfig instance = {.quantum = 0, .curr_quantum = 0};

    // Return a pointer to the instance
    return &instance;
}

// To be accessed by signal handlers
void *ready_queue = NULL;
queue_t *queue = NULL;
bool endScheduler = false;

float total_waiting_time = 0;             // sum of waiting times
float total_weighted_turnaround_time = 0; // sum of weighted turnaround times
float total_running_time = 0;             // sum of running times
float *wta_values = NULL;                 // array of weighted turnaround times
int total_processes = 0;                  // total number of processes that come so far
int idx = 0;                              // index of the wta_values array
int waste_time = 0;                       // cpu wasted time

PCB *running_process = NULL;
FILE *logFile, *perf;

//================================= SIGNAL HANDLERS =================================//
static void initializeProcesses(int signum);
static void terminateRunningProcess(int signum);
static void noMoreProcesses(int signum);
static void clearResources(int signum);
//=============================== SCHEDULER FUNCTIONS ===============================//
static void *allocateDataStructure(scheduling_algo selected_algo);
static PCB *getRunningProcess(scheduling_algo selected_algo);
static PCB *popRunningProcess(scheduling_algo selected_algo);
static short is_running_queue_empty(scheduling_algo selected_algo);
static void generateProcesses();
static void addToReadyQueue(PCB *process);
//==================================== LOG DATA =====================================//
static float calculate_std_wta();
static void addPref(FILE *file);
static void addFinishLog(FILE *file, int currentTime, int processId, char *state, int arrivalTime, int totalRuntime, int waitingTime, int TA, float WTA);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Use: ./scheduler <scheduling_algo> <quantum>");
        exit(EXIT_FAILURE);
    }

    // Open file for writing
    logFile = fopen("scheduler.log", "w");
    perf = fopen("scheduler.perf", "w");

    // Check if the file was opened successfully
    if (logFile == NULL)
    {
        printf("Error opening scheduler.log!\n");
        return 1;
    }
    // Check if the file was opened successfully
    if (perf == NULL)
    {
        printf("Error opening scheduler.perf!\n");
        return 1;
    }

    wta_values = malloc(sizeof(float) * total_processes);

    // Set signal handlers for process initialization and termination
    signal(SIGUSR1, initializeProcesses);
    signal(SIGUSR2, noMoreProcesses);
    signal(SIGALRM, terminateRunningProcess);
    signal(SIGINT, clearResources);

    // Get instance of scheduler configuration and set it
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    schedulerConfig->selected_algorithm = (scheduling_algo)atoi(argv[1]);
    schedulerConfig->quantum = atoi(argv[2]);
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    // Array of scheduling functions corresponding to each algorithm
    void (*scheduleFunction[])(void *) = {scheduleHPF, scheduleSRTN, scheduleRR};

    int selectedAlgorithmIndex = schedulerConfig->selected_algorithm - 1;
    size_t prev_time = -1;

    // Allocate the data structure depending on the selected algorithm
    ready_queue = allocateDataStructure(schedulerConfig->selected_algorithm);
    queue = create_queue();

    initClk();

    printf("Scheduler id: %d\n", getpid());

    while (1)
    {
        int curr_time = getClk();
        // Handle context switching if the queue front changed
        generateProcesses();
        PCB *front_process = getRunningProcess(schedulerConfig->selected_algorithm);
        if (((running_process != NULL) != (front_process != NULL)) || (running_process && running_process->fork_id != front_process->fork_id))
        {
            printf("Context Switching\n");
            contentSwitch(front_process, running_process, getClk(), logFile);
            running_process = front_process;
            if (selectedAlgorithmIndex == RR)
                schedulerConfig->curr_quantum = schedulerConfig->quantum;
            curr_time = getClk();
        }
        // Run selected algorithm if the clock has ticked
        if (curr_time != prev_time)
        {
            prev_time = curr_time;
            printf("Time Step: %ld\n", prev_time);
            scheduleFunction[selectedAlgorithmIndex](ready_queue);
        }

        if (is_running_queue_empty(schedulerConfig->selected_algorithm) && is_queue_empty(queue) && endScheduler)
        {
            break;
        }
    }
    printf("Generating Output files!!!!\n\n");
    addPref(perf);
    fflush(logFile);
    fflush(perf);

    clearResources(0);

    // Upon termination release the clock resources.
    destroyClk(false);
    return 0;
}

//================================= SIGNAL HANDLERS =================================//

/**
 * initializeProcesses - Initializes processes based on the number of processes to be created.
 *
 * Description: On a received signal from the generator, initializes processes, forks them,
 *              and adds them to the ready queue.
 */
static void initializeProcesses(int signum)
{
    int msgQId = msgget(SHKEY, 0666 | IPC_CREAT);
    msgbuf_t msgbuf;

    while (msgrcv(msgQId, &msgbuf, sizeof(msgbuf.message), 0, IPC_NOWAIT) != -1)
    {
        PCB *process = NULL;
        process = malloc(sizeof(PCB));
        process->file_id = msgbuf.message.id;
        process->arrival = process->last_stop_time = msgbuf.message.arrival;
        process->runtime = msgbuf.message.runtime;
        process->priority = msgbuf.message.priority;
        process->start_time = -1;
        process->waiting_time = 0;

        enqueue(queue, (void *)process);
    }

    signal(SIGUSR1, initializeProcesses);
}

/**
 * terminateRunningProcess - Terminates the currently running process
 *
 * @param signum: The signal number that triggered the termination.
 *
 * Description: Waits for the currently running process to terminate and then removes
 *              it from the ready queue.
 */
static void terminateRunningProcess(int signum)
{
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    scheduling_algo selected_algo = schedulerConfig->selected_algorithm;

    int stat_loc;
    PCB *process = popRunningProcess(selected_algo);
    pid_t process_id = process->fork_id;

    //printQueue(ready_queue);

    wta_values = realloc(wta_values, sizeof(float) * total_processes);
    if (wta_values == NULL)
    {
        perror("Can not resize the array of WTA values");
        exit(EXIT_FAILURE);
    }

    wta_values[idx] = (float)(getClk() - process->arrival) / process->runtime;

    total_waiting_time += process->waiting_time;
    total_weighted_turnaround_time += wta_values[idx++];
    total_running_time += process->runtime;

    addFinishLog(logFile,
                 getClk(),
                 running_process->file_id,
                 "finished",
                 running_process->arrival,
                 running_process->runtime,
                 running_process->waiting_time,
                 getClk() - running_process->arrival,
                 (float)(getClk() - running_process->arrival) / running_process->runtime);

    free(running_process);

    running_process = NULL;

    printf("Process %d Terminated at %d.\n", process_id, getClk());
    //printQueue(ready_queue);

    waitpid(process_id, &stat_loc, 0);
    signal(SIGALRM, terminateRunningProcess);
}

/**
 * noMoreProcesses - Informs the scheduler that no more processes would be sent.
 */
static void noMoreProcesses(int signum)
{
    endScheduler = true;
}

/**
 * clearResources - Deallocates resources used by the scheduler
 * @param signum: The signal number
 * 
 * This function is a signal handler for cleaning up resources when SIGINT (Ctrl+C) is received.
 * It deallocates the data structures used by the scheduler, closes opened files, and prints a message.
 */
static void clearResources(int signum)
{
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    scheduling_algo selected_algo = schedulerConfig->selected_algorithm;

    // Deallocate the data structures
    if (selected_algo == RR)
        queue_free((queue_t *)ready_queue);
    else
        fib_heap_free((fib_heap_t *)ready_queue);
    queue_free((queue_t *)queue);

    // Close opened files
    fclose(logFile);
    fclose(perf);

    printf("Cleared Scheduler Resources\n");
}

//=============================== SCHEDULER FUNCTIONS ===============================//

/**
 * allocateDataStructure - Allocates and returns a data structure based on the selected scheduling algorithm
 * @param selected_algo: The selected scheduling algorithm (RR for Round Robin, others for other algorithms)
 * @return A void pointer to the allocated data structure
 *
 * This function allocates a data structure based on the selected scheduling algorithm.
 * For Round Robin (RR), it allocates a queue. For other algorithms, it allocates a Fibonacci heap.
 */
static void *allocateDataStructure(scheduling_algo selected_algo)
{
    if (selected_algo == RR)
        return (void *)create_queue();
    else
        return (void *)fib_heap_alloc();
}

/**
 * getRunningProcess - Retrieves the running process from the ready queue based on the selected algorithm
 * @param selected_algo: The selected scheduling algorithm (RR for Round Robin, others for other algorithms)
 * @return A pointer to the running process
 *
 * This function retrieves the running process from the ready queue based on the selected scheduling algorithm.
 * For Round Robin (RR), it returns the front of the queue. For other algorithms, it returns the minimum element of the Fibonacci heap.
 */
static PCB *getRunningProcess(scheduling_algo selected_algo)
{
    if (selected_algo == RR)
        return (PCB *)front((queue_t *)ready_queue);
    else
        return (PCB *)fib_heap_min((fib_heap_t *)ready_queue);
}

/**
 * popRunningProcess - Removes and returns the running process from the ready queue based on the selected algorithm
 * @param selected_algo: The selected scheduling algorithm (RR for Round Robin, others for other algorithms)
 * @return A pointer to the removed running process
 *
 * This function removes and returns the running process from the ready queue based on the selected scheduling algorithm.
 * For Round Robin (RR), it dequeues the front of the queue. For other algorithms, it extracts the minimum element of the Fibonacci heap.
 */
static PCB *popRunningProcess(scheduling_algo selected_algo)
{
    if (selected_algo == RR)
        return (PCB *)dequeue((queue_t *)ready_queue);
    else
        return (PCB *)fib_heap_extract_min((fib_heap_t *)ready_queue);
}

/**
 * is_running_queue_empty - Checks if the running queue is empty
 * @param selected_algo: The selected scheduling algorithm
 * @return 1 if the running queue is empty, otherwise 0
 * 
 * This function checks if the running queue is empty based on the selected scheduling algorithm.
 */
static short is_running_queue_empty(scheduling_algo selected_algo)
{
    if (selected_algo == RR)
        return is_queue_empty((queue_t *)ready_queue);
    else
        return fib_heap_size((fib_heap_t *)ready_queue) == 0;
}

/**
 * generateProcesses - Generates processes from the queue
 *
 * This function generates processes by forking and executing a program
 * with arguments based on the information stored in the queue.
 */
static void generateProcesses()
{
    if (!queue)
        return;
    char *args[6];

    // Get path to the process.out
    char absolute_path[PATH_SIZE];
    getAbsolutePath(absolute_path, "process.out");

    // Initialize the args
    args[0] = absolute_path;
    args[5] = NULL; // Null-terminate the argument list
    for (int i = 1; i < 5; i++)
        args[i] = (char *)malloc(12); // Assuming a 32-bit int can be at most 11 digits, plus 1 for null terminator
    while (!is_queue_empty(queue))
    {
        PCB *process = (PCB *)dequeue(queue);
        sprintf(args[1], "%d", process->file_id);
        sprintf(args[2], "%d", process->arrival);
        sprintf(args[3], "%d", process->runtime);
        sprintf(args[4], "%d", process->priority);

        printf("Generating Process #%d\n", process->file_id);
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Couldn't fork a process in scheduler");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            execvp(args[0], args);
            perror("Couldn't use execvp");
            exit(EXIT_FAILURE);
        }
        usleep(100 * 1000);
        process->fork_id = pid;
        process->state = NEWBIE;
        addToReadyQueue(process);
    }
    for (int i = 1; i < 5; i++)
        free(args[i]);
}

/**
 * addToReadyQueue - Adds a process to the ready queue based on the scheduling algorithm.
 *
 * @param process: Pointer to the process to be added.
 *
 * Description: Adds a process to the ready queue based on the selected scheduling algorithm.
 */
static void addToReadyQueue(PCB *process)
{
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    scheduling_algo selectedAlgorithm = schedulerConfig->selected_algorithm;

    switch (selectedAlgorithm)
    {
    case RR:
        enqueue((queue_t *)ready_queue, (void *)process);
        break;
    case SRTN:
        fib_heap_insert((fib_heap_t *)ready_queue, (void *)process, process->runtime);
        break;
    case HPF:
        fib_heap_insert((fib_heap_t *)ready_queue, (void *)process, process->priority);
    }

    total_processes++;
}

//==================================== LOG DATA =====================================//

/**
 * calculate_std_wta - Calculates the standard deviation of weighted turnaround time (WTA)
 * 
 * This function calculates the standard deviation of WTA using the formula:
 * standard deviation = sqrt((sum of squared differences from mean) / total_processes)
 * 
 * @return The standard deviation of WTA
 */
static float calculate_std_wta()
{
    float mean = total_weighted_turnaround_time / total_processes;
    float sum = 0.0;
    for (int i = 0; i < total_processes; i++)
    {
        sum += pow(wta_values[i] - mean, 2);
    }
    float variance = sum / total_processes;
    return sqrt(variance);
}

/**
 * addPref - Adds performance metrics to a log file
 * @param file: Pointer to the log file
 * 
 * This function adds CPU utilization, average WTA, average waiting time, and standard deviation of WTA
 * to a log file.
 */
static void addPref(FILE *file)
{
    fprintf(file, "CPU utilization = %.2f%%\n", (float)((total_running_time) / (float)getClk()) * 100.0);
    fprintf(file, "Avg WTA = %.2f\n", total_weighted_turnaround_time / total_processes);
    fprintf(file, "Avg Waiting = %.2f\n", total_waiting_time / total_processes);
    fprintf(file, "STD WTA = %.2f\n", calculate_std_wta());
}

/**
 * addFinishLog - Writes process finish information to a log file
 * @param file: Pointer to the log file
 * @param currentTime: The current time
 * @param processId: The ID of the finished process
 * @param state: The state of the finished process
 * @param arrivalTime: The arrival time of the finished process
 * @param totalRuntime: The total runtime of the finished process
 * @param waitingTime: The waiting time of the finished process
 * @param TA: The turnaround time of the finished process
 * @param WTA: The weighted turnaround time of the finished process
 * 
 * This function writes process finish information to a log file in a specific format.
 */
static void addFinishLog(FILE *file, int currentTime, int processId, char *state, int arrivalTime, int totalRuntime, int waitingTime, int TA, float WTA)
{
    fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime, processId, state, arrivalTime, totalRuntime, 0, waitingTime, TA, WTA);
}

// void printQueue(queue_t *head)
// {
//     queue_t *temp = *head;
//     while (temp)
//     {
//         PCB *process = (PCB *)(temp->process);
//         printf("(%d, %d, %d)", process->file_id, process->priority, process->runtime);
//         if (temp->next)
//             printf("->");
//         temp = temp->next;
//     }
//     printf("\n");
// }
