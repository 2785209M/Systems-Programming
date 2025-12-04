#include <stdio.h>
#include <string.h>

#define MAX_STRLEN 512
#define MAX_CHAR 26

/* This is the O(N^2) solution. */
char* common_chars_O_N2(const char* a, const char* b) {
    static char result[MAX_STRLEN]; // Buffer for the result
    int k = 0;
    for (int i = 0; i < strlen(a); i++) {
        for (int j = 0; j < strlen(b); j++) {
            if (a[i] == b[j] && (result[k-1] != a[i] || k == 0)) {
                result[k] = a[i];
                k++;
                break;
            }
        }
    }
    result[k] = '\0';
    return result;
}

/* This is the O(N) solution. It assumes that strings contain only lower-case letters */
char* common_chars_O_N(const char* a, const char* b) {
    static char result[MAX_STRLEN]; // Buffer for the result
    int found[MAX_CHAR];    //Array to mark found characters
    for (int i = 0 ; i < MAX_CHAR ; i++)
        found[i] = 0;

    int k = 0;

    // Mark characters in string b
    for (int i = 0; i < strlen(b); i++) {
        found[b[i]-'a'] = 1;    //Mark all characters found in second string
    }
    for (int i = 0 ; i < strlen(a) ; i++) {
        if (found[a[i]-'a']) {
            result[k] = a[i];
            found[a[i]-'a'] = 0;    //Unmark found character to avoid duplicates
            k++;
        }
    }


    result[k] = '\0'; // Null-terminate the result string
    return result;
}


int main() {
    const char* str1 = "hello";
    const char* str2 = "world";
    printf("Common characters O(N^2): %s\n", common_chars_O_N2(str1, str2));
    printf("Common characters O(N): %s\n", common_chars_O_N(str1, str2));

    return 0;
}
