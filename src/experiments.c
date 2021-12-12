#include <stdlib.h>

int main() {
    char *test = (char *) calloc(4, sizeof(char));
    strcpy(test, "-420.69");
    printf("string: %s\n", test);

    float holder = strtof(test, NULL);

    printf("int:    %f\n", holder);
}