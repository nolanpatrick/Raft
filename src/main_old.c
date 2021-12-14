#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stack.c"
#include "typechecking.c"
//#include "experiments.c"

#define FILE_BUF_MAX 640000 // 640 KB
#define TOKEN_LEN_MAX 100    // 100 characters max per token (will probably need to change in the future)

typedef enum {
    TT_datatype_int, 
    TT_datatype_float, 
    TT_datatype_str, 
    TT_op_noarg, 
    TT_op_unary, 
    TT_op_binary,
    TT_declaration,
    TT_null
} TokenType;

typedef struct {
    TokenType T_Type;
    char *val;
    int len;
    int line;
} Token;

void throwError(const char *filename, int line, char *message, char *operator) {
    printf("%s:%d: Error: %s: %s\n", filename, line, message, operator);
}

int main(void) {
    const char *filename = "test/program2.n";

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


        int limit_tokens = 10; // May be dynamically increased and reallocated as necessary
        Token *program = (Token *) malloc(sizeof(Token) * limit_tokens); // Store program tokens

        int num_tokens = 0, prev_delim = 0;
        int line_count = 1;
        for (int i = 0; i < len_file; i++) {
            if (file_buffer[i] > 1 && file_buffer[i] < 127) {
                if (file_buffer[i] == '\n') line_count++;
                if (file_buffer[i] != '\n' && file_buffer[i] != '\r' && file_buffer[i] != ' ') {
                    if (file_buffer[i+1] == ' ' || file_buffer[i+1] == '\n' || file_buffer[i+1] == '\r' || file_buffer[i+1] == '\0') {
                        Token current_token;
                        current_token.len = i + 1 - prev_delim;
                        current_token.line = line_count;

                        current_token.val = (char *) calloc(TOKEN_LEN_MAX, sizeof(char)); // allocate memory for current token value
                        memcpy(current_token.val, &file_buffer[prev_delim], i+1-prev_delim);

                        program[num_tokens] = current_token;
                        //printf("Found token: \'%s\' of length: %d on line: %d\n", program[num_tokens].val, program[num_tokens].len, line_count);
                        num_tokens++;
                    }
                }
            }

            if (file_buffer[i] == ' ' || file_buffer[i] == '\n' || file_buffer[i] == '\r')
                prev_delim = i + 1;

            if (num_tokens >= limit_tokens) {
                limit_tokens += 10;
                program = realloc(program, sizeof(Token) * limit_tokens);
            }
        }

        //strIsNumeric(program[0].val);
        printf("========================================%d\n", num_tokens);
        for (int i = 0; i < num_tokens; i++) {
            printf("Token: %s\n", program[i].val);
            if (strIsNumeric(program[i].val)){
                printf("** Numeric Token: %s\n", program[i].val);
                program[i].T_Type = TT_int;
                pushNumToStack(strtol(program[i].val, NULL, 10));
            }
            else if (strIsFloatNumeric(program[i].val)){
                printf("** Float Numeric Token: %s\n", program[i].val);
                program[i].T_Type = TT_int;
                pushNumToStack(strtof(program[i].val, NULL));
            }
            else if (strIsStringLiteral(program[i].val)){
                printf("** String Literal Token: %s\n", program[i].val);
                program[i].T_Type = TT_str;
                pushStrToStack(program[i].val);
            }
            else if (strIsArithmeticOperator(program[i].val)){
                printf("** Arithmetic Token: %s\n", program[i].val);
                program[i].T_Type = TT_binary_op;
                if (MainStack.height <= 1) {
                    throwError(filename, program[i].line, "Stack contents insufficient for operation", program[i].val);
                }
                if (!strcmp(program[i].val, "+")) {
                    float v1 = popFromStack();
                    float v2 = popFromStack();
                    pushNumToStack(v1 + v2);
                }
            }
            else if (strIsBooleanShort(program[i].val)){
                printf("** Boolean Short Token: %s\n", program[i].val);
                program[i].T_Type = TT_binary_op;
            }
            else if (strIsBooleanLong(program[i].val)){
                printf("** Boolean Long Token: %s\n", program[i].val);
                program[i].T_Type = TT_binary_op;
            }
            else if (strIsGenericOperator(program[i].val)){
                printf("** Generic Operator Token: %s\n", program[i].val);
                program[i].T_Type = TT_unary_op;
            }
            else if (strIsConstVarDeclaration(program[i].val)){
                printf("** Const Init Token: %s\n", program[i].val);
                program[i].ttype = 3;
            }
            else if (i > 0 && program[i-1].ttype == 3) {
                printf("** Const Dec Token: %s\n", program[i].val);
            }
            else {
                printf("** Unknown Token: %s\n", program[i].val);
            }
        }

        // Stack MainStack declared in "stack.c"
/* 
        for (int i = 0; i < num_tokens; i++) {
            if (program[i].ttype == 0) { // type: int
                pushIntToStack(program[i].val);
            }
            if (program[i].ttype == 1) { // type: operator
                if (program)
            }
            if (program[i].ttype == 2) { // type: string literal

            }
            if (program[i].ttype == 3) { // type: const declaration

            }
        } */

        return 0;
    }
}