#include <stdio.h>
#include <stdlib.h>
#include "date.h"

int main(void) {
    // Test date_create
    Date *d = date_create("25/10/2023 14:30");
    if (!d) {
        printf("Failed to create date\n");
        return 1;
    }

    // Test date_format_last_seen
    char *s = date_format_last_seen(d, NULL);
    if (s) {
        printf("Format test: %s\n", s);
        free(s);
    }

    // Test date_duplicate
    Date *d2 = date_duplicate(d);
    if (d2) {
        char *s2 = date_format_last_seen(d2, d);
        if (s2) {
            printf("Duplicate test: %s\n", s2);
            free(s2);
        }
        date_destroy(d2);
    }

    date_destroy(d);
    return 0;
}