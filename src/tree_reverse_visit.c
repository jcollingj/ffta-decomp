#include "tree_reverse_visit.h"

void TraverseTreeReverseInOrder(const struct TreeCountOwner *owner,
                                TreeReverseVisitCallback callback, void *context)
{
    struct TreeCountNode *node = owner->root;
    struct TreeCountNode *previous = (struct TreeCountNode *)-1;
    struct TreeCountNode *root = node;

    while (node != 0) {
        struct TreeCountNode *right = node->child_b;

        if (right != 0 && right != previous) {
            struct TreeCountNode *left = node->child_a;

            if (left != previous) {
                previous = node;
                node = right;
                continue;
            }
            goto return_from_node;
        }

        {
            struct TreeCountNode *left = node->child_a;

            if (left == previous)
                goto return_from_node;
        }

        if (callback(node, context) == 0)
            return;

        {
            struct TreeCountNode *left = node->child_a;

            if (left != 0) {
                previous = node;
                node = left;
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
