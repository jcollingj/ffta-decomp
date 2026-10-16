#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_search_callback.h"
#include "../../include/tree_count.h"

typedef char host_result_offset_matches_pointer_alignment[
    offsetof(struct TreeSearchContext, result) == sizeof(void *) ? 1 : -1];
typedef char host_context_contains_key_and_pointer[
    sizeof(struct TreeSearchContext) == 2 * sizeof(void *) ? 1 : -1];

static void TestMismatchLeavesNativeContextUnchanged(void)
{
    struct TreeCountNode node = {0xA1B2C3D4, 0x13579BDF, 0, 0, 0, 0};
    struct TreeSearchContext context = {0x2468ACE0, 0};
    const struct TreeSearchContext before = context;

    assert(ListSearchCallback(&node, &context) == 1);
    assert(memcmp(&context, &before, sizeof(context)) == 0);
}

static void TestMatchStoresNativeNodePointer(void)
{
    struct TreeCountNode node = {0x01020304, 0x89ABCDEF, 0, 0, 0, 0};
    struct TreeSearchContext context = {0x89ABCDEF, 0};

    assert(ListSearchCallback(&node, &context) == 0);
    assert(context.key == 0x89ABCDEF);
    assert(context.result == &node);
}

static void TestSameBaseAliasUsesNativeFields(void)
{
    union {
        struct TreeCountNode node;
        struct TreeSearchContext context;
    } aliased;
    union {
        struct TreeCountNode node;
        struct TreeSearchContext context;
    } expected;
    struct TreeCountNode *expected_result = &aliased.node;

    memset(&aliased, 0, sizeof(aliased));
    aliased.node.unknown00 = 0xCAFEBABE;
    aliased.node.unknown04 = 0xCAFEBABE;
    memset(&expected, 0, sizeof(expected));
    expected.node.unknown00 = 0xCAFEBABE;
    expected.node.unknown04 = 0xCAFEBABE;
    memcpy((unsigned char *)&expected +
               offsetof(struct TreeSearchContext, result),
           &expected_result, sizeof(expected_result));

    /* Both views begin at the same native address. The result pointer write
     * overlaps bytes at the native result-field offset after the comparison.
     * On target these are the node's second word; on this 64-bit host they
     * occupy the parent field. Compare the complete native storage image. */
    assert(ListSearchCallback(&aliased.node, &aliased.context) == 0);
    assert(aliased.context.key == 0xCAFEBABE);
    assert(aliased.context.result == &aliased.node);
    assert(memcmp(&aliased, &expected, sizeof(aliased)) == 0);
}

int main(void)
{
    TestMismatchLeavesNativeContextUnchanged();
    TestMatchStoresNativeNodePointer();
    TestSameBaseAliasUsesNativeFields();
    return 0;
}
