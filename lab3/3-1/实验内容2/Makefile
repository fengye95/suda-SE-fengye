# 编译器设置
CC = gcc
CFLAGS = -std=c11 -g


# 目标文件
TARGETS = referee player

# 默认目标
all: $(TARGETS)

test: $(TARGETS)
	@echo "请手动在三个终端中运行以下命令："
	@echo "终端1: ./referee 10"
	@echo "终端2: ./player"
	@echo "终端3: ./player"
	@echo ""
	@echo "或者使用以下命令自动打开终端："
	@-gnome-terminal -- ./referee 10>/dev/null &
	@sleep 2
	@-gnome-terminal -- ./player>/dev/null &
	@-gnome-terminal -- ./player>/dev/null &

# 裁判进程
referee: referee.c 
	$(CC) $(CFLAGS) referee.c -o referee 

# 玩家进程  
player: player.c
	$(CC) $(CFLAGS) player.c -o player

# 清理
clean:
	rm -f $(TARGETS) *.o


