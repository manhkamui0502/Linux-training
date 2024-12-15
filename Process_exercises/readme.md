## Usage
```
./open_file [file_name]
```

## Check the returned value:
```
echo $?
```
* If operation succeeded, return `0`
* If operation failed, return `-1`, which show `255` after using `echo $?`

## Extra excersice
1. Write a program call A that creates child process called B then print its PID 
and PPID.
2. Write a program called A that creates a child process called B, print PID and 
PPID. Using systemcall
waitpid() to get ending state of B. B will sleep for 2 second. Therefore, the 
parent A will print out the exit status of B after 2 seconds
3. Write a program that create a child process, then both parent and child will 
write to the file. Then child process will be changed to the new process by
using execl(). The child process now will print the content of the file that 
written before (open_file).
* Usage
  ```
  ./ex3 test.txt
  ```
4. Use SIGCHLD to prevent ZOMBIE state.
