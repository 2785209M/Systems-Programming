#include "memberlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Stucture for mebernode data
struct membernode {
	User user;            //all user data
	struct membernode **next;  // array of forward pointers
	int level;            //node height
};

// Skip list structure
struct memberlist {
	struct membernode *head;
	int max_level;
};

struct memberiterator {
	struct membernode *curr;
};

//create nodes
static MemberNode *make_node(const char *name, const Date *d, int level) {
	MemberNode *n = malloc(sizeof(MemberNode));
	if (!n) return NULL;

	n->level = level;

	//copy username
	n->user.username = malloc(strlen(name) + 1);
	if (!n->user.username) {
		free(n);
		return NULL;
	}
	strcpy(n->user.username, name);

	//copy date
	if (d)
		n->user.last_activity_date = date_duplicate(d);
	else
		n->user.last_activity_date = NULL;

	n->user.status = ONLINE;

	//forward pointer array allocation
	n->next = malloc(sizeof(MemberNode*) * (level + 1));
	if (!n->next) {
		date_destroy(n->user.last_activity_date);
		free(n->user.username);
		free(n);
		return NULL;
	}
	for (int i = 0; i <= level; i++)
		n->next[i] = NULL;

	return n;
}

//Skip list Creation
MemberList *memberlist_create() {
	MemberList *ml = malloc(sizeof(MemberList));
	if (!ml) return NULL;

	ml->max_level = 0;

	//Create head node with max level
	ml->head = make_node("", NULL, MAX_LEVEL - 1);
	if (!ml->head) {
		free(ml);
		return NULL;
	}

	//set forward pointers tp null
	for (int i = 0; i < MAX_LEVEL; i++)
		ml->head->next[i] = NULL;

	return ml;
}
//Destroy member list
void memberlist_destroy(MemberList *mlist) {
	if (!mlist) return;
	//free all current nodes
	MemberNode *curr = mlist->head->next[0];
	while (curr) {
		MemberNode *tmp = curr->next[0];
		free(curr->user.username);
		date_destroy(curr->user.last_activity_date);
		free(curr->next);
		free(curr);
		curr = tmp;
	}

	free(mlist->head->next);
	free(mlist->head->user.username);
	date_destroy(mlist->head->user.last_activity_date);
	free(mlist->head);
	free(mlist);
}

//find node by username
static MemberNode *find_node(MemberList *mlist, const char *name, MemberNode **update) {
	MemberNode *x = mlist->head;
	

	for (int i = MAX_LEVEL - 1; i >= 0; i--) {
		while (x->next[i] && strcmp(x->next[i]->user.username, name) < 0) {
			x = x->next[i];
		}
		if (update) update[i] = x;
	}


	if (x->next[0] && strcmp(x->next[0]->user.username, name) == 0)
		return x->next[0];

	return NULL;
}

//Add user to current server
int memberlist_add(MemberList *mlist, const char *username, const Date *d) {
	if (!mlist || !username || !d) return 0;

	MemberNode *prev[MAX_LEVEL];
	MemberNode *found = find_node(mlist, username, prev);

	if (found) {
		//update the user
		found->user.status = ONLINE;
		date_destroy(found->user.last_activity_date);
		found->user.last_activity_date = date_duplicate(d);
		return 1;
	}

	//Get level for new node
	int lvl = select_level();
	if (lvl > mlist->max_level)
		mlist->max_level = lvl;

	MemberNode *node = make_node(username, d, lvl);
	if (!node) return 0;

	//Add the node tot he skip list
	for (int i = 0; i <= lvl; i++) {
		node->next[i] = prev[i]->next[i];
		prev[i]->next[i] = node;
	}

	return 1;
}

//remove a node by username
int memberlist_remove(MemberList *mlist, const char *username) {
	if (!mlist || !username) return 0;

	MemberNode *prev[MAX_LEVEL];
	MemberNode *node = find_node(mlist, username, prev);
	if (!node) return 0;

	for (int i = 0; i <= node->level; i++) {
		if (prev[i]->next[i] != node) break;
		prev[i]->next[i] = node->next[i];
	}

	free(node->user.username);
	date_destroy(node->user.last_activity_date);
	free(node->next);
	free(node);

	return 1;
}

//update user status and last activity date
int memberlist_update_status(MemberList *mlist, const char *username, UserStatus status, const Date *d) {
	if (!mlist || !username || !d) return 0;

	//find user for update
	MemberNode *node = find_node(mlist, username, NULL);
	if (!node) return 0;

	node->user.status = status;
	date_destroy(node->user.last_activity_date);
	node->user.last_activity_date = date_duplicate(d);

	return 1;
}


//Create iterator
MemberIterator *memberlist_iter_create(MemberList *mlist) {
	if (!mlist) return NULL;

	MemberIterator *iter = malloc(sizeof(MemberIterator));
	if (!iter) return NULL;

	iter->curr = mlist->head->next[0];
	return iter;
}

//next node in iteration
MemberNode *memberlist_iter_next(MemberIterator *iter) {
	if (!iter || !iter->curr) return NULL;

	MemberNode *res = iter->curr;
	iter->curr = iter->curr->next[0];
	return res;
}

//Delete iterator
void memberlist_iter_destroy(MemberIterator *iter) {
	if (iter) free(iter);
}

//Access for node data
const char *membernode_username(MemberNode *node) {
	return node ? node->user.username : NULL;
}

UserStatus *membernode_status(MemberNode *node) {
	return node ? &node->user.status : NULL;
}

Date *membernode_last_activity_date(MemberNode *node) {
	return node ? node->user.last_activity_date : NULL;
}
