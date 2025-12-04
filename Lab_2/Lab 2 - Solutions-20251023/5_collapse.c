#include <stdio.h>
#include <ctype.h>

int main() {
    char c='0'; 
    int flag = 0; //flag: false if previous character is whitespace, true otherwise
    while (scanf("%c", &c) == 1) {
        if (isspace(c)) {
            if (!flag) {  //if flag is false
                printf(" "); //print single whitespace
                flag = 1;   //set the flag for whitespace
            }
        }
        else {
            printf("%c", c);
            flag = 0;   //unset the flag, not a whitespace
        }
    }
    printf("\n");
}
