#include "tree_insert.h"
#include "allocator_wrappers.h"

struct TreeCountNode *InsertTreeNode(struct Container *owner, ListPayload payload,
                                     int key)
{
    struct TreeCountNode *node;
    struct TreeCountNode *parent;

    node = AllocatorAllocate(owner->allocator_owner, 24);
    node->unknown04 = payload;
    node->unknown00 = (u32)key;
    node->parent = 0;
    node->child_a = 0;
    node->child_b = 0;
    node->flags = 0;

    parent = owner->tree_root;
    if (parent == 0) {
        owner->tree_root = node;
    } else {
        while (parent != 0) {
            if (key < (int)parent->unknown00) {
                if (parent->child_a == 0) {
                    parent->child_a = node;
                    break;
                }
                parent = parent->child_a;
            } else {
                if (parent->child_b == 0) {
                    parent->child_b = node;
                    break;
                }
                parent = parent->child_b;
            }
        }
        node->parent = parent;
    }
    return node;
}

asm(".align 2,0");
