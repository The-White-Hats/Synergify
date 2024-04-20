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

int main(int argc, char *argv[])
{
    if (argc != 3) {
        perror("Use: ./scheduler <scheduling_algo> <quantum>");
        exit(EXIT_FAILURE);
    }

    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    schedulerConfig->selected_algorithm = (scheduling_algo) atoi(argv[1]);
    schedulerConfig->quantum = atoi(argv[1]);
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    void (*scheduleFunction[])(pqueue_t **) = {scheduleHPF, scheduleSRTN, scheduleRR};
    int selectedAlgorithmIndex, incoming_requests, prevTime = -1;
    pqueue_t **ready_queue = malloc(sizeof(pqueue_t *));
    initClk();

    // Commented unused variables for now
    

    selectedAlgorithmIndex = schedulerConfig->selected_algorithm - 1;

    // TODO implement the scheduler :)
    // upon termination release the clock resources.
    while (1)
    {
        while (getClk() == prevTime);
        prevTime = getClk();
        // 1. Check Process Generator Signal.
        incoming_requests = 0;  // temp value
        // 2. Create Process if the generator flag is set.
        if (incoming_requests > 0)
        {
            initializeProcesses(ready_queue, incoming_requests);
        }
        // 3. Run selected algorithm.
        scheduleFunction[selectedAlgorithmIndex](ready_queue);
    }

    destroyClk(true);
}

/**
 * initializeProcesses - Initializes processes based on the number of processes to be created.
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param num_of_processes: Number of processes to be initialized.
 *
 * Description: Initializes processes, forks them, and adds them to the ready queue.
 */
void initializeProcesses(pqueue_t **head, int num_of_processes)
{
    while (num_of_processes--)
    {
        process_info_t *process = NULL;
        // TODO: Receive The Processes Info From The Message Queue

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Couldn't fork a process in scheduler");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            char *args[5];
            args[0] = "./process.out";
            // Allocate memory for each string in args
            for (int i = 1; i < 5; i++) {
                args[i] = (char*)malloc(12); // Assuming a 32-bit int can be at most 11 digits, plus 1 for null terminator
            }
            sprintf(args[1], "%d", process->id);
            sprintf(args[2], "%d", process->arrival);
            sprintf(args[3], "%d", process->runtime);
            sprintf(args[4], "%d", process->priority);

            execvp(args[0], args);
            perror("Couldn't use execvp");
            exit(EXIT_FAILURE);
        }
        process->fork_id = pid;
        addToReadyQueue(head, process);
    }
}

/**
 * addToReadyQueue - Adds a process to the ready queue based on the scheduling algorithm.
 *
 * @param head: Pointer to the pointer to the head of the priority queue.
 * @param process: Pointer to the process to be added.
 *
 * Description: Adds a process to the ready queue based on the selected scheduling algorithm.
 */
void addToReadyQueue(pqueue_t **head, process_info_t *process)
{
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    scheduling_algo selectedAlgorithm = schedulerConfig->selected_algorithm;

    switch (selectedAlgorithm)
    {
    case RR:
        push(head, (void *)process, 0);
        break;
    case SRTN:
        pqueue_t *running_process = *head;
        if (running_process != NULL && running_process->priority > process->runtime)
        {
            // TODO: Send Signal To Running Process To Sleep.
        }
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
}