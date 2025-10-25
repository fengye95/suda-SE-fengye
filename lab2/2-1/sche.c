
#include <stdio.h>
#include <stdlib.h>
#include "sche.h"

// 将一个新的PCB插入到就绪队列（循环队列）
PCB* push(PCB *tail, PCB *new_PCB) {
    if (tail == NULL) {
        // 队列为空，创建循环
        new_PCB->next = new_PCB;
        return new_PCB;
    } else {
        // 插入到队尾，保持循环
        new_PCB->next = tail->next;  // new_PCB指向队首
        tail->next = new_PCB;        // 原队尾指向new_PCB
        return new_PCB;              // 返回新的队尾
    }
}

// 从就绪队列中取出一个进程（从队首）
PCB* pop(PCB *tail, PCB **ready_queue) {
    if (tail == NULL) {
        return NULL;  // 队列为空
    }
    
    PCB *head = tail->next;  // 队首元素
    
    if (head == tail) {
        // 只有一个元素
        *ready_queue = NULL;
        tail->next = NULL;  // 解除循环
        return head;
    } else {
        // 多个元素，移除队首
        tail->next = head->next;  // 队尾直接指向新的队首
        *ready_queue = tail;      // 更新队列指针
        head->next = NULL;        // 解除链接
        return head;
    }
}

void _print_PCB(struct PCB *current) {
    printf("%s     %c     %d         %d        %d\n", 
                current->name, current->state, current->priority, 
                current->runtime, current->usedtime);
}

// 打印当前运行PCB， 运行前状态
void print_current_PCB(struct PCB *current) {
    printf("**** 当前正在运行的进程是: %s\n", current->name);
    printf("qname state priority runtime usedtime\n");
    printf("%s     %c     %d         %d        %d\n", 
                current->name, current->state, current->priority, 
                current->runtime, current->usedtime);
    printf("\n");
}

// 打印就绪队列状态
void print_ready_queue(PCB *tail) {
    if (tail == NULL) {
        printf("就绪队列为空\n");
        return;
    }
    
    PCB *current = tail->next;  // 从队首开始
    PCB *start = current;
    
    printf("**** 当前就绪队列状态：\n");
    printf("qname state priority runtime usedtime\n");
    
    do {
        // 打印'就绪态' PCB信息 
        _print_PCB(current);
        current = current->next;
    } while (current != start);
    printf("\n");
}

// 初始化就绪队列
PCB* init_ready_queue() {
    int n;
    PCB *ready_queue_tail = NULL;  // 循环队列的尾指针
    PCB *new_PCB;

    printf("请输入被调用的进程数目: ");
    scanf("%d", &n);
    printf("\n");

    for (int i = 0; i < n; i++) {
        // 初始化一个PCB
        new_PCB = malloc(sizeof(PCB));
        new_PCB->usedtime = 0;
        new_PCB->state = 'W';
        new_PCB->next = NULL;

        printf("进程号No.%d:\n", i);
        printf("输入进程名: "); scanf("%s", new_PCB->name);
        printf("输入进程优先数: "); scanf("%d", &new_PCB->priority);
        printf("输入进程运行时间: "); scanf("%d", &new_PCB->runtime);
        printf("\n");

        // 将新的PCB加入就绪队列
        ready_queue_tail = push(ready_queue_tail, new_PCB);
    }
    
    return ready_queue_tail;  // 返回队列尾指针
}


// FIFO调度
void FIFO_schedule(PCB **ready_queue_tail) {
    printf("\n=== 开始短作业优先(SJF)调度 ===\n\n");

    int current_time = 0;
    
    while (*ready_queue_tail != NULL) {
        // 取出队首进程运行
        PCB *running_process = pop(*ready_queue_tail, ready_queue_tail);
        running_process->state = 'R';
        
        // 打印当前运行进程的PCB
        printf("时间 %d: ", current_time);
        print_current_PCB(running_process);
        
        // FIFO是非抢占式的，让进程一直运行到完成
        printf("进程 %s 开始运行，需要时间 %d\n", running_process->name, running_process->runtime);
        
        // 模拟进程完整运行
        current_time += running_process->runtime;
        running_process->usedtime = running_process->runtime;
        
        // 进程完成
        running_process->state = 'F';
        printf("时间 %d: 进程 %s 完成！\n\n", current_time, running_process->name);
        free(running_process);
        
        // 打印当前就绪队列状态
        if (*ready_queue_tail != NULL) {
            print_ready_queue(*ready_queue_tail);
        }

        printf("\n=======================================\n\n");
    }
    
    printf("\n所有进程执行完毕！\n");
}

// 时间片轮转调度
void RR_schedule(PCB **ready_queue_tail) {
    printf("\n=== 开始时间片轮转调度 ===\n\n");

    int time_slice = 1;  // 时间片大小
    int current_time = 0;
    
    while (*ready_queue_tail != NULL) {
        // 取出队首进程运行
        PCB *running_process = pop(*ready_queue_tail, ready_queue_tail);
        running_process->state = 'R';
        
        // 打印当前运行进程的PCB
        print_current_PCB(running_process);
        
        // 模拟运行一个时间片
        running_process->usedtime += time_slice;
        current_time += time_slice;
        
        if (running_process->usedtime >= running_process->runtime) {
            // 进程完成
            running_process->state = 'F';
            printf("进程 %s 完成！\n\n", running_process->name);
            free(running_process);
        } else {
            // 进程未完成，放回队尾
            running_process->state = 'W';
            *ready_queue_tail = push(*ready_queue_tail, running_process);
        }
        
        // 打印当前就绪队列状态
        if (*ready_queue_tail != NULL) {
            print_ready_queue(*ready_queue_tail);
        }

        printf("\n=======================================\n\n\n");
    }
    
    printf("\n所有进程执行完毕！\n");
}

// 从就绪队列中获取优先级最高的进程（数字越大优先级越高）
PCB* get_highest_priority(PCB *tail, PCB **ready_queue) {
    if (tail == NULL) {
        return NULL;
    }
    
    PCB *current = tail->next;  // 队首
    PCB *start = current;
    PCB *highest_priority_process = current;
    
    // 遍历整个循环队列，找到优先级最高的进程
    do {
        if (current->priority > highest_priority_process->priority) {
            highest_priority_process = current;
        }
        current = current->next;
    } while (current != start);
    
    // 从队列中移除这个进程
    if (highest_priority_process == tail->next) {
        // 如果是队首，直接pop
        return pop(tail, ready_queue);
    } else {
        // 找到要移除进程的前一个节点
        PCB *prev = tail->next;
        while (prev->next != highest_priority_process) {
            prev = prev->next;
        }
        
        // 从队列中移除
        prev->next = highest_priority_process->next;
        
        // 如果移除的是队尾，需要更新队尾指针
        if (highest_priority_process == tail) {
            *ready_queue = prev;
        }
        
        highest_priority_process->next = NULL;
        return highest_priority_process;
    }
}

// 优先级调度（数字越大优先级越高）
void PR_schedule(PCB **ready_queue_tail) {
    printf("\n=== 开始优先级调度 ===\n\n");

    int time_slice = 1;  // 时间片大小
    int current_time = 0;
    
    while (*ready_queue_tail != NULL) {
        // 获取优先级最高的进程运行（替换RR调度算法的pop）
        PCB *running_process = get_highest_priority(*ready_queue_tail, ready_queue_tail);
        running_process->state = 'R';
        
        // 打印当前运行进程的PCB
        printf("时间 %d: ", current_time);
        print_current_PCB(running_process);
        
        // 模拟运行一个时间片
        running_process->usedtime += time_slice;
        current_time += time_slice;
        
        if (running_process->usedtime >= running_process->runtime) {
            // 进程完成
            running_process->state = 'F';
            printf("进程 %s 完成！\n\n", running_process->name);
            free(running_process);
        } else {
            // 进程未完成，放回队尾（优先级-1）
            running_process->state = 'W';
            running_process->priority --;
            *ready_queue_tail = push(*ready_queue_tail, running_process);
        }
        
        // 打印当前就绪队列状态
        if (*ready_queue_tail != NULL) {
            print_ready_queue(*ready_queue_tail);
        }

        printf("\n=======================================\n\n");
    }
    
    printf("\n所有进程执行完毕！\n");
}

// 从就绪队列中获取运行时间最短的进程
PCB* get_shortest_job(PCB *tail, PCB **ready_queue) {
    if (tail == NULL) {
        return NULL;
    }
    
    PCB *current = tail->next;  // 队首
    PCB *start = current;
    PCB *shortest_job = current;
    
    // 遍历整个循环队列，找到运行时间最短的进程
    do {
        if (current->runtime < shortest_job->runtime) {
            shortest_job = current;
        }
        current = current->next;
    } while (current != start);
    
    // 从队列中移除这个进程
    if (shortest_job == tail->next) {
        // 如果是队首，直接pop
        return pop(tail, ready_queue);
    } else {
        // 找到要移除进程的前一个节点
        PCB *prev = tail->next;
        while (prev->next != shortest_job) {
            prev = prev->next;
        }
        
        // 从队列中移除
        prev->next = shortest_job->next;
        
        // 如果移除的是队尾，需要更新队尾指针
        if (shortest_job == tail) {
            *ready_queue = prev;
        }
        
        shortest_job->next = NULL;
        return shortest_job;
    }
}

// SJF调度（短作业优先）
void SJF_schedule(PCB **ready_queue_tail) {
    printf("\n=== 开始短作业优先(SJF)调度 ===\n\n");

    int current_time = 0;
    
    while (*ready_queue_tail != NULL) {
        // 获取运行时间最短的进程运行
        PCB *running_process = get_shortest_job(*ready_queue_tail, ready_queue_tail);
        running_process->state = 'R';
        
        // 打印当前运行进程的PCB
        printf("时间 %d: ", current_time);
        print_current_PCB(running_process);
        
        // SJF是非抢占式的，让进程一直运行到完成
        printf("进程 %s 开始运行，需要时间 %d\n", running_process->name, running_process->runtime);
        
        // 模拟进程完整运行
        current_time += running_process->runtime;
        running_process->usedtime = running_process->runtime;
        
        // 进程完成
        running_process->state = 'F';
        printf("时间 %d: 进程 %s 完成！\n\n", current_time, running_process->name);
        free(running_process);
        
        // 打印当前就绪队列状态
        if (*ready_queue_tail != NULL) {
            print_ready_queue(*ready_queue_tail);
        }

        printf("\n=======================================\n\n");
    }
    
    printf("\n所有进程执行完毕！\n");
}