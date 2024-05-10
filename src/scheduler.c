#include "clk.h"
#include "header.h"
#include "gui/gui.h"
#include "ds/fib_heap.h"
#include "ds/queue.h"
#include "buddy_memory.h"
#include <math.h>

//================================= SIGNAL HANDLERS =================================//
static void initializeProcesses(int signum);
static void terminateRunningProcess(int signum);
static void noMoreProcesses(int signum);
static void clearResources(int signum);
static void processDecremented(int signum);

//=============================== SCHEDULER FUNCTIONS ===============================//
static void *allocateDataStructure(scheduling_algo selected_algo);
static PCB *getRunningProcess(scheduling_algo selected_algo);
static PCB *popRunningProcess(scheduling_algo selected_algo);
static short is_running_queue_empty(scheduling_algo selected_algo);
static void generateProcesses();
static void addToStateQueue(PCB *process);
static void addToBlockQueue(PCB *process);
static void addToReadyQueue(PCB *process);
static void checkBlockQueue(int freed_memsize);
SchedulerConfig *getSchedulerConfigInstance();

//====================================== GUI ========================================//
void createTaskManager(pthread_t *gui_thread);

//==================================== LOG DATA =====================================//
static float calculate_std_wta();
static void addPerf(FILE *file);
static void addFinishLog(FILE *file, int currentTime, int processId,
                         char *state, int arrivalTime, int totalRuntime,
                         int waitingTime, int TA, float WTA);

//====================== GLOBAL VARIABLES (scheduelr related) =======================//
void *ready_queue = NULL;
buddy_tree_t *buddy_system_tree = NULL;
queue_t *queue = NULL, *block_queue = NULL;
bool endScheduler = false;
PCB *running_process = NULL;
int selectedAlgorithmIndex;
void (*scheduleFunction[])(void *) = {scheduleHPF, scheduleSRTN, scheduleRR};

//====================== GLOBAL VARIABLES (log file related) =======================//
float total_waiting_time = 0;             // sum of waiting times
float total_weighted_turnaround_time = 0; // sum of weighted turnaround times
float total_running_time = 0;             // sum of running times
float *wta_values = NULL;                 // array of weighted turnaround times
int total_processes = 0;                  // total number of processes that come so far
int idx = 0;                              // index of the wta_values array
int waste_time = 0;                       // cpu wasted time
FILE *logFile, *perfFile;
const char *const SCHEDULER_LOG_NAME = "scheduler.log";
const char *const SCHEDULER_PERF_NAME = "scheduler.perf";

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Use: ./scheduler <scheduling_algo> <quantum>");
        exit(EXIT_FAILURE);
    }

    /* Open output files */
    logFile = fopen(SCHEDULER_LOG_NAME, "w");
    perfFile = fopen(SCHEDULER_PERF_NAME, "w");

    if (logFile == NULL)
    {
        printf("Error opening %s!\n", SCHEDULER_LOG_NAME);
        return 1;
    }
    if (perfFile == NULL)
    {
        printf("Error opening %s!\n", SCHEDULER_PERF_NAME);
        return 1;
    }

    wta_values = malloc(sizeof(float) * total_processes);

    // Set signal handlers for process initialization and termination
    signal(SIGUSR1, initializeProcesses);
    signal(SIGUSR2, noMoreProcesses);
    signal(SIGALRM, terminateRunningProcess);
    signal(SIGINT, clearResources);
    signal(SIGPWR, processDecremented);

    // Get instance of scheduler configuration and set it
    SchedulerConfig *schedulerConfig = getSchedulerConfigInstance();
    schedulerConfig->selected_algorithm = (scheduling_algo)atoi(argv[1]);
    schedulerConfig->quantum = atoi(argv[2]);
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    selectedAlgorithmIndex = schedulerConfig->selected_algorithm - 1;

    int prev_time = -1;
    float prev_time_float = -.5;

    // Allocate the data structure depending on the selected algorithm
    buddy_system_tree = create_buddy_tree();
    ready_queue = allocateDataStructure(schedulerConfig->selected_algorithm);
    block_queue = create_queue();
    queue = create_queue();

    // Create task manager gui
    pthread_t gui_thread;
    createTaskManager(&gui_thread);

    initClk();

    while (1)
    {
        int curr_time = getClk();
        float curr_time_float = getClkFloat();
        generateProcesses();

        if (curr_time_float - prev_time_float == 1)
        {
            PCB *front_process = getRunningProcess(schedulerConfig->selected_algorithm);

            if (((running_process != NULL) != (front_process != NULL)) ||
                (running_process && running_process->fork_id != front_process->fork_id))
            {
                contentSwitch(front_process, running_process, getClk(), logFile);
                running_process = front_process;
                if (selectedAlgorithmIndex == RR)
                    schedulerConfig->curr_quantum = schedulerConfig->quantum;
                curr_time = getClk();
            }

            prev_time_float = curr_time_float;
        }

        // Run selected algorithm if the clock has ticked
        if (curr_time != prev_time)
        {
            PCB *front_process = getRunningProcess(schedulerConfig->selected_algorithm);
            prev_time = curr_time;
        }

        if (is_running_queue_empty(schedulerConfig->selected_algorithm) && is_queue_empty(queue) && endScheduler)
        {
            break;
        }
    }

    addPerf(perfFile);
    fflush(logFile);
    fflush(perfFile);

    // Join the gui thread
    if (pthread_join(gui_thread, NULL) != 0)
    {
        perror("Error joining thread");
        exit(EXIT_FAILURE);
    }

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
        process->memsize = msgbuf.message.memsize;
        process->ptr_mem = NULL;
        // TODO: allocate a new memory with size msgbuf.message.memsize and assign it to the process

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
    schedulerConfig->curr_quantum = schedulerConfig->quantum;

    int stat_loc;
    PCB *process = popRunningProcess(selected_algo);
    pid_t process_id = process->fork_id;

    wta_values = realloc(wta_values, sizeof(float) * total_processes);
    if (wta_values == NULL)
    {
        perror("Can not resize the array of WTA values");
        exit(EXIT_FAILURE);
    }

    wta_values[idx] = (process->runtime == 0) ? 0 : (float)(getClk() - process->arrival) / process->runtime;

    total_waiting_time += process->waiting_time;
    total_weighted_turnaround_time += wta_values[idx++];
    total_running_time += process->runtime;
    float WTA = (running_process->runtime == 0) ? 0 : (float)(getClk() - running_process->arrival) / running_process->runtime;
    addFinishLog(logFile,
                 getClk(),
                 running_process->file_id,
                 "finished",
                 running_process->arrival,
                 running_process->runtime,
                 running_process->waiting_time,
                 getClk() - running_process->arrival,
                 WTA);

    // TODO: Free allocate memory for the process from the buddy system.
    int freed_memsize = running_process->memsize;
    free_memory((buddy_node_t *)running_process->ptr_mem);
    free(running_process);

    running_process = NULL;
    checkBlockQueue(freed_memsize);

    printf("block:\n");
    queue_print(block_queue);

    waitpid(process_id, &stat_loc, 0);
    signal(SIGALRM, terminateRunningProcess);
}

/**
 * noMoreProcesses - Informs the scheduler that no more processes would be sent.
 * @param signum: Signal number.
 *
 * It sets a flag *endScheduler* with true, which we check over to know if there
 * are any more processes that would be sent or not.
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
        queue_free((queue_t *)ready_queue, true);
    else
        fib_heap_free((fib_heap_t *)ready_queue, 1);
    queue_free(queue, true);
    queue_free(block_queue, true);
    buddy_free(buddy_system_tree, true);

    // Close opened files
    fclose(logFile);
    fclose(perfFile);

    killpg(getgid(), SIGINT);
}

/**
 * processDecremented - Decrement the time for the current running process.
 * @param signum: Signal number.
 */
static void processDecremented(int signum)
{
    scheduleFunction[selectedAlgorithmIndex](ready_queue);
    signal(SIGPWR, processDecremented);
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

        pid_t pid = fork();
        if (pid == -1)
        {
            // perror("Couldn't fork a process in scheduler");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            execvp(args[0], args);
            perror("Couldn't use execvp");
            exit(EXIT_FAILURE);
        }
        usleep(10 * 1000);
        process->fork_id = pid;
        process->state = NEWBIE;
        addToStateQueue(process);
    }
    for (int i = 1; i < 5; i++)
        free(args[i]);
}

/**
 * addToBlockedQueue - Adds a process to the block queue.
 *
 * @param process: Pointer to the process to be added.
 */
static void addToStateQueue(PCB *process)
{
    total_processes++;
    buddy_node_t* buddy_node = allocate_memory(process->memsize, buddy_system_tree);
    
    if (buddy_node == NULL) return addToBlockQueue(process);
    
    process->ptr_mem = (void *)buddy_node;
    addToReadyQueue(process);
}

/**
 * addToBlockedQueue - Adds a process to the block queue.
 *
 * @param process: Pointer to the process to be added.
 */
static void addToBlockQueue(PCB *process)
{
    process->state = BLOCKED;
    enqueue(block_queue, (void *)process);
}

/**
 * addToReadyQueue - Adds a process to the ready queue based on the scheduling algorithm.
 *
 * @param process: Pointer to the process to be added.
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
}

static void checkBlockQueue(int freed_memsize)
{
    printf("before:\n");
    queue_print(block_queue);
    queue_node_t *dummy, *parent, *delete, *iterator;
    bool isHead;

    if (is_queue_empty(block_queue))    return;

    iterator = block_queue->head;
    dummy = malloc(sizeof(queue_node_t));
    parent = dummy;
    parent->next = iterator;

    while (freed_memsize && iterator)
    {
        PCB *process = (PCB *)iterator->data;
        int memsize = process->memsize;
        if (freed_memsize >= memsize) {
            // Try to allocate memory
            process->ptr_mem = (void *)allocate_memory(memsize, buddy_system_tree);
            if (process->ptr_mem == NULL) {
                perror("freed block in buddy tree\n");
                return;
            }

            process->state = NEWBIE;
            addToReadyQueue(process);

            // Remove allocated block size from the free memory
            freed_memsize -= pow(2, ceil(log2(memsize)));
            isHead = (iterator == block_queue->head);
            iterator = iterator->next;
            if (isHead) {
                printf("Blocked process is head!\n");
                block_queue->head = iterator;
                if (!iterator) block_queue->tail = NULL;
                queue_print(block_queue);
            }
            free(parent->next);
            parent->next = iterator;
            continue;
        }
        parent = iterator;
        iterator = iterator->next;
    }

    free(dummy);
}

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

//====================================== GUI ========================================//

void createTaskManager(pthread_t *gui_thread)
{
    int result = pthread_create(gui_thread, NULL, initTaskManager, ready_queue);
    if (result != 0)
    {
        perror("GUI Thread Failed");
        exit(EXIT_FAILURE);
    }
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
 * addPerf - Adds performance metrics to a log file
 * @param file: Pointer to the log file
 *
 * This function adds CPU utilization, average WTA, average waiting time, and standard deviation of WTA
 * to a log file.
 */
static void addPerf(FILE *file)
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
static void addFinishLog(FILE *file, int currentTime, int processId,
                         char *state, int arrivalTime, int totalRuntime,
                         int waitingTime, int TA, float WTA)
{
    fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
            currentTime, processId, state, arrivalTime, totalRuntime, 0, waitingTime, TA, WTA);
}


void queue_print(queue_t *my_queue)
{
	if (!my_queue->head) return;

	queue_node_t *temp = my_queue->head;
	while (temp)
	{
        PCB *process = temp->data;
		printf("(%d)", process->file_id);
		temp = temp->next;
        if (temp)
            printf("->");
	}
    printf("\n");
}