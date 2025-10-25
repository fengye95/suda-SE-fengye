#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

// 使用与裁判相同的消息结构
struct msg_packet
{
    long msg_type;
    int choice;
    int round;
    int player_id;
    int timeout_seconds;
};

#define SOCKET_PATH "/tmp/rock_paper_scissors.sock"

// 连接到Socket服务器
int connect_to_server()
{
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket failed");
        exit(1);
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect failed - 请先启动裁判进程");
        exit(1);
    }

    return sockfd;
}

// 处理裁判消息
void handle_referee_message(int sockfd, int player_id)
{
    struct msg_packet message;
    
    while (1)
    {
        // 阻塞等待裁判指令
        ssize_t bytes = recv(sockfd, &message, sizeof(message), 0);
        if (bytes != sizeof(message)) {
            if (bytes == 0) {
                printf("裁判已断开连接\n");
            } else {
                perror("recv failed");
            }
            break;
        }

        // 随机出拳
        message.choice = rand() % 3;
        message.player_id = player_id;

        // 回复裁判
        if (send(sockfd, &message, sizeof(message), 0) == -1) {
            perror("send failed");
            break;
        }

        printf("玩家%d: 第%d回合出拳 %s\n",
               player_id, message.round,
               message.choice == 0 ? "石头" : 
               message.choice == 1 ? "剪刀" : "布");
    }
}

int main()
{
    srand(time(NULL) + getpid());

    printf("玩家进程启动 (PID: %d)\n", getpid());

    // 连接到Socket服务器
    int sockfd = connect_to_server();
    printf("Socket连接成功 (fd: %d)\n", sockfd);

    // 等待裁判分配玩家ID
    struct msg_packet id_message;
    printf("等待裁判分配ID...\n");
    ssize_t bytes = recv(sockfd, &id_message, sizeof(id_message), 0);
    if (bytes != sizeof(id_message)) {
        perror("接收ID失败");
        exit(1);
    }
    int player_id = id_message.player_id;
    printf("裁判分配为玩家%d\n", player_id);

    // 通知裁判已准备好
    struct msg_packet ready_msg = {0};
    ready_msg.player_id = player_id;
    ready_msg.msg_type = 1;  // 准备就绪消息类型
    if (send(sockfd, &ready_msg, sizeof(ready_msg), 0) == -1) {
        perror("发送准备就绪消息失败");
        exit(1);
    }
    printf("已通知裁判准备就绪\n");

    printf("等待裁判指令...\n");

    // 处理裁判消息
    handle_referee_message(sockfd, player_id);

    // 关闭连接
    close(sockfd);
    
    printf("玩家进程结束\n");
    printf("游戏结束，按任意键退出...");
    getchar();
    
    return 0;
}