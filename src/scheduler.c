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
pqueue_t **ready_queue, **queue;
void generateProcesses();
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
    schedulerConfig->quantum = atoi(argv[2]);
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    // Array of scheduling functions corresponding to each algorithm
    void (*scheduleFunction[])(pqueue_t **) = {scheduleHPF, scheduleSRTN, scheduleRR};

    int selectedAlgorithmIndex = schedulerConfig->selected_algorithm - 1;
    size_t prev_time = 0;

    ready_queue = malloc(sizeof(pqueue_t *));
    (*ready_queue) = NULL;
    queue = malloc(sizeof(pqueue_t *));
    (*queue) = NULL;

    initClk();

    printf("Scheduler id: %d\n", getpid());
    PCB *running_process = NULL;
    while (1)
    {
        int curr_time = getClk();
        // Handle context switching if the queue front changed
        generateProcesses();
        PCB *front_process = (*ready_queue == NULL ? NULL : ((PCB *)((*ready_queue)->process)));
        if (((running_process != NULL) != (front_process != NULL)) || (running_process && running_process->fork_id != front_process->fork_id))
        {
            printf("Context Switching\n");
            contentSwitch(front_process, running_process);
            running_process = front_process;
            if (selectedAlgorithmIndex == RR)
                schedulerConfig->curr_quantum = schedulerConfig->quantum;
            curr_time = getClk();
            printf("Finished Context Switching\n");
        }
        // Run selected algorithm if the clock has ticked
        if (curr_time != prev_time)
        {
            prev_time = curr_time;
            printf("Time Step: %ld\n", prev_time);
            scheduleFunction[selectedAlgorithmIndex](ready_queue);
        }
    }

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
    printf("SIGUSR1 received\n");
    int msgQId = msgget(SHKEY, 0666 | IPC_CREAT);
    msgbuf_t msgbuf;

    while (msgrcv(msgQId, &msgbuf, sizeof(msgbuf.message), 0, IPC_NOWAIT) != -1)
    {
        PCB *process = NULL;
        process = malloc(sizeof(PCB));
        process->file_id = msgbuf.message.id;
        process->arrival = msgbuf.message.arrival;
        process->runtime = msgbuf.message.runtime;
        process->priority = msgbuf.message.priority;

        printf("Current time: %d, Process #%d\n", getClk(), process->file_id);
        push(queue, (void *)process, 0);
    }

    if (msgctl(msgQId, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, initializeProcesses);
}

void generateProcesses() {
    if (!(*queue)) return;
    char *args[6];
    char absolute_path[PATH_SIZE];
    getAbsolutePath(absolute_path, "process.out");
    args[0] = absolute_path;
    args[5] = NULL; // Null-terminate the argument list
    // Allocate memory for each string in args
    for (int i = 1; i < 5; i++)
        args[i] = (char *)malloc(12); // Assuming a 32-bit int can be at most 11 digits, plus 1 for null terminator

    while ((*queue) != NULL) {
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
        usleep(1000);
        kill(pid, SIGSTOP);
        printf("Process %d Paused\n", pid);
        process->fork_id = pid;
        process->state = READY;
        addToReadyQueue(process);
    }
    for (int i = 1; i < 5; i++)
        free(args[i]);
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
}
