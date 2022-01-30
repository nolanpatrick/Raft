#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "rll/RLL.h"
#include "rll/RLL_PTR.h"
#include "lib/string_tests.c"

#define FILE_BUF_MAX 640000 // 640 KB
#define STACK_MAX 10240

char *version_string = "0.5.0";
char global_filepath[1024];

typedef struct { // Program token
    int   T_int;
    char *T_str;
    int line;
    int loc;
    Operations op;
} Token;

typedef struct {
    Operations op;
    char * word;
} keyword;

const keyword reserved[] = {
    {op_null,      "null"},
    {op_func,      "func"},
    // {op_func_decl, ""},
    // Function declaration is declared during parsing stage 1
    {op_func_start, ":"},
    {op_return,    "end"},
    {op_add,       "+"},
    {op_subtract,  "-"},
    {op_multiply,  "*"},
    {op_divide,    "/"},
    {op_gt,        ">"},
    {op_lt,        "<"},
    {op_eq,        "="},
    {op_and,       "and"},
    {op_or,        "or"},
    {op_swap,      "swap"},
    {op_dup,       "dup"},
    {op_over,      "over"},
    {op_rot,       "rot"},
    {op_drop,      "drop"},
    // {op_ipush,     ""},
    // {op_spush,     ""},
    // Push declared during parsing stage 1
    {op_iprint,    "iprint"},
    {op_sprint,    "sprint"},
    {op_cr,        "cr"},
    {op_nbsp,      "nbsp"},
    // {op_anchor,    ":"},
    // {op_goto,      "goto"},
    // Goto & Anchor instructions are deprecated as of v0.5.0, and are left
    // here for completeness only.
    {op_do,        "do"},
    {op_while,     "while"},
    {op_if,        "if"},
    {op_fi,        "fi"},
    {op_dstack,    "dstack"},
};

Operations get_op(char * kw) {
    // This idea is borrowed from StackOverflow user 'plinth'

    const int num_keywords = sizeof(reserved) / sizeof(keyword);
    for (int i = 0; i < num_keywords; i++) {
        if (!strcmp(kw, reserved[i].word)) {
            return(reserved[i].op);
        }
    }
    return(op_null);
}

// ===== Function signatures =====

int main(int argc, char *argv[]);

void parseTokens(char *file_buffer, int flag_debug);

void parse_file(char * file_buffer, int flag_buffer);

void build_program(Token *Program, int token_count, int flag_debug);

void interpretProgram(Token *Program, int token_count, int flag_debug);

void throwError(const char *filename, int line, int token, char *message, char *operator);

void helpMessage();

// ======================================================================================

int main(int argc, char *argv[]) {

    int flag_debug = 0;    // Show parsing stages and debug info if enabled
    int flag_run = 0;      // Whether or not to begin parsing and execution

    if (argc == 1) helpMessage();

    for (int i = 0; i < argc; i++) {
        // TODO: Restructure argv parsing to be more reliable and consistent
        if (!strcmp(argv[i], "-r") && argv[i+1] == NULL) {
            helpMessage();
            printf("\nError: please provide a path to the input file.\n");
            exit(1);
        }
        if (!strcmp(argv[i], "-r") && argv[i+1] != NULL) {
            strcpy(global_filepath, argv[i+1]);
            flag_run = 1;
        }
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
            helpMessage();
            exit(0);
        }
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
            printf("[WARN] Enabled debugging mode\n");
            flag_debug = 1;
        }

    }
    if (flag_run) {

        if (flag_debug) printf("[INFO] Reading %s\n", global_filepath);
        FILE *fp = fopen(global_filepath, "r");

        if (fp == NULL) {
            printf("Error: could not read file: \'%s\'", global_filepath);
        } else {
            // Read file
            char *file_buffer = (char *) malloc (FILE_BUF_MAX);

            int file_index = 0;
            while (fscanf(fp, "%c", &file_buffer[file_index]) != EOF) file_index++;
            file_buffer[file_index] = '\0';

            fclose(fp);

            parse_file(file_buffer, flag_debug);
        }
    }
    return 0;
}

void parse_file(char * file_buffer, int flag_debug) {
    if (flag_debug) printf(" *** Tokenization stage ***\n");

    Token raw_program[1024];

    int len_file = strlen(file_buffer);

    int token_count = 0;
    int line_count = 0;
    int line_token_count = 0;
    
    char * line_save_ptr, * token_save_ptr;

    char * line_token_string = strtok_r(file_buffer, "\n", &line_save_ptr);

    while(line_token_string) {
        line_count++;
        char * token_string = strtok_r(line_token_string, " ", &token_save_ptr);

        while (token_string) {
            Token new_token;
            new_token.line = line_count;
            new_token.loc = line_token_count;

            if (!strcmp(token_string, "(")) { // Capture comment as a single token
                char * token_string_comment = strtok_r(NULL, ")", &token_save_ptr);
                
                if (token_string_comment == NULL) {
                    throwError(global_filepath, new_token.line, new_token.loc, "missing ')' or malformed comment", "(");
                } else {
                    // Found valid comment text
                    int j = strlen(token_string_comment);
                    while (strcmp(&token_string_comment[--j], " ")); // Remove trailing whitespace
                    token_string_comment[j] = '\0';

                    new_token.T_str = malloc(strlen(token_string_comment));
                    strcpy(new_token.T_str, token_string_comment);
                    new_token.op = op_comment;

                    printf(" [x] Instruction: '%s', %d\n", token_string_comment, new_token.op);
                }
            }

            else if (!strcmp(token_string, "\"")) { // Capture string literal as a single token
                char * token_string_strlit = strtok_r(NULL, "\"", &token_save_ptr);

                if (token_string_strlit == NULL) {
                    throwError(global_filepath, new_token.line, new_token.loc, "missing '\"' or malformed string literal", "\"");
                } else {
                    // Found valid string text
                    int j = strlen(token_string_strlit);
                    while (strcmp(&token_string_strlit[--j], " ")); // Remove trailing whitespace
                    token_string_strlit[j] = '\0';

                    new_token.T_str = malloc(strlen(token_string_strlit));
                    strcpy(new_token.T_str, token_string_strlit);
                    new_token.op = op_spush;

                    printf(" [x] Instruction: '%s', %d\n", token_string_strlit, new_token.op);
                }
            }

            else {
                new_token.T_str = malloc(strlen(token_string));
                strcpy(new_token.T_str, token_string);

                new_token.op = get_op(token_string);

                if (new_token.op == op_null) {
                    if (strIsNumeric(token_string)) {
                        new_token.op = op_ipush;
                    } else if (raw_program[token_count].op == op_func) {
                        new_token.op = op_func_decl;
                    } else {
                        printf(" [ ] Unknown token: '%s'\n", token_string);
                    }
                }

                printf(" [x] Instruction: '%s', %d\n", token_string, new_token.op);
            }

            raw_program[token_count] = new_token;

            token_string = strtok_r(NULL, " ", &token_save_ptr);

            //line_count++;
            line_token_count++;
            token_count++;
        }

        line_token_string = strtok_r(NULL, "\n", &line_save_ptr);

        line_token_count = 0;
    }
    build_program(raw_program, token_count, flag_debug);
}

void build_program(Token *Program, int token_count, int flag_debug){
    if (flag_debug) printf("\n===== Starting Classification Stage =====\n");
    for (int i = 0; i < token_count; i++) {
        printf("Token: '%s'\n", Program[i].T_str);
    }

    struct _FuncNode MainProgram = FuncInitialize();

    for (int i = 0; i < token_count - 1; i++) {
        printf("found token %s\n", Program[i].T_str);
        if (Program[i].op == op_func) {
            struct _FuncNode * function;
            function = FuncPush(&MainProgram, Program[i].T_str);

            int k = i + 1;

            while (Program[k].op != op_return) {
                if (Program[k].op == op_ipush) {
                    OpPushInt(function, Program[k].op, strtol(Program[k].T_str, NULL, 10));
                }
                else {
                    OpPushStr(function, Program[k].op, Program[k].T_str);
                }
                printf("Adding token '%s' of type %d to function '%s'\n", Program[k].T_str, Program[k].op, Program[i + 1].T_str);
                k++;
            }
        }
    }

    FuncPrint(&MainProgram);



    //printf("test: %p\n", MainProgram.func);
/*
    for (int i = 0; i < token_count; i++) {
        if (!strcmp(Program[i].T_str, "func")) {
            printf("FUNCTION: %s AT: %d\n", Program[i+1].T_str, i);
            for (int j = 0; j < 24; j++) {
                //if (!strcmp(Program[i+1].T_str, reserved_keywords[j])) {
                //    printf("Error: That's a reserved keyword!\n");
                //    exit(1);
                //}
            }
            struct _FuncNode * Function;
            Function = FuncPush(&MainProgram, Program[i+1].T_str);
            int k = i+1;
            while (strcmp(Program[k].T_str, "end")) {
                if (strIsNumeric(Program[k].T_str)) {

                    OpPush(Function, op_null, strtol(Program[k].T_str, NULL, 10));
                }
                printf("Keyword: '%s', Operation: '%d'\n", Program[k].T_str, get_op(Program[k].T_str));
                k++;
            }
        }
    }
    FuncPrint(&MainProgram);
    */
}
/*
void interpretProgram(Token *Program, int token_count, int flag_debug) {
    if (flag_debug) printf("\n===== Starting Interpretation Stage =====\n");

    int stack_height = 0;
    int Stack[STACK_MAX];

    if (flag_debug) {
        printf("===== Goto anchors =====\n");
        for (int i = 0; i < anchor_count; i++) {
            printf("  Index: %d, Loc: %d\n", i, Anchor[i]);
        } printf("========================\n");
    }


    int RuntimeAnchorStack[STACK_MAX];
    int runtime_anchor_stack_height = 0;

    int program_index = 0;
    while (program_index < token_count) {
        
        switch (Program[program_index].OpType) {
            int a, b, c, d;
            int l, s;
            int jump_valid;
            int if_level;
            case op_null:
                //printf("[internal] found null token\n");
                break;
            case op_add:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a + b;
                break;

            case op_subtract:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a - b;
                break;

            case op_multiply:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a * b;
                break;

            case op_divide:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a / b;
                Stack[stack_height++] = a % b;
                break;

            case op_lt:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a < b;
                break;

            case op_gt:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a > b;
                break;

            case op_eq:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a == b;
                break;

            case op_and:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a && b;
                break;

            case op_or:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a || b;
                break;
            
            case op_swap:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = b;
                Stack[stack_height++] = a;
                break;
            
            case op_dup:
                if (stack_height < 1) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                a = Stack[--stack_height];

                Stack[stack_height++] = a;
                Stack[stack_height++] = a;
                break;

            case op_over:
                if (stack_height < 2) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = a;
                Stack[stack_height++] = b;
                Stack[stack_height++] = a;
                break;

            case op_rot:
                if (stack_height < 3) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                c = Stack[--stack_height];
                b = Stack[--stack_height];
                a = Stack[--stack_height];

                Stack[stack_height++] = b;
                Stack[stack_height++] = c;
                Stack[stack_height++] = a;
                break;

            case op_drop:
                if (stack_height < 1) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                a = Stack[--stack_height];
                break;

            case op_ipush:
                Stack[stack_height++] = Program[program_index].T_int;
                break;

            case op_spush:
                l = strlen(Program[program_index].T_str) - 1;
                for (int j = l; j >= 0; j--) {
                    Stack[stack_height++] = Program[program_index].T_str[j];
                }
                Stack[stack_height++] = strlen(Program[program_index].T_str);
                break;

            case op_iprint:
                if (stack_height < 1) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                a = Stack[--stack_height];
                printf("%d", a);
                break;

            case op_sprint:
                if (stack_height < 1) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                l = Stack[--stack_height];
                for (int j = 0; j < l; j++) {
                    printf("%c", Stack[--stack_height]);
                }
                //printf("\n");
                break;

            case op_anchor:
                Anchor[Program[program_index].T_int] = program_index;
                anchor_count++;
                break; 

            case op_goto:
                if (anchor_count == 0) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "No anchor points have been defined", Program[program_index].T_str);
                }
                a = Stack[--stack_height];
                if (a < anchor_count) {
                    program_index = Anchor[a];
                    break;
                } else {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Invalid jump location", Program[program_index].T_str);
                }

            case op_do:
                RuntimeAnchorStack[++runtime_anchor_stack_height] = program_index;
                break;

            case op_while:
                if (runtime_anchor_stack_height == 0) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Invalid loop: No corresponding 'do' statement has been defined", Program[program_index].T_str);
                }
                a = Stack[--stack_height];
                if (a) {
                    program_index = RuntimeAnchorStack[runtime_anchor_stack_height];
                    break;
                } else {
                    runtime_anchor_stack_height--;
                    break;
                }
                
            case op_if:
                if (stack_height < 1) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack contents insufficient for operation", Program[program_index].T_str);
                }
                if_level = 1;
                s = program_index + 1;
                while (if_level > 0) {
                    if (Program[s].OpType == op_if) if_level++;
                    else if (Program[s].OpType == op_fi) if_level--;
                    s++;
                }
                a = Stack[--stack_height];
                if (!a) {
                    program_index = s - 1;
                }
                break;
                
            case op_fi:
                break;            
            
            case op_dstack:
                if (!flag_debug) {
                    throwError(global_filepath, Program[program_index].line, Program[program_index].loc, "Stack debug not allowed outside of debug mode", Program[program_index].T_str);
                }
                printf("=== start dstack ===\n -> called at loc: %d\n -> current state (%d): [", program_index, stack_height);
                for (int j = 0; j < stack_height; j++){
                    printf(" %d ", Stack[j]);
                } printf("] (top)\n====================");
                break;
                

            case op_cr:
                printf("\n");
                break;

            case op_nbsp:
                printf(" ");
                break;

            default:
                break;

        }
        program_index++;
    }
    if (stack_height > 0) {
        printf("[WARN] Unhandled data on stack: ");
        for (int j = 0; j < stack_height; j++) {
            printf("%d ", Stack[j]);
        }
    }
}
*/

void throwError(const char *filename, int line, int token, char *message, char *operator) {
    printf("\n%s:%d:%d Error: %s: '%s'\n", filename, line, token, message, operator);
    exit(1);
}

void helpMessage() {
    printf("Raft interpreter %s\n", version_string);
    printf("usage: raft [OPTIONS] -r <path_to_program>\n\n");
    printf("required arguments:\n");
    printf("  -r <file>       run program from file\n");
    printf("\noptional arguments:\n");
    printf("  -d, --debug     run program in interpreter debugging mode\n");
    printf("  -h, --help      display this help message and exit\n");
}

