#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date.h"
#include "memberlist.h"

#define USAGE "usage: %s [log-file] ...\n"
#define LINE_BUFFER_SIZE 1024

// Helper function to convert a UserStatus enum to a string for printing
static const char *status_to_string(UserStatus status) 
{
	switch (status) {
	case ONLINE:
		return "ONLINE";
	case AWAY:
		return "AWAY";
	case OFFLINE:
		return "OFFLINE";
	default:
		return "UNKNOWN";
	}
}

// Helper function to convert a string command to a UserStatus enum
static int string_to_status(const char *str, UserStatus *status) 
{
	if (strcmp(str, "ONLINE") == 0) {
		*status = ONLINE;
		return 1;
	}
	if (strcmp(str, "AWAY") == 0) {
		*status = AWAY;
		return 1;
	}
	if (strcmp(str, "OFFLINE") == 0) {
		*status = OFFLINE;
		return 1;
	}
	return 0; // Invalid status string
}

// Processes a single log file, adding/updating/removing members from the list
static void process_file(FILE *fd, MemberList *mlist) 
{
	char buffer[LINE_BUFFER_SIZE];

	// Each line format: "dd/mm/yyyy hh:mm COMMAND username [status]"
	while (fgets(buffer, sizeof(buffer), fd) != NULL) {
		if (strlen(buffer) <
		    18) { // Basic sanity check for timestamp + space
			continue;
		}

		// Isolate the timestamp string
		char datestr[17];
		strncpy(datestr, buffer, 16);
		datestr[16] = '\0';

		Date *d = date_create(datestr);
		if (!d) {
			fprintf(stderr,
				"Warning: Skipping malformed date line: %s",
				buffer);
			continue;
		}

		// Use sscanf for the rest of the line
		char command[32], username[256], extra[256];
		int items = sscanf(buffer + 17, "%31s %255s %255s", command,
				   username, extra);

		if (items < 2) {
			date_destroy(d);
			continue; // Not enough parts to be a valid command
		}

		int ret = 0;
		if (strcmp(command, "JOIN") == 0) {
			ret = memberlist_add(mlist, username, d);
			if (!ret) {
				printf("Error adding user\n");
			}
		} else if (strcmp(command, "LEAVE") == 0) {
			ret = memberlist_remove(mlist, username);
		} else if (strcmp(command, "STATUS") == 0 && items == 3) {
			UserStatus new_status;
			if (string_to_status(extra, &new_status)) {
				memberlist_update_status(mlist, username,
							 new_status, d);
			}
		}

		// The memberlist makes its own copies, so we must destroy the
		// temporary one
		date_destroy(d);
	}
}

int main(int argc, char *argv[]) 
{
	if (argc < 2) {
		fprintf(stderr, USAGE, argv[0]);
		return 1;
	}

    // Create a member list
	MemberList *mlist = memberlist_create();
	if (!mlist) {
		fprintf(stderr, "Error: Failed to create member list.\n");
		return 2;
	}

    // Process log file or process stdin if no filed provided
	const char *filename = argv[1];
    FILE * fd;
    if (strcmp(filename, "-") == 0) {
        fd = stdin;
    } else {
        fd = fopen(filename, "r");
    }

    if (!fd) {
        fprintf(stderr, "Error opening file.\n");
        return 2;
    }
    process_file(fd, mlist);
    if (fd != stdin) {
        fclose(fd);
    }

    //Create current date for comparison
#ifndef LIVE
    Date * now = date_create("10/10/2025 12:00");
#else
    Date * now = date_now();
#endif
	// Output all members and their last status
	MemberIterator *it = memberlist_iter_create(mlist);
	if (it) {
		MemberNode *node;
		while ((node = memberlist_iter_next(it)) != NULL) {
			const char *username = membernode_username(node);
			UserStatus *status = membernode_status(node);
			Date *date = membernode_last_activity_date(node);
			char *date_str = date_format_last_seen(date, now);

			if (username && date_str) {
                if (*status == ONLINE) {
                    printf("%s (%s)\n", username, status_to_string(*status));
                }
                else {
    				printf("%s (%s: %s)\n", username,
				       status_to_string(*status), date_str);
                }
			}
			free(date_str); // date_format allocates a new string
		}
		memberlist_iter_destroy(it);
	}
    date_destroy(now);
	memberlist_destroy(mlist);
	return 0;
}
