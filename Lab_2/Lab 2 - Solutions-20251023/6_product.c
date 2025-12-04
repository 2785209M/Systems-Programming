#include <stdio.h>

#define ARRAY_LEN 10
int main() {
    int A[ARRAY_LEN] = {5, 3, 16, 23, 17, 8, 9, 3, 2, 12};
    int Output[ARRAY_LEN];
    
    int product_left = 1; 
    int product_right = 1;

    // Initialize Output array
    for (int i = 0 ; i < ARRAY_LEN ; i++) {
        Output[i] = 1;
    }

    // First pass: Calculate products of all numbers left of A[i]
    for (int i = 0 ; i < ARRAY_LEN ; i++) {
        Output[i] *= product_left;
        product_left *= A[i];
    }

    //Second pass: Calculate products of all number right of A[i] and multiply 
    for (int i = ARRAY_LEN - 1; i >= 0 ; i--) {
        Output[i] *= product_right;
        product_right *= A[i];
    }

    //Print
    for (int i = 0 ; i < ARRAY_LEN ; i++) {
        printf("%d ", Output[i]);
    }
    printf("\n");

}
