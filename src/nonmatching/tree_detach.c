#include "tree_detach.h"

void DetachTreeNode(struct Container *owner, struct TreeCountNode *node)
{
    struct TreeCountNode *parent;
    struct TreeCountNode *subtree;
    struct TreeCountNode *cursor;
    struct TreeCountNode *child;

    parent = node->parent;
    if (parent != 0) {
        if (parent->child_a == node)
            parent->child_a = 0;
        else if (parent->child_b == node)
            parent->child_b = 0;
        node->parent = 0;
    } else {
        owner->tree_root = 0;
        subtree = node->child_a;
        if (subtree != 0) {
            owner->tree_root = subtree;
            node->child_a = 0;
        } else {
            subtree = node->child_b;
            if (subtree != 0) {
                owner->tree_root = subtree;
                node->child_b = 0;
            }
        }
        if (owner->tree_root != 0)
            owner->tree_root->parent = 0;
    }

    subtree = node->child_a;
    if (subtree != 0) {
        cursor = owner->tree_root;
        for (;;) {
            if ((int)subtree->unknown00 < (int)cursor->unknown00) {
                child = cursor->child_a;
                if (child == 0) {
                    cursor->child_a = subtree;
                    break;
                }
            } else {
                child = cursor->child_b;
                if (child == 0) {
                    cursor->child_b = subtree;
                    break;
                }
            }
            cursor = child;
        }
        subtree->parent = cursor;
        node->child_a = 0;
    }

    subtree = node->child_b;
    if (subtree != 0) {
        cursor = owner->tree_root;
        for (;;) {
            if ((int)subtree->unknown00 < (int)cursor->unknown00) {
                child = cursor->child_a;
                if (child == 0) {
                    cursor->child_a = subtree;
                    break;
                }
            } else {
                child = cursor->child_b;
                if (child == 0) {
                    cursor->child_b = subtree;
                    break;
                }
            }
            cursor = child;
        }
        subtree->parent = cursor;
        node->child_b = 0;
    }
    node->parent = 0;
}

#ifndef TREE_DETACH_HOST_TEST
asm(".align 2,0");
#endif
