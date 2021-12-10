#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_BUF_MAX 640000 // 640 KB

typedef struct {
    char *raw;
    int *type;
    // types: 0-int, 1-operation, 2-strlit
} Token;

int main(void) {
    const char *filename = "C:\\data\\lipsum.txt";

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        fprintf(stderr, "Error: Could not read file");
        return 1;
    } else {
        printf("[INFO] Reading %s\n", filename);

        char *file_buffer = (char *) malloc (FILE_BUF_MAX);

        int len_file = 0;
        while (fscanf(fp, "%c", &file_buffer[len_file]) != EOF) len_file++;
        file_buffer[len_file] = '\0';
        fclose(fp);

        printf("%s\n", file_buffer);
        printf("token size %d\n", sizeof(Token));

        Token *program_tokens;
        int current_alloc = 16;
        int *token_loc = malloc(current_alloc * sizeof(int));

        int token_index = 1, j = 0; // j = raw file character index
        token_loc[0] = 0;
        while (j < len_file) {
            printf("%c\n", file_buffer[j]);
            if (file_buffer[j] == ' ') {

                if (j >= current_alloc) {
                    current_alloc += 16;
                    token_loc = realloc(token_loc, current_alloc * sizeof(int)); // dynamically allocate 16 bytes at a time
                    printf("[INFO] Allocated [%d bytes] of memory...", current_alloc);
                }

                token_loc[token_index] = j;
                printf("space #%02d found at %d\n", token_index, j);
                token_index++;
            }
            j++;
        }

        free(token_loc);
        return 0;
    }
}