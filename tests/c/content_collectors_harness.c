#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "content_collectors.h"
#include "container.h"
#include "list_count.h"
#include "tree_count.h"

#define LIST_SIZE 4
#define TREE_SIZE 7
#define OUTPUT_SIZE 8

static const signed char list_payload_index[3][LIST_SIZE] = {
    { 0, -1, 1, 0 },
    { 2, 3, 2, 3 },
    { 4, 5, -1, 6 }
};
static const signed char tree_payload_index[3][TREE_SIZE] = {
    { 7, 8, 9, -1, 7, 8, 9 },
    { 3, 2, 1, 0, 3, 2, 1 },
    { 4, 6, 5, -1, 4, 6, 5 }
};

struct Fixture {
    struct ListCountContext context;
    struct Container owners[3];
    struct ListCountNode list_nodes[3][LIST_SIZE];
    struct TreeCountNode tree_nodes[3][TREE_SIZE];
    int payloads[12];
};

struct GuardedOutput {
    uintptr_t before;
    void *values[OUTPUT_SIZE];
    uintptr_t after;
};

typedef int (*Collector)(struct ListCountContext *, void **);

static void initialize(struct Fixture *fixture)
{
    static const unsigned char left[TREE_SIZE] = { 255, 0, 255, 1, 255, 4, 255 };
    static const unsigned char right[TREE_SIZE] = { 255, 2, 255, 5, 255, 6, 255 };
    static const unsigned char parent[TREE_SIZE] = { 1, 3, 1, 255, 5, 3, 5 };
    unsigned owner, i;

    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->context.unknown10 = &fixture->owners[0];
    fixture->context.unknown14 = &fixture->owners[1];
    fixture->context.unknown18 = &fixture->owners[2];
    for (owner = 0; owner < 3; owner++) {
        fixture->owners[owner].allocator_owner = 0;
        fixture->owners[owner].list_head = &fixture->list_nodes[owner][0];
        fixture->owners[owner].tree_root = &fixture->tree_nodes[owner][3];
        for (i = 0; i < LIST_SIZE; i++) {
            struct ListCountNode *node = &fixture->list_nodes[owner][i];
            int index = list_payload_index[owner][i];
            node->unknown00 = index < 0 ? 0 :
                (ListPayload)(unsigned long)&fixture->payloads[index];
            node->previous = i == 0 ? 0 :
                &fixture->list_nodes[owner][i - 1];
            node->next = i + 1 == LIST_SIZE ? 0 :
                &fixture->list_nodes[owner][i + 1];
        }
        for (i = 0; i < TREE_SIZE; i++) {
            struct TreeCountNode *node = &fixture->tree_nodes[owner][i];
            int index = tree_payload_index[owner][i];
            node->unknown00 = 0x10203040u + owner * 0x100u + i;
            node->unknown04 = index < 0 ? 0 :
                (ListPayload)(unsigned long)&fixture->payloads[index];
            node->parent = parent[i] == 255 ? 0 :
                &fixture->tree_nodes[owner][parent[i]];
            node->child_a = left[i] == 255 ? 0 :
                &fixture->tree_nodes[owner][left[i]];
            node->child_b = right[i] == 255 ? 0 :
                &fixture->tree_nodes[owner][right[i]];
            node->flags = 0xC0010000u + owner * TREE_SIZE + i;
        }
    }
}

static void initialize_output(struct GuardedOutput *output)
{
    unsigned i;

    output->before = (uintptr_t)0x13579BDFu;
    output->after = (uintptr_t)0x2468ACE0u;
    for (i = 0; i < OUTPUT_SIZE; i++)
        output->values[i] = (void *)(uintptr_t)(0x11110000u + i * 4u);
}

static void run_case(Collector collector, unsigned owner_index, int is_tree,
                    int reverse, unsigned count)
{
    static const unsigned tree_in_order[TREE_SIZE] = { 0, 1, 2, 3, 4, 5, 6 };
    static const unsigned tree_reverse[TREE_SIZE] = { 6, 5, 4, 3, 2, 1, 0 };
    struct Fixture actual, expected_fixture;
    struct GuardedOutput output, expected_output;
    const void *expected_values[OUTPUT_SIZE];
    unsigned i;
    int result;

    initialize(&actual);
    initialize_output(&output);
    if (is_tree) {
        struct TreeCountNode *nodes = actual.tree_nodes[owner_index];
        if (count == 0)
            actual.owners[owner_index].tree_root = 0;
        else if (count == 1) {
            actual.owners[owner_index].tree_root = &nodes[3];
            nodes[3].parent = 0;
            nodes[3].child_a = 0;
            nodes[3].child_b = 0;
        }
    } else {
        struct ListCountNode *nodes = actual.list_nodes[owner_index];
        if (count == 0)
            actual.owners[owner_index].list_head = 0;
        else if (count == 1) {
            actual.owners[owner_index].list_head = &nodes[0];
            nodes[0].previous = 0;
            nodes[0].next = 0;
        }
    }
    memcpy(&expected_fixture, &actual, sizeof(actual));
    memcpy(&expected_output, &output, sizeof(output));

    if (count != 0) {
        for (i = 0; i < count; i++) {
            int payload_index;
            if (is_tree) {
                unsigned node_index = count == 1 ? 3 :
                    (reverse ? tree_reverse[i] : tree_in_order[i]);
                payload_index = tree_payload_index[owner_index][node_index];
            } else {
                payload_index = list_payload_index[owner_index][i];
            }
            expected_values[i] = payload_index < 0 ? 0 :
                &actual.payloads[payload_index];
        }
    }
    for (i = 0; i < count; i++)
        expected_output.values[i] = (void *)expected_values[i];

    result = collector(&actual.context, output.values);

    assert(result == (int)count);
    assert(memcmp(&actual, &expected_fixture, sizeof(actual)) == 0);
    assert(memcmp(&output, &expected_output, sizeof(output)) == 0);
}

static void test_collector(Collector collector, unsigned owner_index,
                           int is_tree, int reverse)
{
    run_case(collector, owner_index, is_tree, reverse, 0);
    run_case(collector, owner_index, is_tree, reverse, 1);
    run_case(collector, owner_index, is_tree, reverse,
             is_tree ? TREE_SIZE : LIST_SIZE);
}

int main(void)
{
    test_collector(CollectContextListPayloadsAt10, 0, 0, 0);
    test_collector(CollectContextListPayloadsAt18, 2, 0, 0);
    test_collector(CollectContextTreePayloadsInOrderAt10, 0, 1, 0);
    test_collector(CollectContextTreePayloadsReverseAt10, 0, 1, 1);
    test_collector(CollectContextTreePayloadsInOrderAt14, 1, 1, 0);
    test_collector(CollectContextTreePayloadsReverseAt14, 1, 1, 1);
    test_collector(CollectContextTreePayloadsInOrderAt18, 2, 1, 0);
    test_collector(CollectContextTreePayloadsReverseAt18, 2, 1, 1);
    return 0;
}
