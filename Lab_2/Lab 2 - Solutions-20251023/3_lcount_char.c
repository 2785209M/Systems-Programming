#include <stdio.h>

int main() {
    char c='0'; 
    int line_count = 0;
    while (scanf("%c", &c) == 1) {
        if (c == '\n' || c == EOF) {
           line_count++; 
        }
    }
    printf("Line count: %d\n", line_count);
}
