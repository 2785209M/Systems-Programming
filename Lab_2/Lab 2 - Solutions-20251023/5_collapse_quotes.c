#include <stdio.h>
#include <ctype.h>

int main() {
    char c='0'; 
    int flag = 0; //flag: false if previous character is whitespace, true otherwise
    int flag_quote = 0; //another flag for quotes
    while (scanf("%c", &c) == 1) {
        if (isspace(c)) {
            if (!flag) {  //if flag is false
                if (!flag_quote) { //if not in quotes
                    printf(" "); //print single whitespace
                    flag = 1;   //set the flag for whitespace
                }
                else { //if in quotes
                    printf("%c", c);    //print the whitespace
                }
            }
        }
        else {
            if (c == '"') {     //keep track of pairs of quotes
                if (!flag_quote) {  
                    flag_quote = 1; 
                }
                else {
                    flag_quote = 0;
                }
            }
            printf("%c", c);
            flag = 0;   //unset the flag, not a whitespace
        }
    }
    printf("\n");
}
