#ifndef FFTA_SAVE_VALUE21B4_H
#define FFTA_SAVE_VALUE21B4_H
#include "types.h"
/* Partial save-image view; gameplay meaning of this byte remains unverified. */
struct SaveValue21B4 { u8 prefix[0x21b4]; u8 value; };
extern struct SaveValue21B4 gSaveData;
void SetFlag60AtSelectedValues(void);
#endif
