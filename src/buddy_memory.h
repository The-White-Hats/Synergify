#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_SIZE 1024
#define true 1
#define false 0

typedef short bool;

/**
 * struct buddy_node_s - A structure for holding buddy tree nodes information.
 * @left: pointer to the left child of the node.
 * @right: pointer to the right child of the node.
 * @parent: pointer to the parent of the node.
 * @allocated_memory: void pointer to the memory allocated by the node.
 * @order: the order of the node.
 * @is_free: a bool indicating if the node is occupied or free.
 */
typedef struct buddy_node_s buddy_node_t;
typedef struct buddy_node_s
{
  buddy_node_t *left, *right;
  buddy_node_t *parent;
  void *allocated_memory;
  uint8_t order;
  bool is_free;

} buddy_node_t;

/**
 * struct buddy_tree_s - A structure for holding the buddy_tree information.
 * @root: pointer to the root node of the buddy_tree.
 * @max_memory_size: the max memory size from which we would get the order of the root.
 */
typedef struct buddy_tree_s
{
  buddy_node_t *root;
  int max_memory_size;

} buddy_tree_t;

/**
 * convert_order_to_size - takes an order value and convert it to a size.
 * @order: the order value.
 * @return the integer value of order.
 */
int convert_order_to_size(uint8_t order);

/**
 * convert_size_to_order - takes an integer size and returns the ceil order value.
 * @memory_size: the size value.
 * @return the ceil order.
 */
uint8_t convert_size_to_order(int memory_size);

/**
 * allocate_memory - takes a memory_size, and allocates it using buddy system.
 * @memory_size: the amount of size to allocate.
 * @buddy_tree: the buddy tree from which we would allocate the memory.
 * @return pointer to the buddy block that contains the allocated memory,
 * or to null if it failed.
 */
buddy_node_t *allocate_memory(int memory_size, buddy_tree_t *buddy_tree);

/**
 * free_memory - takes a ptr to a buddy block and free it from the system.
 * @block: pointer to the buddy block.
 * @buddy_tree: pointer to the buddy tree from which we would free the node.
 * @return a boolean indicating if it was freed or not.
 */
bool free_memory(void *block, buddy_tree_t *buddy_tree);

/**
 * print_tree - takes a pointer to the tree and prints the tree.
 * @buddy_tree: pointer to the tree.
 */
void print_tree(buddy_tree_t *buddy_tree);

/**
 * create_buddy_tree - creates the buddy tree, initializes and returns it.
 * @return pointer to the buddy tree.
 */
buddy_tree_t *create_buddy_tree();

/**
 * create_buddy_node - takes an order value and creates and initializes a new node.
 * @order: it's order.
 * @return the created node.
 */
buddy_node_t *create_buddy_node(uint8_t order);

/**
 * insert_buddy_node - takes a node (starting from root) and an order value
 * and recursively insert a new node into the buddy_tree in it's approprate order value.
 * @node: the node from which we would start (the root).
 * @order: the order value.
 * @return a pointer to the allocated buddy_node.
 */
buddy_node_t *insert_buddy_node(buddy_node_t *node, uint8_t order);

/**
 * create_children - takes a node and creates and initilizes its 2 children nodes.
 * @node: pointer to the parent node.
 */
void create_children(buddy_node_t *node);