#include "list_traversal.h"

void TraverseTreeInOrder(const struct TreeCountOwner *owner,
                         TreeTraversalCallback callback, void *context)
{
    struct TreeCountNode *node = owner->root;
    struct TreeCountNode *previous = (struct TreeCountNode *)-1;
    struct TreeCountNode *root = node;

    while (node != 0) {
        struct TreeCountNode *left = node->child_a;

        if (left != 0 && left != previous) {
            struct TreeCountNode *right = node->child_b;

            if (right != previous) {
                previous = node;
                node = left;
                continue;
            }
            goto return_from_node;
        }

        {
            struct TreeCountNode *right = node->child_b;

            if (right == previous)
                goto return_from_node;
        }

        if (callback(node, context) == 0)
            return;

        {
            struct TreeCountNode *right = node->child_b;

            if (right != 0) {
                previous = node;
                node = right;
                continue;
            }
        }

return_from_node:
        if (node == root)
            break;
        previous = node;
        node = node->parent;
    }
}

asm(".align 2,0");
