#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * struct heap_node_s - Structure representing a node in a Fibonacci heap
 * @element: the element stored in the node
 * @key: the key associated with the element
 * @parent: pointer to the parent node
 * @child: pointer to the leftmost child node
 * @left: pointer to the left sibling node
 * @right: pointer to the right sibling node
 * @degree: number of children of the node
 * @marked: flag indicating whether the node has lost a child since it became a child of another node
 */
typedef struct heap_node_s {
  void *element;
  int key;
  struct heap_node_s *parent;
  struct heap_node_s *child;
  struct heap_node_s *left;
  struct heap_node_s *right;
  size_t degree;
  short marked;
} heap_node_t;

/**
 * struct fib_heap_s - Structure representing a Fibonacci heap
 * @num_of_nodes: the number of nodes in the heap
 * @min: pointer to the node with the minimum key in the heap
 */
typedef struct fib_heap_s {
  size_t num_of_nodes;
  heap_node_t *min;
} fib_heap_t;

/**
 * fib_heap_alloc - function to allocate memory for a new Fibonacci heap
 *
 * @return a pointer to the newly allocated Fibonacci heap
 */
fib_heap_t *fib_heap_alloc();

/**
 * fib_heap_insert - function to insert a new element into a Fibonacci heap
 * @param heap: the Fibonacci heap
 * @param element: the element to insert
 * @param key: the key associated with the element
 * 
 * @return 1 if node is inserted, 0 otherwise
 */
short fib_heap_insert(fib_heap_t *heap, void *element, int key);

/**
 * fib_heap_extract_min - function to extract the minimum element from a Fibonacci heap
 * @param heap: the Fibonacci heap
 *
 * @return the extracted minimum element
 */
void *fib_heap_extract_min(fib_heap_t *heap);

/**
 * fib_heap_decrease_min_key - function to decrease the key of the minimum element in a Fibonacci heap
 * @param heap: the Fibonacci heap
 * @param new_key: the new key value
 */
void fib_heap_decrease_min_key(fib_heap_t *heap, int new_key);

/**
 * fib_heap_min - function to get the minimum element of a Fibonacci heap
 * @param heap: the Fibonacci heap
 *
 * @return a pointer to the minimum element
 */
void* fib_heap_min(fib_heap_t* heap);

/**
 * fib_heap_size - function to get the number of nodes in a Fibonacci heap
 * @param heap: the Fibonacci heap
 *
 * @return the number of nodes in the heap, or -1 if the heap is not allocated
 */
int fib_heap_size(fib_heap_t *heap);

/**
 * fib_heap_is_healthy - function to check if a Fibonacci heap is healthy
 * @param heap: the Fibonacci heap
 *
 * @return 1 if the heap is healthy, 0 otherwise
 */
short fib_heap_is_healthy(fib_heap_t *heap);

/**
 * fib_heap_free - function to free memory allocated for a Fibonacci heap
 * @param heap: the Fibonacci heap to free
 */
void fib_heap_free(fib_heap_t *heap);