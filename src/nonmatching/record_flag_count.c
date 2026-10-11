#include "types.h"
/* Partial views only: list ownership and gameplay names remain unverified. */
struct FlagRecord { u32 id:10; } __attribute__((packed));
struct FlagRecordNode {
    const struct FlagRecord *record;
    u32 unknown04;
    struct FlagRecordNode *next;
};
struct FlagListSaveView { u8 prefix[0x28c8]; struct FlagRecordNode *head; };
extern struct FlagListSaveView gSaveData;
extern const u16 gCountedRecordIds[];
extern u8 GetGameFlag(u16 flag);
static inline int HasRecord383(void)
{
    struct FlagRecordNode *node = gSaveData.head;
    while (node) {
        if (node->record->id == 383) return 1;
        node = node->next;
    }
    return 0;
}
u16 CountSelectedRecordFlags(void)
{
    int count = 0;
    const u16 *id = gCountedRecordIds;
    while (*id != 65535) {
        if (GetGameFlag(*id + 767)) count++;
        id++;
    }
    if (GetGameFlag(54) || HasRecord383()) count++;
    return count;
}
