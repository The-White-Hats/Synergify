#include "../headers.h"
#include "../ds/process_info_queue.h"

int main() {
  process_info_t process1;
  process1.id = 20;
  process1.priority = 1;
  process1.runtime = 60;

  process_info_t process2;
  process2.id = 50;
  process2.priority = 2;
  process2.runtime = 70;

  process_info_queue_t* queue = create_queue();

  enqueue(queue, &process1);
  enqueue(queue, &process2);

  const process_info_t* front_copy = front(queue);
  printf("\nfront info: %d %d %d\n", front_copy->id, front_copy->priority, front_copy->runtime);

  printf("is empty? %hd\n", is_queue_empty(queue));

  process_info_t* pro = dequeue(queue);
  printf("is empty? %hd\n", is_queue_empty(queue));
  pro = dequeue(queue);
  printf("is empty? %hd\n", is_queue_empty(queue));
  printf("\npro info: %d %d %d\n", pro->id, pro->priority, pro->runtime);

  return 0;
}