#ifndef FFTA_SAVE_COLLECTION_H
#define FFTA_SAVE_COLLECTION_H
#include "types.h"
/* Partial save layout; unknown record payload and gameplay names retained. */
struct SaveCollectionRecord { u32 word00; u8 unknown04[12]; };
struct SaveCollectionView {
    u8 prefix[0x1f68];
    u8 counter;
    u8 unknown1f69[0xf0f];
    struct SaveCollectionRecord records[20];
};
extern struct SaveCollectionView gSaveData;
typedef char SaveCollectionRecordSize[(sizeof(struct SaveCollectionRecord) == 16) ? 1 : -1];
u8 HasSaveCollectionRecord(void);
u8 CountSaveCollectionRecords(void);
void IncrementSaveCounterBelow99(void);
#endif
