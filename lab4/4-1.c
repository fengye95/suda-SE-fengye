
#include <stdio.h>
#include <stdlib.h>

typedef unsigned long long uint64;

// 分配结果枚举
typedef enum
{
    SUCCESS = 0,
    FAIL_NO_FIT = -1,
    FAIL_INVALID = -2
} AllocResult;

// 内存块状态枚举
typedef enum
{
    FREE = 0,
    ALLOCATED = 1
} BlockStatus;

struct MemoryBlock
{
    uint64 start_addr;        // 起始地址
    uint64 size;              // 分区大小
    BlockStatus status;       // 状态
    uint64 process_id;        // 所属进程的id
    struct MemoryBlock *prev; // 前驱节点
    struct MemoryBlock *next; // 后继节点
};

// 内存管理器
struct MemoryManager
{
    struct MemoryBlock *head; // 链表头节点
    uint64 total_size;        // 内存总大小
};

// 内存请求结构
struct request
{
    int process_id; // 进程ID
    char op;        // 操作类型：'a'-分配, 'r'-释放
    uint64 size;    // 请求的内存大小（对于分配操作）
};


// 全局变量
struct MemoryManager MemoryManager; // 内存管理器



// 函数声明
struct request *load_requests(FILE *file, int request_count);
void initialize_memory(uint64 total_size);
AllocResult first_fit(uint64 size, uint64 process_id);
AllocResult best_fit(uint64 size, uint64 process_id);
AllocResult worst_fit(uint64 size, uint64 process_id);
AllocResult release_memory(uint64 process_id);
void display_memory_status();


int main()
{
    FILE *file = fopen("data/4-1.txt", "r");
    if (!file)
    {
        fprintf(stderr, "错误：无法打开文件 data/4-1.txt\n");
        return 1;
    }

    // 读取配置信息
    uint64 total_memory;
    int request_count;

    if (fscanf(file, "%llu", &total_memory) != 1)
    {
        fprintf(stderr, "错误：读取内存大小失败\n");
        fclose(file);
        return 1;
    }

    if (fscanf(file, "%d", &request_count) != 1)
    {
        fprintf(stderr, "错误：读取请求数量失败\n");
        fclose(file);
        return 1;
    }

    printf("=== 内存管理模拟器 ===\n");
    printf("总内存大小: %llu KB\n", total_memory);
    printf("请求数量: %d\n", request_count);

    // 初始化内存
    initialize_memory(total_memory);

    // 加载请求序列
    struct request *requests = load_requests(file, request_count);
    fclose(file);

    if (!requests)
    {
        fprintf(stderr, "错误：加载请求序列失败\n");
        return 1;
    }

    // 选择分配算法
    int algorithm_choice;
    printf("\n请选择分配算法:\n");
    printf("1. 首次适应算法 (First Fit)\n");
    printf("2. 最佳适应算法 (Best Fit)\n");
    printf("3. 最坏适应算法 (Worst Fit)\n");
    printf("请输入选择 (1-3): ");

    scanf("%d", &algorithm_choice);
    AllocResult (*fit)(uint64, uint64);

    // 设置函数指针
    switch (algorithm_choice)
    {
    case 1:
        fit = first_fit;
        printf("\n使用算法: 首次适应算法\n");
        break;
    case 2:
        fit = best_fit;
        printf("\n使用算法: 最佳适应算法\n");
        break;
    case 3:
        fit = worst_fit;
        printf("\n使用算法: 最坏适应算法\n");
        break;
    default:
        fprintf(stderr, "\n错误：无效的算法选择\n");
        exit(1);
    }

    printf("\n开始处理请求序列...\n");

    // 处理每个请求
    for (int i = 0; i < request_count; i++)
    {
        printf("\n>>> 处理请求 %d: 进程%d %c ",
               i + 1, requests[i].process_id, requests[i].op);

        if (requests[i].op == 'a')
            printf("%lluKB\n", requests[i].size);
        else
            printf("(释放内存)\n");

        AllocResult result;

        if (requests[i].op == 'a')
        {
            // 分配内存请求
            result = fit(requests[i].size, requests[i].process_id);

            if (result == SUCCESS)
            {
                printf("  分配成功 - 进程%d 获得 %lluKB 内存\n",
                       requests[i].process_id, requests[i].size);
            }
            else
            {
                printf("  分配失败 - 无法为进程%d 分配 %lluKB 内存\n",
                       requests[i].process_id, requests[i].size);
            }
        }
        else if (requests[i].op == 'r')
        {
            // 释放内存请求
            result = release_memory(requests[i].process_id);
            if (result == SUCCESS)
            {
                printf("  释放成功 - 进程%d 的内存已释放\n", requests[i].process_id);
            }
            else
            {
                printf("  释放失败 - 未找到进程%d 的已分配内存\n", requests[i].process_id);
            }
        }
        else
        {
            fprintf(stderr, "错误：未知操作类型 '%c'\n", requests[i].op);
            continue;
        }

        // 实时显示当前内存状态
        display_memory_status();
    }

    // 显示最终统计信息
    printf("\n=== 模拟完成 ===\n");
    printf("总共处理了 %d 个请求\n", request_count);
    printf("使用的算法: ");
    switch (algorithm_choice)
    {
    case 1:
        printf("首次适应算法\n");
        break;
    case 2:
        printf("最佳适应算法\n");
        break;
    case 3:
        printf("最坏适应算法\n");
        break;
    }

    // 最终内存状态
    display_memory_status();

    // 清理资源
    free(requests);

    // 清理内存链表
    struct MemoryBlock *current = MemoryManager.head;
    while (current != NULL)
    {
        struct MemoryBlock *next = current->next;
        free(current);
        current = next;
    }

    printf("程序执行完毕\n");
    return 0;
}

/**
 * 从已打开的文件加载内存请求序列
 * @param file 已打开的文件指针（指向请求序列开始位置）
 * @param request_count 请求数量
 *
 * @return 请求数组指针，失败返回NULL
 */
struct request *load_requests(FILE *file, int request_count)
{
    if (!file || request_count <= 0)
    {
        return NULL;
    }

    // 分配内存
    struct request *requests = malloc(request_count * sizeof(struct request));
    if (!requests)
    {
        fprintf(stderr, "Error: Memory allocation failed for requests\n");
        return NULL;
    }

    // 读取请求序列
    for (int i = 0; i < request_count; i++)
    {
        int pid;
        char op;
        uint64 size;

        // 解析请求行：进程ID 操作类型 大小
        if (fscanf(file, "%d %c %llu", &pid, &op, &size) >= 2)
        {
            requests[i].process_id = pid;
            requests[i].op = op;
            requests[i].size = size;
        }
        else
        {
            fprintf(stderr, "Error: Invalid request format at line %d\n", i);
            free(requests);
            return NULL;
        }
    }

    return requests;
}

void initialize_memory(uint64 total_size)
{
    // 初始化内存管理器
    MemoryManager.total_size = total_size;
    MemoryManager.head = NULL;

    // 创建初始的内存块（整个内存作为一个空闲块）
    struct MemoryBlock *initial_block = malloc(sizeof(struct MemoryBlock));
    if (!initial_block)
    {
        fprintf(stderr, "Error: Failed to allocate initial memory block\n");
        return;
    }

    initial_block->start_addr = 0;
    initial_block->size = total_size;
    initial_block->status = FREE;
    initial_block->process_id = 0;
    initial_block->prev = NULL;
    initial_block->next = NULL;

    MemoryManager.head = initial_block;

    printf("Memory initialized with total size: %llu\n", total_size);
}


/**
 * 首次适应算法
 * @param size 请求大小
 * @return 分配结果
 */
AllocResult first_fit(uint64 size, uint64 process_id)
{
    struct MemoryBlock *current = MemoryManager.head;

    while (current != NULL)
    {
        if (current->status == FREE && current->size >= size)
        {
            // 找到合适分区，进行分配
            if (current->size > size)
            {
                // 拆分分区
                struct MemoryBlock *new_block = malloc(sizeof(struct MemoryBlock));
                new_block->start_addr = current->start_addr + size;
                new_block->size = current->size - size;
                new_block->status = FREE;
                new_block->process_id = 0;

                // 插入新分区到链表
                new_block->prev = current;
                new_block->next = current->next;
                if (current->next != NULL)
                {
                    current->next->prev = new_block;
                }
                current->next = new_block;

                // 调整当前分区大小
                current->size = size;
            }

            current->status = ALLOCATED;
            current->process_id = process_id;
            return SUCCESS;
        }
        current = current->next;
    }

    return FAIL_NO_FIT;
}

/**
 * 最佳适应算法
 * @param size 请求大小
 * @return 分配结果
 */
AllocResult best_fit(uint64 size, uint64 process_id)
{
    struct MemoryBlock *current = MemoryManager.head;
    struct MemoryBlock *best_block = NULL;

    while (current != NULL)
    {
        if (current->status == FREE && current->size >= size)
        {
            if (best_block == NULL || current->size < best_block->size)
            {
                best_block = current;
            }
        }
        current = current->next;
    }

    if (best_block != NULL)
    {
        // 分配最佳分区
        if (best_block->size > size)
        {
            // 拆分分区
            struct MemoryBlock *new_block = malloc(sizeof(struct MemoryBlock));
            new_block->start_addr = best_block->start_addr + size;
            new_block->size = best_block->size - size;
            new_block->status = FREE;
            new_block->process_id = 0;

            new_block->prev = best_block;
            new_block->next = best_block->next;
            if (best_block->next != NULL)
            {
                best_block->next->prev = new_block;
            }
            best_block->next = new_block;

            best_block->size = size;
        }

        best_block->status = ALLOCATED;
        best_block->process_id = process_id;
        return SUCCESS;
    }

    return FAIL_NO_FIT;
}

/**
 * 最坏适应算法
 * @param size 请求大小
 * @return 分配结果
 */
AllocResult worst_fit(uint64 size, uint64 process_id)
{
    struct MemoryBlock *current = MemoryManager.head;
    struct MemoryBlock *worst_block = NULL;

    while (current != NULL)
    {
        if (current->status == FREE && current->size >= size)
        {
            if (worst_block == NULL || current->size > worst_block->size)
            {
                worst_block = current;
            }
        }
        current = current->next;
    }

    if (worst_block != NULL)
    {
        // 分配最大分区
        if (worst_block->size > size)
        {
            // 拆分分区
            struct MemoryBlock *new_block = malloc(sizeof(struct MemoryBlock));
            new_block->start_addr = worst_block->start_addr + size;
            new_block->size = worst_block->size - size;
            new_block->status = FREE;
            new_block->process_id = 0;

            new_block->prev = worst_block;
            new_block->next = worst_block->next;
            if (worst_block->next != NULL)
            {
                worst_block->next->prev = new_block;
            }
            worst_block->next = new_block;

            worst_block->size = size;
        }

        worst_block->status = ALLOCATED;
        worst_block->process_id = process_id;
        return SUCCESS;
    }

    return FAIL_NO_FIT;
}

/**
 * 释放指定进程的内存分区
 * @param process_id 进程ID
 * @return 操作结果
 */
AllocResult release_memory(uint64 process_id)
{
    struct MemoryBlock *current = MemoryManager.head;
    int found = 0;

    while (current != NULL)
    {
        if (current->status == ALLOCATED && current->process_id == process_id)
        {
            current->status = FREE;
            current->process_id = 0;
            found = 1;

            // 合并相邻的空闲分区
            // 向后合并
            if (current->next != NULL && current->next->status == FREE)
            {
                current->size += current->next->size;
                struct MemoryBlock *to_remove = current->next;
                current->next = to_remove->next;
                if (to_remove->next != NULL)
                {
                    to_remove->next->prev = current;
                }
                free(to_remove);
            }

            // 向前合并
            if (current->prev != NULL && current->prev->status == FREE)
            {
                current->prev->size += current->size;
                current->prev->next = current->next;
                if (current->next != NULL)
                {
                    current->next->prev = current->prev;
                }
                struct MemoryBlock *to_remove = current;
                current = current->prev;
                free(to_remove);
            }
        }
        current = current->next;
    }

    return found ? SUCCESS : FAIL_INVALID;
}

/**
 * 显示内存状态
 */
void display_memory_status()
{
    printf("\n=== 内存状态 ===\n");
    printf("总内存大小: %llu KB\n", MemoryManager.total_size);

    printf("\n--- 已分配分区表 ---\n");
    printf("起始地址\t大小\t进程ID\n");

    struct MemoryBlock *current = MemoryManager.head;
    int allocated_count = 0;
    uint64 allocated_total = 0;

    while (current != NULL)
    {
        if (current->status == ALLOCATED)
        {
            printf("0x%08llx\t%llu\t%llu\n",
                   current->start_addr, current->size, current->process_id);
            allocated_count++;
            allocated_total += current->size;
        }
        current = current->next;
    }
    if (allocated_count == 0)
    {
        printf("无已分配分区\n");
    }

    printf("\n--- 空闲分区表 ---\n");
    printf("起始地址\t大小\n");

    current = MemoryManager.head;
    int free_count = 0;
    uint64 free_total = 0;

    while (current != NULL)
    {
        if (current->status == FREE)
        {
            printf("0x%08llx\t%llu\n", current->start_addr, current->size);
            free_count++;
            free_total += current->size;
        }
        current = current->next;
    }
    if (free_count == 0)
    {
        printf("无空闲分区\n");
    }

    printf("\n--- 统计信息 ---\n");
    printf("内存利用率: %.2f%%\n", (double)allocated_total / MemoryManager.total_size * 100);
    printf("已分配分区数: %d, 空闲分区数: %d\n", allocated_count, free_count);
    printf("=================================\n\n");
}
