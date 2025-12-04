#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "memberlist.h"
#include "date.h"

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

struct memberlist{
    int max_level;
    MemberNode *head_pointer;
};

struct membernode{
    User user;
    MemberNode **next;
    int level;
};

struct memberiterator{
    MemberNode *current; 
};

// Helper function for memberlist_create
static MemberNode *create_head_node(void){
    // Allocate memory to the new head node
    MemberNode *head = malloc(sizeof(MemberNode));
    if(!head) return NULL;

    head->level = MAX_LEVEL -1;
    // Allocate enough memory for the head node to store pointers to every level of the list
    head->next = malloc(sizeof(MemberNode *) * MAX_LEVEL);

    // Check next exists
    if(!head->next){
        free(head);
        return NULL;
    }

    // Set every pointer to NULL
    for(int i = 0; i < MAX_LEVEL; i++) head->next[i] = NULL;

    // Set the head node to be empty and offline
    head->user.username = NULL;
    head->user.last_activity_date = NULL;
    head->user.status = OFFLINE;

    return head;
}

/*
 * memberlist_create creates an empty MemberList.
 */
MemberList *memberlist_create(){
    MemberList *m = malloc(sizeof(MemberList));
    if(!m) return NULL;

    m->max_level = 0;

    m->head_pointer = create_head_node();
    if(!m->head_pointer){
        free(m);
        return NULL;
    }

    return m;
}

/*
 * memberlist_destroy destroys the list structure, including all nodes,
 * usernames, and user data (including the Date object for each user).
 */
void memberlist_destroy(MemberList *mlist){
    if(!mlist) return;

    // Iterate to level 0 and free nodes
    MemberNode *cur = mlist->head_pointer->next[0];
    while(cur){
        MemberNode *next = cur->next[0];
        if(cur->user.username) free(cur->user.username);
        if(cur->user.last_activity_date) date_destroy(cur->user.last_activity_date);
        if(cur->next) free(cur->next);
        free(cur);
        cur = next;
    }

    if(mlist->head_pointer){
        if(mlist->head_pointer->next) free(mlist->head_pointer->next);
        free(mlist->head_pointer);
    }

    free(mlist);
}

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
int memberlist_add(MemberList *mlist, const char *username, const Date *d){
    if(!mlist || !username || !d) return 0;

    MemberNode *update[MAX_LEVEL];
    MemberNode *current = mlist->head_pointer;

    for (int i = 0; i < MAX_LEVEL; i++) update[i] = mlist->head_pointer;

    // Search & record previous nodes
    for (int i = mlist->max_level; i>= 0; i--){
        while(current->next[i] != NULL && strcmp(current->next[i]->user.username, username) < 0){
            current = current->next[i];
        }
        update[i] = current;
    }

    // Change to Level 0
    current = current->next[0];

    // Possible match at level 0
    if(current != NULL && strcmp(current->user.username, username) == 0){
        // Update existing node
        current->user.status = ONLINE;
        date_destroy(current->user.last_activity_date);
        current->user.last_activity_date = date_duplicate(d);
        if(!current->user.last_activity_date) return 0;
        return 1;
    }

    int new_level = select_level();
    if(new_level<0) return 0;
    if(new_level >= MAX_LEVEL) new_level = MAX_LEVEL - 1;

    // For levels higher than max set update[i] = head
    if(new_level > mlist->max_level){
        for (int i = mlist->max_level + 1; i <= new_level; i++){
            update[i] = mlist->head_pointer;
        }
        mlist->max_level = new_level;
    }

    // Allocate memory to and initialize new node
    MemberNode *n = malloc(sizeof(MemberNode));
    if(!n) return 0;

    // Check that the user has a username
    n->user.username = strdup(username);
    if(!n->user.username) {
        free(n); 
        return 0;
    }

    // Create and check that last activity date has been set
    n->user.last_activity_date = date_duplicate(d);
    if(!n->user.last_activity_date) {
        free(n->user.username); 
        free(n); 
        return 0; 
    }

    n->user.status = ONLINE;
    n->level = new_level;

    // Set the next node in the skip list and check it exists
    n->next = malloc(sizeof(MemberNode *) * (new_level+1));
    if(!n->next){
        date_destroy(n->user.last_activity_date);
        free(n->user.username);
        free(n);
        return 0;
    }

    for (int i = 0; i<=new_level; i++){
        n->next[i] = update[i]->next[i];
        update[i]->next[i] = n;
    }

    return 1;
}

/*
 * Called by server-monitor.c when processing LEAVE commands.
 *
 * memberlist_remove permanently removes a user from the list.
 * This function deallocates the node, the username string, and all user data.
 * Returns 1 if successful, 0 if the user was not found.
 */
int memberlist_remove(MemberList *mlist, const char *username){
    if(!mlist || !username) return 0;

    MemberNode *update[MAX_LEVEL];
    MemberNode *current = mlist->head_pointer;

    // Top to bottom search, find previous nodes at each level
    for (int i = mlist->max_level; i >= 0; i--){
        while (current->next[i] != NULL &&
            strcmp(current->next[i]->user.username, username) < 0){
                current = current->next[i];
            }
            update[i] = current;
    }

    // Move to possible target Node
    current = current->next[0];

    // Check if they've been found
    if(current == NULL || strcmp(current->user.username, username) != 0){
        return 0; // Not found
    }

    // Unlink the Node from all levels where it appears
    for (int i = 0; i <= mlist->max_level; i++){
        if (update[i]->next[i] == current){
            update[i]->next[i] = current->next[i];
        }
    }

    // Free everything owned by this code
    free(current->user.username);
    date_destroy(current->user.last_activity_date);
    free(current->next);
    free(current);

    // Adjust max_level if top levels are now empty
    while (mlist->max_level > 0 && mlist->head_pointer->next[mlist->max_level] == NULL){
        mlist->max_level--;
    }
    
    return 1; // Success
}

/*
 * Called by server-monitor.c when processing STATUS commands.
 *
 * memberlist_update_status finds a user and updates their status and timestamp.
 * Returns 1 if successful, 0 if the user was not found.
 */
int memberlist_update_status(MemberList *mlist, const char *username, UserStatus status, const Date *d){
    if(!mlist ||!username || !d) return 0;

    MemberNode *current = mlist->head_pointer;

    // Start from top level for speed
    for (int i = mlist->max_level; i >= 0; i--){
        while (current->next[i] != NULL && strcmp(current->next[i]->user.username, username) < 0){
            current = current->next[i];
        }
    }
    
    // Drop to level 0
    current = current->next[0];

    if (current == NULL || strcmp(current->user.username, username) != 0) return 0;

    // Update User data
    current->user.status = status;

    // Replace the stored date
    if(current->user.last_activity_date) date_destroy(current->user.last_activity_date);
    current->user.last_activity_date = date_duplicate(d);
    if(!current->user.last_activity_date) return 0;

    return 1;
}

// Iteration
/*
 * Iterators provide a sequential view of the skip list for output purposes.
 * The iterator traverses level 0 of the skip list only.
 * Used by server-monitor.c when printing the "members" list.
 */

/*
 * memberlist_iter_create creates an iterator to traverse the list.
 */
MemberIterator *memberlist_iter_create(MemberList *mlist){
    if(!mlist || !mlist->head_pointer) return NULL;

    // Allocate memory to the iterator
    MemberIterator *iter = malloc(sizeof(MemberIterator));
    if(iter == NULL) return NULL;

    // Skip head node
    iter->current = mlist->head_pointer->next[0];

    return iter;
}

/*
 * memberlist_iter_next returns the next node in the sequence.
 */
MemberNode *memberlist_iter_next(MemberIterator *iter){
    if(!iter || !iter->current) return NULL;

    MemberNode *node = iter->current;
    iter->current = iter->current->next[0];
    return node;
}

/*
 * memberlist_iter_destroy destroys the iterator.
 */
void memberlist_iter_destroy(MemberIterator *iter){
    if (iter) free(iter);
}

// Accessors

/*
 * membernode_username returns the username from a node.
 */
const char *membernode_username(MemberNode *node){
    if(!node) return NULL;

    return node->user.username;
}

/*
 * membernode_status returns the status of the user from a node.
 */
UserStatus *membernode_status(MemberNode *node){
    if(!node) return NULL;

    return &(node->user.status);
}

/*
 * membernode_last_activity_date returns the user's last activity date from a
 * node.
 */
Date *membernode_last_activity_date(MemberNode *node){
    if(!node) return NULL;

    return node->user.last_activity_date;
}