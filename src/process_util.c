#include "header.h"

/**
 * decrementRemainingCPUTime - Decrement the remaining CPU time of a process.
 * @param process: Pointer to the process information structure.
 */
void decrementRemainingCPUTime(process_info_t* process) {
  process->runtime--;
}

// Calculate other data useful for logging