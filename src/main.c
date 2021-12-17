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
    TT_datatype_int, 
    TT_datatype_str, 
    TT_op_noarg, 
    TT_op_arg,
    TT_declaration,
    TT_decl_str,
    TT_var,
    TT_null
} TokenType;

typedef enum { // Types of variables allowed in program
    CV_datatype_int, 
    CV_datatype_str
} CVType;

typedef struct { // Program token
    TokenType T_Type;
    union {
        int   T_int;
        char *T_str;
        char *T_op;
    };
    int line;
    int loc;
} Token;

typedef struct { // Temporary token which holds entire line of initial file
    char *lineValue;
} lineToken;

typedef enum { // Mutability state of constants and variables
    CV_mutable,
    CV_immutable
} Mutability;

typedef struct { // Variable and constant structure
    Mutability status;
    CVType CV_Type;
    char *name;
    union {
        int CV_int;
        char *CV_str;
    };
} ProgramConVar; 

void throwError(const char *filename, int line, int token, char *message, char *operator) {
    printf("%s:%d:%d Error: %s: '%s'\n", filename, line, token, message, operator);
    exit(1);
}

int tokenize(char *file_buffer,int flag_verbose){

    int len_file = strlen(file_buffer);
    
    int token_count = 0;
    int program_var_count = 0;

    Token *Program = (Token *) malloc(sizeof(Token));                               // Program tokens
    ProgramConVar *ProgramVars   = (ProgramConVar *) malloc(sizeof(ProgramConVar)); // Program variables

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
            NewToken.loc = line_token_count;

            NewToken.T_str = calloc(strlen(token_str), sizeof(char));
            memcpy(NewToken.T_str, token_str, strlen(token_str));
            if (flag_verbose) printf(" - %d, %d:%d Token string: [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_str);

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

            if (!tokenize(file_buffer, flag_verbose)) printf("Error: tokenization failed");
        }
    }
    return 0;
}