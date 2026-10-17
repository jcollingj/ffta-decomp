#include "content_coordinate_lookup.h"
#include "content_unit_coordinate_select.h"

struct ContentConsumerEntry *FindContentEntryByPosition(
    struct ListCountContext *context, int x, int y)
{
    struct ContentCoordinateQuery query;

    query.selected = 0;
    query.x = x;
    query.y = y;
    VisitLinkedNodes(context->unknown10, ContentCoordinateSelectCallback,
                     &query);
    return query.selected;
}

struct ContentConsumerEntry *FindContentEntryByUnitCoordinates(
    struct ListCountContext *context, int x, int y)
{
    struct ContentCoordinateQuery query;

    query.selected = 0;
    query.x = x;
    query.y = y;
    VisitLinkedNodes(context->unknown10, ContentUnitCoordinateSelectCallback,
                     &query);
    return query.selected;
}

asm(".align 2, 0");
