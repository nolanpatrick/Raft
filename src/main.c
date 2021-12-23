#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typechecking.c"

#define FILE_BUF_MAX 640000 // 640 KB
#define STACK_MAX 10240

char *version_string = "0.4.1";
char global_filepath[1024];

typedef enum { // Types of tokens allowed in program
    TT_null,
    TT_comment_content,
    TT_str_content,
    TT_anchor,
    TT_int,
    TT_op 
} TokenType;

typedef enum {
    op_null,
    op_add,
    op_subtract,
    op_multiply,
    op_divide,

    op_gt,
    op_lt,
    op_eq,
    op_and,
    op_or,

    op_swap,
    op_dup,
    op_over,
    op_rot,
    op_drop,

    op_ipush,
    op_spush,
    op_iprint,
    op_sprint,

    op_cr,
    op_nbsp,
    op_anchor,
    op_goto,

    op_do,
    op_while,
    op_if,
    op_fi,

    op_dstack
} Operations;

typedef struct { // Program token
    TokenType T_Type;
    int   T_int;
    char *T_str;
    Operations OpType;
    int line;
    int loc;
} Token;

// ===== Function signatures =====

int main(int argc, char *argv[]);

void parseTokens(char *file_buffer, int flag_debug);

void classifyTokens(Token *Program, int token_count, int flag_debug);

void interpretProgram(Token *Program, int token_count, int flag_debug);

void throwError(const char *filename, int line, int token, char *message, char *operator);

void helpMessage();

// ===== Jump anchors =============

int anchor_count = 0;
int Anchor[STACK_MAX]; // Hold goto anchor point locations

// ===============================

int main(int argc, char *argv[]) {

    int flag_debug = 0;    // Show parsing stages and debug info if enabled
    int flag_run = 0;      // Whether or not to begin parsing and execution

    if (argc == 1) helpMessage();

    for (int i = 0; i < argc; i++) {
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

            parseTokens(file_buffer, flag_debug);
        }
    }
    return 0;
}

void parseTokens(char *file_buffer, int flag_debug){
    if (flag_debug) printf("===== Starting Tokenization Stage =====\n");

    Token* Program = malloc(sizeof(Token)); // Program tokens

    int len_file = strlen(file_buffer);
    int token_count = 0;
    //int program_var_count = 0;

    int line_count = 0;
    int line_token_count = 0;

    char *line_save_ptr, *token_save_ptr;
    char *line_token_str = strtok_r(file_buffer, "\n", &line_save_ptr);
    
    while (line_token_str) {
        line_count++; // Line count starts at zero, most editors start at 1.
        char *token_str = strtok_r(line_token_str, " ", &token_save_ptr);

        while (token_str) {
            Program = realloc(Program, sizeof(Token) * (token_count + 2));

            Token NewToken;
            NewToken.line = line_count;
            NewToken.loc = line_token_count; // TODO: Fix token count around comments and strings

            if (!strcmp(token_str, "(")) { // Capture comments
                char *token_str_comment = strtok_r(NULL, ")", &token_save_ptr);

                int j = strlen(token_str_comment);
                while (strcmp(&token_str_comment[--j], " ")); // For removing trailing whitespace
                token_str_comment[j] = '\0';

                NewToken.T_Type = TT_comment_content;
                NewToken.OpType = op_null;

                NewToken.T_str = calloc(strlen(token_str_comment), sizeof(char));
                strcpy(NewToken.T_str, token_str_comment);
            }
            else if (!strcmp(token_str, "\"")) { // Capture strings
                char *token_str_content = strtok_r(NULL, "\"", &token_save_ptr);

                int j = strlen(token_str_content);
                while (strcmp(&token_str_content[--j], " ")); // For removing trailing whitespace
                token_str_content[j] = '\0';

                NewToken.T_Type = TT_str_content;
                NewToken.OpType = op_spush;

                NewToken.T_str = calloc(strlen(token_str_content), sizeof(char));
                strcpy(NewToken.T_str, token_str_content);
            }
            else if (!strcmp(token_str, ":")) { // Capture strings
                char *token_str_anchor = strtok_r(NULL, ":", &token_save_ptr);

                int j = strlen(token_str_anchor);
                while (strcmp(&token_str_anchor[--j], " ")); // For removing trailing whitespace
                token_str_anchor[j] = '\0';

                NewToken.T_Type = TT_anchor;
                NewToken.OpType = op_anchor;
                NewToken.T_int = strtol(token_str_anchor, NULL, 10);

                NewToken.T_str = calloc(strlen(token_str_anchor), sizeof(char));
                strcpy(NewToken.T_str, token_str_anchor);

                Anchor[NewToken.T_int] = token_count;
                anchor_count++;
            }

            else {
                NewToken.T_Type = TT_null;
                NewToken.T_str = calloc(strlen(token_str), sizeof(char));
                memcpy(NewToken.T_str, token_str, strlen(token_str));
            }

            if (flag_debug) {
                printf(" - %d, %d:Token string: [%s]\n", token_count, NewToken.line, NewToken.T_str);
            }

            Program[token_count] = NewToken;
            token_str = strtok_r(NULL, " ", &token_save_ptr);

            token_count++;
            line_token_count++;
        }

        line_token_str = strtok_r(NULL, "\n", &line_save_ptr);
        line_token_count = 0;
    }
    if (token_count == 0) {
        printf("%s:0:0: Error: tokenization failed or file empty", global_filepath);
        exit(1);
    } else {
        classifyTokens(Program, token_count, flag_debug);
    }
}

void classifyTokens(Token *Program, int token_count, int flag_debug){
    if (flag_debug) printf("\n===== Starting Classification Stage =====\n");
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
        } else if (!strcmp(Program[i].T_str, "over")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_over;
        } else if (!strcmp(Program[i].T_str, "rot")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_rot;
        } else if (!strcmp(Program[i].T_str, "drop")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_drop;
        } else if (strIsNumeric(Program[i].T_str) && Program[i].T_Type != TT_comment_content && Program[i].T_Type != TT_str_content && Program[i].T_Type != TT_anchor) {
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
        } else if (!strcmp(Program[i].T_str, "cr")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_cr;
        } else if (!strcmp(Program[i].T_str, "nbsp")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_nbsp;
        } else if (!strcmp(Program[i].T_str, "goto")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_goto;
        } else if (!strcmp(Program[i].T_str, "<")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_lt;
        } else if (!strcmp(Program[i].T_str, ">")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_gt;
        } else if (!strcmp(Program[i].T_str, "eq")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_eq;
        } else if (!strcmp(Program[i].T_str, "and")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_and;
        } else if (!strcmp(Program[i].T_str, "or")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_or;
        } else if (!strcmp(Program[i].T_str, "do")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_do;
        } else if (!strcmp(Program[i].T_str, "while")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_while;
        } else if (!strcmp(Program[i].T_str, "if")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_if;
        } else if (!strcmp(Program[i].T_str, "fi")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_fi;
        } else if (!strcmp(Program[i].T_str, "dstack")) {
            Program[i].T_Type = TT_op;
            Program[i].OpType = op_dstack;
        } else if (Program[i].T_Type != TT_comment_content && Program[i].T_Type != TT_str_content && Program[i].T_Type != TT_anchor) {
            throwError(global_filepath, Program[i].line, Program[i].loc, "Unknown keyword", Program[i].T_str);
        }
    }


    if (flag_debug) {
        for (int i = 0; i < token_count; i++){
        if (Program[i].T_Type == TT_str_content)
            printf("STR, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        else if (Program[i].T_Type == TT_comment_content)
            printf("COM, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        else if (Program[i].T_Type == TT_int)
            printf("INT, '%d': %d\n", Program[i].T_int, Program[i].T_Type);
        else if (Program[i].T_Type == TT_op)
            printf("OPR, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        else if (Program[i].T_Type == TT_anchor)
            printf("ANC, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        else
            printf("000, '%s': %d\n", Program[i].T_str, Program[i].T_Type);
        }
    }
    interpretProgram(Program, token_count, flag_debug);
}

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

            /* case op_anchor:
                Anchor[Program[program_index].T_int] = program_index;
                anchor_count++;
                break; */

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

