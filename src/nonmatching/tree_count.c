#include "tree_count.h"

/* Original interval is [0xC7B20, 0xC7B74), 84 bytes; its final `bx r1`
   begins at 0xC7B72. The current candidate symbol is only 82 bytes. */
u32 CountTreeNodes(const struct TreeCountOwner *owner)
{
    u32 count;
    const struct TreeCountNode *root = owner->root;
    const struct TreeCountNode *node;
    const struct TreeCountNode *previous;

    if (root == 0)
        return 0;

    node = root;
    count = 0;
    previous = (const struct TreeCountNode *)-1;
    root = node;
    while (node != 0) {
        const struct TreeCountNode *child_b = node->child_b;

        if (child_b != 0 && child_b != previous) {
            const struct TreeCountNode *child_a = node->child_a;

            if (child_a == previous)
                goto return_from_node;
            previous = node;
            node = child_b;
            continue;
        }

        {
            const struct TreeCountNode *child_a = node->child_a;

            if (child_a == previous)
                goto return_from_node;
            count++;
            if (child_a == 0)
                goto return_from_node;
            previous = node;
            node = child_a;
            continue;
        }

return_from_node:
        if (node == root)
            break;
        previous = node;
        node = node->parent;
    }

    return count;
}
