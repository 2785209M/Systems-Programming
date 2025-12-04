#include <stdio.h>
#include <string.h>
/* readline: read a line from standard input,#include <stdio.h>
return its length or 0 */

int readline(char line[], int max) {
if (fgets(line, max, stdin) == NULL)
return 0;
else
return strlen(line);
}

/* writeline: write line to standard output,
return number of chars written */
int writeline(const char line[]) {
fputs(line, stdout);
return strlen(line);
}

int main(){
    char string[50];
    int len;

    while((len = readline(string, sizeof(string))) > 0) {writeline(string);}
}