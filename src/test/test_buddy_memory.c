#include "../buddy_memory.h"

int main()
{
  buddy_tree_t *buddy_system_tree = create_buddy_tree();

  buddy_node_t *node_one = allocate_memory(1024, buddy_system_tree);
  buddy_node_t *node_two = allocate_memory(27, buddy_system_tree);
  buddy_node_t *node_three = allocate_memory(152, buddy_system_tree);
  buddy_node_t *node_four = allocate_memory(66, buddy_system_tree);

  // allocate_memory(11, buddy_system_tree);
  // allocate_memory(20, buddy_system_tree);
  // allocate_memory(1, buddy_system_tree);
  // allocate_memory(2, buddy_system_tree);
  // allocate_memory(5, buddy_system_tree);
  // allocate_memory(100, buddy_system_tree);
  // allocate_memory(35, buddy_system_tree);

  print_tree(buddy_system_tree->root, 0);

  free_memory(node_one);
  free_memory(node_two);
  free_memory(node_three);
  free_memory(node_four);

  node_one = allocate_memory(256, buddy_system_tree);
  node_two = allocate_memory(256, buddy_system_tree);
  node_three = allocate_memory(256, buddy_system_tree);
  node_four = allocate_memory(256, buddy_system_tree);
  allocate_memory(256, buddy_system_tree);
  if (!node_four)
  {
    printf("returned NULL\n");
  }

  print_tree(buddy_system_tree->root, 0);
  free_memory(node_one);
  free_memory(node_two);
  free_memory(node_three);
  free_memory(node_four);

  print_tree(buddy_system_tree->root, 0);
  printf("----------------------------\n");
  node_two = allocate_memory(27, buddy_system_tree);
  node_three = allocate_memory(152, buddy_system_tree);
  node_four = allocate_memory(66, buddy_system_tree);
  print_tree(buddy_system_tree->root, 0);

  free_memory(node_two);
  free_memory(node_three);
  free_memory(node_four);

  free(buddy_system_tree->root);
  free(buddy_system_tree);
}