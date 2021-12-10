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
    const char *filename = "C:\\data\\program.cst";

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

        //Token *program_tokens;
        int *token_loc = malloc(sizeof(int));


        int current_alloc = 16;
        token_loc = realloc(token_loc, current_alloc * sizeof(int));

        // for (int i = 0; i < 6000; i++){
        //     token_loc[i] = i;
        //     printf("[%d]: %d\n", i, token_loc[i]);
        // }

        int token_index = 1, j = 0;
        token_loc[0] = 0;
        while (j < len_file) {
            printf("%c\n", file_buffer[j]);
            if (file_buffer[j] == ' ') {
                if (j >= current_alloc) {
                    current_alloc += 16;
                    token_loc = realloc(token_loc, current_alloc * sizeof(int)); // dynamically allocate 16 bytes at a time
                }

                token_loc[token_index] = j;
                printf("space #%02d found at %d\n", token_index, j);
                token_index++;
            }
            j++;
        }

        //printf("tokens: %d", &token_loc);

        free(token_loc);

        /* 
        for (int i = 0; i < len_file; i++) {
            if (file_buffer[i] == ' ') {
                printf("token delim: %d\n", i);
                AppendToIntArray(&token_loc, i);
            }
        }
        AppendToIntArray(&token_loc, len_file + 1); */
    }
    return 0;
}