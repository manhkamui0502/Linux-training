/**
 * Task 2: Write a program to compare between two mapping methods in POSIX 
 * shared memory (Private file mapping and Shared file mapping)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define SHM_NAME "/shm_compare"  // Name of the shared memory object
#define FILE_SIZE 512            // Size of the shared memory object

// Function to write content to an output file
void write_output(const char *output_file, const char *content) {
  // Open file for writing
  int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);  
  if (fd == -1) {
    perror("Error opening output file");
    exit(EXIT_FAILURE);
  }
  // Write content to the file
  if (write(fd, content, strlen(content)) == -1) {  
    perror("Error writing to output file");
    close(fd);
    exit(EXIT_FAILURE);
  }
  close(fd);
}

// Demonstrates private memory mapping
void private_mapping_example(int shm_fd, const char *child_output, const char *parent_output) {
  // Create a memory mapping with MAP_PRIVATE
  char *mapped_mem = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, shm_fd, 0);
  if (mapped_mem == MAP_FAILED) {
    perror("Error mapping memory for private mapping");
    close(shm_fd);
    exit(EXIT_FAILURE);
  }
  pid_t p = fork();

  if (p < 0) {
    perror("Error fork()!");
    close(shm_fd);
    exit(EXIT_FAILURE);
  } else if (p == 0) {  // Child process
    // Modify the mapped memory and write to child output file
    strcpy(mapped_mem, "Child Private Mapping");
    write_output(child_output, mapped_mem);
    // Unmap the memory
    munmap(mapped_mem, FILE_SIZE);  
    close(shm_fd);
    exit(0);  // Exit child process
  } else {  // Parent process
    // Wait for the child 
    wait(NULL);  
    // Write the parent's view of the memory to the output file
    write_output(parent_output, mapped_mem);
  }
  // Unmap the memory
  munmap(mapped_mem, FILE_SIZE);  
}

// Demonstrates shared memory mapping
void shared_mapping_example(int shm_fd, const char *child_output, const char *parent_output) {
  // Create a memory mapping with MAP_SHARED
  char *mapped_mem = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (mapped_mem == MAP_FAILED) {
    perror("Error mapping memory for shared mapping");
    close(shm_fd);
    exit(EXIT_FAILURE);
  }

  pid_t p = fork();
  if (p < 0) {
    perror("Error fork()!");
    close(shm_fd);
    exit(EXIT_FAILURE);
  } else if (p == 0) {  // Child process
    // Modify the mapped memory and write to child output file
    strcpy(mapped_mem, "Child Shared Mapping !");
    write_output(child_output, mapped_mem);
    // Unmap the memory
    munmap(mapped_mem, FILE_SIZE);  
    close(shm_fd);
    exit(0); 
  } else {  // Parent process
    // Wait for the child 
    wait(NULL);  
    // Write the parent's view of the memory to the output file
    write_output(parent_output, mapped_mem);
  }
  // Unmap the memory
  munmap(mapped_mem, FILE_SIZE);  
}

int main() {
  // Create a shared memory object
  int shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (shm_fd == -1) {
    perror("Error creating shared memory object");
    return 1;
  }

  // Set the size of the shared memory object
  if (ftruncate(shm_fd, FILE_SIZE) == -1) {
    perror("Error setting shared memory size");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 1;
  }

  // Output files for private and shared mappings
  const char *private_child_output = "private_child_output.txt";
  const char *private_parent_output = "private_parent_output.txt";
  const char *shared_child_output = "shared_child_output.txt";
  const char *shared_parent_output = "shared_parent_output.txt";

  // Demonstrate private file mapping
  private_mapping_example(shm_fd, private_child_output, private_parent_output);

  // Demonstrate shared file mapping
  shared_mapping_example(shm_fd, shared_child_output, shared_parent_output);

  // Clean up the shared memory object
  close(shm_fd);
  shm_unlink(SHM_NAME);

  // Display the saved output file information
  printf("Private Mapping: %s, %s\n", private_child_output, private_parent_output);
  printf("hared Mapping: %s, %s\n", shared_child_output, shared_parent_output);

  return 0;
}
