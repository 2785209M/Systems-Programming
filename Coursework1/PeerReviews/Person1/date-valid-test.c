#include <stdio.h>
#include <stdlib.h>
#include "date.h"

static void test_create_valid(void) {
    printf("== Test: create valid date ==\n");
    Date *d = date_create("01/10/2025 14:30");
    if (date_valid(d)) {
        printf("PASS: valid date accepted\n");
    } else {
        printf("FAIL: valid date rejected\n");
    }
    date_destroy(d);
}

static void test_create_invalid(void) {
    printf("== Test: create invalid date ==\n");
    Date *d = date_create("32/12/9999 14:30"); // invalid day
    if (!d) {
        printf("PASS: invalid date rejected\n");
    } else {
        printf("FAIL: invalid date accepted\n");
        date_destroy(d);
    }
}

int main(void) {
    test_create_valid();
    test_create_invalid();
    return 0;
}