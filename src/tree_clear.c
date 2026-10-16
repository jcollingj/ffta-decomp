#include "tree_clear.h"
#include "allocator_wrappers.h"
#include "tree_count.h"

/* Original interval is [0xC79E0, 0xC7A34), 84 bytes: 82 bytes of
   instructions through `bx r0` and two zero alignment bytes. */
void FreeTreeNodes(struct Container *owner)
{
    void *allocator_owner;
    struct TreeCountNode *node;
    struct TreeCountNode *previous;
    struct TreeCountNode *root;

    allocator_owner = owner->allocator_owner;
    node = owner->tree_root;
    previous = (struct TreeCountNode *)-1;
    root = node;

    if (node != 0) do {
        struct TreeCountNode *child_a = node->child_a;
        struct TreeCountNode *child_b = node->child_b;

        if (child_a != 0 && child_a != previous && child_b != previous) {
            previous = node;
            node = child_a;
            continue;
        }

        if (child_b != 0 && child_b != previous) {
            previous = node;
            node = child_b;
            continue;
        }

        AllocatorFree(allocator_owner, node);
        if (node == root)
            break;
        previous = node;
        node = node->parent;
    } while (node != 0);

    owner->tree_root = 0;
}

asm(".align 2,0");
