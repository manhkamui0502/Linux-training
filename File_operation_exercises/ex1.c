#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/falloc.h>


#define BUFFER_LEN 4096

// cp src dest
// Function to copy with holes
int copy_with_holes(char *src, char *dest) {
	ssize_t byte_read, byte_written;
	char buffer[BUFFER_LEN];

	// Open src file
	int src_fd = open(src, O_RDONLY);
  if (src_fd == -1) {
		printf("Open source file failed!\n");
  	return 1;
 	}

	// Open destination file
	// O_WRONLY: Open for writing only.
	// O_CREAT: Create the file if it doesn't exist.
	// O_TRUNC: Erase the contents of the file if it exists.
	int dst_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (dst_fd == -1) {
		printf("Open destination file failed!\n");
		close(dst_fd);
  	return 1;
 	}
	
	// Read 4096 byte at the same time and store in buffer
	while ((byte_read = read(src_fd, buffer, BUFFER_LEN)) > 0) {
		if (byte_read > 0) {
			for (ssize_t i = 0; i < byte_read; i++) {
				if (buffer[i] != '\0') { // If character is not a null terminator
					byte_written = write(dst_fd, &buffer[i], 1);
					if (byte_written != 1) {
						printf("Write failed!\n");
						return 1;
					}
				} else { 
					// Create a hole in the dest file at this index i

					// Get current destination file offset
					off_t current_pos = lseek(dst_fd, 0, SEEK_CUR);
					
					if (current_pos == -1) {
						printf("Getting current position in target failed!\n");
						close(src_fd);
						close(dst_fd);
						return 1;
					}
					
					// Try to allocating blocks and marking them as uninitialized
					if (fallocate(dst_fd, FALLOC_FL_KEEP_SIZE, current_pos, 1) == -1) {
						printf("Creating hole in target failed!\n");
						close(src_fd);
						close(dst_fd);
						return -1;
					}
				}
			} 
		}
	}

	close(src_fd);
	close(dst_fd);
	return 0;
}


int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Missing arguments!\n");
		return 1;
	} else if (argc > 3) {
		printf("Too many arguments!\n");
		return 1;
	}

	if (copy_with_holes(argv[1], argv[2]) != 0) {
		printf("Operation failed! \n");
	}

	return 0;
}
