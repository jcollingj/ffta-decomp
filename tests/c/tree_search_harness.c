#include <assert.h>
#include <string.h>
#include "../../include/tree_search.h"

static void TestEmptyRootReturnsNull(void)
{
    struct Container owner = {0, 0, 0};
    const struct Container before = owner;

    assert(TreeSearchByWord1(&owner, 0x12345678) == 0);
    assert(memcmp(&owner, &before, sizeof(owner)) == 0);
}

static ListPayload PayloadFromPointer(const void *pointer)
{
    return (ListPayload)(unsigned long)pointer;
}

static void TestInOrderFirstDuplicateAndMissingKey(void)
{
    const ListPayload same_low_a = (ListPayload)0x11112222CAFEBABEULL;
    const ListPayload same_low_b = (ListPayload)0x33334444CAFEBABEULL;
    struct TreeCountNode root = {20, 0x11111111, 0, 0, 0, 0};
    struct TreeCountNode left = {10, 0, 0, 0, 0, 0};
    struct TreeCountNode left_right = {15, 0, 0, 0, 0, 0};
    struct TreeCountNode right = {30, 0, 0, 0, 0, 0};
    struct Container owner = {0, 0, &root};

    left.unknown04 = same_low_a;
    left_right.unknown04 = same_low_b;
    right.unknown04 = same_low_a;
    if (sizeof(ListPayload) != 8) {
        left.unknown04 = 0xCAFEBABEu;
        left_right.unknown04 = 0xDEADBEEFu;
        right.unknown04 = 0xCAFEBABEu;
    }

    root.child_a = &left;
    root.child_b = &right;
    left.parent = &root;
    left.child_b = &left_right;
    left_right.parent = &left;
    right.parent = &root;

    const struct TreeCountNode root_before = root;
    const struct TreeCountNode left_before = left;
    const struct TreeCountNode left_right_before = left_right;
    const struct TreeCountNode right_before = right;
    const struct Container owner_before = owner;

    if (sizeof(ListPayload) == 8) {
        /* These payloads share their low 32 bits. A narrowed search would
           return left for both queries. */
        assert((u32)same_low_a == (u32)same_low_b);
        assert(TreeSearchByWord1(&owner, same_low_a) == &left);
        assert(TreeSearchByWord1(&owner, same_low_b) == &left_right);
    } else {
        assert(TreeSearchByWord1(&owner, 0xCAFEBABEu) == &left);
        assert(TreeSearchByWord1(&owner, 0xDEADBEEFu) == &left_right);
    }
    assert(TreeSearchByWord1(&owner, 0x11111111UL) == &root);
    assert(TreeSearchByWord1(&owner, 0x76543210UL) == 0);

    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(&root, &root_before, sizeof(root)) == 0);
    assert(memcmp(&left, &left_before, sizeof(left)) == 0);
    assert(memcmp(&left_right, &left_right_before, sizeof(left_right)) == 0);
    assert(memcmp(&right, &right_before, sizeof(right)) == 0);
}

static void TestRootMatchStopsBeforeLaterDuplicate(void)
{
    struct TreeCountNode root = {1, 9, 0, 0, 0, 0};
    struct TreeCountNode left = {2, 8, 0, 0, 0, 0};
    struct TreeCountNode right = {3, 9, 0, 0, 0, 0};
    struct Container owner = {0, 0, &root};
    root.child_a = &left;
    root.child_b = &right;
    left.parent = &root;
    right.parent = &root;

    const struct TreeCountNode root_before = root;
    const struct TreeCountNode left_before = left;
    const struct TreeCountNode right_before = right;
    const struct Container owner_before = owner;

    /* The root is the first in-order match. If traversal continued through
     * the right subtree, the callback would overwrite result with right. */
    assert(TreeSearchByWord1(&owner, 9) == &root);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(&root, &root_before, sizeof(root)) == 0);
    assert(memcmp(&left, &left_before, sizeof(left)) == 0);
    assert(memcmp(&right, &right_before, sizeof(right)) == 0);
}

static void TestNumericBoundariesAndNativePointerRoundtrip(void)
{
    struct TreeCountNode root;
    struct Container owner;
    struct {
        u32 marker;
        u32 guard;
    } entry = {0x13579BDFu, 0x2468ACE0u};
    const ListPayload numeric[] = {0, 0x80000000UL, 0xFFFFFFFFUL};
    unsigned i;

    memset(&root, 0x5A, sizeof(root));
    memset(&owner, 0xA5, sizeof(owner));
    owner.tree_root = &root;
    root.parent = 0;
    root.child_a = 0;
    root.child_b = 0;
    root.flags = 0xA5000000u;
    root.unknown00 = 0x10203040u;
    for (i = 0; i < sizeof(numeric) / sizeof(numeric[0]); i++) {
        const struct Container owner_before = owner;

        root.unknown04 = numeric[i];
        {
            const struct TreeCountNode call_before = root;
            assert(TreeSearchByWord1(&owner, numeric[i]) == &root);
            assert(memcmp(&call_before, &root, sizeof(root)) == 0);
            assert(memcmp(&owner_before, &owner, sizeof(owner)) == 0);
        }
    }

    root.unknown04 = PayloadFromPointer(&entry);
    assert(root.unknown04 == PayloadFromPointer(&entry));
    {
        const struct TreeCountNode before = root;
        const struct Container owner_before = owner;
        const struct SearchEntryImage { u32 marker; u32 guard; } entry_before =
            {entry.marker, entry.guard};
        assert(TreeSearchByWord1(&owner, PayloadFromPointer(&entry)) == &root);
        assert(memcmp(&before, &root, sizeof(root)) == 0);
        assert(memcmp(&owner_before, &owner, sizeof(owner)) == 0);
        assert(memcmp(&entry_before, &entry, sizeof(entry)) == 0);
    }
    assert(entry.marker == 0x13579BDFu);
    assert(entry.guard == 0x2468ACE0u);
}

int main(void)
{
    TestEmptyRootReturnsNull();
    TestInOrderFirstDuplicateAndMissingKey();
    TestRootMatchStopsBeforeLaterDuplicate();
    TestNumericBoundariesAndNativePointerRoundtrip();
    return 0;
}
