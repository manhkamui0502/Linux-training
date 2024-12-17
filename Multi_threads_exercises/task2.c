/**
 * Task2:  Write a program creating 2 threads. All of them use a handler function. 
 * If thread 1 is running, printing some verifying information. 
 * If thread 2 is running, providing a struct called human with some field: 
 *    full name, born year, phone number, hometown. 
 *    Then , print that infor to terminal
 */

#include <stdio.h>      
#include <unistd.h>
#include <sys/times.h>
#include <pthread.h>

#define NUMBER_OF_THREADS   2

struct human_identify {
  char full_name[50];
  int born_year;
  char phone_number[10];
  char home_town[50];
};


// Thread function for print Verification every 2 seconds
void *thread_1_verify(void *arg) {
  while (1) {
    printf("\nVerification: Thread 1 is running!\n");
    sleep(2);
  }
  pthread_exit(NULL);
}

// Thread function for print human identify every 3 seconds
void *thread_2_show_info(void *arg) {
  struct human_identify *human_id = (struct human_identify *)arg;
  while (1) {
    printf("\nFull name: %s\nBorn year: %d\nPhone number: %s\nHome town: %s\n", 
           human_id->full_name, 
           human_id->born_year, 
           human_id->phone_number, 
           human_id->home_town);

    sleep(3);
  }
  pthread_exit(NULL);
}

int main() {
  // create an array of pthread
  pthread_t threads[NUMBER_OF_THREADS];

  // create a person_a
  struct human_identify person_a = {"Tran Duc Manh", 
                                    2002, 
                                    "0123456789", 
                                    "Vinh Phuc"};

  // create pthreads
  pthread_create(&threads[0], NULL, thread_1_verify, NULL);
  pthread_create(&threads[1], NULL, thread_2_show_info, (void *)&person_a);

  // pthread_join() function waits for the thread specified by thread to terminate.
  for (int i = 0; i < NUMBER_OF_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0; 
}
