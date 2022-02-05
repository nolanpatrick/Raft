#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *strtok_unsafe(char **__s, char __delim) {
    int l = strlen(*__s);
    char *s = malloc(sizeof(char) * strlen(*__s));
    strcpy(s, *__s);

    char *_return_str;
    for (int i = 0; i < l; i++) {
        if (s[i] == __delim) {
            _return_str = malloc(sizeof(char) * l);
            strncpy(_return_str, *__s, i);

            *__s = *__s + i + 1;
            return(_return_str);
        }
    }
}

// Reference implementation of strtok_unsafe
/*
int main() {

    char *test;
    test = malloc(sizeof(char) * 64);
    strcpy(test, "abc\ndef\n\nghi\n\n\nabcd eftlakjd\n");
    
    // 1. abc
    // 2. def
    // 3.
    // 4. ghi
    // 5.
    // 6.
    // 7. abcd, eftlakjd
    
    
    printf("tokenizer experiments: %s\n", test);

    int line = 1;
    int loc = 0;
    
    char *line_token_str = strtok_unsafe(&test, '\n');
    
    while (*test) {
        printf("%d:%d :: \'%s\'\n", line, loc, line_token_str);
        
        line_token_str = strtok_unsafe(&test, '\n');
        line++;
        while (strchr(line_token_str, ' ')) {
            char *token_str = strtok_unsafe(&line_token_str, ' ');
            printf("%d:%d :: \'%s\'\n", line, loc, token_str);
            loc++;
        }
        
    }
    
    if (line_token_str) printf("%d:%d :: \'%s\'\n", line, loc, line_token_str);
}
*/

