/**
 * Task 1: Count from 1 to 1 billion using multithreading with each mechanism: 
 * semaphore, mutex, shared memory, and spinlock.       
 */
#include <stdio.h>      
#include <sys/times.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_COUNT           1000000000
#define NUMBER_OF_THREADS   4

int count = 0;

// Semaphore
sem_t semaphore;

// Function to add up numbers to 1 billion using a semaphore
void *count_with_semaphore(void* arg) {
  while (1) {
    sem_wait(&semaphore);
    if (count >= MAX_COUNT) {
      sem_post(&semaphore); // Allow other threads to exit gracefully
      pthread_exit(NULL);
    }
    count++;
    sem_post(&semaphore);
  }
  pthread_exit(NULL);
}

int main() {
  // Create a buffer to store the start time
  struct tms start_buffer;
  clock_t start_time = times(&start_buffer);

  // Create threads array
  pthread_t threads[NUMBER_OF_THREADS];

  // Initialize semaphore
  /**
   * 0: Semaphore is shared between threads of the same process.
   * Non-zero: Semaphore is shared between processes 
   * (requires the semaphore to be in shared memory).
   */
  sem_init(&semaphore, 0, 1);

  // Create threads
  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_create(&threads[i], NULL, count_with_semaphore, NULL);
  }

  // Wait for all threads to complete
  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // Create a buffer to store the end time
  struct tms end_buffer;
  clock_t end_time = times(&end_buffer);

  // Destroy semaphore
  sem_destroy(&semaphore);

  // Print out the time consumed
  printf("User time: %ld ticks, System time: %ld ticks\n",
         end_buffer.tms_utime - start_buffer.tms_utime,
         end_buffer.tms_stime - start_buffer.tms_stime);

  // Print the final counter value
  printf("Final counter value: %d\n", count);

  return 0;
}
