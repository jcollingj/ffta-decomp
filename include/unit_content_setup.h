#ifndef FFTA_UNIT_CONTENT_SETUP_H
#define FFTA_UNIT_CONTENT_SETUP_H

#include "content_record.h"
#include "unit.h"

void ApplyUnitContentProperty33(u8 *unit);
void LoadUnitContentEquipment(u8 *unit,
                              const struct ContentSourceEntry *source);

#endif
