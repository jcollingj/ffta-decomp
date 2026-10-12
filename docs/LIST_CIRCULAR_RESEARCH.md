# Circular list visitor compiler research

This note preserves the bounded natural-C source-shape search for ROM file interval `[0xC7CE4, 0xC7D18)`. It is a 52-byte copied region: 50 bytes of Thumb function and two zero alignment bytes. The semantic implementation and harness are in `src/nonmatching/list_circular_visit.c` and `tests/c/list_circular_visit_harness.c`; this research file records why the retained source is the best tested shape so another agent can reproduce the search without ignored probe artifacts.

## Decode and probe setup

The original entry saves `r4-r7,lr`, saves `r8`, copies the start node to `r5`, callback to `r8`, context to `r7`, owner to `r6`, and current node to `r4`. Each iteration calls the Thumb trampoline at `0x08142264`, whose instruction is `bx r8` followed by `mov r8,r8`. Callback zero exits. Otherwise it loads current `next` at `+8`; a null successor causes a fresh owner-head load at `+4`; equality with the original start ends the traversal. The callback result uses zero/nonzero semantics.

Probe command for every candidate:

```sh
python3 tools/decomp/probe.py /tmp/lcvhNN.c --start 0xc7ce4 --end 0xc7d18 --reference-elf build/reference-snapshots/verified-266-r8-allocator.elf
```

Immutable reference ELF SHA-256: `98ed294a2fc5c31c032cca2027b3b4e0e554edd670eecc954b39e2422c7dc234`. Original block SHA-256: `9fb421aee6d42eb44d85707ae772692af26417f9f8f46f84110e81716f187fe5`.

All candidates used the same wrapper and the exact body shown below. Source hashes cover the complete source file, including this wrapper and final `.align` directive. The wrapper was:

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    /* insert the candidate body here */
}
asm(".align 2,0");
```

## Results

`size` is candidate block bytes; `diff` is differing bytes against the original 52-byte interval. Candidate code bodies did not receive individual host behavior tests; the retained semantic implementation passed the focused harness described above. H05 was the expanded-loop form and compiled to 84 bytes.

| ID | Source SHA-256 | Size | Differing bytes | Initial copy sequence after prologue |
|---|---|---:|---:|---|
| H01 | `f6811916908b2227ea232ec350eded4105ba9980b23e4838a053c000d623edbd` | 52 | 26 | `mov r8, r0, add r5, r1, #0, add r7, r2, #0, add r6, r3, #0, add r4, r5, #0` |
| H02 | `464f2d4af50b1387da2353c25e1ade85f92ef4fde421901cdd7032d48109cd72` | 52 | 26 | `mov r8, r0, add r5, r1, #0, add r7, r2, #0, add r6, r3, #0, add r4, r5, #0` |
| H03 | `60246634d53342822a6b004a20d27009eefa7c6c6b720064704f1ba9e2dbc618` | 52 | 26 | `mov r8, r0, add r5, r1, #0, add r7, r2, #0, add r6, r3, #0, add r4, r5, #0` |
| H04 | `045f5e3236f2aa89d4b221b6eb8ca6f679c80b38b71514ad26914290d0a49e24` | 52 | 26 | `mov r8, r0, add r5, r1, #0, add r7, r2, #0, add r6, r3, #0, add r4, r5, #0` |
| H05 | `c390ca383745893a113ce7cc9ea6a4c8b8facba6bee9767320d77ba8781ea027` | 84 | 72 | `mov r8, r0, add r5, r1, #0, add r7, r2, #0, add r6, r3, #0, add r4, r5, #0` |
| H06 | `1d633ee2e2652776339be51004db1b82ced733490d9ba3a245467d61b331517c` | 52 | 26 | `mov r8, r0, add r5, r1, #0, add r7, r2, #0, add r6, r3, #0, add r4, r5, #0` |
| H07 | `14c46ac53d86677276317bb4c73cb77075cc82ef27556efc0b556a7f57e76d75` | 52 | 26 | `mov r8, r0, add r5, r1, #0, add r7, r2, #0, add r6, r3, #0, add r4, r5, #0` |
| H08 | `86ec2f3090f20085b08cf906cb19571a1c79a841d4b877a50fe2835497508a8d` | 52 | 7 | `mov r8, r2, add r7, r3, #0, add r6, r0, #0, add r5, r1, #0, add r4, r5, #0` |
| H09 | `3995f6890451d8982da64e2c9613cef564abf73760db67ae2d362e78f1ec3d86` | 52 | 7 | `mov r8, r2, add r7, r3, #0, add r5, r1, #0, add r4, r5, #0, add r6, r0, #0` |
| H10 | `b729da3a282a0615ee37df594a38aadae343fb810d82003b5f65a5a9366265e8` | 52 | 8 | `mov r8, r2, add r7, r3, #0, add r5, r1, #0, add r4, r5, #0, add r6, r0, #0` |
| H11 | `21add45d754c0f13686140e30a9986b1a98957ef1daacd57f3896edd2affc25a` | 52 | 5 | `mov r8, r2, add r7, r3, #0, add r5, r1, #0, add r6, r0, #0, add r4, r5, #0` |
| H12 | `b6bd696057c61a6407c58ff705ae8e9f1110fdc8ef10216f3ef2d38d82f3a70b` | 52 | 7 | `mov r8, r2, add r7, r3, #0, add r6, r0, #0, add r4, r1, #0, add r5, r4, #0` |

Original copy sequence is `add r5,r1,#0; mov r8,r2; add r7,r3,#0; add r6,r0,#0; add r4,r5,#0`. H11 is the best tested source shape: it preserves the original target registers and gets the loop body, `r8` trampoline call, return sequence, function size, and padding right. Its five differing bytes are solely the entry-copy instruction ordering (the first three copies are emitted as callback, context, start rather than start, callback, context). No inline assembly, register-variable declarations, optimization changes, or compiler flags were used.

## Exact source hypotheses

The complete candidate source files follow. Rebuild each by saving the code block as `/tmp/lcvhNN.c`; each block is byte-for-byte the source whose hash is listed above.

### H01

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *node = start;
    do {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = owner->head;
    } while (node != start);
}
asm(".align 2,0");
```

### H02

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *node = start;
    for (;;) {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = owner->head;
        if (node == start) break;
    }
}
asm(".align 2,0");
```

### H03

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *node = start;
    while (1) {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = owner->head;
        if (node == start) break;
    }
}
asm(".align 2,0");
```

### H04

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *node = start;
    u32 result;
    do {
        result = callback(node, context);
        if (result == 0) break;
        node = node->next;
        if (node == 0) node = owner->head;
    } while (node != start);
}
asm(".align 2,0");
```

### H05

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *node = start;
    while (node == start) {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = owner->head;
        if (node == start) break;
        while (node != start) {
            if (callback(node, context) == 0) return;
            node = node->next;
            if (node == 0) node = owner->head;
        }
    }
}
asm(".align 2,0");
```

### H06

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *node = start;
    while (callback(node, context) != 0) {
        node = node->next;
        if (node == 0) node = owner->head;
        if (node == start) break;
    }
}
asm(".align 2,0");
```

### H07

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *node;
    node = start;
    do {
        if (!callback(node, context)) break;
        node = node->next;
        if (!node) node = owner->head;
    } while (node != start);
}
asm(".align 2,0");
```

### H08

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountOwner *list = owner;
    struct ListCountNode *first = start;
    struct ListCountNode *node = first;
    do {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = list->head;
    } while (node != first);
}
asm(".align 2,0");
```

### H09

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *first = start;
    struct ListCountNode *node = first;
    struct ListCountOwner *list = owner;
    do {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = list->head;
    } while (first != node);
}
asm(".align 2,0");
```

### H10

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *first = start;
    struct ListCountNode *node = first;
    struct ListCountOwner *list = owner;
    do {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = list->head;
    } while (node != first);
}
asm(".align 2,0");
```

### H11

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountNode *first = start;
    struct ListCountOwner *list = owner;
    struct ListCountNode *node = first;
    do {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = list->head;
    } while (first != node);
}
asm(".align 2,0");
```

### H12

```c
#include "list_circular_visit.h"
void VisitCircularListFrom(struct ListCountOwner *owner, struct ListCountNode *start, ListCircularVisitCallback callback, void *context)
{
    struct ListCountOwner *list = owner;
    struct ListCountNode *node = start;
    struct ListCountNode *first = start;
    do {
        if (callback(node, context) == 0) break;
        node = node->next;
        if (node == 0) node = list->head;
    } while (first != node);
}
asm(".align 2,0");
```

## Retained implementation evidence

The retained repository source uses the H11 local-alias shape with the project header and multiline formatting. Its SHA-256 is `2c7c4f20943c05219e1d13e2e3440d2193380aa9190f42f78a8462c794247704`. The isolated probe reports 52 candidate bytes and 5 differences; the candidate function symbol is 50 bytes, followed by two zero bytes from `.align 2,0`. The host harness passed with head/middle/tail starts on one-, three-, and six-node lists, all stop positions on fixed traces, callback result 7, next-link mutation, wrap-time owner-head mutation, and whole-fixture comparisons.
