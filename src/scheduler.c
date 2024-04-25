#include "clk.h"
#include "header.h"

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
pqueue_t **ready_queue;

float total_waiting_time = 0; // sum of waiting times
float total_weighted_turnaround_time = 0; // sum of weighted turnaround times
float *wta_values = NULL; // array of weighted turnaround times
int total_processes = 0; //total number of processes that come so far
int idx = 0; //index of the wta_values array
int waste_time = 0; //cpu wasted time

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
    fprintf(file, "CPU utilization = %.2f%%\n", (float)((getClk() - waste_time) / (float)getClk()) * 100.0);
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
    FILE *log = fopen("schedular.log.txt", "a");   // "a" to append to the file
    FILE *perf = fopen("scheduler.perf.txt", "a"); // "a" to append to the file

    // Check if the file was opened successfully
    if (log == NULL)
    {
        printf("Error opening schedular.log!\n");
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

    // Get instance of scheduler configuration and set it
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    schedulerConfig->selected_algorithm = (scheduling_algo)atoi(argv[1]);
    schedulerConfig->quantum = atoi(argv[2]);
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    // Array of scheduling functions corresponding to each algorithm
    void (*scheduleFunction[])(pqueue_t **) = {scheduleHPF, scheduleSRTN, scheduleRR};

    int selectedAlgorithmIndex = schedulerConfig->selected_algorithm - 1;
    size_t prevTime = 0;

    ready_queue = malloc(sizeof(pqueue_t *));
    (*ready_queue) = NULL;

    initClk();

    printf("Scheduler id: %d\n", getpid());

    PCB *running_process = NULL;
    while (1)
    {
        // Handle context switching if the queue front changed
        PCB *front_process = (*ready_queue == NULL ? NULL : ((PCB *)((*ready_queue)->process)));
        if (running_process != front_process)
        {
            contentSwitch(front_process, running_process, getClk(), log);
            running_process = front_process;
            if (selectedAlgorithmIndex == RR)
                schedulerConfig->curr_quantum = schedulerConfig->quantum;
        }

        // Run selected algorithm if the clock has ticked
        if (getClk() != prevTime)
        {
            if (*ready_queue == NULL)
                waste_time++;
            prevTime = getClk();
            printf("Time Step: %ld\n", prevTime);
            scheduleFunction[selectedAlgorithmIndex](ready_queue);
        }
    }

    addPref(perf);

    fclose(log);
    fclose(perf);

    // Upon termination release the clock resources.
    destroyClk(true);
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
    printf("SIGUSR1 received\n");
    pqueue_t **head = ready_queue;

    int msgQId = msgget(SHKEY, 0666 | IPC_CREAT);
    msgbuf_t msgbuf;

    char *args[6];
    args[0] = "./bin/process.out";
    args[5] = NULL; // Null-terminate the argument list

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

        // Allocate memory for each string in args
        for (int i = 1; i < 5; i++)
            args[i] = (char *)malloc(12); // Assuming a 32-bit int can be at most 11 digits, plus 1 for null terminator
        sprintf(args[1], "%d", process->file_id);
        sprintf(args[2], "%d", process->arrival);
        sprintf(args[3], "%d", process->runtime);
        sprintf(args[4], "%d", process->priority);

        printf("Current time: %d\n", getClk());

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
        usleep(1000);
        printf("Process %d paused\n", pid);
        // kill(pid, SIGSTOP);
        process->fork_id = pid;
        process->state = NEWBIE;
        addToReadyQueue(process);
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
void terminateRunningProcess(int signum)
{
    pqueue_t **head = ready_queue;
    int stat_loc;
    pid_t sid = wait(&stat_loc), process_id = ((PCB *)((*head)->process))->fork_id;
    if (sid != process_id)
    {
        perror("Terminated Process isn't the running process");
        exit(EXIT_FAILURE);
    }

    wta_values = realloc(wta_values, sizeof(float) * total_processes);
    if (wta_values == NULL)
    {
        perror("Can not resize the array of WTA values");
        exit(EXIT_FAILURE);
    }

    wta_values[idx] = (float)(getClk() - ((PCB *)((*head)->process))->arrival) / ((PCB *)((*head)->process))->runtime;

    ((PCB *)((*head)->process))->state = FINISHED;
    total_waiting_time += ((PCB *)((*head)->process))->waiting_time;
    total_weighted_turnaround_time += wta_values[idx++];

    pop(head);
    ready_queue = head;
    printf("Process %d Terminated at %d.\n", sid, getClk());
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
