#include <stdio.h>
#include <string.h>

#define MAX_LINE_SIZE 1024
/* readline: read a line from standard input, return its length or 0 */
int readline(char line[], int max) {
    if (fgets(line, max, stdin) == NULL)
        return 0;
    else
        return strlen(line);
}

int main() {
    char line[MAX_LINE_SIZE]; 
    int line_count = 0;
    while (readline(line, MAX_LINE_SIZE) > 0) {
        line_count++;
    }
    printf("Line count: %d\n", line_count);
    return 0;
}
