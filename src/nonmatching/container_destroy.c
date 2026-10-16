#include "container_destroy.h"
#include "allocator_wrappers.h"
#include "list_free.h"
#include "list_count.h"
#include "tree_count.h"

void *DestroyContainer(struct Container *owner)
{
    struct Container *list_owner = owner;
    struct Container *tree_owner;
    struct Container *release_owner;
    void *captured_list_allocator = list_owner->allocator_owner;
    struct ListCountNode *head = list_owner->list_head;
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
    list_owner->list_head = 0;

    tree_owner = owner;
    captured_tree_allocator = tree_owner->allocator_owner;
    node = tree_owner->tree_root;
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
        AllocatorFree(captured_tree_allocator, node);
        if (node == root)
            break;
        previous = node;
        node = node->parent;
    } while (node != 0);
    tree_owner->tree_root = 0;

    release_owner = owner;
    final_allocator = release_owner->allocator_owner;
    AllocatorFree(final_allocator, release_owner);
    return final_allocator;
}

asm(".align 2,0");
