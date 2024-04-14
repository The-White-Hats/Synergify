typedef struct process_info_queue_node_s process_info_queue_node_t;

/**
 * struct process_info_queue_node_s - Node structure for a process information queue
 * @data: Pointer to the process information stored in the node
 * @next: Pointer to the next node in the queue
 *
 * Description: Node structure for a singly linked list representing a queue of process information.
 */
typedef struct process_info_queue_node_s
{
	process_info_t* data;
	process_info_queue_node_t* next;
} process_info_queue_node_t;

/**
 * struct process_info_queue_s - Structure for managing a queue of process information
 * @head: Pointer to the first node in the queue
 * @tail: Pointer to the last node in the queue
 * @size: Number of elements in the queue
 *
 * Description: Structure for managing a queue of process information using a singly linked list implementation.
 */
typedef struct process_info_queue_s
{
	process_info_queue_node_t *head;
	process_info_queue_node_t *tail;
	int size;
} process_info_queue_t;

/**
 * create_queue - Creates a new empty process information queue
 *
 * Return: A pointer to the newly created process information queue, or NULL if memory allocation fails
 */
process_info_queue_t *create_queue()
{
	process_info_queue_t *queue = malloc(sizeof(process_info_queue_t));
	if (!queue)
	{
		exit(-1);
	}

	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;

	return queue;
}

/**
 * enqueue - Enqueues a process into the queue
 * @queue: A pointer to the process information queue
 * @process: A pointer to the process information to be enqueued
 *
 * Return: true if the process is enqueued successfully, otherwise false
 */
bool enqueue(process_info_queue_t *queue, process_info_t *process)
{
	process_info_queue_node_t* node = malloc(sizeof(process_info_queue_node_t));
	if (!node)
	{
		return false;
	}

	node->data = process;
	node->next = NULL;

	if (queue->tail == NULL) {
		queue->head = node;
	} else queue->tail->next = node;
	queue->tail = node;
	queue->size += 1;

	return true;
}

/**
 * dequeue - Dequeues a process from the queue
 * @queue: A pointer to the process information queue
 *
 * Return: A pointer to the dequeued process information, or NULL if the queue is empty
 */
process_info_t *dequeue(process_info_queue_t *queue)
{
	if (!queue->head) return NULL;
	process_info_t* process = queue->head->data;
	process_info_queue_node_t* next = queue->head->next;

	free(queue->head);
	queue->head = next;
	queue->size -= 1;
	return process;
}

/**
 * front - Retrieves the process at the front of the queue without dequeuing it
 * @queue: A pointer to the process information queue
 *
 * Return: A pointer to the process information at the front of the queue, or NULL if the queue is empty
 */
const process_info_t* front(process_info_queue_t* queue)
{
	if (!queue->head) return NULL;

	return (const process_info_t*)(queue->head->data);
}

/**
 * is_queue_empty - Checks if the queue is empty
 * @queue: A pointer to the process information queue
 *
 * Return: true if the queue is empty, otherwise false
 */
bool is_queue_empty(process_info_queue_t *queue)
{
	return queue->size == 0;
}
