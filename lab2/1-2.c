#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        return 1;
    } else if (pid == 0) {
        // 子进程执行 ls 命令
        execl("/bin/ls", "ls", NULL);
        // 如果 exec 失败，执行下面语句
        perror("exec failed");
        exit(1);
    } else {
        // 父进程等待子进程
        int status;
        wait(&status);
        printf("子进程执行完毕，退出代码(%d)\n", status);
    }

    return 0;
}