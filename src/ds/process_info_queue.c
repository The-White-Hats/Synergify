typedef struct process_info_queue_node_s process_info_queue_node_t;

typedef struct process_info_queue_node_s
{
	process_info_t* data;
	process_info_queue_node_t* next;
} process_info_queue_node_t;

typedef struct process_info_queue_s
{
	process_info_queue_node_t *head;
	process_info_queue_node_t *tail;
	int size;
} process_info_queue_t;

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

const process_info_t* front(process_info_queue_t* queue)
{
	if (!queue->head) return NULL;

	return (const process_info_t*)(queue->head->data);
}

bool is_queue_empty(process_info_queue_t *queue)
{
	return queue->size == 0;
}