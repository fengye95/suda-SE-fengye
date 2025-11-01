#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <source_file> <target_file>\n", program_name);
    fprintf(stderr, "Example: %s source.txt target.txt\n", program_name);
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

    FILE *source = fopen(source_filename, "rb");
    if (source == NULL) {
        perror("Error: Failed to open source file");
        return 1;
    }

    FILE *target = fopen(target_filename, "wb");
    if (target == NULL) {
        perror("Error: Failed to open target file");
        fclose(source);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t total_bytes = 0;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, source)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, target);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Error: Failed to write to target file\n");
            fclose(source);
            fclose(target);
            return 1;
        }
        total_bytes += bytes_written;
    }

    // 检查读取是否出错
    if (ferror(source)) {
        fprintf(stderr, "Error: Failed to read from source file\n");
        fclose(source);
        fclose(target);
        return 1;
    }

    fclose(source);
    fclose(target);

    printf("This program backup file based on C Library!\n");
    printf("Source: %s\n", source_filename);
    printf("Target: %s\n", target_filename);
    printf("Total bytes copied: %zu\n", total_bytes);
    printf("Success in reading source file.\n");
    printf("Success in writing target file.\n");
    printf("Success in closing both files.\n");

    return 0;
}