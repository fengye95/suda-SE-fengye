#include <stdio.h>
#include <stdlib.h>

typedef struct PCB {
    char name[10];
    int priority;   // 优先级
    int runtime;    // 需要运行时间
    int usedtime;   // 已运行时间
    char state;     // 状态：W(就绪), R(运行), F(完成)
    struct PCB *next;
} PCB;

// 将一个新的PCB插入到就绪队列（循环队列）
extern PCB* push(PCB *tail, PCB *new_PCB) ;

// 从就绪队列中取出一个进程（从队首）
extern PCB* pop(PCB *tail, PCB **ready_queue) ;

// _开头：辅助函数：打印单个PCB信息
extern void _print_PCB(struct PCB *current) ;

// 打印当前运行PCB， 运行前状态
extern void print_current_PCB(struct PCB *current) ;

// 打印就绪队列状态
extern void print_ready_queue(PCB *tail) ;

// 初始化就绪队列
extern PCB* init_ready_queue() ;

// FIFO调度
void FIFO_schedule(PCB **ready_queue_tail);

// 时间片轮转调度
extern void RR_schedule(PCB **ready_queue_tail);

// 优先级调度
extern void PR_schedule(PCB **ready_queue_tail);

// 从就绪队列中获取优先级最高的进程（数字越大优先级越高）
PCB* get_highest_priority(PCB *tail, PCB **ready_queue) ;

// 从就绪队列中获取运行时间最短的进程
PCB* get_shortest_job(PCB *tail, PCB **ready_queue) ;

// 短作业优先调度
extern void SJF_schedule(PCB **ready_queue_tail);