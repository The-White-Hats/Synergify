#include "headers.h"
#include <sys/msg.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  initClk();
  // Commented unused variables for now
  // SchedulerConfig* schedulerConfig = getSchedulerConfigInstance();
  // void (*scheduleFunction[])(pqueue_t **, rprocess_t *) = {scheduleHPF,
  // scheduleSRTN, scheduleRR}; int selectedAlgorithmIndex =
  // schedulerConfig->selected_algorithm - 1;
  // TODO implement the scheduler :)
  // upon termination release the clock resources.
  
  int msgq_id = msgget(SHKEY, 0666 | IPC_CREAT);
  msgbuf_t msgbuf;

  while (1)
  {
    while (msgrcv(msgq_id, &msgbuf, sizeof(msgbuf.message), 0, IPC_NOWAIT) != -1)
    {
      printf("A new process arrived at time: %d\n", getClk());
      printf("id: %d\n", msgbuf.message.id);
      printf("arrival time: %d\n", msgbuf.message.arrival);
      printf("runtime: %d\n", msgbuf.message.runtime);
      printf("priority: %d\n", msgbuf.message.priority);
    }
  }

  destroyClk(true);
}
