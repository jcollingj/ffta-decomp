#include "tree_count.h"

/* Count descendants from an arbitrary root. The original routine takes the
   root node directly and stops when the parent walk returns to that node. */
u32 CountTreeRootNodes(const struct TreeCountNode *root)
{
    u32 count;
    const struct TreeCountNode *previous;
    const struct TreeCountNode *node;

    count = 0;
    previous = (const struct TreeCountNode *)-1;
    node = root;
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

asm(".align 2,0");
