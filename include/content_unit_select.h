#ifndef FFTA_CONTENT_UNIT_SELECT_H
#define FFTA_CONTENT_UNIT_SELECT_H

#include "content_consumer.h"

/* Select the first content entry whose unit has field 27 bit 12 set. */
int ContentUnitSelectCallback(void *entry, void *context);

#endif
