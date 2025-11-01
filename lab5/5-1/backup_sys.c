#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <source_file> <target_file>\n", program_name);
    fprintf(stderr, "Example: %s source.dat target.dat\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error: Invalid number of arguments.\n");
        print_usage(argv[0]);
        return 1;
    }

    const char *source_filename = argv[1];
    const char *target_filename = argv[2];

    // 检查源文件和目标文件是否同名
    if (strcmp(source_filename, target_filename) == 0) {
        fprintf(stderr, "Error: Source and target files cannot be the same.\n");
        return 1;
    }

    int source = open(source_filename, O_RDONLY);
    if (source == -1) {
        perror("Error: Failed to open source file");
        return 1;
    }

    // 创建目标文件，权限为644 (rw-r--r--)
    int target = open(target_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (target == -1) {
        perror("Error: Failed to open target file");
        close(source);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    size_t total_bytes = 0;

    while ((bytes_read = read(source, buffer, BUFFER_SIZE)) > 0) {
        ssize_t bytes_written = write(target, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Error: Failed to write to target file\n");
            close(source);
            close(target);
            return 1;
        }
        total_bytes += bytes_written;
    }

    // 检查读取是否出错
    if (bytes_read == -1) {
        perror("Error: Failed to read from source file");
        close(source);
        close(target);
        return 1;
    }

    close(source);
    close(target);

    printf("This program backup file based on Linux system call!\n");
    printf("Source: %s\n", source_filename);
    printf("Target: %s\n", target_filename);
    printf("Total bytes copied: %zu\n", total_bytes);
    printf("Success in reading source file.\n");
    printf("Success in writing target file.\n");
    printf("Success in closing both files.\n");

    return 0;
}