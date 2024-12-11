/**
 * Write a program that create a child process, then both parent and child will 
 * write to the file. Then child process will be changed to the new process by
 * using execl(). The child process now will print the content of the file that 
 * written before (open_file).
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void write_string_to_file(int pid, int fd, char string[]) {
  ssize_t byte_written;
  for (int i = 0; i < strlen(string); i++) {
    byte_written = write(fd, &string[i], 1);
    if (byte_written != 1) {
      if (pid == 0) {
        printf("Parent write failed!\n");
      } else {
        printf("Child write failed!\n");
      }
      return;
    }
  }
}

int main(int argc, char **argv) {
  char parent_string[] = "Parent";
  char child_string[] = "Child";

  pid_t pid = getpid();
  int p = 0;
	if (argc <= 1) {
		printf("Missing arguments !\n");
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments !\n");
		return -1;
	}

  printf("Parent PID: %d\n", pid);

  //Open file
	int fd = open(argv[1], O_WRONLY | O_TRUNC);
  if (fd == -1) {
		printf("Open failed !\n");
  	return -1;
 	}

  p = fork();
  if (p == 0) {
    write_string_to_file(p, fd, child_string);
    printf("I am child, I will switch to open a file\n");
    if (execl("open_file", "open_file", argv[1], NULL) == -1) {
      printf("Switch task failed!\n");
    }
  } else {
    printf("I am parent!\n");
    write_string_to_file(p, fd, parent_string);
  }

  close(fd);
	return 0;
}
	
