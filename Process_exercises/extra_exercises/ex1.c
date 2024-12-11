/**
 * Write a program call A that creates child process called B 
 * then print its PID and PPID.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
  int p = 0;
  pid_t pid = getpid();
  printf("Parent PID: %d\n", pid);

  p = fork();
  if (p == 0) {
    printf("* Child * I am child!\n");
    int child_pid = getpid();
    int child_ppid = getppid();
    printf("* Child * pid = : %d, parent pid: = %d\n", child_pid, child_ppid);
  } else {
    printf("* Parent * I am parent!\n");
    printf("* Parent * pid = : %d\n", pid);
  }

	return 0;
}
	
