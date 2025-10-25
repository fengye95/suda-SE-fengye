#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h> // 添加这个头文件
#include <time.h>
#include <errno.h>

#define MAX_ROUNDS 100
#define TIMEOUT 5
#define SOCKET_PATH "/tmp/rock_paper_scissors.sock"

// 消息结构 - 保持不变
struct msg_packet
{
    long msg_type;
    int choice;
    int round;
    int player_id;
    int timeout_seconds;
};

// 游戏状态 - 修改为Socket连接
struct game_state
{
    int player1_fd; // 改为Socket文件描述符
    int player2_fd;
    int rounds;
    int scores[3];
    int timeouts[3];
};

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

// 创建Socket服务器
int create_socket_server()
{
    // 创建Unix域套接字，使用面向连接的TCP风格通信
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket failed");
        exit(1);
    }

    // 删除之前的socket文件 - 防止之前运行残留的socket文件导致绑定失败
    unlink(SOCKET_PATH);

    // 配置socket地址结构
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;  // 使用Unix域套接字（本地进程间通信）
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 将socket绑定到指定的文件路径
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind failed");
        exit(1);
    }

    // 开始监听连接，最多允许2个连接在队列中等待（对应两个玩家）
    if (listen(server_fd, 2) == -1)
    {
        perror("listen failed");
        exit(1);
    }

    printf("Socket服务器创建成功 (路径: %s)\n", SOCKET_PATH);
    return server_fd;  // 返回服务器socket文件描述符
}

// 等待玩家连接
void wait_for_players_connection(int server_fd, struct game_state *game)
{
    printf("等待玩家连接...\n");

    // 接受第一个玩家连接
    game->player1_fd = accept(server_fd, NULL, NULL);
    if (game->player1_fd == -1)
    {
        perror("接受玩家1连接失败");
        exit(1);
    }
    printf("玩家1 连接成功!\n");

    // 接受第二个玩家连接
    game->player2_fd = accept(server_fd, NULL, NULL);
    if (game->player2_fd == -1)
    {
        perror("接受玩家2连接失败");
        exit(1);
    }
    printf("玩家2 连接成功!\n");

    printf("✅ 所有玩家连接成功！开始游戏...\n");
}

// 发送消息给玩家
void send_to_player(int player_fd, int round)
{
    struct msg_packet message;
    message.msg_type = 1; // 保持兼容
    message.round = round;
    message.player_id = (player_fd == -1) ? 1 : 2; // 简化处理
    message.timeout_seconds = TIMEOUT;

    if (send(player_fd, &message, sizeof(message), 0) == -1)
    {
        printf("发送给玩家消息失败: ");
        perror("send failed");
        exit(1);
    }
}

// 接收玩家消息（带超时）
int receive_from_player(int player_fd, int *choice)
{
    struct msg_packet message;
    time_t start_time = time(NULL);

    while (time(NULL) - start_time < TIMEOUT)
    {
        // 设置非阻塞接收
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(player_fd, &read_fds);

        struct timeval tv = {0, 100000}; // 100ms超时
        int ready = select(player_fd + 1, &read_fds, NULL, NULL, &tv);

        if (ready > 0)
        {
            ssize_t bytes = recv(player_fd, &message, sizeof(message), MSG_DONTWAIT);
            if (bytes == sizeof(message))
            {
                *choice = message.choice;
                return 1;
            }
            else if (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                printf("接收玩家消息失败: ");
                perror("recv failed");
                return -1;
            }
        }
        else if (ready == -1)
        {
            perror("select failed");
            return -1;
        }
    }

    *choice = -1; // 超时
    return 0;
}

// 判断胜负 - 完全不变
int judge_winner(int choice1, int choice2)
{
    if (choice1 == choice2)
        return 0;
    if (choice1 == -1)
        return 2;
    if (choice2 == -1)
        return 1;
    if ((choice1 + 1) % 3 == choice2)
        return 1;
    return 2;
}

// 运行游戏回合 - 主要修改通信部分
void run_game_round(struct game_state *game, int round)
{
    printf("\n=== 第 %d 回合 ===\n", round);

    int choice1, choice2;

    // 通知玩家1出拳
    printf("裁判: 通知玩家1出拳...\n");
    send_to_player(game->player1_fd, round);

    // 等待玩家1回复
    if (receive_from_player(game->player1_fd, &choice1) <= 0)
    {
        printf("玩家1超时！\n");
        game->timeouts[1]++;
    }

    // 通知玩家2出拳
    printf("裁判: 通知玩家2出拳...\n");
    send_to_player(game->player2_fd, round);

    // 等待玩家2回复
    if (receive_from_player(game->player2_fd, &choice2) <= 0)
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

// 显示最终结果 - 完全不变
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

    // 创建Socket服务器
    int server_fd = create_socket_server();
    printf("Socket创建成功 (fd: %d)\n", server_fd);

    printf("请在其他终端启动玩家进程: ./player\n");

    // 等待玩家连接
    wait_for_players_connection(server_fd, &game);

    // 关闭服务器Socket，不再接受新连接
    close(server_fd);

    // 运行所有回合
    for (int i = 1; i <= rounds; i++)
    {
        run_game_round(&game, i);
        usleep(100 * 1000); // 回合间隔100ms
    }

    // 显示最终结果
    show_final_results(&game);

    // 清理资源
    close(game.player1_fd);
    close(game.player2_fd);
    unlink(SOCKET_PATH);

    printf("Socket已清理\n");
    printf("游戏结束，按任意键退出...");
    getchar();

    return 0;
}