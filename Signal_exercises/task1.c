#include <stdio.h>
#include <signal.h>

int main(int argc, char **argv) {
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

    // Now, user can't use Ctrl + C to terminate the program because SIGINT was
    // blocked. To stop the process, use Ctrl + Z
    while(1);

    return 0;
}
	
