#define _POSIX_C_SOURCE 200809L
#include "date.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Opaque structure
struct date {
	struct tm timeinfo;   //time break down
	time_t time;    //comparison time
};

//Create date
Date *date_create(const char *datestr) {
	if (!datestr) return NULL;

	int day, month, year, hour, minute;
	if (sscanf(datestr, "%2d/%2d/%4d %2d:%2d",
			   &day, &month, &year, &hour, &minute) != 5) {
		return NULL;
	}

//Standardise
	struct tm parsed_tm = {0};
	parsed_tm.tm_mday = day;
	parsed_tm.tm_mon = month - 1;     // Convert to 0-based 
	parsed_tm.tm_year = year - 1900;  // tm_year counts from 1900 
	parsed_tm.tm_hour = hour;
	parsed_tm.tm_min = minute;
	parsed_tm.tm_sec = 0;
	parsed_tm.tm_isdst = -1;

	time_t t = mktime(&parsed_tm);
	if (t == (time_t)-1) return NULL;

	Date *new_date = malloc(sizeof(Date));
	if (!new_date) return NULL;

	new_date->timeinfo = parsed_tm;
	new_date->time = t;

	if (!date_valid(new_date)) {
		free(new_date);
		return NULL;
	}

	return new_date;
}

//valid feild ranges
int date_valid(const Date *date) {
	if (!date) return 0;

	int y = date->timeinfo.tm_year + 1900;
	int m = date->timeinfo.tm_mon + 1;
	int d = date->timeinfo.tm_mday;

	if (y < 1900 || y > 9999) return 0;
	if (m < 1 || m > 12) return 0;
	if (d < 1 || d > 31) return 0;
	if (date->timeinfo.tm_hour < 0 || date->timeinfo.tm_hour > 23) return 0;
	if (date->timeinfo.tm_min < 0 || date->timeinfo.tm_min > 59) return 0;

	return 1;
}

//Duplicate the date
Date *date_duplicate(const Date *original) {
	if (!original) return NULL;

	Date *copy = malloc(sizeof(Date));
	if (!copy) return NULL;

	*copy = *original;
	return copy;
}

//Create readible format
char *date_format_last_seen(const Date *last, const Date *current) {
	if (!last || !current) return NULL;

	char buffer[64];

	//Same day show exact time
	if (last->timeinfo.tm_year == current->timeinfo.tm_year && last->timeinfo.tm_mon == current->timeinfo.tm_mon && last->timeinfo.tm_mday == current->timeinfo.tm_mday) {
		snprintf(buffer, sizeof(buffer), "last seen at %02d:%02d", last->timeinfo.tm_hour, last->timeinfo.tm_min);
	}
	//Same month show days ago
	else if (last->timeinfo.tm_year == current->timeinfo.tm_year && last->timeinfo.tm_mon == current->timeinfo.tm_mon) {
		struct tm last_tm = last->timeinfo;
		struct tm now_tm = current->timeinfo;

		last_tm.tm_hour = last_tm.tm_min = last_tm.tm_sec = 0;
		now_tm.tm_hour = now_tm.tm_min = now_tm.tm_sec = 0;

		long days_diff = (long)((mktime(&now_tm) - mktime(&last_tm)) / 86400);
		if (days_diff < 0) days_diff = 0;

		snprintf(buffer, sizeof(buffer), "last seen %ld days ago", days_diff);
	}
	//More than month show specific date
	else {
		snprintf(buffer, sizeof(buffer), "last seen on %02d/%02d/%04d", last->timeinfo.tm_mday, last->timeinfo.tm_mon + 1, last->timeinfo.tm_year + 1900);
	}

	return strdup(buffer);
}

//Current date
Date *date_now(void) {
	time_t t = time(NULL);
	if (t == (time_t)-1) return NULL;

	struct tm *tm_ptr = localtime(&t);
	if (!tm_ptr) return NULL;

	Date *now_date = malloc(sizeof(Date));
	if (!now_date) return NULL;

	now_date->timeinfo = *tm_ptr;
	now_date->time = t;
	return now_date;
}

//Delete Date
void date_destroy(Date *d) {
	free(d);
}

