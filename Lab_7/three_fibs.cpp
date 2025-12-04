#include <stdio.h>

int fib(int x, int x1 = 0, int x2 = 1) {
    return (x == 0) ? x2 : fib(x - 1, x1 + x2, x1);
}

int main(){
    printf("%d\n",fib(40));
    printf("%d\n",fib(41));
    printf("%d\n",fib(42));

    return 0;
}