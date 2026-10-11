#include <assert.h>
#include <string.h>
#include "../../src/ability_state_setter.c"
#include "../../src/ability_state_query.c"
#include "../../src/item_abilities.c"

const struct ItemAbilitySet gItemAbilitySets[] = {
    {0, 0, {{0, 0}}},
    {3, 0, {{2, 9}, {255, 7}, {6, 10}}}
};
static int property_calls, sync_calls;
u32 GetItemProperty(u16 item, u8 selector)
{
    assert(item == 1 && selector == 18);
    property_calls++;
    return 1;
}
void SyncPairedAbilityState(u8 *unit, u8 ability)
{
    assert(ability == 9);
    assert(IsAbilityAvailable(unit, ability)); /* Flag is set before synchronization. */
    sync_calls++;
}
int main(void)
{
    u8 unit[0x108];
    memset(unit, 0, sizeof(unit));
    SetAbilityAvailable(0, 9);
    assert(IsAbilityAvailable(0, 9) == 0);
    assert(IsAbilityAvailable(unit, 9) == 0);
    unit[0x49] = 0x35;
    SetAbilityAvailable(unit, 9);
    assert(unit[0x49] == 0xb5);
    SetAbilityAvailable(unit, 9);
    assert(unit[0x49] == 0xb5);
    assert(IsAbilityAvailable(unit, 9) == 1);
    assert(unit[0x48] == 0 && unit[0x4a] == 0);

    ApplyItemAbilityFlags(0, 0);
    assert(property_calls == 0 && sync_calls == 0);
    unit[7] = 2;
    unit[0x49] = 0x25;
    unit[0x4a] = 0x11;
    ApplyItemAbilityFlags(unit, 1);
    assert(property_calls == 1 && sync_calls == 1);
    assert(unit[0x49] == 0xa5); /* Preserve progress when setting availability. */
    assert(unit[0x4a] == 0x11); /* The other job's entry is skipped. */
    assert(unit[0x3d] == 7);    /* Special 255 entry follows its own path. */
    return 0;
}
