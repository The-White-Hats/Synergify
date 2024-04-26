#include "clk.h"
#include "header.h"
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
pqueue_t **ready_queue, **queue;
void generateProcesses();
bool endScheduler = false;

float total_waiting_time = 0;             // sum of waiting times
float total_weighted_turnaround_time = 0; // sum of weighted turnaround times
float total_running_time = 0;             // sum of running times
float *wta_values = NULL;                 // array of weighted turnaround times
int total_processes = 0;                  // total number of processes that come so far
int idx = 0;                              // index of the wta_values array
int waste_time = 0;                       // cpu wasted time

PCB *running_process = NULL;
FILE *logFile;
float calculate_std_wta()
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

void addPref(FILE *file)
{
    fprintf(file, "CPU utilization = %.2f%%\n", (float)((total_running_time) / (float)getClk()) * 100.0);
    fprintf(file, "Avg WTA = %.2f\n", total_weighted_turnaround_time / total_processes);
    fprintf(file, "Avg Waiting = %.2f\n", total_waiting_time / total_processes);
    fprintf(file, "STD WTA = %.2f\n", calculate_std_wta());
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Use: ./scheduler <scheduling_algo> <quantum>");
        exit(EXIT_FAILURE);
    }

    // Open file for writing
    logFile = fopen("schedular.log", "w");
    FILE *perf = fopen("scheduler.perf", "w");

    // Check if the file was opened successfully
    if (logFile == NULL)
    {
        printf("Error opening schedular.logFile!\n");
        return 1;
    }
    // Check if the file was opened successfully
    if (perf == NULL)
    {
        printf("Error opening schedular.perf!\n");
        return 1;
    }

    wta_values = malloc(sizeof(float) * total_processes);

    // Set signal handlers for process initialization and termination
    signal(SIGUSR1, initializeProcesses);
    signal(SIGCHLD, terminateRunningProcess);
    signal(SIGUSR2, noMoreProcesses);

    // Get instance of scheduler configuration and set it
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    schedulerConfig->selected_algorithm = (scheduling_algo)atoi(argv[1]);
    schedulerConfig->quantum = atoi(argv[2]);
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    // Array of scheduling functions corresponding to each algorithm
    void (*scheduleFunction[])(pqueue_t **) = {scheduleHPF, scheduleSRTN, scheduleRR};

    int selectedAlgorithmIndex = schedulerConfig->selected_algorithm - 1;
    size_t prev_time = -1;

    ready_queue = malloc(sizeof(pqueue_t *));
    (*ready_queue) = NULL;
    queue = malloc(sizeof(pqueue_t *));
    (*queue) = NULL;

    initClk();

    printf("Scheduler id: %d\n", getpid());

    while (1)
    {
        int curr_time = getClk();
        // Handle context switching if the queue front changed
        generateProcesses();
        PCB *front_process = (*ready_queue == NULL ? NULL : ((PCB *)((*ready_queue)->process)));
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

        if ((*ready_queue) == NULL && (*queue) == NULL && endScheduler)
        {
            break;
        }
    }
    printf("Generating Output files!!!!\n");
    addPref(perf);

    fclose(logFile);
    fclose(perf);

    // Upon termination release the clock resources.
    destroyClk(false);
    return 0;
}

/**
 * initializeProcesses - Initializes processes based on the number of processes to be created.
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param num_of_processes: Number of processes to be initialized.
 *
 * Description: On a received signal from the generator, initializes processes, forks them,
 *              and adds them to the ready queue.
 */
void initializeProcesses(int signum)
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

        push(queue, (void *)process, 0);
    }

    signal(SIGUSR1, initializeProcesses);
}

void generateProcesses()
{
    if (!(*queue))
        return;
    char *args[6];
    char absolute_path[PATH_SIZE];
    getAbsolutePath(absolute_path, "process.out");
    args[0] = absolute_path;
    args[5] = NULL; // Null-terminate the argument list
    // Allocate memory for each string in args
    for (int i = 1; i < 5; i++)
        args[i] = (char *)malloc(12); // Assuming a 32-bit int can be at most 11 digits, plus 1 for null terminator
    while ((*queue) != NULL)
    {
        PCB *process = (PCB *)((*queue)->process);
        pop(queue);
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

        printf("Process %d Paused \n", pid);
        process->fork_id = pid;
        process->state = NEWBIE;
        addToReadyQueue(process);
    }
    for (int i = 1; i < 5; i++)
        free(args[i]);
}

void addFinishLog(FILE *file, int currentTime, int processId, char *state, int arrivalTime, int totalRuntime, int waitingTime, int TA, float WTA)
{
    fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime, processId, state, arrivalTime, totalRuntime, 0, waitingTime, TA, WTA);
}

/**
 * terminateRunningProcess - Terminates the currently running process
 *
 * @param signum: The signal number that triggered the termination.
 *
 * Description: Waits for the currently running process to terminate and then removes
 *              it from the ready queue.
 */
void terminateRunningProcess(int signum)
{
    pqueue_t **head = ready_queue;
    int stat_loc;
    pid_t process_id = ((PCB *)((*head)->process))->fork_id;

    printQueue(ready_queue);

    wta_values = realloc(wta_values, sizeof(float) * total_processes);
    if (wta_values == NULL)
    {
        perror("Can not resize the array of WTA values");
        exit(EXIT_FAILURE);
    }

    wta_values[idx] = (float)(getClk() - ((PCB *)((*head)->process))->arrival) / ((PCB *)((*head)->process))->runtime;

    total_waiting_time += ((PCB *)((*head)->process))->waiting_time;
    total_weighted_turnaround_time += wta_values[idx++];
    total_running_time += ((PCB *)((*head)->process))->runtime;

    pop(head);
    ready_queue = head;
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
    printQueue(ready_queue);

    waitpid(process_id, &stat_loc, 0);
    signal(SIGCHLD, terminateRunningProcess);
}

/**
 * addToReadyQueue - Adds a process to the ready queue based on the scheduling algorithm.
 *
 * @param process: Pointer to the process to be added.
 *
 * Description: Adds a process to the ready queue based on the selected scheduling algorithm.
 */
void addToReadyQueue(PCB *process)
{
    pqueue_t **head = ready_queue;
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    scheduling_algo selectedAlgorithm = schedulerConfig->selected_algorithm;

    switch (selectedAlgorithm)
    {
    case RR:
        push(head, (void *)process, 0);
        break;
    case SRTN:
        push(head, (void *)process, process->runtime);
        break;
    case HPF:
        if (*head == NULL)
        {
            push(head, (void *)process, process->priority);
            break;
        }
        pqueue_t **temp_head = &((*head)->next);
        push(temp_head, (void *)process, process->priority);
        (*head)->next = *temp_head;
        break;
    }
    ready_queue = head;
    total_processes++;
}

/**
 * noMoreProcesses - Informs the scheduler that no more processes would be sent.
 */
void noMoreProcesses(int signum)
{
    endScheduler = true;
}

void printQueue(pqueue_t **head)
{
    pqueue_t *temp = *head;
    while (temp)
    {
        PCB *process = (PCB *)(temp->process);
        printf("(%d, %d, %d)", process->file_id, process->priority, process->runtime);
        if (temp->next)
            printf("->");
        temp = temp->next;
    }
    printf("\n");
}