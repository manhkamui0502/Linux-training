/**
 * Write a program call A that creates child process called B 
 * then print its PID and PPID.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
  int p = 0;
  pid_t pid = getpid();
  printf("Parent PID: %d\n", pid);

  p = fork();
  if (p == 0) { // Child process
    printf("* Child * I am child!\n");
    int child_pid = getpid();
    int child_ppid = getppid();
    printf("* Child * pid = : %d, parent pid: = %d\n", child_pid, child_ppid);

    printf("* Child * Exiting after 2 seconds with status 42\n");
    sleep(2); // Sleep for 2 seconds
    _exit(42); // Exit with status 42

  } else {
    printf("* Parent * I am parent!\n");
    printf("* Parent * pid = : %d\n", pid);
    int status;
    pid_t child_pid = waitpid(p, &status, 0); // Wait for the child process to finish

    if (WIFEXITED(status)) {
      printf("* Parent * Child exited with status: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("* Parent * Child terminated by signal: %d\n", WTERMSIG(status));
    } else {
      printf("* Parent * Child terminated abnormally.\n");
    }
  }

	return 0;
}
	
