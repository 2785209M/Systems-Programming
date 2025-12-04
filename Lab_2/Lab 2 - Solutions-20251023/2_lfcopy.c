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

/* writeline: write line to standard output, returning number of chars written */
int writeline(const char line[]) {
    fputs(line, stdout);
    return strlen(line);
}

int main() {
    char line[MAX_LINE_SIZE]; 
    while (readline(line, MAX_LINE_SIZE) > 0) {
        writeline(line);
    }
    return 0;
}

