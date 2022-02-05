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

            if (!strcmp(token_string, "(")) // Capture comment as a single token
            {
                if (strstr(token_save_ptr, ")") == NULL)
                {
                    throwError(global_filepath, new_token.line, new_token.loc, "missing ')' or malformed comment", "(");
                } 
                else
                {
                    char * token_string_comment = strtok_r(NULL, ")", &token_save_ptr);
                    new_token.T_str = malloc(strlen(token_string_comment));
                    strcpy(new_token.T_str, token_string_comment);
                    new_token.op = op_comment;

                    //printf(" [x] Instruction: '%s', %d\n", token_string_comment, new_token.op);
                }
            }

            else if (!strcmp(token_string, "s\"")) // Capture string literal as a single token
            {
                if (strstr(token_save_ptr, "\"") == NULL)
                {
                    throwError(global_filepath, new_token.line, new_token.loc, "missing quote or malformed string literal", "\"");
                }
                else
                {
                    char * token_string_strlit = strtok_r(NULL, "\"", &token_save_ptr);
                    new_token.T_str = malloc(strlen(token_string_strlit));
                    strcpy(new_token.T_str, token_string_strlit);
                    new_token.op = op_spush;

                    //printf(" [x] Instruction: '%s', %d\n", token_string_strlit, new_token.op);
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
                    else
                    {
                        //printf(" [ ] Unknown token: '%s'\n", token_string);
                    }
                }

                //printf(" [x] Instruction: '%s', %d\n", token_string, new_token.op);
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

void build_program(Token *Program, int token_count, int flag_debug)
{
    if (flag_debug) printf("\n===== Starting Classification Stage =====\n");

    struct _FuncNode MainProgram = FuncInitialize();

    for (int i = 0; i < token_count - 1; i++)
    {
        if (Program[i].op == op_func)
        {
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
    struct _RetNode MainRetStack = RetInitialize();

    struct _FuncNode * ExecFunction;

    struct _OpNode * ExecOp;

    struct _Node MainStack = Initialize();

    //RetPrint(&MainRetStack);

    ExecFunction = p;
    while (strcmp(ExecFunction->handle, "main"))
    {
        ExecFunction = ExecFunction->func;
    }

    if (strcmp(ExecFunction->handle, "main"))
    {
        fprintf(stderr, "Error: could not fund program entry point (main)\n");
        exit(1);
    }
    else
    {
        if (flag_debug) printf("[debug] found entry point (main) at %p, function starts at %p\n", ExecFunction, ExecFunction->ptr);
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
                        RetPush(&MainRetStack, ExecOp->ptr);
                        if (flag_debug) 
                        {
                            printf("\n[debug] jumping to function '%s' at %p\n", ExecOp->data_s, next);
                            printf("\n[debug] return stack length %d\n", RetLength(&MainRetStack));
                            RetPrint(&MainRetStack);
                        }
                        ExecOp = next->ptr;
                    }
                    break;
                }
                else
                {
                    fprintf(stderr, "Error: unknown keyword '%s'", ExecOp->data_s);
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
                if (RetLength(&MainRetStack) < 1)
                {
                    //fprintf(stderr, "Error: return stack contents insufficient for continued execution");
                    exit(1);
                }
                ExecOp = RetPop(&MainRetStack);
                if (flag_debug) printf("\n[debug] return stack length %d\n", RetLength(&MainRetStack));
                if (flag_debug) printf("\n[debug] returning to %p\n", ExecOp);
                break;
            }
            case op_func:
            {
                fprintf(stderr, "Error: functions may not be declared within functions");
                exit(1);
                break;
            }
            case op_func_init:
            {
                if (flag_debug) printf("\n[debug] executing function at %p\n", ExecOp);
                ExecOp = ExecOp->ptr;
                break;
            }
            case op_add:
            {
                if (LinkLength(&MainStack) < 2)
                {
                    fprintf(stderr, "Error: stack contents insufficient for operation '+'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation '-'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation '*'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation '/'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation '>'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation '<'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation '='");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'and'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'or'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'swap'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'dup'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'over'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'rot'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'drop'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'iprint'");
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
                    fprintf(stderr, "Error: stack contents insufficient for operation 'spush'");
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
            case op_cpush:
            {
                fprintf(stderr, "Error: character push is not implemented");
                exit(1);
                break;
            }
            case op_cprint:
            {
                fprintf(stderr, "Error: 'cprint' is not implemented");
                exit(1);
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
            case op_anchor:
            {
                fprintf(stderr, "Error: anchors are no longer supported (this should be unreachable)");
                exit(1);
                break;
            }
            case op_goto:
            {
                fprintf(stderr, "Error: 'goto' is no longer supported");
                exit(1);
                break;
            }
            case op_do:
            {
                fprintf(stderr, "Error: loops are not yet implemented");
                exit(1);
                break;
            }
            case op_while:
            {
                fprintf(stderr, "Error: loops are not yet implemented");
                exit(1);
                break;
            }
            case op_if:
            {
                fprintf(stderr, "Error: conditionals are not yet implemented");
                exit(1);
                break;
            }
            case op_fi:
            {
                fprintf(stderr, "Error: conditionals are not yet implemented");
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

