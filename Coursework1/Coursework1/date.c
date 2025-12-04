#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "date.h"

struct date{
    int year, month, day, hour, minute;
};

/*
 * date_create creates a Date structure from `datestr`.
 * `datestr` is expected to be of the form "dd/mm/yyyy hh:mm".
 * Returns a pointer to a new Date structure if successful, NULL otherwise.
 * The caller is responsible for destroying the returned date.
 */
Date *date_create(const char *datestr) {
    if (!datestr) return NULL;

    // Assign memory to the new date struct
    Date *d = malloc(sizeof *d);
    if (!d) return NULL;

    // Check thre date format is correct
    if (sscanf(datestr, "%d/%d/%d %d:%d", &d->day, &d->month, &d->year, &d->hour, &d->minute) != 5) {
        free(d);
        return NULL;
    }

    // Use the date_valid function to check the date is valid
    if (!date_valid(d)) {
        free(d);
        return NULL;
    }

    return d;
}

/*
 * date_valid checks if a Date structure is valid.
 * Returns 1 if valid, 0 otherwise.
 *
 * This function is used internally by functions that parse or copy
 * dates, such as date_create(), to verify that all fields
 * (day, month, year, hour, minute) fall within valid ranges before storing or
 * using the date.
 */
static int is_leap_year(int year) {
    // Check if the given year is a leap year
    if(!year) return 0;
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int days_in_month(int month, int year) {
    if(!month || !year) return 0;

    // Check how many days can be in the given month
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
        case 4: case 6: case 9: case 11: return 30;
        case 2: return is_leap_year(year) ? 29 : 28;
        default: return 0;
    }
}

int date_valid(const Date *d) {
    if (!d) return 0;
    int valid_days = days_in_month(d->month, d->year);

    // Check that all numbers are valid
    if (d->year < 1) return 0;
    if (d->month < 1 || d->month > 12) return 0;
    if (d->hour < 0 || d->hour > 23) return 0;
    if (d->minute < 0 || d->minute > 59) return 0;

    if (valid_days == 0) return 0;
    if (d->day < 1 || d->day > valid_days) return 0;

    return 1;
}

/*
 * date_duplicate creates a new copy of a Date structure.
 * Returns a pointer to the new Date structure, or NULL on failure.
 * The caller is responsible for destroying the returned date.
 */
Date *date_duplicate(const Date *d){
    if(!d){return NULL;}

    // Assign memory to the new duplicate date struct
    Date *new_d = malloc(sizeof(*new_d));
    if(!new_d){return NULL;}

    // Set the parameters of the duplicate to be the same as the original
    *new_d = *d;
    
    return new_d;
}

/*
 * date_format_last_seen returns a newly allocated string describing how long
 * ago a user was last seen (last), relative to the current date (now).
 * - If the date is today, returns "last seen at hh:mm".
 * - If within the same month, returns "last seen X days ago".
 * - Otherwise, returns "last seen on dd/mm/yyyy".
 * The caller is responsible for freeing the returned string.
 * Returns NULL on memory allocation failure.
 */
char *date_format_last_seen(const Date *last, const Date *now){
    if(!last) return NULL;

    Date *tmp_now = NULL;
    const Date *n = now;
    
    // If now isn't given, use date_now to calculate now
    if(!n){
        tmp_now = date_now();
        if(!tmp_now) return NULL;
        n = tmp_now;
    }
    
    // Allocate memory to the result
    char *result = malloc(128);
    if(!result){
        if(tmp_now) date_destroy(tmp_now);
        return NULL;
    }

    // Set the result format based on the date last seen
    if(n->year == last->year && n->month == last->month && n->day == last->day) { 
        snprintf(result, 128, "last seen at %02d:%02d", last->hour, last->minute); }
    else if(n->year == last->year && n->month == last->month) {
        int nofdays = n->day - last->day;
        if(nofdays < 0) nofdays = 0; // If the number of days is <0 (invalid) clamp to 0
        snprintf(result, 128, "last seen %d days ago", nofdays); } 
    else { snprintf(result, 128, "last seen on %02d/%02d/%04d", last->day, last->month, last->year); }

    // Free the memory allocated to the local variable 'now'
    if(tmp_now) date_destroy(tmp_now);

    return result;
}

/*
 * date_now creates a Date structure for the current system date and time.
 * This function should be implemented using <time.h> facilities such as
 * time(), localtime(), and struct tm.
 * Returns a pointer to the new Date structure, or NULL on failure.
 * The caller is responsible for destroying the returned date.
 *
 * This function is used internally by date_format_last_seen()
 * to compare "last seen" times to the current system date and time.
 */
Date *date_now(void) {
    time_t current_time = time(NULL);
    struct tm *time_info = localtime(&current_time);
    if (!time_info) return NULL;

    // allocate memory to the current_date instance
    Date *current_date = malloc(sizeof *current_date);
    if (!current_date) return NULL;

    // set the parameters of current_date using localtime from the time.h header file
    current_date->day    = time_info->tm_mday;
    current_date->month  = time_info->tm_mon + 1;
    current_date->year   = time_info->tm_year + 1900;
    current_date->hour   = time_info->tm_hour;
    current_date->minute = time_info->tm_min;

    return current_date;
}

/*
 * date_destroy frees all memory associated with a Date structure.
 */
void date_destroy(Date *d){
    if(!d) return;
    free(d);
}