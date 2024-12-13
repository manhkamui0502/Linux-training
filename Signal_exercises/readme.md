# TASKS

## Task 1: Write a C program to block the Ctrl + C signal (SIGINT) from the user. 
```C
  // Create a signal set to store the blocked signals
  sigset_t sig_set;

  // Initialize the signal set
  sigemptyset(&sig_set);
  // Add SIGINT to the set
  sigaddset(&sig_set, SIGINT);
  // Block SIGINT
  if (sigprocmask(SIG_BLOCK, &sig_set, NULL) == -1) {
      perror("Failed to block SIGINT");
      return 1;
  }
```
* Now, user can't use Ctrl + C to terminate the program because SIGINT was blocked. To stop the process, use Ctrl + Z.



## Task 2: Write a C program to print the signal mask of the current process. 
* At first,  will block the SIGINT to easy to demonstrate
```C  
  // Create a new current_mask to store the current signal set
  sigset_t current_mask;
  char sig_name[10] = {0};

  /**
   * Block the Interrupt signal (SIGINT)
   */
  // Empty the sig_set
  sigemptyset(&current_mask);
  // Add SIGINT to sig_set
  sigaddset(&current_mask, SIGINT);
  // Block SIGINT
  if (sigprocmask(SIG_BLOCK, &current_mask, NULL) == -1) {
    perror("Failed to block SIGINT");
    return 1;
  }
```

* Then, i will print the Active and Blocked signals:
```C
  printf("- - - Active signal - - - \n");
  // Iterate over all signals (from 1 to NSIG)
  for (int i = 1; i <= NSIG; i++) {
    if (sigismember(&current_mask, i) == 0) {
      psignal(i, sig_name);
    }
  }

  // Show blocked signal
  printf("- - - Blocked signal - - - \n");
  for (int i = 1; i <= NSIG; i++) {
    if (sigismember(&current_mask, i) == 1) {
      psignal(i, sig_name);
    }
  }
```

* The result should show that the only Blocked signal is Interrupt:
```
Output:
- - - Active signal - - - 
Hangup
Quit
Illegal instruction
Trace/breakpoint trap
Aborted
Bus error
Floating point exception
Killed
User defined signal 1
Segmentation fault
User defined signal 2
Broken pipe
Alarm clock
Terminated
Stack fault
[to be continue]
- - - Blocked signal - - - 
Interrupt
```

## Task 3: Write a C program to block SIGINT, then check if SIGINT is in the pending signals, and if so, exit the program.
* At first, block the SIGINT as 2 tasks above.
* Then create a loop, we will wait for the interrupt signal in inside the loop.
* If a SIGINT was raised, the mask of pending signals is returned in set, which is `pending_set`.
  ```C
  if (sigpending(&pending_set) == -1) {
    perror("Failed to retrieve pending signals");
    return 1;
  }
  ```
* Then we will check in the `pending_set`, if there is a SIGINT, exit after 3 seconds.
  ```C
  // Check if SIGINT is pending
  if (sigismember(&pending_set, SIGINT)) {
    printf("Interrupt signal (SIGINT) is pending!\n");
    printf("Sleeping for 3 seconds before exiting...\n");
    sleep(3);
    return 0;
  }
  ```