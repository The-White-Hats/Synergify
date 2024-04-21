#include "headers.h"

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

  destroyClk(true);
}
