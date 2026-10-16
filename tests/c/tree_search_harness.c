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

static void TestInOrderFirstDuplicateAndMissingKey(void)
{
    struct TreeCountNode root = {20, 0x11111111, 0, 0, 0, 0};
    struct TreeCountNode left = {10, 7, 0, 0, 0, 0};
    struct TreeCountNode left_right = {15, 7, 0, 0, 0, 0};
    struct TreeCountNode right = {30, 7, 0, 0, 0, 0};
    struct Container owner = {0, 0, &root};

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

    /* In-order visits left, left-right, root, right. The repeated key at
     * right must not replace the first match at left. */
    assert(TreeSearchByWord1(&owner, 7) == &left);
    assert(TreeSearchByWord1(&owner, 0x11111111) == &root);
    assert(TreeSearchByWord1(&owner, 0xDEADBEEF) == 0);

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

int main(void)
{
    TestEmptyRootReturnsNull();
    TestInOrderFirstDuplicateAndMissingKey();
    TestRootMatchStopsBeforeLaterDuplicate();
    return 0;
}
