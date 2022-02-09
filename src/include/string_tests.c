int strIsNumeric(char *input) {
    //printf("strIsNumeric(): length %d\n", strlen(input));
    int isNegative = 0;
    if (input[0] == '-') isNegative = 1;
    if (isNegative) {
        for (int i = 1; i < (int) strlen(input); i++){
            if (input[i] < '0' || input[i] > '9') return 0;
        }
    }
    else {
        for (int i = 0; i < (int) strlen(input); i++){
            if (input[i] < '0' || input[i] > '9') return 0;
        }
    }
    return 1;
}