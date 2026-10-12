#include "tree_collect.h"

u32 TreeCollectInOrder(struct Container *owner, void **output)
{
    struct ListCallbackState state;

    if (owner->tree_root == 0)
        return 0;

    state.cursor = output;
    state.count = 0;
    TraverseTreeInOrder(owner, ListCallbackAppend, &state);
    return state.count;
}

u32 TreeCollectReverseInOrder(struct Container *owner, void **output)
{
    struct ListCallbackState state;

    if (owner->tree_root == 0)
        return 0;

    state.cursor = output;
    state.count = 0;
    TraverseTreeReverseInOrder(owner, ListCallbackAppend, &state);
    return state.count;
}
