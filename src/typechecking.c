char ArithmeticOperators[] = {
    '+', 
    '-', 
    '*', 
    '/', 
    '%', 
    '\0'
};

char BooleanShortComparisons[] = {
    '>',
    '<',
    '=',
    '\0'
};

int strIsNumeric(char *input) {
    //printf("strIsNumeric(): length %d\n", strlen(input));
    int isNegative = 0;
    if (input[0] == '-') isNegative = 1;
    if (isNegative) {
        for (int i = 1; i < strlen(input); i++){
            if (input[i] < '0' || input[i] > '9') return 0;
        }
    }
    else {
        for (int i = 0; i < strlen(input); i++){
            if (input[i] < '0' || input[i] > '9') return 0;
        }
    }
    return 1;
}

int strIsFloatNumeric(char *input) {
    float a = fmod(strtof(input, NULL), 1.0);
    //printf("Modulo op: %f", a);
    if (a != 0.0 && input != "."){
        return 1;
    }
    return 0;
}

int strIsStringLiteral(char *input) {
    if (input[0] == '\"' && input[strlen(input)-1] == '\"') return 1;
    return 0;
}

int strIsArithmeticOperator(char *input) {
    //printf("arith op %d\n", strlen(ArithmeticOperators));
    for (int i = 0; i < strlen(ArithmeticOperators); i++) {
        
        if (input[0] == ArithmeticOperators[i]) return 1;
    }
    return 0;
}

int strIsBooleanShort(char *input) {
    for (int i = 0; i < strlen(BooleanShortComparisons); i++) {
        char *b_short = (char *) calloc(1, sizeof(char));
        strncpy(b_short, &BooleanShortComparisons[i], sizeof(char));
        if (!strcmp(input, b_short)) return 1;
    }
    return 0;
}

int strIsBooleanLong(char *input) {
    if (!strcmp(input, "and") || !strcmp(input, "or") || !strcmp(input, "invert")) return 1;
    return 0;
}

int strIsGenericOperator(char *input) {
    if (!strcmp(input, "cr") || !strcmp(input, ".") || !strcmp(input, "emit") || !strcmp(input, "swap") || !strcmp(input, "dup")) return 1;
    return 0;
}

int strIsConstVarDeclaration(char *input) {
    if (!strcmp(input, "const") || !strcmp(input, "var")) return 1;
    return 0;
}