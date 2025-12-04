#include <stdio.h>
#include <thread>

int fib(int x, int x1 = 0, int x2 = 1) {
    return (x == 0) ? x2 : fib(x - 1, x1 + x2, x1);
}

int main(){
    auto t = std::thread([]{
    printf("%d\n",fib(40));
    });
    printf("%d\n",fib(41));
    printf("%d\n",fib(42));

    return 0;
}
