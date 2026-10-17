#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main() {
    char input[1000];
    int i = 0;

    printf("Enter a statement: ");
    fgets(input, sizeof(input), stdin);

    while (input[i] != '\0') {
        if (isspace(input[i])) {
            i++;
        }
        else if (isalpha(input[i]) || input[i] == '_') {
            printf("Identifier/Keyword: ");
            while (isalnum(input[i]) || input[i] == '_') {
                putchar(input[i]);
                i++;
            }
            printf("\n");
        }
        else if (isdigit(input[i])) {
            printf("Number: ");
            while (isdigit(input[i]) || input[i] == '.') {
                putchar(input[i]);
                i++;
            }
            printf("\n");
        }
        else {
            printf("Operator/Symbol: %c\n", input[i]);
            i++;
        }
    }

    return 0;
}