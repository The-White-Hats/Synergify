#include "buddy_memory.h"
#include "./ds/queue.h"

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

  buddy_node_t **target = (buddy_node_t **)malloc(sizeof(buddy_node_t *));
  (*target) = NULL;

  insert_buddy_node(buddy_tree->root, target, order);

  if (!(*target))
  {
    free(target);
    return NULL;
  }

  check_and_split(target, order);

  buddy_node_t *ptr = (*target);
  free(target);

  ptr->is_free = false;
  ptr->allocated_memory = malloc(pow(2, ptr->order));

  return ptr;
}

void check_and_split(buddy_node_t **target, uint8_t order)
{
  if (!target || !(*target))
    return;

  while ((*target) && (*target)->order > order)
  {
    create_children((*target), (*target)->i, (*target)->j);
    (*target) = (*target)->left;
  }
}

void free_memory(buddy_node_t *block)
{
  if (!block)
    return;

  if (!block->parent) // block is the root
  {
    // free the allocated memory only not the block and return
    free(block->allocated_memory);
    block->allocated_memory = NULL;
    block->is_free = true;
    return;
  }

  buddy_node_t *parent = block->parent;
  buddy_node_t *block_to_free = NULL;

  if (block == parent->left)
    block_to_free = parent->left;
  else
    block_to_free = parent->right;

  free(block_to_free->allocated_memory);
  block_to_free->allocated_memory = NULL;
  block_to_free->is_free = true;

  while (parent)
  {
    // If both children are free, merge them.
    if (parent->left->is_free && parent->right->is_free)
    {
      free(parent->left);
      free(parent->right);
      parent->left = NULL;
      parent->right = NULL;
      parent->is_free = true;
      parent = parent->parent;
    }
    else
      break;
  }
}

buddy_tree_t *create_buddy_tree()
{
  buddy_tree_t *buddy_tree = (buddy_tree_t *)malloc(sizeof(buddy_tree_t));

  buddy_tree->max_memory_size = MAX_SIZE;
  buddy_tree->root = create_buddy_node(convert_size_to_order(MAX_SIZE), 0, MAX_SIZE - 1);

  return buddy_tree;
}

buddy_node_t *create_buddy_node(uint8_t order, int i, int j)
{
  buddy_node_t *node = (buddy_node_t *)malloc(sizeof(buddy_node_t));

  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->allocated_memory = NULL;
  node->order = order;
  node->is_free = true;
  node->i = i;
  node->j = j;

  return node;
}

void create_children(buddy_node_t *node, int i, int j)
{
  node->left = create_buddy_node(node->order - 1, i, (j + i) / 2);

  node->right = create_buddy_node(node->order - 1, (j + i) / 2 + 1, j);

  node->left->parent = node;
  node->right->parent = node;
  node->is_free = false;
}

void insert_buddy_node(buddy_node_t *node, buddy_node_t **target, uint8_t order)
{
  if (!node)
    return;

  if (node->order >= order && node->is_free)
  {
    if (!(*target) || (*target)->order > node->order)
      (*target) = node;
  }

  insert_buddy_node(node->left, target, order);
  insert_buddy_node(node->right, target, order);
}

void print_tree(buddy_node_t *root, int level)
{
  if (root == NULL)
    return;
  for (int i = 0; i < level; i++)
    printf(i == level - 1 ? "|-" : "  ");
  printf("%d", root->order);
  printf(!root->allocated_memory ? "fr\n" : "oc\n");
  print_tree(root->left, level + 1);
  print_tree(root->right, level + 1);
}

static void buddy_tree_free(buddy_node_t *root, bool deleteData)
{
  if (root == NULL)
    return;
  buddy_tree_free(root->left, deleteData);
  buddy_tree_free(root->right, deleteData);

  free(root->allocated_memory);
  free(root);
}

/**
 * buddy_free - Frees all memory allocated for the queue elements
 * @param buddy_tree: A pointer to the buddy tree
 * @param deleteData: Determine if the data is freed or not
 */
void buddy_free(buddy_tree_t *buddy_tree, bool deleteData)
{
  if (buddy_tree == NULL)
    return;
  buddy_tree_free(buddy_tree->root, deleteData);
  free(buddy_tree);
}