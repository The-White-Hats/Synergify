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

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Use: ./scheduler <scheduling_algo> <quantum>");
        exit(EXIT_FAILURE);
    }

    // Set signal handlers for process initialization and termination
    signal(SIGUSR1, initializeProcesses);
    signal(SIGCHLD, terminateRunningProcess);

    // Get instance of scheduler configuration and set it
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    schedulerConfig->selected_algorithm = (scheduling_algo)atoi(argv[1]);
    schedulerConfig->quantum = atoi(argv[1]);
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    // Array of scheduling functions corresponding to each algorithm
    void (*scheduleFunction[])(pqueue_t **) = {scheduleHPF, scheduleSRTN, scheduleRR};

    int selectedAlgorithmIndex = schedulerConfig->selected_algorithm - 1;
    size_t prevTime = 0;
    ready_queue = malloc(sizeof(pqueue_t *));
    initClk();

    pid_t running_pid;
    while (1)
    {
        // Handle context switching if the queue front changed
        pid_t front_pid = (*ready_queue == NULL ? -1 : ((process_info_t *)((*ready_queue)->process))->fork_id);
        if (running_pid != front_pid)
        {
            contentSwitch(front_pid, running_pid);
            running_pid = front_pid;
            if (selectedAlgorithmIndex == RR)
                schedulerConfig->curr_quantum = schedulerConfig->quantum;
        }

        // Run selected algorithm if the clock has ticked
        if (getClk() != prevTime)
        {
            prevTime = getClk();
            printf("Time Step: %ld\n", prevTime);
            scheduleFunction[selectedAlgorithmIndex](ready_queue);
        }
    }

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
    // TODO: Receive The number of processes from a shared memory.
    size_t num_of_processes = 1;
    char *args[6];
    args[0] = "./b";
    args[5] = NULL; // Null-terminate the argument list
    while (num_of_processes--)
    {
        process_info_t *process = NULL;
        process = malloc(sizeof(process_info_t));
        process->id = 1;
        process->arrival = 0;
        process->runtime = 5;
        process->priority = 5;
        // TODO: Receive The Processes Info From The Message Queue
        
        // Allocate memory for each string in args
        for (int i = 1; i < 5; i++)
            args[i] = (char *)malloc(12); // Assuming a 32-bit int can be at most 11 digits, plus 1 for null terminator
        sprintf(args[1], "%d", process->id);
        sprintf(args[2], "%d", process->arrival);
        sprintf(args[3], "%d", process->runtime);
        sprintf(args[4], "%d", process->priority);

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
        kill(pid, SIGSTOP);
        process->fork_id = pid;
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
    pid_t sid = wait(&stat_loc), process_id = ((process_info_t *)((*head)->process))->fork_id;
    if (sid != process_id)
    {
        perror("Terminated Process isn't the running process");
        exit(EXIT_FAILURE);
    }
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
void addToReadyQueue(process_info_t *process)
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
}