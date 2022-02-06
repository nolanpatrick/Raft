#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/structure.c"
#include "include/string_tests.c"

#define FILE_BUF_MAX 640000 // 640 KB
//#define STACK_MAX 10240

char *version_string = "0.5.0";
char global_filepath[1024];

typedef struct // Program token
{
    int   T_int;
    char *T_str;
    int line;
    int loc;
    Operations op;
} Token;

// ===== Function signatures =====

int main(int argc, char *argv[]);

void parseTokens(char *file_buffer, int flag_debug);

void parse_file(char * file_buffer, int flag_buffer);

void build_program(Token *Program, int token_count, int flag_debug);

void interpret_program(struct _FuncNode * p, int flag_debug);

void throwError(const char *filename, int line, int token, char *message, char *operator);

void helpMessage();

// ======================================================================================

int main(int argc, char *argv[])
{

    int flag_debug = 0;    // Show parsing stages and debug info if enabled
    int flag_run = 0;      // Whether or not to begin parsing and execution

    if (argc == 1) helpMessage();

    for (int i = 0; i < argc; i++)
    {
        // TODO: Restructure argv parsing to be more reliable and consistent
        if (!strcmp(argv[i], "-r") && argv[i+1] == NULL)
        {
            helpMessage();
            printf("\nError: please provide a path to the input file.\n");
            exit(1);
        }
        if (!strcmp(argv[i], "-r") && argv[i+1] != NULL)
        {
            strcpy(global_filepath, argv[i+1]);
            flag_run = 1;
        }
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            helpMessage();
            exit(0);
        }
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug"))
        {
            printf("[WARN] Enabled debugging mode\n");
            flag_debug = 1;
        }

    }
    if (flag_run)
    {
        if (flag_debug) printf("[INFO] Reading %s\n", global_filepath);
        FILE *fp = fopen(global_filepath, "r");

        if (fp == NULL)
        {
            printf("Error: could not read file: \'%s\'", global_filepath);
        }
        else
        {
            // Read file
            char file_buffer[FILE_BUF_MAX];

            int file_index = 0;
            while (fscanf(fp, "%c", &file_buffer[file_index]) != EOF) file_index++;
            file_buffer[file_index] = '\0';

            fclose(fp);

            parse_file(file_buffer, flag_debug);
        }
    }
    return 0;
}

void parse_file(char * file_buffer, int flag_debug)
{
    if (flag_debug) printf(" *** Tokenization stage ***\n");

    Token raw_program[1024];

    int len_file = strlen(file_buffer);

    int token_count = 0;
    int line_count = 0;
    int line_token_count = 0;
    
    char * line_save_ptr, * token_save_ptr;

    char * line_token_string = strtok_r(file_buffer, "\n", &line_save_ptr);

    while(line_token_string)
    {
        line_count++;
        char * token_string = strtok_r(line_token_string, " ", &token_save_ptr);

        while (token_string)
        {
            Token new_token;
            new_token.line = line_count;
            new_token.loc = line_token_count;

            if (get_op(token_string) == op_comment_init) // Capture comment as a single token
            {
                if (strstr(token_save_ptr, get_keyword(op_comment_end)) == NULL)
                {
                    throwError(global_filepath, new_token.line, new_token.loc, "missing ')' or malformed comment", "(");
                } 
                else
                {
                    char * token_string_comment = strtok_r(NULL, get_keyword(op_comment_end), &token_save_ptr);
                    new_token.T_str = malloc(strlen(token_string_comment));
                    strcpy(new_token.T_str, token_string_comment);
                    new_token.op = op_comment;
                }
            }

            else if (get_op(token_string) == op_string_init) // Capture string literal as a single token
            {
                if (strstr(token_save_ptr, get_keyword(op_string_end)) == NULL)
                {
                    throwError(global_filepath, new_token.line, new_token.loc, "missing quote or malformed string literal", "\"");
                }
                else
                {
                    char * token_string_strlit = strtok_r(NULL, get_keyword(op_string_end), &token_save_ptr);
                    new_token.T_str = malloc(strlen(token_string_strlit));
                    strcpy(new_token.T_str, token_string_strlit);
                    new_token.op = op_spush;
                }
            }

            else
            {
                new_token.T_str = malloc(strlen(token_string));
                strcpy(new_token.T_str, token_string);

                new_token.op = get_op(token_string);

                if (new_token.op == op_null)
                {
                    if (strIsNumeric(token_string))
                    {
                        new_token.op = op_ipush;
                    } 
                    else if (raw_program[token_count].op == op_func)
                    {
                        new_token.op = op_func_decl;
                    }
                }
            }

            raw_program[token_count] = new_token;

            token_string = strtok_r(NULL, " ", &token_save_ptr);

            line_token_count++;
            token_count++;
        }

        line_token_string = strtok_r(NULL, "\n", &line_save_ptr);

        line_token_count = 0;
    }
    build_program(raw_program, token_count, flag_debug);
}

void build_program(Token *Program, int token_count, int flag_debug)
{
    if (flag_debug) printf("\n===== Starting Classification Stage =====\n");

    struct _FuncNode MainProgram = FuncInitialize();

    for (int i = 0; i < token_count - 1; i++)
    {
        if (Program[i].op == op_func)
        {
            if (IsFunc(&MainProgram, Program[i+1].T_str)) // make sure program hasn't already defined constant or function
            {
                fprintf(stderr, "Error: redefinition: '%s' has already been defined\n", Program[i+1].T_str);
                exit(1);
            }

            struct _FuncNode * function;
            function = FuncPush(&MainProgram, Program[i+1].T_str);

            int k = i + 2;

            while (Program[k].op != op_return)
            {
                if (Program[k].op == op_ipush)
                {
                    OpPushInt(function, Program[k].op, strtol(Program[k].T_str, NULL, 10));
                }
                else
                {
                    OpPushStr(function, Program[k].op, Program[k].T_str);
                }
                k++;
            }
            OpPush(function, op_return);
            i = k;
        }
        else if (Program[i].op == op_const)
        {
            if (IsFunc(&MainProgram, Program[i+1].T_str)) // make sure program hasn't already defined constant or function
            {
                fprintf(stderr, "Error: redefinition: '%s' has already been defined\n", Program[i+1].T_str);
                exit(1);
            }

            struct _FuncNode * function;
            function = FuncPush(&MainProgram, Program[i+1].T_str);

            int k = i + 2;

            while (Program[k].op != op_return)
            {
                if (Program[k].op == op_ipush)
                {
                    OpPushInt(function, Program[k].op, strtol(Program[k].T_str, NULL, 10));
                }
                else
                {
                    OpPushStr(function, Program[k].op, Program[k].T_str);
                }
                k++;
            }
            OpPush(function, op_return);
            i = k;
        }
    }
    if (flag_debug) MemMapPrint(&MainProgram);
    interpret_program(&MainProgram, flag_debug);
}

void interpret_program(struct _FuncNode * p, int flag_debug) 
{
    // TODO: Move various program structure definitions to main
    // to clean up program and make interactive mode easier to
    // implement later.

    struct _CallNode MainCallStack = CallInitialize();

    struct _FuncNode * ExecFunction;

    struct _OpNode * ExecOp;

    struct _Node MainStack = Initialize();

    struct _Node RetStack = Initialize();

    if (!IsFunc(p, "main"))
    {
        fprintf(stderr, "Error: could not fund program entry point (main)\n");
        exit(1);
    }
    else
    {
        ExecFunction = GetFuncAddr(p, "main");
        if (flag_debug) 
        {
            printf("[debug] found entry point (main) at %p, function starts at %p\n", ExecFunction, ExecFunction->ptr);
        }
    }

    ExecOp = ExecFunction->ptr;

    while (ExecOp)
    {
        switch(ExecOp->op)
        {
            case op_null:
            {
                if (IsFunc(p, ExecOp->data_s))
                {
                    struct _FuncNode * next = GetFuncAddr(p, ExecOp->data_s);
                    if (next != NULL)
                    {
                        ExecOp->op = op_func_call;
                        CallPush(&MainCallStack, ExecOp->ptr);
                        if (flag_debug) 
                        {
                            printf("\n[debug] jumping to function '%s' at %p\n", ExecOp->data_s, next);
                            printf("\n[debug] return stack length %d\n", CallLength(&MainCallStack));
                            CallPrint(&MainCallStack);
                        }
                        ExecOp = next->ptr;
                    }
                    break;
                }
                else
                {
                    fprintf(stderr, "Error: unknown keyword '%s'\n", ExecOp->data_s);
                    exit(1);
                    break;
                }
            }
            case op_comment:
            {
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_return:
            {
                if (CallLength(&MainCallStack) < 1)
                {
                    if (LinkLength(&MainStack) > 0)
                    {
                        fprintf(stderr, "Error: unhandled data on the stack\n");
                    }
                    exit(1);
                }
                if (LinkLength(&RetStack) > 0)
                {
                    fprintf(stderr, "Error: unhandled data on the return stack\n");
                    exit(1);
                }
                ExecOp = CallPop(&MainCallStack);
                if (flag_debug) printf("\n[debug] return stack length %d\n", CallLength(&MainCallStack));
                if (flag_debug) printf("\n[debug] returning to %p\n", ExecOp);
                break;
            }
            case op_func:
            {
                fprintf(stderr, "Error: functions may not be declared within functions\n");
                exit(1);
                break;
            }
            case op_func_init:
            {
                if (flag_debug) printf("\n[debug] executing function at %p\n", ExecOp);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_ret_push:
            {
                if (LinkLength(&MainStack) < 1)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '>R'\n");
                    exit(1);
                }
                int a = LinkPop(&MainStack);
                LinkPush(&RetStack, a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_ret_pop:
            {
                if (LinkLength(&RetStack) < 1)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'R>'\n");
                    exit(1);
                }
                int a = LinkPop(&RetStack);
                LinkPush(&MainStack, a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_ret_fetch:
            {
                if (LinkLength(&RetStack) < 1)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'R@'\n");
                    exit(1);
                }
                int a = LinkPop(&RetStack);
                LinkPush(&RetStack, a);
                LinkPush(&MainStack, a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_add:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '+'\n");
                    exit(1);
                }
                int a = LinkPop(&MainStack);
                int b = LinkPop(&MainStack);
                LinkPush(&MainStack, a + b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_subtract:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '-'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a - b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_multiply:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '*'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a * b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_divide:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '/'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a / b);
                LinkPush(&MainStack, a % b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_gt:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '>'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a > b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_lt:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '<'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a < b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_eq:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '='\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a == b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_and:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'and'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a && b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_or:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'or'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a || b);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_swap:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'swap'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, b);
                LinkPush(&MainStack, a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_dup:
            {
                if (LinkLength(&MainStack) < 1)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'dup'\n");
                    exit(1);
                }
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a);
                LinkPush(&MainStack, a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_over:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'over'\n");
                    exit(1);
                }
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, a);
                LinkPush(&MainStack, b);
                LinkPush(&MainStack, a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_rot:
            {
                if (LinkLength(&MainStack) < 3)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'rot'\n");
                    exit(1);
                }
                int c = LinkPop(&MainStack);
                int b = LinkPop(&MainStack);
                int a = LinkPop(&MainStack);
                LinkPush(&MainStack, b);
                LinkPush(&MainStack, c);
                LinkPush(&MainStack, a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_drop:
            {
                if (LinkLength(&MainStack) < 1)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'drop'\n");
                    exit(1);
                }
                int a = LinkPop(&MainStack);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_ipush:
            {
                LinkPush(&MainStack, ExecOp->data_i);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_iprint:
            {
                if (LinkLength(&MainStack) < 1)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'iprint'\n");
                    exit(1);
                }
                int a = LinkPop(&MainStack);
                printf("%d", a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_spush:
            {
                int a = strlen(ExecOp->data_s) - 1;
                for (int i = a; i >= 0; i--)
                {
                    LinkPush(&MainStack, ExecOp->data_s[i]);
                }
                LinkPush(&MainStack, a + 1);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_sprint:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'spush'\n");
                    exit(1);
                }
                int a = LinkPop(&MainStack);
                for (int i = 0; i < a; i++)
                {
                    printf("%c", LinkPop(&MainStack));
                }
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_cprint:
            {
                if (LinkLength(&MainStack) < 1)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation 'cprint'\n");
                    exit(1);
                }
                int a = LinkPop(&MainStack);
                printf("%c", a);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_cr:
            {
                printf("\n");
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_nbsp:
            {
                printf(" ");
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_do:
            {
                fprintf(stderr, "Error: loops are not yet implemented\n");
                exit(1);
                break;
            }
            case op_while:
            {
                fprintf(stderr, "Error: loops are not yet implemented\n");
                exit(1);
                break;
            }
            case op_if:
            {
                fprintf(stderr, "Error: conditionals are not yet implemented\n");
                exit(1);
                break;
            }
            case op_fi:
            {
                fprintf(stderr, "Error: conditionals are not yet implemented\n");
                exit(1);
                break;
            }
            case op_dstack:
            {
                struct _Node * curr = MainStack.ptr;
                while (curr) {
                    printf("%d ", curr->data);
                }
                ExecOp = ExecOp->ptr;
                break;
            }
        }
    }
    Cleanup(p);
}

void throwError(const char *filename, int line, int token, char *message, char *operator)
{
    printf("\n%s:%d:%d Error: %s: '%s'\n", filename, line, token, message, operator);
    exit(1);
}

void helpMessage()
{
    printf("Raft interpreter %s\n", version_string);
    printf("usage: raft [OPTIONS] -r <path_to_program>\n\n");
    printf("required arguments:\n");
    printf("  -r <file>       run program from file\n");
    printf("\noptional arguments:\n");
    printf("  -d, --debug     run program in interpreter debugging mode\n");
    printf("  -h, --help      display this help message and exit\n");
}

