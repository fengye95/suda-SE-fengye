#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>

// 使用与裁判相同的消息结构
struct msg_packet
{
    long msg_type;       // 消息类型：玩家ID(1/2)或玩家ID|4(5/6)
    int choice;          // 出拳选择：0=石头,1=剪刀,2=布,-1=超时
    int round;           // 回合数
    int player_id;       // 发送者ID：1=玩家1, 2=玩家2
    int timeout_seconds; // 超时时间(秒)
};

#define MSG_TYPE_ID_ASSIGN 100   // 裁判→玩家：分配ID
#define MSG_TYPE_CONNECT_ACK 101 // 玩家→裁判：连接确认

// 获取消息队列
int get_message_queue()
{
    key_t key = ftok("/tmp", 'R');
    int msgid = msgget(key, 0666);
    if (msgid == -1)
    {
        perror("msgget failed - 请先启动裁判进程");
        exit(1);
    }
    return msgid;
}

// 发送连接确认消息
void send_connection_message(int msgid, int player_id)
{
    struct msg_packet message;
    message.msg_type = MSG_TYPE_CONNECT_ACK;
    message.player_id = player_id;
    message.choice = 0;
    message.round = 0;
    printf("玩家%d 发送连接请求\n", player_id);
    if (msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0) == -1)
    {
        perror("发送连接消息失败");
        exit(1);
    }
    else
    {
        printf("玩家%d: 已连接\n", player_id);
    }
}

// 处理裁判消息
void handle_referee_message(int msgid, int player_id)
{
    struct msg_packet message;
    while (1)
    {
        // 阻塞等待裁判指令
        if (msgrcv(msgid, &message, sizeof(message) - sizeof(long), player_id, 0) == -1)
        {
            perror("msgrcv failed");
            break;
        }

        // 随机出拳
        message.choice = rand() % 3;
        message.msg_type = player_id | 4; // 回复消息类型

        // 回复裁判
        if (msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0) == -1)
        {
            perror("msgsnd failed");
            break;
        }

        printf("玩家%d: 第%d回合出拳 %s\n",
               player_id, message.round,
               message.choice == 0 ? "石头" : message.choice == 1 ? "剪刀"
                                                                  : "布");
    }
}

// 获取分配的玩家ID
int get_assigned_id(int msgid)
{
    struct msg_packet message;
    msgrcv(msgid, &message, sizeof(message) - sizeof(long), MSG_TYPE_ID_ASSIGN, 0);
    return message.player_id;
}

int main()
{
    srand(time(NULL) + getpid());

    printf("玩家进程启动 (PID: %d)\n", getpid());

    // 连接消息队列
    int msgid = get_message_queue();
    printf("消息队列连接成功 (ID: %d)\n", msgid);

    // 获取玩家ID
    int player_id = get_assigned_id(msgid);
    printf("获取到玩家id为%d\n", player_id);

    // 发送连接确认
    send_connection_message(msgid, player_id);

    printf("等待裁判指令...\n");

    // 处理裁判消息
    handle_referee_message(msgid, player_id);

    printf("玩家进程结束\n");

    printf("游戏结束，按任意键退出...");
    getchar();
    return 0;
}