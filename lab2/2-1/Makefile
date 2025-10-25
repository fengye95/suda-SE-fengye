# 编译器
CC = gcc

# 最终目标
TARGET = main

# 所有的目标文件
OBJS = sche.o main.o

# 默认目标
sche: $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# 使用推导规则编译 .c 文件为 .o 文件
# 这条规则会自动处理：sche.o: sche.c
#                 main.o: main.c
%.o: %.c
	$(CC) -c $< -o $@

# 清理目标
clean:
	rm -f $(OBJS) $(TARGET)
