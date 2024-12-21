/**
 * Task 1: Count from 1 to 1 billion using multithreading with each mechanism: 
 * semaphore, mutex, shared memory, and spinlock.     
 */
#include <stdio.h> 
#include <stdlib.h>   
#include <unistd.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

// Shared memory object name
#define SHM_NAME "/shm_counter"

// Shared memory size
#define SHM_SIZE sizeof(shared_data_t)

#define MAX_COUNT           1000000000
#define NUMBER_OF_PROCESS   4

long step = MAX_COUNT / NUMBER_OF_PROCESS;

// Data struct to be stored
typedef struct
{
  /* data */
  pthread_mutex_t mutex;
  int count;
}shared_data_t;

// Function to add up number to 1.000.000.000 using shared memory
void count_with_shared_memory(void* arg) {
  shared_data_t* data = (shared_data_t *)arg;
  pthread_mutex_lock(&data->mutex);
  // Each thread increments the counter "step" times
  for (int i = 0; i < step; i++) {
    (data->count)++;
  }
  pthread_mutex_unlock(&data->mutex);
}

int main() {
  // create a buffer to store start time
  struct tms start_buffer;
  clock_t start_time = times(&start_buffer);
  // Create threads array
  pid_t pids[NUMBER_OF_PROCESS];
  shared_data_t *shared_data;

    // Open the shared memory object
  int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed");
    return 1;
  }

  // Set the size of the shared memory object
  if (ftruncate(shm_fd, SHM_SIZE) == -1) {
    perror("ftruncate failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 1;
  }

  // Map the shared memory object to the process's memory space
  shared_data = (shared_data_t *) mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared_data == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 1;
  }
  
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); // Enable sharing between processes
  // Initialize the mutex
  if (pthread_mutex_init(&shared_data->mutex, &attr) != 0) {
    perror("Mutex init failed");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 1;
  }

  // Initialize the counter in shared memory
  shared_data->count = 0;

  // Create 4 process to count
  for (int i = 0; i < NUMBER_OF_PROCESS; i++) {
    pids[i] = fork();
    if (pids[i] < 0) {
      // Fork failed
      perror("fork");
      return 1;
    } else if (pids[i] == 0) {
      // Child process
      printf("Child %d: PID = %d, Parent PID = %d\n", i + 1, getpid(), getppid());
      // Perform counting task
      count_with_shared_memory(shared_data);
      exit(0); // Exit child process
    }
  }

  // Wait 
  for (int i = 0; i < NUMBER_OF_PROCESS; i++) {
    wait(NULL);
  }

  // create a buffer to store end time
  struct tms end_buffer;
  clock_t end_time = times(&end_buffer);

  // print out the time consumed
  printf("User time: %ld ticks, System time: %ld ticks\n",
     end_buffer.tms_utime - start_buffer.tms_utime,
     end_buffer.tms_stime - start_buffer.tms_stime);

  // Print the final counter value
  printf("Final counter value: %d\n", shared_data->count);

  // Clean up
  munmap(shared_data, SHM_SIZE);
  close(shm_fd);
  shm_unlink(SHM_NAME);

  return 0; 
}
