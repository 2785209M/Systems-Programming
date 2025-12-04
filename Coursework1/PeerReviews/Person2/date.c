#include "date.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct date {
    int day;
    int month;
    int year;
    int hour;
    int minute;
};

Date *date_create(const char *datestr) {
    if (!datestr) return NULL;
    Date *d = malloc(sizeof(Date));
    if (!d) return NULL;
    if (sscanf(datestr, "%d/%d/%d %d:%d", &d->day, &d->month, &d->year, &d->hour, &d->minute) != 5) {
        free(d);
        return NULL;
    }
    if (!date_valid(d)) {
        free(d);
        return NULL;
    }
    return d;
}

int date_valid(const Date *d) {
    if (!d) return 0;
    if (d->month < 1 || d->month > 12) return 0;
    if (d->day < 1 || d->day > 31) return 0;
    if (d->year < 0) return 0; // assuming year >=0
    if (d->hour < 0 || d->hour > 23) return 0;
    if (d->minute < 0 || d->minute > 59) return 0;
    // Basic month day check
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (d->month == 2 && ((d->year % 4 == 0 && d->year % 100 != 0) || d->year % 400 == 0)) days_in_month[1] = 29;
    if (d->day > days_in_month[d->month - 1]) return 0;
    return 1;
}

Date *date_duplicate(const Date *d) {
    if (!d) return NULL;
    Date *dup = malloc(sizeof(Date));
    if (!dup) return NULL;
    dup->day = d->day;
    dup->month = d->month;
    dup->year = d->year;
    dup->hour = d->hour;
    dup->minute = d->minute;
    return dup;
}

char *date_format_last_seen(const Date *last, const Date *now) {
    if (!last || !now) return NULL;
    char *str = malloc(256);
    if (!str) return NULL;
    if (last->year == now->year && last->month == now->month && last->day == now->day) {
        sprintf(str, "last seen at %02d:%02d", last->hour, last->minute);
    } else if (last->year == now->year && last->month == now->month) {
        int days = now->day - last->day;
        sprintf(str, "last seen %d days ago", days);
    } else {
        sprintf(str, "last seen on %02d/%02d/%04d", last->day, last->month, last->year);
    }
    return str;
}

Date *date_now(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    if (!tm) return NULL;
    Date *d = malloc(sizeof(Date));
    if (!d) return NULL;
    d->day = tm->tm_mday;
    d->month = tm->tm_mon + 1;
    d->year = tm->tm_year + 1900;
    d->hour = tm->tm_hour;
    d->minute = tm->tm_min;
    return d;
}

void date_destroy(Date *d) {
    free(d);
}
