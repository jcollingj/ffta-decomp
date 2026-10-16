#include "tree_search.h"
#include "list_traversal.h"

struct TreeCountNode *TreeSearchByWord1(const struct Container *owner, u32 key)
{
    struct TreeSearchContext context;

    if (owner->tree_root == 0)
        return 0;

    context.key = key;
    context.result = 0;
    TraverseTreeInOrder(owner, ListSearchCallback, &context);
    return context.result;
}
