#ifndef _DATE_H_INCLUDED_
#define _DATE_H_INCLUDED_

#include <time.h>

// Opaque structure for a date/timestamp.
typedef struct date Date;

/*
 * date_create creates a Date structure from `datestr`.
 * `datestr` is expected to be of the form "dd/mm/yyyy hh:mm".
 * Returns a pointer to a new Date structure if successful, NULL otherwise.
 * The caller is responsible for destroying the returned date.
 */
Date *date_create(const char *datestr);

/*
 * date_valid checks if a Date structure is valid.
 * Returns 1 if valid, 0 otherwise.
 *
 * This function is used internally by functions that parse or copy
 * dates, such as date_create(), to verify that all fields
 * (day, month, year, hour, minute) fall within valid ranges before storing or
 * using the date.
 */
int date_valid(const Date *d);

/*
 * date_duplicate creates a new copy of a Date structure.
 * Returns a pointer to the new Date structure, or NULL on failure.
 * The caller is responsible for destroying the returned date.
 */
Date *date_duplicate(const Date *d);

/*
 * date_format_last_seen returns a newly allocated string describing how long
 * ago a user was last seen (last), relative to the current date (now).
 * - If the date is today, returns "last seen at hh:mm".
 * - If within the same month, returns "last seen X days ago".
 * - Otherwise, returns "last seen on dd/mm/yyyy".
 * The caller is responsible for freeing the returned string.
 * Returns NULL on memory allocation failure.
 */
char *date_format_last_seen(const Date *last, const Date *now);

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
Date *date_now(void);

/*
 * date_destroy frees all memory associated with a Date structure.
 */
void date_destroy(Date *d);

#endif /* _DATE_H_INCLUDED_ */
