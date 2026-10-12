#include <assert.h>
#include <stddef.h>
#include <string.h>

#define TABLE_LENGTH 65536
#define TABLE_CENTER 32768

/* The host linker aliases the target array symbol to the center of this
 * backing allocation. Thus signed-short offsets address allocated slots on
 * both sides of the alias, while the candidate retains its array declaration. */
static const unsigned char gObjects[TABLE_LENGTH];

#if defined(__APPLE__)
#define ASM_PREFIX "_"
#else
#define ASM_PREFIX ""
#endif
#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)
__asm__(".data\n"
        ".globl " ASM_PREFIX "gContentPointersBacking\n"
        ASM_PREFIX "gContentPointersBacking:\n"
        ".space (" STRINGIFY(TABLE_CENTER) " * " STRINGIFY(__SIZEOF_POINTER__) ")\n"
        ".globl " ASM_PREFIX "gContentPointers\n"
        ASM_PREFIX "gContentPointers:\n"
        ".space (" STRINGIFY(TABLE_CENTER) " * " STRINGIFY(__SIZEOF_POINTER__) ")\n");
extern const void *gContentPointersBacking[TABLE_LENGTH];

#include "../../src/nonmatching/content_pointers.c"

int main(void)
{
    static const void *table_before[TABLE_LENGTH];
    unsigned input;
    for (input = 0; input < TABLE_LENGTH; input++)
        gContentPointersBacking[input] = &gObjects[input];
    memcpy(table_before, gContentPointersBacking, sizeof(table_before));
    for (input = 0; input < TABLE_LENGTH; input++) {
        unsigned wrapped;
        unsigned adjustment;
        int signed_index;
        unsigned centered_index;
        const void *expected;
        if (input == 0) {
            expected = &gObjects[TABLE_CENTER];
        } else {
            adjustment = input > 85 ? 540 : 625;
            wrapped = (input + adjustment) & 0xffffu;
            signed_index = wrapped < 0x8000u ? (int)wrapped : (int)wrapped - 0x10000;
            centered_index = (unsigned)(TABLE_CENTER + signed_index);
            expected = gContentPointersBacking[centered_index];
        }
        assert(GetRemappedContentPointer((u16)input) == expected);
    }
    assert(memcmp(table_before, gContentPointersBacking, sizeof(table_before)) == 0);

    /* Fixed vectors independently pin the branch boundary, signed-short
     * transition, and u16 wraparound. */
    assert(GetRemappedContentPointer(0) == &gObjects[TABLE_CENTER]);
    assert(GetRemappedContentPointer(85) == &gObjects[TABLE_CENTER + 710]);
    assert(GetRemappedContentPointer(86) == &gObjects[TABLE_CENTER + 626]);
    assert(GetRemappedContentPointer(32227) == &gObjects[TABLE_CENTER + 32767]);
    assert(GetRemappedContentPointer(32228) == &gObjects[TABLE_CENTER - 32768]);
    assert(GetRemappedContentPointer(65535) == &gObjects[TABLE_CENTER + 539]);
    return 0;
}
