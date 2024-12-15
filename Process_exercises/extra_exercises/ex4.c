/**
 * Use SIGCHLD to prevent zombie state
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

// Signal handler for SIGCHLD
void handle_sigchld(int sig) {
  /**
   * -1: Wait for any child process to terminate.
   * 
   * WNOHANG: This makes the call non-blocking. If no child processes have 
   * terminated yet, waitpid() returns immediately with a return value of 0, 
   * meaning the parent can continue executing without waiting.
   */

  while (waitpid(-1, NULL, WNOHANG) > 0) {
    printf("Child process reaped\n");
  }
}

int main(int argc, char **argv) {
  pid_t p;

  // Set up SIGCHLD handler
  signal(SIGCHLD, handle_sigchld);

  p = fork();
  if (p == 0) {
    // Child process
    printf("Child process (PID: %d) running...\n", getpid());
    sleep(2); // Simulate work
    printf("Child process (PID: %d) exiting...\n", getpid());
    exit(0);
  } else {
    // Parent process continues running
    printf("Parent process (PID: %d) is running...\n", getpid());
    sleep(5); // Simulate parent work for 5s then exit
    /**
     * If SIGCHLD was receive, it wake parent from sleep state, then the parent
     * will exit early, right after receiving the signal and hanlding. 
     */
    printf("Parent process exiting...\n");

  }
	return 0;
}
	
