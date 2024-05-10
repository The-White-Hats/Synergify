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

buddy_node_t *allocate_memory(int memory_size, buddy_tree_t *buddy_tree)
{
  uint8_t order = convert_size_to_order(memory_size);

  if (memory_size > buddy_tree->max_memory_size || order < 0)
  {
    return NULL;
  }

  buddy_node_t *node = insert_buddy_node(buddy_tree->root, order);
}

buddy_tree_t *create_buddy_tree()
{
  buddy_tree_t *buddy_tree = (buddy_tree_t *)malloc(sizeof(buddy_tree_t));

  buddy_tree->max_memory_size = MAX_SIZE;
  buddy_tree->root = create_buddy_node(convert_size_to_order(MAX_SIZE));
}

buddy_node_t *create_buddy_node(uint8_t order)
{
  buddy_node_t *node = (buddy_node_t *)malloc(sizeof(buddy_node_t));

  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->allocated_memory = NULL;
  node->order = order;
  node->is_free = true;

  return node;
}

void create_children(buddy_node_t *node)
{
  node->left = create_buddy_node(node->order - 1);
  node->right = create_buddy_node(node->order - 1);
  node->left->parent = node;
  node->right->parent = node;
  node->is_free = false;
}

buddy_node_t *insert_buddy_node(buddy_node_t *node, uint8_t order)
{
  if (!node)
    return NULL;

  if (node->order == order)
  {
    if (node->is_free)
      return node;
    else
      return NULL;
  }

  if (!node->left && !node->right)
  {
    if (!node->is_free) // Occupied node
      return NULL;

    create_children(node);
  }

  buddy_node_t *target_node = NULL;

  target_node = insert_buddy_node(node->left, order);
  if (target_node)
    return target_node;

  target_node = insert_buddy_node(node->right, order);
  return target_node;
}