#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

typedef struct queue_node_s queue_node_t;


typedef struct queue_node_s
{
	void* data;
	queue_node_t* next;
} queue_node_t;

typedef struct queue_s
{
	queue_node_t *head;
	queue_node_t *tail;
	int size;
} queue;

queue *create_queue()
{
	queue *my_queue = malloc(sizeof(queue));
	if (!my_queue)
	{
		exit(-1);
	}

	my_queue->head = NULL;
	my_queue->tail = NULL;
	my_queue->size = 0;

	return my_queue;
}

bool enqueue(queue *my_queue, void *data)
{
	queue_node_t* node = malloc(sizeof(queue_node_t));
	if (!node)
	{
		return false;
	}

	node->data = data;
	node->next = NULL;

	if (my_queue->tail == NULL) {
		my_queue->head = node;
	} else my_queue->tail->next = node;
	my_queue->tail = node;
	my_queue->size += 1;

	return true;
}

void *dequeue(queue *my_queue)
{
	if (!my_queue->head) return NULL;
	void* data = my_queue->head->data;
	queue_node_t* next = my_queue->head->next;

	free(my_queue->head);
	my_queue->head = next;
	my_queue->size -= 1;
	return data;
}

const void* front(queue* my_queue)
{
	if (!my_queue->head) return NULL;

	return (const void*)(my_queue->head->data);
}

bool is_queue_empty(queue *my_queue)
{
	return my_queue->size == 0;
}