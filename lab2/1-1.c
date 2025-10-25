#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        exit(1);
    } else if (pid == 0) {
        // 子进程
        printf("这是子进程, PID = %d, 父进程PID = %d\n", getpid(), getppid());
        printf("子进程退出\n");
        exit(0);
    } else {
        // 父进程
        printf("这是父进程, PID = %d, 子进程PID = %d\n", getpid(), pid);
        wait(&status);  // 等待子进程结束
        printf("子进程退出代码(%d), 父进程退出\n", status);
    }

    return 0;
}