#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void show_process_name(int pid) {
    char filename[256];
    sprintf(filename, "/proc/%d/comm", pid);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("Invalid pid\n");
        return;
    }

    char c;
    while (read(fd, &c, 1) > 0) {
        printf("%c", c);
    }

    close(fd);
}

int main() {
    int pid;
    printf("Input process ID: \n");
    scanf("%d", &pid);
    show_process_name(pid);
    return 0;
}

