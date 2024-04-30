#include "fib_heap.h"

/**
 * node_init - function to initialize a new heap node
 * @param key: the key of the new node
 * @param element: the element to be stored in the new node
 *
 * @return a pointer to the newly initialized heap node
 */
static heap_node_t *node_init(int key, void *element)
{
  heap_node_t *node = malloc(sizeof(heap_node_t));
  if (!node)
  {
    perror("Heap Node: Memory allocation failed");
    exit(EXIT_FAILURE);
  }
  node->key = key;
  node->element = element;
  node->parent = NULL;
  node->child = NULL;
  node->left = node->right = node;
  node->degree = 0;
  node->marked = 0;
  return node;
}

/**
 * node_free - function to free memory allocated for a heap node
 * @param node: the heap node to free
 * @param free_elem: determine if element is freed too.
 */
static void node_free(heap_node_t *const node, short free_elem)
{
  if (node->element != NULL && free_elem)
  {
    free(node->element);
  }
  free(node);
}

/**
 * node_add - function to add a node to the right of another node
 * @param old: the node to the left of which to add the new node
 * @param toAdd: the node to add
 */
static void node_add(heap_node_t *const old, heap_node_t *const toAdd)
{
  heap_node_t *old_right = old->right;
  if (toAdd == old || toAdd == old_right)
  {
    perror("Adding the same node to itself");
    exit(EXIT_FAILURE);
  }
  toAdd->right = old_right;
  old_right->left = toAdd;
  toAdd->left = old;
  old->right = toAdd;
}

/**
 * node_remove - function to remove a node from a list
 * @param node: the node to remove
 */
static void node_remove(heap_node_t *const node)
{
  heap_node_t *node_right = node->right;

  node->left->right = node_right;
  node_right->left = node->left;
}

/**
 * fib_heap_alloc - function to allocate memory for a new Fibonacci heap
 *
 * @return a pointer to the newly allocated Fibonacci heap
 */
fib_heap_t *fib_heap_alloc()
{
  fib_heap_t *heap = malloc(sizeof(fib_heap_t));
  if (!heap)
  {
    perror("Heap: Memory allocation failed");
    exit(EXIT_FAILURE);
  }

  heap->min = NULL;
  heap->num_of_nodes = 0;

  return heap;
}

/**
 * fib_heap_add - function to add a node to a Fibonacci heap
 * @param heap: the Fibonacci heap
 * @param node: the node to add
 *
 * @return 1 if node is inserted, 0 otherwise
 */
static short fib_heap_add(fib_heap_t *heap, heap_node_t *node)
{
  heap_node_t *old_node;

  if (!heap)
    return 0;

  node->parent = NULL;
  node->marked = 0;

  old_node = heap->min;
  if (old_node)
  {
    node_add(old_node, node);
  }
  if (!heap->min || heap->min->key > node->key)
    heap->min = node;

  return 1;
}

/**
 * fib_heap_insert - function to insert a new element into a Fibonacci heap
 * @param heap: the Fibonacci heap
 * @param element: the element to insert
 * @param key: the key associated with the element
 *
 * @return 1 if node is inserted, 0 otherwise
 */
short fib_heap_insert(fib_heap_t *heap, void *element, int key)
{
  heap_node_t *node;

  if (!heap)
    return 0;
  node = node_init(key, element);
  if (fib_heap_add(heap, node))
  {
    heap->num_of_nodes++;
    return 1;
  }
  return 0;
}

/**
 * fib_heap_union - function to union two Fibonacci heaps
 * @param node_1: the first heap node
 * @param node_2: the second heap node
 *
 * @return a pointer to the root of the resulting heap
 */
static heap_node_t *fib_heap_union(heap_node_t *node_1, heap_node_t *node_2)
{
  heap_node_t *temp;
  if (node_1->key > node_2->key)
  {
    temp = node_1;
    node_1 = node_2;
    node_2 = temp;
  }

  node_remove(node_2);

  node_2->parent = node_1;
  if (!node_1->child)
  {
    node_1->child = node_2;
    node_2->right = node_2;
    node_2->left = node_2;
  }
  else
  {
    node_add(node_1->child, node_2);
  }

  node_1->degree++;
  node_2->marked = 0;

  return node_1;
}

/**
 * fib_heap_consolidate - function to consolidate a Fibonacci heap
 * @param heap: the Fibonacci heap to consolidate
 */
static void fib_heap_consolidate(fib_heap_t *heap)
{
  const size_t ARRAY_SIZE = log2(heap->num_of_nodes) + 1;
  heap_node_t **root_list = malloc(sizeof(heap_node_t *) * ARRAY_SIZE);
  size_t num_of_roots = 0, degree;
  heap_node_t *temp, *occupying_degree, *next;
  memset(root_list, 0, sizeof(heap_node_t *) * ARRAY_SIZE);

  temp = heap->min;
  if (temp)
  {
    do
    {
      num_of_roots++;
      temp = temp->right;
    } while (temp != heap->min);
  }

  while (num_of_roots > 0)
  {
    degree = temp->degree;
    next = temp->right;

    while (1)
    {
      occupying_degree = root_list[degree];
      if (!occupying_degree)
        break;

      temp = fib_heap_union(occupying_degree, temp); // merge the two trees

      root_list[degree] = NULL;
      degree++;
    }

    root_list[degree] = temp;
    temp = next;
    num_of_roots--;
  }

  heap->min = NULL;
  for (size_t i = 0; i < ARRAY_SIZE; i++)
  {
    occupying_degree = root_list[i];
    if (!occupying_degree)
      continue;
    if (!heap->min)
    {
      heap->min = occupying_degree;
    }
    else
    {
      node_remove(occupying_degree);
      node_add(heap->min, occupying_degree);

      if (heap->min->key > occupying_degree->key)
      {
        heap->min = occupying_degree;
      }
    }
  }
}

/**
 * fib_heap_extract_min - function to extract the minimum element from a Fibonacci heap
 * @param heap: the Fibonacci heap
 *
 * @return a pointer to the extracted minimum element
 */
void *fib_heap_extract_min(fib_heap_t *heap)
{
  heap_node_t *min, *child, *sibling, *node_to_free;
  size_t num_of_children;
  void *element;

  if (!heap)
  {
    perror("Heap isn't allocated");
    exit(EXIT_FAILURE);
  }

  min = heap->min;
  if (!min)
    return NULL;

  element = min->element;
  num_of_children = min->degree;
  child = min->child;
  while (num_of_children > 0)
  {
    sibling = child->right;

    node_remove(child);

    fib_heap_add(heap, child); // Lazy insert child as a separate tree.

    child = sibling;
    num_of_children--;
  }

  node_remove(min);
  node_to_free = min;
  if (min == min->right)
  {
    heap->min = NULL;
  }
  else
  {
    heap->min = min->right;
    fib_heap_consolidate(heap);
  }
  heap->num_of_nodes--;
  free(node_to_free);
  return element;
}

/**
 * fib_heap_decrease_min_key - function to decrease the key of the minimum element in a Fibonacci heap
 * @param heap: the Fibonacci heap
 * @param new_key: the new key value
 */
void fib_heap_decrease_min_key(fib_heap_t *heap, int new_key)
{
  if (!heap)
  {
    perror("Heap isn't allocated");
    exit(EXIT_FAILURE);
  }

  if (!heap->min)
    return;
  heap->min->key = new_key;
}

/**
 * fib_heap_min - function to get the minimum element of a Fibonacci heap
 * @param heap: the Fibonacci heap
 *
 * @return the minimum element
 */
void *fib_heap_min(fib_heap_t *heap)
{
  if (!heap)
  {
    perror("Heap isn't allocated");
    exit(EXIT_FAILURE);
  }

  if (!heap->min)
    return NULL;

  return heap->min->element;
}

/**
 * fib_heap_size - function to get the number of nodes in a Fibonacci heap
 * @param heap: the Fibonacci heap
 *
 * @return the number of nodes in the heap, or -1 if the heap is not allocated
 */
int fib_heap_size(fib_heap_t *heap)
{
  return heap ? heap->num_of_nodes : -1;
}

/**
 * fib_heap_is_tree_healthy - function to check if a subtree of a Fibonacci heap is healthy
 * @param node: the root of the subtree
 *
 * @return 1 if the subtree is healthy, 0 otherwise
 */
static short fib_heap_is_tree_healthy(heap_node_t *node)
{
  heap_node_t *current;
  if (!node)
    return 1;
  current = node;

  do
  {
    if (node->key > current->key)
      return 0;

    if (!fib_heap_is_tree_healthy(current->child))
      return 0;

    current = current->right;

  } while (current != node);

  return 1;
}

/**
 * fib_heap_is_healthy - function to check if a Fibonacci heap is healthy
 * @param heap: the Fibonacci heap
 *
 * @return 1 if the heap is healthy, 0 otherwise
 */
short fib_heap_is_healthy(fib_heap_t *heap)
{
  heap_node_t *root;

  if (!heap)
    return 0;

  if (!heap->min)
    return 1;

  // check if the min node points to the node with the lowest key root list
  root = heap->min;
  do
  {
    if (heap->min->key > root->key)
      return 0;
    root = root->right;
  } while (root != heap->min);

  // check that all trees are min-heap ordered.
  root = heap->min;
  do
  {
    if (!fib_heap_is_tree_healthy(root->child))
      return 0;

    root = root->right;
  } while (root != heap->min);

  return 1;
}

/**
 * fib_heap_copy_tree - function to recursively copy a tree of nodes in a Fibonacci heap
 * @param node: the root of the tree to copy
 * @param dist: the destination Fibonacci heap
 * @param keyExtractor: function pointer to extract keys from elements
 */
void fib_heap_copy_tree(heap_node_t *node, fib_heap_t *dist, int (*keyExtractor)(void *))
{
  heap_node_t *current;
  int key;

  if (!node)
    return;
  current = node;

  do
  {
    key = keyExtractor(current->element);
    fib_heap_insert(dist, current->element, key);

    fib_heap_copy_tree(current->child, dist, keyExtractor);

    current = current->right;
  } while (current != node);
}

/**
 * fib_heap_copy - function to copy a Fibonacci heap
 * @param heap: the Fibonacci heap to copy from
 * @param dist: the destination Fibonacci heap
 * @param keyExtractor: function pointer to extract keys from elements
 */
void fib_heap_copy(fib_heap_t *heap, fib_heap_t *dist, int (*keyExtractor)(void *))
{
  heap_node_t *root;
  int key;

  if (!heap)
    return;

  if (!heap->min)
    return;

  // copy all trees nodes.
  root = heap->min;
  do
  {
    key = keyExtractor(root->element);
    fib_heap_insert(dist, root->element, key);
    root = root->right;
  } while (root != heap->min);
}

/**
 * fib_heap_node_free - function to recursively free memory allocated for a heap node and its children
 * @param node: the root of the subtree to free
 * @param free_elem: determine if element is freed too.
 */
static void fib_heap_node_free(heap_node_t *node, short free_elem)
{
  heap_node_t *child, *sibling, *first_child;

  first_child = node->child;
  if (!first_child)
  {
    node_free(node, free_elem);
    return;
  }

  child = first_child->right;

  while (child != first_child)
  {
    sibling = child->right;
    fib_heap_node_free(child, free_elem);
    child = sibling;
  }

  fib_heap_node_free(child, free_elem);

  node_free(node, free_elem);
}

/**
 * fib_heap_free - function to free memory allocated for a Fibonacci heap
 * @param heap: the Fibonacci heap to free
 * @param free_elem: determine if element is freed too.
 */
void fib_heap_free(fib_heap_t *heap, short free_elem)
{
  heap_node_t *root, *node, *sibling;

  if (!heap)
    return;

  root = heap->min;
  if (!root)
  {
    free(heap);
    return;
  }

  node = root->right;

  while (node != root)
  {
    sibling = node->right;
    fib_heap_node_free(node, free_elem);
    node = sibling;
  }

  fib_heap_node_free(node, free_elem);

  free(heap);
  return;
}