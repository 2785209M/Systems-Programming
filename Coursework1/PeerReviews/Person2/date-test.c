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
    Date *d = date_create("32/10/2025 14:30"); // invalid day
    if (!d) {
        printf("PASS: invalid date rejected\n");
    } else {
        printf("FAIL: invalid date accepted\n");
        date_destroy(d);
    }
}

static void test_duplicate(void) {
    printf("== Test: duplicate ==\n");
    Date *d1 = date_create("01/10/2025 14:30");
    Date *d2 = date_duplicate(d1);
    if (d2 && date_valid(d2)) {
        printf("PASS: duplicate created\n");
    } else {
        printf("FAIL: duplicate failed\n");
    }
    date_destroy(d1);
    date_destroy(d2);
}

static void test_last_seen_today(void) {
    printf("== Test: last seen today ==\n");
    Date *now = date_now();
    Date *dup = date_duplicate(now);
    char *msg = date_format_last_seen(dup, now);
    printf("Output: %s\n", msg);
    free(msg);
    date_destroy(now);
    date_destroy(dup);
}

static void test_last_seen_days_ago(void) {
    printf("== Test: last seen days ago ==\n");
    Date *ref = date_create("05/10/2025 12:00");
    Date *two_days = date_create("03/10/2025 10:00");
    char *msg = date_format_last_seen(two_days, ref);
    printf("Output: %s\n", msg);
    free(msg);
    date_destroy(ref);
    date_destroy(two_days);
}

static void test_last_seen_old(void) {
    printf("== Test: last seen old ==\n");
    Date *ref = date_create("30/09/2025 12:00");
    Date *old = date_create("20/08/2025 15:00");
    char *msg = date_format_last_seen(old, ref);
    printf("Output: %s\n", msg);
    free(msg);
    date_destroy(ref);
    date_destroy(old);
}

int main(void) {
    test_create_valid();
    test_create_invalid();
    test_duplicate();
    test_last_seen_today();
    test_last_seen_days_ago();
    test_last_seen_old();
    return 0;
}

