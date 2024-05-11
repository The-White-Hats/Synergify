#include <stdio.h>
#include "../ds/priority_queue.h"

typedef struct process_info_s {
  int id;
  int priority;
  int runtime;
} process_info_t;

void printQueue(pqueue_t** head) {
  pqueue_t*temp = *head;
  while(temp) {
    process_info_t* process = (process_info_t*)(temp->process);
    printf("(%d, %d, %d)", process->id, process->priority, process->runtime);
    if (temp->next) printf("->");
    temp = temp->next;
  }
  printf("\n");
}

int main() {
  process_info_t process1;
  process1.id = 20;
  process1.priority = 1;
  process1.runtime = 60;

  process_info_t process2;
  process2.id = 50;
  process2.priority = 2;
  process2.runtime = 70;

  pqueue_t** my_pqueue = malloc(sizeof(pqueue_t*));

  push(my_pqueue, (void*)&process2, process2.priority);
  printQueue(my_pqueue);
  push(my_pqueue, (void*)&process1, process1.priority);
  printQueue(my_pqueue);
  push(my_pqueue, (void*)&process1, process1.priority);
  printQueue(my_pqueue);
  pop(my_pqueue);
  printQueue(my_pqueue);

  process_info_t* front = (process_info_t*)((*my_pqueue)->process);
  printf("\nfront info: %d %d %d\n", front->id, front->priority, front->runtime);

  printf("is empty? %hd\n", isEmpty(my_pqueue));
  pop(my_pqueue);
  printf("is empty? %hd\n", isEmpty(my_pqueue));
  front = (process_info_t*)((*my_pqueue)->process);
  printf("\nfront info: %d %d %d\n", front->id, front->priority, front->runtime);
  printQueue(my_pqueue);
  pop(my_pqueue);
  printf("is empty? %hd\n", isEmpty(my_pqueue));
  //printf("is node deleted? %hd\n", front == NULL);

  return 0;
}