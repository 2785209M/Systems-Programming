#ifndef _MEMBERLIST_H_INCLUDED_
#define _MEMBERLIST_H_INCLUDED_

#include "date.h"
#include <stdint.h>
#include <stdlib.h>

// Skip list parameters
/*
 * MAX_LEVEL sets the maximum height of the skip list towers.
 * Each node gets a random level between 1 and MAX_LEVEL.
 * A typical rule of thumb is MAX_LEVEL ≈ log2(N), where N is
 * the expected number of elements. Here we fix it to 16, which
 * is more than enough for lists up to about 65,000 members.
 */
#define MAX_LEVEL 32

/*
 * P is the probability used when assigning levels.
 * Each level is chosen with probability P of going "up" again.
 * With P = 0.5, about 1/2 of the nodes are at level ≥1,
 * 1/4 at level ≥2, 1/8 at level ≥3, and so on.
 */
#define P 0.5

// User datatypes

typedef enum { ONLINE, AWAY, OFFLINE } UserStatus;

typedef struct {
	char *username;
	UserStatus status;
	Date *last_activity_date;
} User;

// Opaque datatypes
/*
 * Implementation requirements (read carefully):
 *
 * You must implement the MemberList ADT as a skip list.
 * - Each MemberList contains:
 *     // current maximum level
 *     // one head pointer per level
 * - Each MemberNode contains:
 *     // dynamically allocated user data
 *     // array of forward pointers
 *
 * Each level of the skip list must be NULL-terminated.
 *
 * These structures must be defined in memberlist.c.
 * They are opaque to other modules (e.g. server-monitor.c).
 */

typedef struct memberlist MemberList;
typedef struct membernode MemberNode;
typedef struct memberiterator MemberIterator;

/*
 * select_level() randomly determines the height (level) of a new node’s tower.
 * This randomness is what gives the skip list its expected O(log n) behavior.
 * Returns the new node's level.
 *
 * It is called internally by memberlist_add() when inserting new users.
 */
static inline int select_level() {
	int level = 0;
	while (((double)rand() / RAND_MAX) < P && level < MAX_LEVEL - 1) {
		level++;
	}
	return level;
}

// Construction/Destruction

/*
 * memberlist_create creates an empty MemberList.
 */
MemberList *memberlist_create();

/*
 * memberlist_destroy destroys the list structure, including all nodes,
 * usernames, and user data (including the Date object for each user).
 */
void memberlist_destroy(MemberList *mlist);

// CRUD operations

/*
 * Called by server-monitor.c when processing JOIN commands.
 *
 * memberlist_add adds a new user to the list or updates an existing one to
 * ONLINE.
 * - If the user does not exist, a new entry is created with status ONLINE.
 * - If the user exists, their status is updated to ONLINE and their timestamp
 * is updated. The function makes its own internal copies of the username and
 * date.
 * Returns 1 if successful, 0 on failure.
 *
 * Implementation notes:
 *  - Use select_level() to determine the height of each new node.
 *  - Ensure that all pointers (especially next[]) are correctly initialized.
 *  - Free any temporary memory on error to avoid leaks.
 */
int memberlist_add(MemberList *mlist, const char *username, const Date *d);

/*
 * Called by server-monitor.c when processing LEAVE commands.
 *
 * memberlist_remove permanently removes a user from the list.
 * This function deallocates the node, the username string, and all user data.
 * Returns 1 if successful, 0 if the user was not found.
 */
int memberlist_remove(MemberList *mlist, const char *username);

/*
 * Called by server-monitor.c when processing STATUS commands.
 *
 * memberlist_update_status finds a user and updates their status and timestamp.
 * Returns 1 if successful, 0 if the user was not found.
 */
int memberlist_update_status(MemberList *mlist, const char *username,
			     UserStatus status, const Date *d);

// Iteration
/*
 * Iterators provide a sequential view of the skip list for output purposes.
 * The iterator traverses level 0 of the skip list only.
 * Used by server-monitor.c when printing the "members" list.
 */

/*
 * memberlist_iter_create creates an iterator to traverse the list.
 */
MemberIterator *memberlist_iter_create(MemberList *mlist);

/*
 * memberlist_iter_next returns the next node in the sequence.
 */
MemberNode *memberlist_iter_next(MemberIterator *iter);

/*
 * memberlist_iter_destroy destroys the iterator.
 */
void memberlist_iter_destroy(MemberIterator *iter);

// Accessors

/*
 * membernode_username returns the username from a node.
 */
const char *membernode_username(MemberNode *node);

/*
 * membernode_status returns the status of the user from a node.
 */
UserStatus *membernode_status(MemberNode *node);

/*
 * membernode_last_activity_date returns the user's last activity date from a
 * node.
 */
Date *membernode_last_activity_date(MemberNode *node);

#endif
