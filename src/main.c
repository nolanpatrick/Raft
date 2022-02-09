#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/structure.c"
#include "include/string_tests.c"

#define FILE_BUF_MAX 640000 // 640 KB
//#define STACK_MAX 10240
#define FILE_TOKEN_MAX 1024

char *version_string = "0.5.0";
char global_filepath[1024];

typedef struct // Program token
{
    int   T_int;
    char T_str[255];
    int line;
    int loc;
    Operations op;
} Token;

// ===== Function signatures =====

int main(int argc, char *argv[]);

void append_file_to_program(struct _FuncNode * dest, char srcpath[], int flag_debug); // pipeline from file path to initialized program memory map

int parse_file(Token * container, char * file_buffer, int flag_buffer);

void build_program(struct _FuncNode * program_memory, Token * program_tokens, int token_count, int flag_debug);

void interpret_program(struct _FuncNode * p, int flag_debug);

void throwError(const char *filename, int line, int token, char *message, char *operator);

void helpMessage();

// ======================================================================================

int main(int argc, char *argv[])
{

    int flag_debug = 0;    // Show parsing stages and debug info if enabled

    if (argc == 1) helpMessage();

    for (int i = 1; i < argc; i++)
    {
        // TODO: Restructure argv parsing to be more reliable and consistent
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug"))
        {
            printf("[WARN] Enabled debugging mode\n");
            flag_debug = 1;
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            helpMessage();
            exit(0);
        }
        else
        {
            if (i != argc-1)
            {
                helpMessage();
                fprintf(stderr, "Error: unrecognized flag: '%s'\n", argv[i]);
                exit(1);
            }
        }
    }

    memcpy(global_filepath, argv[argc-1], strlen(argv[argc-1]));

    if (argc > 1)
    {
        if (flag_debug) printf("[INFO] Reading %s\n", global_filepath);

        struct _FuncNode program_memory = FuncInitialize();

        append_file_to_program(&program_memory, global_filepath, flag_debug);

        if (flag_debug) MemMapPrint(&program_memory);

        interpret_program(&program_memory, flag_debug);

        Cleanup(&program_memory);
    }
    return 0;
}

void append_file_to_program(struct _FuncNode * dest, char srcpath[], int flag_debug)
{
    // Appends a new set of instructions to the program, parsed from the given
    // input file path.
    if (strlen(srcpath) < 1)
    {
        fprintf(stderr, "Error: append_to_program(): path is empty\n");
        exit(1);
    }

    FILE * _fp = fopen(srcpath, "r");
    
    if (_fp == NULL)
    {
        fprintf(stderr, "Error: append_to_program(): could not read file: %s\n", srcpath);
        exit(1);
    }

    printf("[INFO] reading file %s\n", srcpath);
    int file_index = 0;
    char * file_buffer = malloc(FILE_BUF_MAX);
    while (fscanf(_fp, "%c", &file_buffer[file_index]) != EOF) file_index++;
    file_buffer[file_index] = '\0';
    fclose(_fp);

    Token * file_tokenized = malloc(FILE_TOKEN_MAX * sizeof(Token));

    int file_token_count = parse_file(file_tokenized, file_buffer, flag_debug);

    build_program(dest, file_tokenized, file_token_count, flag_debug);

    free(file_tokenized);
    free(file_buffer);
}


int parse_file(Token * container, char * file_buffer, int flag_debug)
{
    if (flag_debug) printf(" *** Tokenization stage ***\n");

    Token * raw_program = container;

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
            //memset(new_token.T_str, 0, 255);

            if (get_op(token_string) == op_comment_init) // Capture comment as a single token
            {
                if (strstr(token_save_ptr, get_keyword(op_comment_end)) == NULL)
                {
                    throwError(global_filepath, new_token.line, new_token.loc, "missing ')' or malformed comment", "(");
                } 
                else
                {
                    char * token_string_comment = strtok_r(NULL, get_keyword(op_comment_end), &token_save_ptr);
                    //new_token.T_str = malloc(strlen(token_string_comment) + 1);
                    strcpy(new_token.T_str, token_string_comment);
                    new_token.op = op_comment;
                }
            }

            else if (get_op(token_string) == op_comment_line) // Capture comment as a single token
            {
                char * token_string_comment = strtok_r(NULL, "\n", &token_save_ptr);
                //new_token.T_str = malloc(strlen(token_string_comment) + 1);
                strcpy(new_token.T_str, token_string_comment);
                new_token.op = op_comment;
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
                    //new_token.T_str = malloc(strlen(token_string_strlit) + 1);
                    strcpy(new_token.T_str, token_string_strlit);
                    new_token.op = op_spush;
                }
            }

            else
            {
                //new_token.T_str = malloc(strlen(token_string) + 1);
                strcpy(new_token.T_str, token_string);

                new_token.op = get_op(token_string);

                if (new_token.op == op_null)
                {
                    if (strIsNumeric(token_string))
                    {
                        new_token.op = op_ipush;
                        //strncpy(new_token.T_str, "", 1);
                    } 
                    else if (raw_program[token_count].op == op_func)
                    {
                        new_token.op = op_func_decl;
                    }
                }
            }
            if (flag_debug) printf("Token: '%s', operation: '%s'\n", new_token.T_str, get_keyword(new_token.op));

            raw_program[token_count] = new_token;

            token_string = strtok_r(NULL, " ", &token_save_ptr);

            line_token_count++;
            token_count++;
        }

        line_token_string = strtok_r(NULL, "\n", &line_save_ptr);

        line_token_count = 0;
    }
    return (token_count);
}

void build_program(struct _FuncNode * program_memory, Token * program_tokens, int token_count, int flag_debug)
{
    if (flag_debug) printf("\n===== Starting Classification Stage =====\n");

    //struct _FuncNode program_memory = FuncInitialize();

    for (int i = 0; i < token_count - 1; i++)
    {
        //printf("TOKEN: %s\n", program_tokens[i].T_str);
        if (program_tokens[i].op == op_include)
        {
            int k = i + 1;

            append_file_to_program(program_memory, program_tokens[k].T_str, flag_debug);

            i = k;
        }
        else if (program_tokens[i].op == op_func)
        {
            if (IsFunc(program_memory, program_tokens[i+1].T_str)) // make sure program hasn't already defined constant or function
            {
                fprintf(stderr, "Error: redefinition: '%s' has already been defined\n", program_tokens[i+1].T_str);
                exit(1);
            }

            struct _FuncNode * function;
            function = FuncPush(program_memory, program_tokens[i+1].T_str);

            int k = i + 2;

            while (program_tokens[k].op != op_return)
            {
                if (program_tokens[k].op == op_ipush)
                {
                    OpPushInt(function, program_tokens[k].op, strtol(program_tokens[k].T_str, NULL, 10));
                }
                else
                {
                    OpPushStr(function, program_tokens[k].op, program_tokens[k].T_str);
                }
                k++;
            }
            OpPush(function, op_return);
            i = k;
        }
        else if (program_tokens[i].op == op_const)
        {
            if (IsFunc(program_memory, program_tokens[i+1].T_str)) // make sure program hasn't already defined constant or function
            {
                fprintf(stderr, "Error: redefinition: '%s' has already been defined\n", program_tokens[i+1].T_str);
                exit(1);
            }

            struct _FuncNode * function;
            function = FuncPush(program_memory, program_tokens[i+1].T_str);

            int k = i + 2;

            while (program_tokens[k].op != op_return)
            {
                if (program_tokens[k].op == op_ipush)
                {
                    OpPushInt(function, program_tokens[k].op, strtol(program_tokens[k].T_str, NULL, 10));
                }
                else
                {
                    OpPushStr(function, program_tokens[k].op, program_tokens[k].T_str);
                }
                k++;
            }
            OpPush(function, op_return);
            i = k;
        }
        else if (program_tokens[i].op != op_comment)
        {
            fprintf(stderr, "Error: instructions outside of function or constant declarations are not allowed at the root level: '%s'\n", program_tokens[i].T_str);
            exit(1);
        }
    }
    //if (flag_debug) MemMapPrint(program_memory);
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
        fprintf(stderr, "Error: could not find program entry point (main)\n");
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
            case op_include:
            {
                fprintf(stderr, "Error: including files is not allowed inside of functions\n");
                exit(1);
                break;
            }
            case op_comment_init:
            {
                fprintf(stderr, "Error: unmatched comment delimiter '('\n");
                exit(1);
                break;
            }
            case op_comment_end:
            {
                fprintf(stderr, "Error: unmatched comment delimiter ')'\n");
                exit(1);
                break;
            }
            case op_comment_line:
            {
                fprintf(stderr, "Error: erroneous line comment delimiter '\\'\n");
                exit(1);
                break;
            }
            case op_string_init:
            {
                fprintf(stderr, "Error: unmatched string delimiter 's\"'\n");
                exit(1);
                break;
            }
            case op_string_end:
            {
                fprintf(stderr, "Error: unmatched string delimiter '\"'\n");
                exit(1);
                break;
            }
            case op_func:
            {
                fprintf(stderr, "Error: functions may not be declared within functions\n");
                exit(1);
                break;
            }
            case op_func_decl:
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
            case op_func_call:
            {
                fprintf(stderr, "Error: interpret_function() :: op_func_call\n");
                // TODO: handle function calls here rather than op_null above
                exit(1);
                break;
            }
            case op_const:
            {
                fprintf(stderr, "Error: constants may not be declared within functions\n");
                exit(1);
                break;
            }
            case op_const_decl:
            {
                fprintf(stderr, "Error: constants may not be declared within functions\n");
                exit(1);
                break;
            }
            case op_const_call:
            {
                fprintf(stderr, "Error: interpret_function() :: op_const_call\n");
                // TODO: handle constant calls here rather than op_null above
                exit(1);
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
                /*int a = */ LinkPop(&MainStack);
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
}

void throwError(const char *filename, int line, int token, char *message, char *operator)
{
    printf("\n%s:%d:%d Error: %s: '%s'\n", filename, line, token, message, operator);
    exit(1);
}

void helpMessage()
{
    printf("Raft interpreter %s\n", version_string);
    printf("usage: raft [OPTIONS] <path_to_program>\n\n");
    printf("optional arguments:\n");
    printf("  -d, --debug     run program in interpreter debugging mode\n");
    printf("  -h, --help      display this help message and exit\n");
}

