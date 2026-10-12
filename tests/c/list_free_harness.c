#include <assert.h>
#include <string.h>

#include "allocator_wrappers.h"
#include "list_free.h"

#define NODE_CAPACITY 5

static void *sExpectedOwner;
static struct ListCountNode *sFreed[NODE_CAPACITY];
static unsigned sFreeCount;
static struct ListCountNode *sMutationTarget;
static int sMutateEarlierLink;

void AllocatorFree(void *owner, void *allocation)
{
    assert(owner == sExpectedOwner);
    assert(sFreeCount < NODE_CAPACITY);
    sFreed[sFreeCount++] = allocation;

    if (sMutateEarlierLink && sFreeCount == 1)
        sMutationTarget->next = 0;
}

static void ResetFreeLog(void *owner)
{
    sExpectedOwner = owner;
    sFreeCount = 0;
    sMutationTarget = 0;
    sMutateEarlierLink = 0;
    memset(sFreed, 0, sizeof(sFreed));
}

static void InitNodes(struct ListCountNode *nodes, unsigned count)
{
    unsigned i;

    memset(nodes, 0, sizeof(*nodes) * count);
    for (i = 0; i < count; i++) {
        nodes[i].unknown00 = 0xA5000000u + i;
        nodes[i].unknown04 = 0x5A000000u + i;
        nodes[i].next = i + 1 < count ? &nodes[i + 1] : 0;
    }
}

static void TestNullNode(void)
{
    int ownerToken;
    struct ListCountNode untouched;
    struct ListCountNode before;

    memset(&untouched, 0xCD, sizeof(untouched));
    memcpy(&before, &untouched, sizeof(before));
    ResetFreeLog(&ownerToken);
    FreeListNodes(&ownerToken, 0);
    assert(sFreeCount == 0);
    assert(memcmp(&untouched, &before, sizeof(untouched)) == 0);
}

static void TestSingleNode(void)
{
    int ownerToken;
    struct ListCountNode nodes[1];
    struct ListCountNode before[1];

    InitNodes(nodes, 1);
    memcpy(before, nodes, sizeof(nodes));
    ResetFreeLog(&ownerToken);
    FreeListNodes(&ownerToken, nodes);
    assert(sFreeCount == 1);
    assert(sFreed[0] == &nodes[0]);
    assert(memcmp(nodes, before, sizeof(nodes)) == 0);
}

static void TestMultipleNodes(void)
{
    int ownerToken;
    struct ListCountNode nodes[NODE_CAPACITY];
    struct ListCountNode before[NODE_CAPACITY];
    unsigned i;

    InitNodes(nodes, NODE_CAPACITY);
    memcpy(before, nodes, sizeof(nodes));
    ResetFreeLog(&ownerToken);
    FreeListNodes(&ownerToken, nodes);
    assert(sFreeCount == NODE_CAPACITY);
    for (i = 0; i < NODE_CAPACITY; i++)
        assert(sFreed[i] == &nodes[NODE_CAPACITY - i - 1]);
    assert(memcmp(nodes, before, sizeof(nodes)) == 0);
}

static void TestCapturedUnwindPath(void)
{
    int ownerToken;
    struct ListCountNode nodes[3];
    struct ListCountNode expected[3];

    InitNodes(nodes, 3);
    memcpy(expected, nodes, sizeof(nodes));
    expected[0].next = 0;

    ResetFreeLog(&ownerToken);
    sMutationTarget = &nodes[0];
    sMutateEarlierLink = 1;
    FreeListNodes(&ownerToken, nodes);

    assert(sFreeCount == 3);
    assert(sFreed[0] == &nodes[2]);
    assert(sFreed[1] == &nodes[1]);
    assert(sFreed[2] == &nodes[0]);
    assert(memcmp(nodes, expected, sizeof(nodes)) == 0);
}

int main(void)
{
    TestNullNode();
    TestSingleNode();
    TestMultipleNodes();
    TestCapturedUnwindPath();
    return 0;
}
