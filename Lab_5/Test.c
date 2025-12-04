#include <stdio.h>

int main(){
    void * ptr1 = malloc(sizeof(int));
    void * ptr2 = ptr1;

    free(ptr1);
}