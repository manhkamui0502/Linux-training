/**
 * Task 1: Count the number of odd numbers from 1 to 1 billion. Write code for 
 * two cases: 
 *    case 1 using multithreading.
 *    case 2 without using multithreading. 
 * Compare the execution time of the two cases. 
 */

#include <stdio.h>      
#include <unistd.h>
#include <sys/times.h>
#include <pthread.h>

#define RANGE               1000000000
#define NUMBER_OF_THREADS   4

struct thread_data {
  long start;
  long end;
  long count;
};

// Function to add up number to 1.000.000.000
long count_odd_number(long start, long end) {
  long count = 0;
  for (long i = start; i < end; i++) {
    if (i % 2 != 0) {
      count++;
    }
  }
  return count;
}

// Thread function for multithreading
void *thread_count_odds(void *arg) {
  struct thread_data *data = (struct thread_data *)arg;
  data->count = count_odd_number(data->start, data->end);
  pthread_exit(NULL);
}

int main() {
  // create a buffer to store start time
  struct tms start_buffer;
  clock_t start_time = times(&start_buffer);

  pthread_t threads[NUMBER_OF_THREADS];
  struct thread_data thread_args[NUMBER_OF_THREADS];
  long step = RANGE / NUMBER_OF_THREADS;
  long multi_thread_count = 0;
  
  /**
   * thread 0 count in (0, 250000000)
   * thread 1 count in (250000001, 500000000)
   * thread 2 count in (500000001, 750000000)
   * thread 3 count in (750000001, 1000000000)
   */

  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    thread_args[i].start = (i == 0) ? step * i : step * i + 1;
    thread_args[i].end = (i == 0) ? thread_args[i].start + step 
                                  : thread_args[i].start + step - 1;
    thread_args[i].count = 0;
    pthread_create(&threads[i], NULL, thread_count_odds, (void *)&thread_args[i]);
  }

  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_join(threads[i], NULL);
    multi_thread_count += thread_args[i].count;
  }
  printf("Odds counted: %ld\n", multi_thread_count);

  // create a buffer to store end time
  struct tms end_buffer;
  clock_t end_time = times(&end_buffer);

  // print out the time consumed
  printf("User time: %ld ticks, System time: %ld ticks\n",
         end_buffer.tms_utime - start_buffer.tms_utime,
         end_buffer.tms_stime - start_buffer.tms_stime);

  return 0; 
}
