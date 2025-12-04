#include <stdio.h>
#include <ctype.h>

int main() {
    char c='0'; 
    int word_count = 0;
    int flag = 0; //flag: false if previous character is whitespace/cntrl/punct/digit, true otherwise
    while (scanf("%c", &c) == 1) {
        if (isdigit(c) || iscntrl(c) || isspace(c) || ispunct(c)) {
            if (!flag) {  //if flag is false
                word_count++; //count a word
                flag = 1;   //set the flag
            }
        }
        else {
            flag = 0;   //
        }
    }
    printf("Word count: %d\n", word_count);
}
