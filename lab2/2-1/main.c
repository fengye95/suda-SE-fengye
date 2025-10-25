#include <stdio.h>
#include <stdlib.h>
#include "sche.h"

// 显示调度算法菜单
void show_schedule_menu()
{
    printf("\n");
    printf("=========================================\n");
    printf("           进程调度算法模拟系统           \n");
    printf("=========================================\n");
    printf("请选择调度算法：\n");
    printf("1. 先来先服务 (FCFS)\n");
    printf("2. 短作业优先 (SJF) - 非抢占式\n");
    printf("3. 优先级调度 (Priority)\n");
    printf("4. 时间片轮转 (Round Robin)\n");
    printf("0. 退出程序\n");
    printf("=========================================\n");
    printf("请输入选择 (0-4): ");
}

int main()
{
    // 选择调度策略
    int option;
    show_schedule_menu();
    scanf("%d", &option);
    printf("\n");

    PCB *ready_queue_tail = init_ready_queue();


    switch (option)
    {
    case 1:
        FIFO_schedule(&ready_queue_tail);
        break;
    case 2:
        SJF_schedule(&ready_queue_tail);
        break;
    case 3:
        PR_schedule(&ready_queue_tail);
        break;
    case 4:
        RR_schedule(&ready_queue_tail);
        break;

    default:
        break;
    }

    return 0;
}