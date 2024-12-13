#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  // Create a signal set to store the blocked signals
  sigset_t block_set, pending_set;

  // Initialize the signal set
  sigemptyset(&block_set);
  // Add SIGINT to the set
  sigaddset(&block_set, SIGINT);
  // Block SIGINT
  if (sigprocmask(SIG_BLOCK, &block_set, NULL) == -1) {
    perror("Failed to block SIGINT");
    return 1;
  }

  printf("SIGINT is now blocked. Press Ctrl+C to send SIGINT.\n");

  while (1) {
    // Check the pending signals
    /**
     * sigpending() returns the set of signals that are pending for
       delivery to the calling thread (i.e., the signals which have been
       raised while blocked).  The mask of pending signals is returned
       in set.
     */
    if (sigpending(&pending_set) == -1) {
      perror("Failed to retrieve pending signals");
      return 1;
    }

    // Check if SIGINT is pending
    if (sigismember(&pending_set, SIGINT)) {
      printf("Interrupt signal (SIGINT) is pending!\n");
      printf("Sleeping for 3 seconds before exiting...\n");
      sleep(3);
      return 0;
    }

    printf("Waiting for SIGINT...\n");
    sleep(1); // Slower
  }

  return 0;
}
