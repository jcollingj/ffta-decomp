#include "container_destroy.h"
#include "allocator_wrappers.h"
#include "list_free.h"
#include "list_count.h"
#include "tree_count.h"

void *DestroyContainer(struct Container *owner)
{
    void *captured_list_allocator = owner->allocator_owner;
    struct ListCountNode *head = owner->list_head;
    void *captured_tree_allocator;
    struct TreeCountNode *node;
    struct TreeCountNode *previous;
    struct TreeCountNode *root;
    void *final_allocator;

    if (head != 0) {
        if (head->next != 0)
            FreeListNodes(captured_list_allocator, head->next);
        AllocatorFree(captured_list_allocator, head);
    }
    owner->list_head = 0;

    captured_tree_allocator = owner->allocator_owner;
    node = owner->tree_root;
    previous = (struct TreeCountNode *)-1;
    root = node;
    while (node != 0) {
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
        AllocatorFree(captured_tree_allocator, node);
        if (node == root)
            break;
        previous = node;
        node = node->parent;
    }
    owner->tree_root = 0;

    final_allocator = owner->allocator_owner;
    AllocatorFree(final_allocator, owner);
    return final_allocator;
}

asm(".align 2,0");
