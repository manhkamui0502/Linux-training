#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int fd;
  char buffer[64];
  int x = 0, y = 0;

  // Open the mouse device file
  fd = open("/dev/input/mice", O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Unable to open /dev/input/mice. Use sudo.\n");
    return 1;
  }

  while (1) {
    int bytes = read(fd, buffer, sizeof(buffer));
    if (bytes > 0) {
      x += buffer[1];
      y += buffer[2];
      printf("Mouse coordinates: (%d, %d)\n", x, y);
    } else {
      fprintf(stderr, "Error reading mouse input.\n");
      break;
    }

    usleep(100000); // Delay for 100ms
  }

  // Close the mouse device file
  close(fd);

  return 0;
}

