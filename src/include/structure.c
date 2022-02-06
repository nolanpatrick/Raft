#pragma once

typedef enum 
{
    op_null,
    op_comment,
    op_return,

    // Delimiters (used in parsing stage 1 only)
    op_comment_init,
    op_comment_end,
    op_string_init,
    op_string_end,

    // Functions
    op_func,
    op_func_decl,
    op_func_init,
    op_func_call,

    // Constants
    op_const,
    op_const_decl,
    op_const_call,

    // Return Stack (for Forth compatibility)
    op_ret_push,
    op_ret_pop,
    op_ret_fetch,

    // Arithmetic
    op_add,
    op_subtract,
    op_multiply,
    op_divide,

    // Boolean
    op_gt,
    op_lt,
    op_eq,
    op_and,
    op_or,

    // Stack manipulation
    op_swap,
    op_dup,
    op_over,
    op_rot,
    op_drop,

    // Integers
    op_ipush,
    op_iprint,

    // Strings & Chars
    op_spush,
    op_sprint,
    op_cprint,

    // Misc.
    op_cr,
    op_nbsp,

    // Loops
    op_do,
    op_while,

    // Conditionals
    op_if,
    op_fi,

    // Debugging
    op_dstack,

    // Internal flags
    op_internal,
    op_unhandled_warn,
    op_unhandled_ignore
} Operations;

typedef struct
{
    Operations op;
    char * word;
} keyword;

const keyword reserved[] = 
{
    {op_null,       "__null"},
    {op_comment,    "__comment"},
    {op_return,     "end"},
    {op_return,     ";"},  // Forth syntax

    {op_comment_init, "("},
    {op_comment_end,  ")"},
    {op_string_init,  "s\""},
    {op_string_end,   "\""},

    {op_func,       "func"},
    {op_func,       ":"},  // Forth syntax
    {op_func_init,  "__func_init"},
    {op_func_call,  "__func_call"},

    {op_const,      "const"},
    {op_const_call, "__const_call"},

    {op_ret_push,   ">R"},
    {op_ret_pop,    "R>"},
    {op_ret_fetch,  "R@"},

    {op_add,        "+"},
    {op_subtract,   "-"},
    {op_multiply,   "*"},
    {op_divide,     "/"},

    {op_gt,         ">"},
    {op_lt,         "<"},
    {op_eq,         "="},
    {op_and,        "and"},
    {op_or,         "or"},

    {op_swap,       "swap"},
    {op_dup,        "dup"},
    {op_over,       "over"},
    {op_rot,        "rot"},
    {op_drop,       "drop"},
    
    {op_ipush,      "__ipush"},
    {op_iprint,     "iprint"},
    {op_iprint,     "."},

    {op_spush,      "__spush"},
    {op_sprint,     "sprint"},

    {op_cprint,     "cprint"},
    {op_cprint,     "emit"},  // Forth syntax

    {op_cr,         "cr"},
    {op_nbsp,       "nbsp"},

    {op_do,         "do"},
    {op_while,      "while"},

    {op_if,         "if"},
    {op_fi,         "fi"},

    {op_dstack,     "dstack"},

    {op_internal,         "#internal"},
    {op_unhandled_warn,   "warn_unhandled"},
    {op_unhandled_ignore, "ignore_unhandled"}
};

typedef struct
{
    enum 
    {
        unhandled_warn,
        unhandled_ignore
    } unhandled;
} Flags;

char * strcase(char h[])
{
    char * switched = malloc(strlen(h));
    for (int i = 0; i < strlen(h); i++)
    {
        if (h[i] <= 122 && h[i] >= 97)
        {
            switched[i] = h[i] - 32;
        }
        else if (h[i] <= 90 && h[i] >= 65)
        {
            switched[i] = h[i] + 32;
        }
        else
        {
            switched[i] = h[i];
        }
        
    }
    return(switched);
}

Operations get_op(char * kw)
{
    // This idea is borrowed from StackOverflow user 'plinth'

    const int num_keywords = sizeof(reserved) / sizeof(keyword);
    for (int i = 0; i < num_keywords; i++)
    {
        if (!strcmp(kw, reserved[i].word) || !strcmp(strcase(kw), reserved[i].word))
        {
            return(reserved[i].op);
        }

    }
    return(op_null);
}

char * get_keyword(Operations op)
{
    const int num_keywords = sizeof(reserved) / sizeof(keyword);
    for (int i = 0; i < num_keywords; i++)
    {
        if (op == reserved[i].op)
        {
            return(reserved[i].word);
        }
    }
    return(NULL);
}

// ===== NODES FOR LINKED LISTS =====

typedef enum {
    head = 0,
    link
} Type;

struct _Node {
    int data;
    Type type;
    struct _Node * ptr;
};

struct _OpNode {
    // Used for sequence of program operations
    Type type;
    Operations op;
    int data_i;
    char * data_s;
    struct _OpNode * ptr;
};

struct _FuncNode {
    // 'head' of functions, stores handle and pointer to function
    Type type;
    char handle[32];
    struct _OpNode * ptr;    // Store pointer to beginning of function
    struct _FuncNode * func; // Store pointer to next function head
};

struct _CallNode {
    // Used for return stack (stores return addresses on each function call)
    Type type;
    struct _OpNode * ret;
    struct _CallNode * ptr;
};

/* struct _ConstNode {
    Type type;
    char handle[32];
    int value;
    struct _ConstNode * ptr;
}; */

// ===== FUNCTIONS FOR LINKED LISTS =====

struct _FuncNode FuncInitialize(void) {
    // Initialize empty program
    struct _FuncNode to_init;
    strcpy(to_init.handle, "root");
    to_init.type = head;
    to_init.ptr = NULL;
    to_init.func = NULL;

    return(to_init);
}

struct _CallNode CallInitialize(void) {
    // Initialize empty return stack
    struct _CallNode to_init;
    to_init.type = head;
    to_init.ptr = NULL;
    to_init.ret = NULL;
    return(to_init);
}
/* 
struct _ConstNode ConstInitialize(void) {
    // Initialize empty const stack
    struct _ConstNode to_init;
    strcpy(to_init.handle, "head");
    to_init.type = head;
    to_init.ptr = NULL;
    return(to_init);
} */

struct _FuncNode * _FuncWalk(struct _FuncNode * n) {
    // Returns pointer to last node in function stack
    struct _FuncNode * curr = n;
    while (curr->func != NULL) {
        curr = curr->func;
    }
    return(curr);
}

struct _CallNode * _CallWalk(struct _CallNode * n) {
    // Returns pointer to last node in return stack
    struct _CallNode * curr = n;
    while (curr->ptr != NULL) {
        curr = curr->ptr;
    }
    return(curr);
}

/* struct _ConstNode * _ConstWalk(struct _ConstNode * n) {
    // Returns pointer to last node in const stack
    struct _ConstNode * curr = n;
    while (curr->ptr != NULL) {
        curr = curr->ptr;
    }
    return(curr);
} */

struct _OpNode * _OpWalk(struct _FuncNode * n) {
    // Returns pointer to last node in operation chain
    struct _OpNode * curr = n->ptr;
    while (curr->ptr != NULL) {
        curr = curr->ptr;
    }
    return(curr);
}

struct _FuncNode * FuncPush(struct _FuncNode * n, char h[]) {
    // Push operation for function stack
    struct _FuncNode * new_FuncNode = malloc(sizeof(struct _FuncNode));
    strcpy(new_FuncNode->handle, h);
    new_FuncNode->type = link;
    new_FuncNode->func = NULL;

    struct _FuncNode * last_FuncNode = _FuncWalk(n);

    last_FuncNode->func = new_FuncNode;

    // Initialize head _OpNode beside _FuncNode
    struct _OpNode * op_head = malloc(sizeof(struct _OpNode));
    op_head->op = op_func_init;
    op_head->data_i = 0;
    op_head->ptr = NULL;
    op_head->type = head;

    new_FuncNode->ptr = op_head;

    return(new_FuncNode);
}

struct _CallNode * CallPush(struct _CallNode * n, struct _OpNode * r) {
    // Push operation for return stack
    struct _CallNode * new_CallNode = malloc(sizeof(struct _CallNode));
    new_CallNode->ptr = NULL;
    new_CallNode->type = link;
    new_CallNode->ret = r;

    struct _CallNode * last_CallNode = _CallWalk(n);

    last_CallNode->ptr = new_CallNode;

    return(new_CallNode);
}

/* struct _ConstNode * ConstPush(struct _ConstNode * n, char h[], int v) {
    // Push operation for const stack
    struct _ConstNode * new_ConstNode = malloc(sizeof(struct _ConstNode));
    strcpy(new_ConstNode->handle, h);
    new_ConstNode->ptr = NULL;
    new_ConstNode->type = link;
    new_ConstNode->value = v;
    
    struct _ConstNode * last_ConstNode = _ConstWalk(n);

    last_ConstNode->ptr = new_ConstNode;

    return(new_ConstNode);
} */

/* int ConstPeek(struct _ConstNode * n, char h[]) {
    struct _ConstNode * curr = n;

    while (curr) {
        if (!strcmp(curr->handle, h) && curr->type == link) {
            return (curr->value);
        }
        curr = curr->ptr;
    }
    return(0);
} */

struct _OpNode * OpPush(struct _FuncNode * n, Operations o) {
    // Push operation for operation chain within function
    struct _OpNode * new_OpNode = malloc(sizeof(struct _OpNode));
    new_OpNode->data_i = 0;
    new_OpNode->op = o;
    new_OpNode->ptr = NULL;
    new_OpNode->type = link;

    struct _OpNode * last_OpNode = _OpWalk(n);

    last_OpNode->ptr = new_OpNode;

    return(new_OpNode);
}

struct _OpNode * OpPushInt(struct _FuncNode * n, Operations o, int i) {
    // Push operation for operation chain within function
    struct _OpNode * new_OpNode = malloc(sizeof(struct _OpNode));
    new_OpNode->data_i = i;
    new_OpNode->op = o;
    new_OpNode->ptr = NULL;
    new_OpNode->type = link;

    struct _OpNode * last_OpNode = _OpWalk(n);

    last_OpNode->ptr = new_OpNode;

    return(new_OpNode);
}

struct _OpNode * OpPushStr(struct _FuncNode * n, Operations o, char * s) {
    // Push operation for operation chain within function
    struct _OpNode * new_OpNode = malloc(sizeof(struct _OpNode));
    new_OpNode->data_s = malloc(strlen(s));
    strcpy(new_OpNode->data_s, s);
    new_OpNode->op = o;
    new_OpNode->ptr = NULL;
    new_OpNode->type = link;

    struct _OpNode * last_OpNode = _OpWalk(n);

    last_OpNode->ptr = new_OpNode;

    return(new_OpNode);
}

void Cleanup(struct _FuncNode * n) {
    struct _FuncNode * curr_FuncNode = n->func;
    while (curr_FuncNode->func != NULL) {
        struct _OpNode * curr_OpNode = curr_FuncNode->ptr;
        while (curr_OpNode->ptr != NULL) {
            struct _OpNode * next_OpNode = curr_OpNode->ptr;
            free(curr_OpNode);
            curr_OpNode = next_OpNode;
        }
        struct _FuncNode * next_FuncNode = curr_FuncNode->func;
        free(curr_FuncNode);
        curr_FuncNode = next_FuncNode;
    }
    n->func = NULL;
}

int CallLength(struct _CallNode * n) {
    // Get the current length of ret stack
    int link_length = 0;

    struct _CallNode * curr = n;
    while (curr->ptr != NULL) {
        curr = curr->ptr;
        link_length++;
    }
    return(link_length);
}

struct _OpNode * CallPop(struct _CallNode * n) {
    // Pop operation for ret stack
    struct _CallNode * curr = n;
    struct _CallNode * next;

    if (curr->ptr == NULL) {
        fprintf(stderr, "Error: ret stack contents insufficient for pop operation\n");
        exit(1);
    } else {
        next = curr->ptr;
        while (next->ptr != NULL) {
            curr = curr->ptr;
            next = next->ptr;
        }
    }

    struct _OpNode * m = next->ret;
    curr->ptr = NULL;

    free(next); // After storing the contents of the last link, we can free its memory.
    return(m);
}

void CallPrint(struct _CallNode * n)
{
    struct _CallNode * curr = n;
    while (curr)
    {
        printf("\n ** _CallNode **\n");
        printf("    addr: %p\n", curr);
        printf("    type: %s\n", curr->type ? "link" : "head");
        printf("     ret: %p\n", curr->ret);
        printf("     ptr: %p\n", curr->ptr);
        printf(" **************\n");
        curr = curr->ptr;
    }
}

void MemMapPrint(struct _FuncNode * n) {
    struct _FuncNode * curr_FuncNode = n;
    while (curr_FuncNode != NULL) {
        printf("\n *** _FuncNode ***\n");
        printf(" *    addr: %p\n", curr_FuncNode);
        printf(" *  handle: %s\n", curr_FuncNode->handle);
        printf(" *    type: %s\n", curr_FuncNode->type ? "link" : "head");
        printf(" *    func: %p\n", curr_FuncNode->func);
        printf(" *     ptr: %p\n", curr_FuncNode->ptr);
        printf(" *****************\n");

        struct _OpNode * curr_OpNode = curr_FuncNode->ptr;
        while (curr_OpNode != NULL) {
            printf("\n           _OpNode: %p\n", curr_OpNode);
            printf("              type: %s\n", curr_OpNode->type ? "link" : "head");
            printf("                op: %d\n", curr_OpNode->op);
            printf("              op_s: '%s'\n", get_keyword(curr_OpNode->op));
            printf("            data_s: '%s'\n", curr_OpNode->data_s);
            printf("            data_i: %d\n", curr_OpNode->data_i);
            printf("               ptr: %p\n", curr_OpNode->ptr);
            curr_OpNode = curr_OpNode->ptr;
        }
        curr_FuncNode = curr_FuncNode->func;
    }
}

int IsFunc(struct _FuncNode * n, char h[])
{
    struct _FuncNode * curr = n;
    while (curr)
    {
        if (!strcmp(curr->handle, h))
        {
            return (1);
        }
        curr = curr->func;
    }
    return (0);
}

struct _FuncNode * GetFuncAddr(struct _FuncNode * n, char h[])
{
    struct _FuncNode * curr = n;
    while (curr)
    {
        if (!strcmp(curr->handle, h))
        {
            return (curr);
        }
        curr = curr->func;
    }
    return (NULL);
}

// ====== Generic Linked List ======

struct _Node Initialize(void) {
    // Initialize empty list
    struct _Node to_init;
    to_init.data = 0;
    to_init.type = head;
    to_init.ptr = NULL;
    return(to_init);
}

struct _Node * _walk(struct _Node * n) {
    // Returns pointer to last node in list
    struct _Node * curr = n;
    while (curr->ptr != NULL) {
        curr = curr->ptr;
    }
    return(curr);
}

void LinkPush(struct _Node * n, int d) {
    // Push operation for using linked list like a stack
    struct _Node * new_node = malloc(sizeof(struct _Node));
    new_node->data = d;
    new_node->type = link;
    new_node->ptr = NULL;

    struct _Node * last_node = _walk(n);

    last_node->ptr = new_node;
}

int LinkPop(struct _Node * n) {
    // Pop operation for using linked list like a stack
    struct _Node * curr = n;
    struct _Node * next;

    if (curr->ptr == NULL) {
        fprintf(stderr, "Error: Insufficient list contents for operation: pop\n");
        exit(1);
    } else {
        next = curr->ptr;
        while (next->ptr != NULL) {
            curr = curr->ptr;
            next = next->ptr;
        }
    }

    int m = next->data;
    curr->ptr = NULL;

    free(next); // After storing the contents of the last link, we can free its memory.
    return(m);
}

int LinkLength(struct _Node * n) {
    // Get the current length of a list
    int link_length = 0;

    struct _Node * curr = n;
    while (curr->ptr != NULL) {
        curr = curr->ptr;
        link_length++;
    }
    return(link_length);
}

int LinkPeek(struct _Node * n) {
    struct _Node * end = _walk(n);
    return(end->data);
}

void LinkInsert(struct _Node * n, int d, int ind) {
    // Insert a link at the specified index
    struct _Node * before = n;
    if (ind > LinkLength(n)) {
        fprintf(stderr, "Error: Insufficient list contents for operation: insert\n");
        exit(1);
    } else if (ind < 0) {
        fprintf(stderr, "Error: Index must be greater than or equal to zero\n");
        exit(1);
    }
    for (int i = 0; i < ind; i++) {
        before = before->ptr;
    }

    struct _Node * after = before->ptr;

    struct _Node * new_node = malloc(sizeof(struct _Node));
    new_node->data = d;
    new_node->type = link;
    new_node->ptr = after;

    before->ptr = new_node;
}

void PrintNodes(struct _Node * n) {
    // Print attributes of all nodes in list
    struct _Node * curr = n;
    while (curr != NULL) {
        printf("== Node == \n");
        printf("  data: %d\n", curr->data);
        printf("  type: %s\n", curr->type? "link" : "head");
        printf("   ptr: %p\n\n", curr->ptr);
        curr = curr->ptr;
    }
}