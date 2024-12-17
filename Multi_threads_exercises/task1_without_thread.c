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

#define RANGE 1000000000

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

int main() {
  // create a buffer to store start time
  struct tms start_buffer;
  clock_t start_time = times(&start_buffer);

  // start counting
  printf ("Odds counted: %ld\n", count_odd_number(0, RANGE));

  // create a buffer to store end time
  struct tms end_buffer;
  clock_t end_time = times(&end_buffer);

  // print out the time consumed
  printf("User time: %ld ticks, System time: %ld ticks\n",
         end_buffer.tms_utime - start_buffer.tms_utime,
         end_buffer.tms_stime - start_buffer.tms_stime);

  return 0; 
}
