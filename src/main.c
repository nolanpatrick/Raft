#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stack.c"
#include "typechecking.c"
//#include "experiments.c"

#define FILE_BUF_MAX 640000 // 640 KB

char *version_string = "0.0.1";

typedef enum { // Types of tokens allowed in program
    TT_null,
    TT_comment_content,
    TT_str_content,
    TT_int,
    TT_op    
} TokenType;

typedef enum {
    op_null,
    op_add,
    op_subtract,
    op_multiply,
    op_divide,
    op_swap,
    op_dup,
    op_over,
    op_rot,
    op_drop,
    op_ipush,
    op_spush,
    op_iprint,
    op_sprint
} Operations;

typedef struct { // Program token
    TokenType T_Type;
    int   T_int;
    char *T_str;
    Operations OpType;
    int line;
    int loc;
} Token;

typedef struct { // Temporary token which holds entire line of initial file
    char *lineValue;
} lineToken;

// ===== Function signatures =====

int main(int argc, char *argv[]);

void parseTokens(char *file_buffer, int flag_verbose);

void classifyTokens(Token *Program, int token_count, int flag_verbose);

void throwError(const char *filename, int line, int token, char *message, char *operator);

void helpMessage();

// ===============================

int main(int argc, char *argv[]) {

    int flag_verbose = 0;  // Whether or not to show parsing steps
    int flag_run = 0;      // Whether or not to begin parsing and execution
    char *input_path_argv; // Path to file as given in argument

    if (argc == 1) helpMessage();

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-i") && argv[i+1] == NULL) {
            helpMessage();
            printf("\nError: please provide a path to the input file.\n");
            exit(1);
        }
        if (!strcmp(argv[i], "-i") && argv[i+1] != NULL) {
            input_path_argv = calloc(strlen(argv[i+1]), sizeof(char));
            strcpy(input_path_argv, argv[i+1]);
            flag_run = 1;
        }
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
            helpMessage();
            exit(0);
        }
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            printf("[WARN] Enabled verbose mode\n\n");
            flag_verbose = 1;
        }
    }
    if (flag_run) {

        if (flag_verbose) printf("[INFO] Reading %s\n", input_path_argv);
        FILE *fp = fopen(input_path_argv, "r");

        if (fp == NULL) {
            fprintf(stderr, "Error: Could not read file");
        } else {
            // Read file
            char *file_buffer = (char *) malloc (FILE_BUF_MAX);

            int file_index = 0;
            while (fscanf(fp, "%c", &file_buffer[file_index]) != EOF) file_index++;
            file_buffer[file_index] = '\0';

            fclose(fp);

            parseTokens(file_buffer, flag_verbose);
        }
    }
    return 0;
}

void parseTokens(char *file_buffer, int flag_verbose){
    printf("===== Starting Tokenization Stage =====\n");

    Token* Program = malloc(sizeof(Token));                       // Program tokens
    //ProgramConVar* ProgramVars = malloc(sizeof(ProgramConVar));   // Program variables

    int len_file = strlen(file_buffer);
    
    int token_count = 0;
    //int program_var_count = 0;

    int line_count = 0;
    int line_token_count = 0;

    char *line_save_ptr, *token_save_ptr;
    char *line_token_str = strtok_r(file_buffer, "\n", &line_save_ptr);
    
    while (line_token_str) {
        char *token_str = strtok_r(line_token_str, " ", &token_save_ptr);
        while (token_str) {
            Program = realloc(Program, sizeof(Token) * (token_count + 2));

            Token NewToken;
            NewToken.line = line_count;
            //NewToken.loc = line_token_count; // TODO: Fix token count around comments and strings

            if (!strcmp(token_str, "{")) { // Capture comments
                char *token_str_comment = strtok_r(NULL, "}", &token_save_ptr);

                int j = strlen(token_str_comment);
                while (strcmp(&token_str_comment[--j], " ")); // For removing trailing whitespace

                NewToken.T_Type = TT_comment_content;

                NewToken.T_str = calloc(j, sizeof(char));
                memcpy(NewToken.T_str, token_str_comment, j);
            }
            else if (!strcmp(token_str, "\"")) { // Capture strings
                char *token_str_content = strtok_r(NULL, "\"", &token_save_ptr);

                int j = strlen(token_str_content);
                while (strcmp(&token_str_content[--j], " ")); // For removing trailing whitespace

                NewToken.T_Type = TT_str_content;
                NewToken.OpType = op_spush;

                NewToken.T_str = calloc(j, sizeof(char));
                memcpy(NewToken.T_str, token_str_content, j);
            }
            else {
                NewToken.T_Type = TT_null;
                NewToken.T_str = calloc(strlen(token_str), sizeof(char));
                memcpy(NewToken.T_str, token_str, strlen(token_str));
            }

            if (flag_verbose) {
                printf(" - %d, %d:NULL Token string: [%s]\n", token_count, NewToken.line, NewToken.T_str);
            }

            Program[token_count] = NewToken;
            token_str = strtok_r(NULL, " ", &token_save_ptr);

            token_count++;
            line_token_count++;
        }
        //printf("=== Line ===\n");
        line_token_str = strtok_r(NULL, "\n", &line_save_ptr);
        line_token_count = 0;
        line_count++;
    }
    if (token_count == 0) {
        printf("Error: tokenization failed or file empty");
        exit(1);
    } else {
        classifyTokens(Program, token_count, flag_verbose);
    }
}

void classifyTokens(Token *Program, int token_count, int flag_verbose){
    printf("\n===== Starting Classification Stage =====\n");
    for (int i = 0; i < token_count; i++){

        if (!strcmp(Program[i].T_str, "+")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_add;
        } else if (!strcmp(Program[i].T_str, "-")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_subtract;
        } else if (!strcmp(Program[i].T_str, "*")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_multiply;
        } else if (!strcmp(Program[i].T_str, "/")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_divide;
        } else if (!strcmp(Program[i].T_str, "swap")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_swap;
        } else if (!strcmp(Program[i].T_str, "dup")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_dup;
        } else if (!strcmp(Program[i].T_str, "rot")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_rot;
        } else if (!strcmp(Program[i].T_str, "drop")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_drop;
        } else if (strIsNumeric(Program[i].T_str)) {
            Program[i].T_Type = TT_int;
            Program[i].OpType = op_ipush;
            Program[i].T_int = strtol(Program[i].T_str, NULL, 10);
        } else if (!strcmp(Program[i].T_str, "swap")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_swap;
        } else if (!strcmp(Program[i].T_str, "sprint")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_sprint;
        } else if (!strcmp(Program[i].T_str, "iprint")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_iprint;
        }

    }
    if (flag_verbose) {
        for (int i = 0; i < token_count; i++){
        if (Program[i].T_Type == TT_str_content)
            printf("STR, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        else if (Program[i].T_Type == TT_comment_content)
            printf("COM, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        else if (Program[i].T_Type == TT_int)
            printf("INT, '%d': %d\n", Program[i].T_int, Program[i].T_Type);
        else if (Program[i].T_Type == TT_op)
            printf("OPR, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        else
            printf("000, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        }
    }
}

void throwError(const char *filename, int line, int token, char *message, char *operator) {
    printf("%s:%d:%d Error: %s: '%s'\n", filename, line, token, message, operator);
    exit(1);
}

void helpMessage() {
    printf("Raft interpreter %s\n", version_string);
    printf("usage: raft [OPTIONS] -i <path_to_program>\n\n");
    printf("required arguments:\n");
    printf("  -i <file>       open file in interpretation mode\n");
    printf("\noptional arguments:\n");
    printf("  -v, --verbose   run program in verbose mode\n");
    printf("  -h, --help      display this help message and exit\n");
}

