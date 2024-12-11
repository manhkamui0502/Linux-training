#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	if (argc <= 1) {
		printf("Missing arguments !\n");
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments !\n");
		return -1;
	}
	
	int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
		printf("Open failed !\n");
  	return -1;
 	}
   	
	char c;
  while (read(fd, &c, 1) > 0) {
    printf("%c", c);
  }

  close(fd);
	return 0;
}
	
