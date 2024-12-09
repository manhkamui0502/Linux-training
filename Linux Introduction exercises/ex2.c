#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int fd;
  char c;

  // Open the file in read mode
  fd = open("note.txt", O_RDONLY);
  if (fd == -1) {
      printf("Open failed!\n");
      return 1;
  }

  // Read and print file content
  while (read(fd, &c, 1) > 0) {
      printf("%c", c);
  }

  // Close file
  close(fd);
  return 0;
}

