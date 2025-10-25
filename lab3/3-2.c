#define _XOPEN_SOURCE 500 // 为了启用usleep() 微秒级的暂停
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 5

// 宏定义： 将不太认识的函数名 换成 熟悉的PV操作
#define P sem_wait
#define V sem_post

// 缓冲区
int buffer[BUFFER_SIZE];
int in = 0;  // 生产者放入位置
int out = 0; // 消费者取出位置

// 三个信号量
sem_t empty; // 空缓冲区数量，初始为BUFFER_SIZE
sem_t full;  // 满缓冲区数量，初始为0
sem_t mutex; // 互斥锁，初始为1

/*
信号量机制在生产者-消费者问题中的应用：

为什么需要三个信号量，而不是仅用一个mutex？

【核心原因】：分离互斥与同步关注点

1. 互斥问题 (mutex解决)
   - 保证任何时候只有一个线程访问缓冲区
   - 防止生产者和消费者同时修改缓冲区导致的竞态条件

2. 同步问题 (empty和full解决先后顺序问题)
   - empty: 控制生产者执行条件 - "有空间才能生产"
   - full:  控制消费者执行条件 - "有产品才能消费"

【单个mutex的局限性】：
   如果只用mutex，线程在条件不满足时（如缓冲区满/空）必须：
   - 先释放mutex让其他线程运行
   - 轮询检查条件，浪费CPU时间
   - 重新获取mutex才能操作
   这会导致"忙等待"和频繁的线程切换开销

【三个信号量的优势】：
   empty和full利用PV操作的阻塞特性：
   - P(empty): 若无空位，生产者自动阻塞，不消耗CPU
   - V(full):  生产后自动唤醒可能阻塞的消费者
   - 类似的，消费者通过P(full)/V(empty)实现同步
   - mutex只负责短暂的临界区保护

【信号量PV操作的本质】：
   P操作：原子地"测试并减1"，若值≤0则阻塞
   V操作：原子地"加1并测试"，若值≤0则唤醒
   这种机制天然适合表达资源数量的同步关系
*/

// 生产者线程
void *producer(void *arg)
{
    int item = 0;

    while (1)
    {
        // 生产产品
        item = rand() % 100;

        // P(empty) - 等待空缓冲区
        P(&empty);

        // P(mutex) - 获取互斥锁
        P(&mutex);

        // 临界区：放入产品
        buffer[in] = item;
        printf("生产者: 产品 %d 放入位置 %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;

        // V(mutex) - 释放互斥锁
        V(&mutex);

        // V(full) - 增加满缓冲区计数
        V(&full);

        usleep(100 * 1000);; // 模拟生产时间 0.1s
    }
    return NULL;
}

// 消费者线程
void *consumer(void *arg)
{
    int item;

    while (1)
    {
        // P(full) - 等待有产品
        P(&full);

        // P(mutex) - 获取互斥锁
        P(&mutex);

        // 临界区：取出产品
        item = buffer[out];
        printf("消费者: 产品 %d 从位置 %d 取出\n", item, out);
        out = (out + 1) % BUFFER_SIZE;

        // V(mutex) - 释放互斥锁
        V(&mutex);

        // V(empty) - 增加空缓冲区计数
        V(&empty);

        usleep(500 * 1000); // 模拟消费时间 0.5s
    }
    return NULL;
}

int main()
{
    pthread_t prod_thread, cons_thread;

    // 初始化信号量
    sem_init(&empty, 0, BUFFER_SIZE); // 初始空位=5
    sem_init(&full, 0, 0);            // 初始产品=0
    sem_init(&mutex, 0, 1);           // 互斥锁=1

    printf("=== 生产者-消费者模拟开始 ===\n");
    printf("缓冲区大小: %d\n", BUFFER_SIZE);
    printf("生产者耗时: 0.1秒\n");
    printf("消费者耗时: 0.5秒\n");
    printf("模拟总时间: 3秒\n");
    printf("============================\n\n");

    // 创建生产者线程
    pthread_create(&prod_thread, NULL, producer, NULL);

    // 创建消费者线程
    pthread_create(&cons_thread, NULL, consumer, NULL);

    // 让两线程运行10s
    sleep(3);

    // 强制线程退出
    pthread_cancel(prod_thread);    // 喊"停！"给生产者
    pthread_cancel(cons_thread);    // 喊"停！"给消费者

    // 等待线程结束（本代码线程是无限循环， 所以通过上方代码强制结束）
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    // 销毁信号量
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);


    return 0;
}