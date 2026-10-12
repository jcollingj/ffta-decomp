#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "allocator_wrappers.h"
#include "container_create.h"
#include "list_count.h"
#include "tree_count.h"

typedef char container_has_three_native_pointer_fields[
    sizeof(struct Container) == 3 * sizeof(void *) ? 1 : -1];
typedef char owner_offset_is_zero[
    offsetof(struct Container, allocator_owner) == 0 ? 1 : -1];
typedef char list_head_follows_owner[
    offsetof(struct Container, list_head) == sizeof(void *) ? 1 : -1];
typedef char tree_root_follows_list_head[
    offsetof(struct Container, tree_root) == 2 * sizeof(void *) ? 1 : -1];
typedef char native_pointer_field_offsets_are_contiguous[
    sizeof(void *) != 4 ||
    (offsetof(struct Container, allocator_owner) == 0 &&
     offsetof(struct Container, list_head) == 4 &&
     offsetof(struct Container, tree_root) == 8) ? 1 : -1];

struct GuardedContainer {
    uint32_t before[4];
    struct Container value;
    uint32_t after[4];
};

static struct GuardedContainer allocation;
static void *expected_owner;
static unsigned allocation_calls;

void *AllocatorAllocate(void *owner, u32 size)
{
    assert(owner == expected_owner);
    assert(size == 12);
    allocation_calls++;
    assert(allocation_calls == 1);
    allocation.before[0] = 0x13579bdfu;
    allocation.before[1] = 0x2468ace0u;
    allocation.before[2] = 0x55aa33ccu;
    allocation.before[3] = 0xa55ac33cu;
    allocation.value.allocator_owner = (void *)(uintptr_t)0x11112222u;
    allocation.value.list_head = (struct ListCountNode *)(uintptr_t)0x33334444u;
    allocation.value.tree_root = (struct TreeCountNode *)(uintptr_t)0x55556666u;
    allocation.after[0] = 0xdeadbeefu;
    allocation.after[1] = 0xc001d00du;
    allocation.after[2] = 0x0badf00du;
    allocation.after[3] = 0xfeedfaceu;
    return &allocation.value;
}

static void check_guards(void)
{
    const uint32_t before[4] = {
        0x13579bdfu, 0x2468ace0u, 0x55aa33ccu, 0xa55ac33cu
    };
    const uint32_t after[4] = {
        0xdeadbeefu, 0xc001d00du, 0x0badf00du, 0xfeedfaceu
    };
    unsigned i;

    for (i = 0; i < 4; i++) {
        assert(allocation.before[i] == before[i]);
        assert(allocation.after[i] == after[i]);
    }
}

static void run_owner_case(void *owner)
{
    struct Container *result;

    expected_owner = owner;
    allocation_calls = 0;
    result = ContainerCreate(owner);
    assert(result == &allocation.value);
    assert(allocation_calls == 1);
    assert(result->allocator_owner == owner);
    assert(result->list_head == 0);
    assert(result->tree_root == 0);
    check_guards();
}

int main(void)
{
    int owner_a;
    struct { unsigned char bytes[17]; } owner_b;

    run_owner_case(&owner_a);
    run_owner_case(&owner_b);
    return 0;
}
