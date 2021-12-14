#define STACK_HEIGHT_MAX 1000
#define STRING_LEN_MAX 100

enum StackItem_Type {SI_int, SI_float, SI_str};

typedef struct {
    enum StackItem_Type SI_type;
    union {
        int   SI_i;
        float SI_f;
        char *SI_s;
    };
} StackItem;

typedef struct {
    StackItem data[STACK_HEIGHT_MAX];
    int height;
} Stack;

Stack MainStack;


int pushIntToStack(int n) {
    if (MainStack.height != STACK_HEIGHT_MAX) {
        
    } else {

    }
}

int pushFloatToStack() {
    if (MainStack.height != STACK_HEIGHT_MAX) {

    } else {

    }
}

/* int pushIntToStack(int input) {
    if (MainStack.height != STACK_HEIGHT_MAX) {
        StackItem new_item;
        new_item.value = input;
        new_item.isStr = 0;
        MainStack.data[MainStack.height++] = new_item;
        return 1;
    }
    return 0;
} */
/* 
int pushToStack(float input) {
    if (MainStack.height != STACK_HEIGHT_MAX) {
        StackItem new_item;
        new_item.value = input;
        new_item.isStr = 0;
        MainStack.data[MainStack.height++] = new_item;
        return 1;
    }
    return 0;
}

int pushStrToStack(char *input) {
    if (MainStack.height != STACK_HEIGHT_MAX) {
        StackItem new_item;
        new_item.value = 0;
        new_item.svalue = (char *) calloc(STRING_LEN_MAX, sizeof(char));
        new_item.isStr = 1;
        strcpy(new_item.svalue, input);
        MainStack.data[MainStack.height++] = new_item;
        return 1;
    }
    return 0;
}

int popFromStack() {
    if (MainStack.height > 0) {
        MainStack.height--;
        return MainStack[MainStack.height+1];
    }
    return 0;
} */
