#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

int main() {
  // Create a new current_mask to store the current signal set
  sigset_t current_mask;
  char sig_name[10] = {0};

  /**
   * Block the Interrupt signal (SIGINT)
   */
  // Empty the sig_set
  sigemptyset(&current_mask);
  // Add SIGINT to sig_set
  sigaddset(&current_mask, SIGINT);
  // Block SIGINT
  if (sigprocmask(SIG_BLOCK, &current_mask, NULL) == -1) {
    perror("Failed to block SIGINT");
    return 1;
  }

  /**
   * Now, the blocked signal will be only Interrupt signal (SIGINT)
   */
  printf("- - - Active signal - - - \n");
  // Iterate over all signals (from 1 to NSIG)
  for (int i = 1; i <= NSIG; i++) {
    if (sigismember(&current_mask, i) == 0) {
      psignal(i, sig_name);
    }
  }

  // Show blocked signal
  printf("- - - Blocked signal - - - \n");
  for (int i = 1; i <= NSIG; i++) {
    if (sigismember(&current_mask, i) == 1) {
      psignal(i, sig_name);
    }
  }

  return 0;
}
