#include <assert.h>
#include <string.h>

#include "allocator_wrappers.h"
#include "container_create.h"
#include "list_count.h"
#include "list_find_word.h"
#include "list_visit.h"
#include "tree_count.h"
#include "list_traversal.h"
#include "tree_reverse_visit.h"
#include "tree_collect.h"

static struct Container storage;
static void *allocator_owner_expected;
static unsigned allocations;

void *AllocatorAllocate(void *owner, u32 size)
{
    assert(owner == allocator_owner_expected);
    assert(size == 12);
    allocations++;
    assert(allocations == 1);
    return &storage;
}

struct ListTrace {
    struct ListCountNode *nodes[4];
    unsigned count;
};

static u32 record_list_node(struct ListCountNode *node, void *context)
{
    struct ListTrace *trace = (struct ListTrace *)context;
    trace->nodes[trace->count++] = node;
    return 1;
}

struct TreeTrace {
    struct TreeCountNode *nodes[4];
    unsigned count;
};

static int record_tree_node(void *value, void *context)
{
    struct TreeTrace *trace = (struct TreeTrace *)context;
    trace->nodes[trace->count++] = (struct TreeCountNode *)value;
    return 1;
}

int main(void)
{
    int allocator_owner;
    struct Container *container;
    struct Container container_before;
    struct ListCountNode list[3];
    struct ListCountNode list_before[3];
    struct TreeCountNode tree[3];
    struct TreeCountNode tree_before[3];
    struct ListTrace list_trace;
    struct TreeTrace tree_trace;
    void *collected[3];

    allocator_owner_expected = &allocator_owner;
    allocations = 0;
    container = ContainerCreate(&allocator_owner);
    assert(container == &storage);
    assert(allocations == 1);
    assert(container->allocator_owner == &allocator_owner);
    assert(container->list_head == 0 && container->tree_root == 0);

    memset(list, 0, sizeof(list));
    list[0].unknown00 = 10;
    list[1].unknown00 = 20;
    list[2].unknown00 = 30;
    list[0].previous = 0;
    list[1].previous = &list[0];
    list[2].previous = &list[1];
    list[0].next = &list[1];
    list[1].next = &list[2];
    container->list_head = &list[0];
    container_before = *container;
    memcpy(list_before, list, sizeof(list));
    assert(CountLinkedNodes(container) == 3);
    assert(FindListNodeByWord(container, 20) == &list[1]);
    memset(&list_trace, 0, sizeof(list_trace));
    VisitLinkedNodes(container, record_list_node, &list_trace);
    assert(list_trace.count == 3);
    assert(list_trace.nodes[0] == &list[0]);
    assert(list_trace.nodes[1] == &list[1]);
    assert(list_trace.nodes[2] == &list[2]);
    assert(memcmp(&container_before, container, sizeof(*container)) == 0);
    assert(memcmp(list_before, list, sizeof(list)) == 0);

    memset(tree, 0, sizeof(tree));
    tree[0].unknown00 = 1;
    tree[1].unknown00 = 2;
    tree[2].unknown00 = 3;
    tree[0].child_a = &tree[1];
    tree[0].child_b = &tree[2];
    tree[1].parent = &tree[0];
    tree[2].parent = &tree[0];
    container->tree_root = &tree[0];
    container_before = *container;
    memcpy(tree_before, tree, sizeof(tree));
    assert(CountTreeNodes(container) == 3); /* Nonmatching research candidate. */
    memset(&tree_trace, 0, sizeof(tree_trace));
    TraverseTreeInOrder(container, record_tree_node, &tree_trace);
    assert(tree_trace.count == 3);
    assert(tree_trace.nodes[0] == &tree[1]);
    assert(tree_trace.nodes[1] == &tree[0]);
    assert(tree_trace.nodes[2] == &tree[2]);
    memset(&tree_trace, 0, sizeof(tree_trace));
    TraverseTreeReverseInOrder(container, record_tree_node, &tree_trace);
    assert(tree_trace.count == 3);
    assert(tree_trace.nodes[0] == &tree[2]);
    assert(tree_trace.nodes[1] == &tree[0]);
    assert(tree_trace.nodes[2] == &tree[1]);
    assert(TreeCollectInOrder(container, collected) == 3);
    assert(collected[0] == &tree[1]);
    assert(collected[1] == &tree[0]);
    assert(collected[2] == &tree[2]);
    assert(TreeCollectReverseInOrder(container, collected) == 3);
    assert(collected[0] == &tree[2]);
    assert(collected[1] == &tree[0]);
    assert(collected[2] == &tree[1]);
    assert(memcmp(&container_before, container, sizeof(*container)) == 0);
    assert(memcmp(tree_before, tree, sizeof(tree)) == 0);
    return 0;
}
