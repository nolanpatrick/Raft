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
    TT_datatype_float, 
    TT_datatype_str, 
    TT_op_noarg, 
    TT_op_unary, 
    TT_op_binary,
    TT_declaration,
    TT_decl_str,
    TT_var,
    TT_null
} TokenType;

typedef enum { // Types of tokens allowed in program
    CV_datatype_int, 
    CV_datatype_float, 
    CV_datatype_str
} CVType;

typedef struct { // Program token
    TokenType T_Type;
    union {
        int   T_int;
        float T_float;
        char *T_str;
        char *T_op;
    };
    int line;
    int loc;
} Token;

typedef struct { // Temporary token which holds entire line of file
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
        float CV_float;
        char *CV_str;
    };
} ProgramConVar;

void throwError(const char *filename, int line, int token, char *message, char *operator) {
    printf("%s:%d:%d Error: %s: '%s'\n", filename, line, token, message, operator);
    exit(1);
}

int interpret_program(char *filename, int flag_verbose){
    //const char *filename = ".\\test\\program2.n";

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
        
        
        int token_count = 0;
        int program_var_count = 0;

        Token *Program = (Token *) malloc(sizeof(Token));                               // Program tokens
        ProgramConVar *ProgramVars   = (ProgramConVar *) malloc(sizeof(ProgramConVar)); // Program variables

        // BEGIN TOKENIZATION

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

                if (strIsNumeric(token_str)) {
                    NewToken.T_Type = TT_datatype_int;
                    NewToken.T_int = strtol(token_str, NULL, 10);
                    if (flag_verbose) printf(" - %d, %d:%d Token INT [%d]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_int);
                } 
                else if (strIsFloatNumeric(token_str)) {
                    NewToken.T_Type = TT_datatype_float;
                    NewToken.T_float = strtof(token_str, NULL);
                    if (flag_verbose) printf(" - %d, %d:%d Token FLT [%.2f]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_float);
                }
                else if (strIsStringLiteral(token_str)){
                    NewToken.T_Type = TT_datatype_str;
                    NewToken.T_str = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewToken.T_str, token_str, strlen(token_str));
                    if (flag_verbose) printf(" - %d, %d:%d Token STR [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_str);
                }
                else if (strIsArithmeticOperator(token_str)){
                    NewToken.T_Type = TT_op_binary;
                    NewToken.T_op = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewToken.T_op, token_str, strlen(token_str));
                    if (flag_verbose) printf(" - %d, %d:%d Token ART [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_op);
                }
                else if (strIsBooleanShort(token_str)){
                    NewToken.T_Type = TT_op_binary;
                    NewToken.T_op = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewToken.T_op, token_str, strlen(token_str));
                    if (flag_verbose) printf(" - %d, %d:%d Token BLS [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_op);
                }
                else if (strIsBooleanLong(token_str)){
                    NewToken.T_Type = TT_op_binary;
                    NewToken.T_op = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewToken.T_op, token_str, strlen(token_str));
                    if (flag_verbose) printf(" - %d, %d:%d Token BLL [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_op);
                }
                else if (strIsGenericOperator(token_str)){
                    NewToken.T_Type = TT_op_unary;
                    NewToken.T_op = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewToken.T_op, token_str, strlen(token_str));
                    if (flag_verbose) printf(" - %d, %d:%d Token GEN [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_op);
                }
                else if (strIsConstVarDeclaration(token_str)){
                    NewToken.T_Type = TT_declaration;
                    NewToken.T_op = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewToken.T_op, token_str, strlen(token_str));
                    if (flag_verbose) printf(" - %d, %d:%d Token DEC [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_op);
                }
                else if (Program[token_count-1].T_Type == TT_declaration) {
                    NewToken.T_Type = TT_decl_str;
                    NewToken.T_str = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewToken.T_str, token_str, strlen(token_str));
                    if (flag_verbose) printf(" - %d, %d:%d Token DCN [%s]\n", token_count, NewToken.line, NewToken.loc, NewToken.T_str);

                    ProgramConVar NewCV;

                    if (!strcmp(Program[token_count-1].T_str, "const")) NewCV.status = CV_immutable;
                    else if (!strcmp(Program[token_count-1].T_str, "var")) NewCV.status = CV_mutable;
                    else printf("Unreachable");

                    NewCV.name = calloc(strlen(token_str), sizeof(char));
                    memcpy(NewCV.name, token_str, strlen(token_str));
                    switch (Program[token_count-2].T_Type){
                        case (TT_datatype_int):
                            NewCV.CV_int = Program[token_count-2].T_int;
                            NewCV.CV_Type = CV_datatype_int;
                            break;
                        case (TT_datatype_float): 
                            NewCV.CV_float = Program[token_count-2].T_float;
                            NewCV.CV_Type = CV_datatype_float;
                            break;
                        case (TT_datatype_str):
                            NewCV.CV_str = calloc(strlen(Program[token_count-2].T_str), sizeof(char));
                            memcpy(NewCV.CV_str, Program[token_count-2].T_str, strlen(Program[token_count-2].T_str));
                            NewCV.CV_Type = CV_datatype_str;
                            break;
                        default:
                            throwError(filename, line_count+1, line_token_count, "Invalid variable or const type", token_str);
                            break;
                    }
                    ProgramVars[program_var_count++] = NewCV;
                    ProgramVars = realloc(ProgramVars, sizeof(ProgramConVar) * (program_var_count+2));
                }

                else {
                    NewToken.T_Type = TT_null;
                    int varFound = 0;
                    for (int i = 0; i < program_var_count; i++){
                        if (!strcmp(ProgramVars[i].name, token_str)) {
                            varFound = 1;
                            switch (ProgramVars[i].CV_Type) {
                                case (CV_datatype_int):
                                    if (flag_verbose) printf(" - %d, %d:%d Token VAR [%s, %d]\n", token_count, NewToken.line, NewToken.loc, ProgramVars[i].name, ProgramVars[i].CV_int);
                                    break;
                                case (CV_datatype_float):
                                    if (flag_verbose) printf(" - %d, %d:%d Token VAR [%s, %.2f]\n", token_count, NewToken.line, NewToken.loc, ProgramVars[i].name, ProgramVars[i].CV_float);
                                    break;
                                case (CV_datatype_str):
                                    if (flag_verbose) printf(" - %d, %d:%d Token VAR [%s, %s]\n", token_count, NewToken.line, NewToken.loc, ProgramVars[i].name, ProgramVars[i].CV_str);
                                    break;
                            }
                            break;
                        }
                    } 
                    if (!varFound) {
                        if (flag_verbose) printf(" - %d, %d:%d Token NUL [%s] ***\n", token_count, NewToken.line, NewToken.loc, token_str);
                        throwError(filename, line_count+1, line_token_count, "Unknown keyword or undeclared variable", token_str);
                    }
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
    }
}

void helpMessage(){
    printf("North interpreter %s\n", version_string);
    printf("usage: north [OPTIONS] -i <path_to_program>\n\n");
    printf("required arguments:\n");
    printf("  -i <file>       open file in interpretation mode\n");
    printf("\noptional arguments:\n");
    printf("  -v, --verbose   run program in verbose mode\n");
    printf("  -h, --help      display this help message and exit\n");
}

int main(int argc, char *argv[]) {

    int flag_verbose = 0;
    int flag_interpret = 0;
    char *input_path;

    if (argc == 1) helpMessage();

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-i") && argv[i+1] == NULL) {
            helpMessage();
            printf("\nError: please provide a path to the input file.\n");
            exit(1);
        }
        if (!strcmp(argv[i], "-i") && argv[i+1] != NULL) {
            input_path = calloc(strlen(argv[i+1]), sizeof(char));
            strcpy(input_path, argv[i+1]);
            flag_interpret = 1;
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
    if (flag_interpret) {
        interpret_program(input_path, flag_verbose);
    }
    return 0;
}