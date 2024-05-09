#pragma once

#include "buddy_memory.h"
#include <math.h>

int convert_order_to_size(uint8_t order)
{
  return pow(2, order);
}

uint8_t convert_size_to_order(int memory_size)
{
  uint8_t order = 0;

  while (pow(2, order) < memory_size)
    order++;

  return order;
}

void *allocate_memory(int memory_size, buddy_tree_t *buddy_tree)
{
  uint8_t order = convert_size_to_order(memory_size);
}

buddy_tree_t *create_buddy_tree()
{
  buddy_tree_t *buddy_tree = (buddy_tree_t *)malloc(sizeof(buddy_tree_t));

  buddy_tree->max_memory_size = MAX_SIZE;
  buddy_tree->root = (buddy_node_t *)malloc(sizeof(buddy_node_t));

  buddy_tree->root->left = NULL;
  buddy_tree->root->right = NULL;
  buddy_tree->root->parent = NULL;
  buddy_tree->root->allocated_memory = NULL;
  buddy_tree->root->order = convert_size_to_order(MAX_SIZE);
  buddy_tree->root->is_free = true;
}