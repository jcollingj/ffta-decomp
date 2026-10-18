#include "tree_rebuild.h"

#include "list_callbacks.h"
#include "tree_detach.h"
#include "tree_reverse_visit.h"

void RebuildTreeByNumericKey(struct Container *owner)
{
    struct Container *tree = owner;
    void *selected_value;

    for (;;) {
        struct TreeCountNode *cursor;
        int key;

        selected_value = 0;
        TraverseTreeReverseInOrder(owner, ListCallbackSelectClear,
                                   &selected_value);
        if (selected_value == 0)
            break;

        DetachTreeNode(owner, selected_value);
        ((struct TreeCountNode *)selected_value)->flags |= 1;

        if (tree->tree_root == 0) {
            tree->tree_root = selected_value;
        } else {
            cursor = tree->tree_root;
            key = (int)((struct TreeCountNode *)selected_value)->unknown00;
            while (1) {
                struct TreeCountNode *child;

                if (key < (int)cursor->unknown00) {
                    child = cursor->child_a;
                    if (child == 0) {
                        cursor->child_a = selected_value;
                        break;
                    }
                } else {
                    child = cursor->child_b;
                    if (child == 0) {
                        cursor->child_b = selected_value;
                        break;
                    }
                }
                cursor = child;
            }
            ((struct TreeCountNode *)selected_value)->parent = cursor;
        }
    }

    TraverseTreeReverseInOrder(owner, ListCallbackClearFlag0, 0);
}

asm(".align 2,0");
