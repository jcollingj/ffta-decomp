#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "unit_pool.h"
#include "../../src/unit_byte_amount.c"

/* Golden values derived from the original instruction sequence: floor(level/10),
 * double it, add one, multiply by byte[0xf0] * 1000, then cap at 0x1869f. */
static void check(unsigned level_byte, unsigned unit_byte, unsigned expected)
{
    struct UnitRecord unit;
    struct UnitRecord before;
    u8 *bytes = (u8 *)&unit;
    memset(&unit, 0xA5, sizeof(unit));
    bytes[9] = (u8)level_byte;
    bytes[0xf0] = (u8)unit_byte;
    before = unit;
    assert(CalculateUnitByteAmount(bytes) == expected);
    assert(memcmp(&unit, &before, sizeof(unit)) == 0);
}

int main(void)
{
    unsigned level_byte, unit_byte;
    struct UnitRecord unit;
    struct UnitRecord before;
    u8 *bytes = (u8 *)&unit;

    /* Fixed vectors exercise quotient boundaries and both sides of the cap. */
    check(0, 0, 0);
    check(9, 98, 98000);
    check(9, 99, 99000);
    check(9, 100, 99999);
    check(10, 33, 99000);
    check(10, 34, 99999);
    check(49, 11, 99000);
    check(49, 12, 99999);
    check(250, 1, 51000);
    check(250, 2, 99999);
    check(255, 255, 99999);

    /* Exhaust every pair of source bytes while checking the entire unit stays read-only. */
    for (level_byte = 0; level_byte < 256; ++level_byte) {
        for (unit_byte = 0; unit_byte < 256; ++unit_byte) {
            unsigned quotient = level_byte / 10;
            unsigned odd_factor = quotient * 2 + 1;
            unsigned expected = odd_factor * unit_byte * 1000;
            if (expected > 99999) expected = 99999;
            memset(&unit, 0x5A, sizeof(unit));
            bytes[9] = (u8)level_byte;
            bytes[0xf0] = (u8)unit_byte;
            before = unit;
            assert(CalculateUnitByteAmount(bytes) == expected);
            assert(memcmp(&unit, &before, sizeof(unit)) == 0);
        }
    }
    return 0;
}
