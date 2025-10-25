#define _XOPEN_SOURCE 500 // 为了启用usleep() 微秒级的暂停

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define MAX_ROUNDS 100
#define TIMEOUT 5                // 5秒超时
#define MSG_TYPE_ID_ASSIGN 100   // 裁判→玩家：分配ID
#define MSG_TYPE_CONNECT_ACK 101 // 玩家→裁判：连接确认

// 消息结构 - IPC通信载体
struct msg_packet
{
    long msg_type;       // 消息类型：玩家ID(1/2)或玩家ID|4(5/6)
    int choice;          // 出拳选择：0=石头,1=剪刀,2=布,-1=超时
    int round;           // 回合数
    int player_id;       // 发送者ID：1=玩家1, 2=玩家2
    int timeout_seconds; // 超时时间(秒)：裁判告知选手的响应时限
};

// 游戏状态 - 全局状态维护
struct game_state
{
    int msgid;       // 消息队列标识符
    int rounds;      // 总回合数
    int scores[3];   // 胜负统计：0=平局,1=玩家1胜,2=玩家2胜
    int timeouts[3]; // 超时统计：[0]未用,[1]=玩家1超时,[2]=玩家2超时
};

// 出拳名称 - 用户友好显示
const char *choice_name(int choice)
{
    switch (choice)
    {
    case 0:
        return "石头";
    case 1:
        return "剪刀";
    case 2:
        return "布";
    case -1:
        return "超时";
    default:
        return "未知";
    }
}

// 等待玩家连接确认
void wait_for_players_connection(int msgid)
{
    
    // 先分配两个玩家ID（简单发送到消息队列）
    struct msg_packet id_message;
    id_message.msg_type = MSG_TYPE_ID_ASSIGN; // 裁判→玩家：分配ID
    id_message.player_id = 1;                 // 玩家1 ID
    msgsnd(msgid, &id_message, sizeof(id_message) - sizeof(long), 0);
    
    id_message.player_id = 2; // 玩家2 ID
    msgsnd(msgid, &id_message, sizeof(id_message) - sizeof(long), 0);
    
    printf("等待玩家连接...\n");

    // 下面是原有的等待连接逻辑
    int connected_players = 0;
    struct msg_packet message;
    time_t start_time = time(NULL);

    while (connected_players < 2 && (time(NULL) - start_time < 30))
    {
        // 非阻塞接收连接确认消息
        // 玩家→裁判：连接确认
        if (msgrcv(msgid, &message, sizeof(message) - sizeof(long), MSG_TYPE_CONNECT_ACK, IPC_NOWAIT) != -1)
        {
            connected_players++;
            printf("玩家%d 连接成功!\n", message.player_id);
        }
        else if (errno != ENOMSG)
        {
            perror("接收连接消息失败");
            break;
        }

        // 显示等待状态
        if (connected_players == 0)
        {
            printf("等待玩家连接中... 已等待 %ld 秒\r", time(NULL) - start_time);
            fflush(stdout);
        }

        sleep(1);
    }

    printf("\n");
    if (connected_players == 2)
    {
        printf("✅ 所有玩家连接成功！开始游戏...\n");
    }
    else if (connected_players == 1)
    {
        printf("⚠️  只有1名玩家连接，继续等待或开始游戏...\n");
    }
    else
    {
        printf("❌ 没有玩家连接，请检查玩家进程是否启动\n");
        exit(1);
    }
}

// 创建和管理消息队列
int create_message_queue()
{
    // ftok(path, id): file to key: 路径和ID生成唯一的IPC key
    // 在裁判和玩家进程中使用相同的（path, id），以获取相同的key访问同一个消息队列
    key_t key = ftok("/tmp", 'R');

    // 删除可能存在的旧队列
    int old_msgid;
    if ((old_msgid = msgget(key, 0666)) != -1)
    {
        printf("清除旧队列成功! 旧队列id: %d\n", old_msgid);
        msgctl(old_msgid, IPC_RMID, NULL);
    }

    // 创建新队列
    int msgid = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
    if (msgid == -1)
    {
        perror("msgget failed");
        exit(1);
    }

    return msgid;
}

// 发送消息给玩家
void send_to_player(int msgid, int player_id, int round)
{
    struct msg_packet message;
    message.msg_type = player_id; // 类型为玩家ID
    message.round = round;        // 回合数
    message.player_id = player_id;
    message.timeout_seconds = TIMEOUT; // 超时时间(秒)

    if (msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0) == -1)
    {
        printf("发送给玩家%d的消息失败: ", player_id);
        perror("msgsnd failed");
        exit(1);
    }
}

// 接收玩家消息（带超时）
int receive_from_player(int msgid, int player_id, int *choice)
{
    struct msg_packet message;
    time_t start_time = time(NULL);

    while (time(NULL) - start_time < TIMEOUT)
    {
        // 非阻塞接收
        if (msgrcv(msgid, &message, sizeof(message) - sizeof(long), player_id | 4, IPC_NOWAIT) != -1)
        {
            *choice = message.choice;
            return 1; // 成功接收
        }

        if (errno != ENOMSG)
        {
            printf("接受玩家%d的消息失败: ", player_id);
            perror("msgrcv failed");
            exit(1);
        }

        usleep(10 * 1000); // 休眠10ms再检查
    }

    *choice = -1; // 超时
    return 0;
}

// 判断胜负
int judge_winner(int choice1, int choice2)
{
    if (choice1 == choice2)
        return 0; // 平局

    if (choice1 == -1)
        return 2; // 玩家1超时，玩家2胜
    if (choice2 == -1)
        return 1; // 玩家2超时，玩家1胜

    // 石头(0)胜剪刀(1), 剪刀(1)胜布(2), 布(2)胜石头(0)
    if ((choice1 + 1) % 3 == choice2)
        return 1; // 玩家1胜
    return 2;     // 玩家2胜
}

// 运行游戏回合
void run_game_round(struct game_state *game, int round)
{
    printf("\n=== 第 %d 回合 ===\n", round);

    int choice1, choice2;

    // 通知玩家1出拳
    printf("裁判: 通知玩家1出拳...\n");
    send_to_player(game->msgid, 1, round);

    // 等待玩家1回复
    if (receive_from_player(game->msgid, 1, &choice1) <= 0)
    {
        printf("玩家1超时！\n");
        game->timeouts[1]++;
    }

    // 通知玩家2出拳
    printf("裁判: 通知玩家2出拳...\n");
    send_to_player(game->msgid, 2, round);

    // 等待玩家2回复
    if (receive_from_player(game->msgid, 2, &choice2) <= 0)
    {
        printf("玩家2超时！\n");
        game->timeouts[2]++;
    }

    // 显示出拳结果
    printf("玩家1出拳: %s\n", choice_name(choice1));
    printf("玩家2出拳: %s\n", choice_name(choice2));

    // 判断胜负
    int winner = judge_winner(choice1, choice2);
    game->scores[winner]++;

    switch (winner)
    {
    case 0:
        printf("结果: 平局\n");
        break;
    case 1:
        printf("结果: 玩家1获胜\n");
        break;
    case 2:
        printf("结果: 玩家2获胜\n");
        break;
    }
    printf("\n");
}

// 显示最终结果
void show_final_results(struct game_state *game)
{
    printf("\n=== 游戏结束 ===\n");
    printf("总回合数: %d\n", game->rounds);
    printf("平局: %d\n", game->scores[0]);
    printf("玩家1胜利: %d (超时: %d)\n", game->scores[1], game->timeouts[1]);
    printf("玩家2胜利: %d (超时: %d)\n", game->scores[2], game->timeouts[2]);

    if (game->scores[1] > game->scores[2])
    {
        printf("最终胜者: 玩家1\n");
    }
    else if (game->scores[2] > game->scores[1])
    {
        printf("最终胜者: 玩家2\n");
    }
    else
    {
        printf("最终结果: 平局\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("用法: %s <回合数>\n", argv[0]);
        return 1;
    }

    int rounds = atoi(argv[1]);
    if (rounds <= 0 || rounds > MAX_ROUNDS)
    {
        printf("回合数必须在1-%d之间\n", MAX_ROUNDS);
        return 1;
    }

    printf("创建石头剪刀布游戏 (%d回合)...\n", rounds);

    // 初始化游戏状态
    struct game_state game;
    memset(&game, 0, sizeof(game));
    game.rounds = rounds;
    game.msgid = create_message_queue();

    printf("消息队列创建成功 (ID: %d)\n", game.msgid);
    printf("请在其他终端启动玩家进程: ./player\n");

    // 等待玩家连接确认
    wait_for_players_connection(game.msgid);

    // 运行所有回合
    for (int i = 1; i <= rounds; i++)
    {
        run_game_round(&game, i);
        usleep(100 * 1000); // 回合间隔100ms
    }

    // 显示最终结果
    show_final_results(&game);

    // 清理消息队列
    msgctl(game.msgid, IPC_RMID, NULL);
    printf("消息队列已清理\n");
    printf("游戏结束，按任意键退出...");
    getchar();

    return 0;
}