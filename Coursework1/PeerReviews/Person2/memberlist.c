#include "memberlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct membernode {
    User user;
    struct membernode *next[MAX_LEVEL];
};

struct memberlist {
    struct membernode *head;
    int level;
};

struct memberiterator {
    struct membernode *current;
};


MemberList *memberlist_create() {
    MemberList *mlist = malloc(sizeof(MemberList));
    if (!mlist) return NULL;
    mlist->head = malloc(sizeof(struct membernode));
    if (!mlist->head) {
        free(mlist);
        return NULL;
    }
    for (int i = 0; i < MAX_LEVEL; i++) {
        mlist->head->next[i] = NULL;
    }
    mlist->level = 0;

    srand((unsigned int)time(NULL));

    return mlist;
}

void memberlist_destroy(MemberList *mlist) {
    if (!mlist) return;
    struct membernode *node = mlist->head->next[0];
    while (node) {
        struct membernode *next = node->next[0];
        free(node->user.username);
        date_destroy(node->user.last_activity_date);
        free(node);
        node = next;
    }
    free(mlist->head);
    free(mlist);
}

int memberlist_add(MemberList *mlist, const char *username, const Date *d) {
    if (!mlist || !username || !d) return 0;
    struct membernode *update[MAX_LEVEL];
    struct membernode *node = mlist->head;
    for (int i = mlist->level; i >= 0; i--) {
        while (node->next[i] && strcmp(node->next[i]->user.username, username) < 0) {
            node = node->next[i];
        }
        update[i] = node;
    }
    node = node->next[0];

    if (node && strcmp(node->user.username, username) == 0) {
        date_destroy(node->user.last_activity_date);
        node->user.last_activity_date = date_duplicate(d);
        node->user.status = ONLINE;
        return node->user.last_activity_date ? 1 : 0;
    } else {
        int level = select_level();
        if (level > mlist->level) {
            for (int i = mlist->level + 1; i <= level; i++) {
                update[i] = mlist->head;
            }
            mlist->level = level;
        }
        struct membernode *new_node = malloc(sizeof(struct membernode));
        if (!new_node) return 0;
        new_node->user.username = strdup(username);
        if (!new_node->user.username) {
            free(new_node);
            return 0;
        }
        new_node->user.status = ONLINE;
        new_node->user.last_activity_date = date_duplicate(d);
        if (!new_node->user.last_activity_date) {
            free(new_node->user.username);
            free(new_node);
            return 0;
        }
        for (int i = 0; i <= level; i++) {
            new_node->next[i] = update[i]->next[i];
            update[i]->next[i] = new_node;
        }
        return 1;
    }
}

int memberlist_remove(MemberList *mlist, const char *username) {
    if (!mlist || !username) return 0;
    struct membernode *update[MAX_LEVEL];
    struct membernode *node = mlist->head;
    for (int i = mlist->level; i >= 0; i--) {
        while (node->next[i] && strcmp(node->next[i]->user.username, username) < 0) {
            node = node->next[i];
        }
        update[i] = node;
    }
    node = node->next[0];
    if (!node || strcmp(node->user.username, username) != 0) return 0;
    for (int i = 0; i <= mlist->level; i++) {
        if (update[i]->next[i] != node) break;
        update[i]->next[i] = node->next[i];
    }
    free(node->user.username);
    date_destroy(node->user.last_activity_date);
    free(node);
    while (mlist->level > 0 && mlist->head->next[mlist->level] == NULL) {
        mlist->level--;
    }
    return 1;
}

int memberlist_update_status(MemberList *mlist, const char *username, UserStatus status, const Date *d) {
    if (!mlist || !username || !d) return 0;
    struct membernode *node = mlist->head->next[0];
    while (node && strcmp(node->user.username, username) < 0) {
        node = node->next[0];
    }
    if (!node || strcmp(node->user.username, username) != 0) return 0;
    node->user.status = status;
    date_destroy(node->user.last_activity_date);
    node->user.last_activity_date = date_duplicate(d);
    return node->user.last_activity_date ? 1 : 0;
}

MemberIterator *memberlist_iter_create(MemberList *mlist) {
    if (!mlist) return NULL;
    MemberIterator *iter = malloc(sizeof(MemberIterator));
    if (!iter) return NULL;
    iter->current = mlist->head->next[0];
    return iter;
}

MemberNode *memberlist_iter_next(MemberIterator *iter) {
    if (!iter || !iter->current) return NULL;
    MemberNode *node = iter->current;
    iter->current = iter->current->next[0];
    return node;
}

void memberlist_iter_destroy(MemberIterator *iter) {
    free(iter);
}

const char *membernode_username(MemberNode *node) {
    return node ? node->user.username : NULL;
}

UserStatus *membernode_status(MemberNode *node) {
    return node ? &node->user.status : NULL;
}

Date *membernode_last_activity_date(MemberNode *node) {
    return node ? node->user.last_activity_date : NULL;
}
