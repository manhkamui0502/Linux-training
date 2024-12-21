/**
 * Task 1: Count from 1 to 1 billion using multithreading with each mechanism: 
 * semaphore, mutex, shared memory, and spinlock.       
 */
#include <stdio.h>      
#include <sys/times.h>
#include <pthread.h>

#define MAX_COUNT           1000000000
#define NUMBER_OF_THREADS   4

int count = 0;

// Spin lock
pthread_spinlock_t spinlock;

// Function to add up number to 1.000.000.000 using spinlock
void *count_with_spin_lock(void* arg) {
  while (1) {
    pthread_spin_lock(&spinlock);
    if (count >= MAX_COUNT) {
      pthread_spin_unlock(&spinlock);
      pthread_exit(NULL);
      break;
    }
    count++;
    pthread_spin_unlock(&spinlock);
  }
  pthread_exit(NULL);
}

int main() {
  // create a buffer to store start time
  struct tms start_buffer;
  clock_t start_time = times(&start_buffer);
  // Create threads array
  pthread_t threads[NUMBER_OF_THREADS];
  
  // Mutex initialize
  pthread_spin_init(&spinlock, 0);

  // Create thread
  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_create(&threads[i], NULL, count_with_spin_lock, NULL);
  }

  // pthread_join() function waits for the thread specified by thread to terminate.
  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // create a buffer to store end time
  struct tms end_buffer;
  clock_t end_time = times(&end_buffer);

  // Remove spinlock
  pthread_spin_destroy(&spinlock);

  // print out the time consumed
  printf("User time: %ld ticks, System time: %ld ticks\n",
         end_buffer.tms_utime - start_buffer.tms_utime,
         end_buffer.tms_stime - start_buffer.tms_stime);

  // Print the final counter value
  printf("Final counter value: %d\n", count);
  
  return 0; 
}
