# Recursive list free helper research

## Recovery bounds and evidence

The candidate starts at ROM offset `0xC7DC8` and ends at `0xC7DEC`. The symbol is 34 bytes (`0xC7DC8–0xC7DEA`); the following halfword at `0xC7DEA` is zero padding, bringing the aligned block end to `0xC7DEC`. The verified reference is `build/verified-272-search-ref.elf`, SHA-256 `831b31e5d45d948f9b300d7cca015712c07cb97df0b0b13edbffcad5598c601a`.

The original bytes were inspected with `arm-none-eabi-objdump -D -Mforce-thumb` over virtual addresses `0x080C7DC8–0x080C7DEC`. The branch and call map is:

| Address | Instruction | Observed effect |
| --- | --- | --- |
| `0xC7DC8` | `push {r4, r5, lr}` | Save recursive-call state. |
| `0xC7DCA` | `adds r5, r0, #0` | Preserve owner. |
| `0xC7DCC` | `adds r4, r1, #0` | Preserve current node. |
| `0xC7DCE` | `cmp r4, #0` | Test node. |
| `0xC7DD0` | `beq 0xC7DE4` | Null node skips all loads and calls and reaches the epilogue. |
| `0xC7DD2` | `ldr r1, [r4, #8]` | Read the node's `next` pointer. |
| `0xC7DD4` | `cmp r1, #0` | Test successor. |
| `0xC7DD6` | `beq 0xC7DDC` | Null successor skips recursion but still reaches this node's free call. |
| `0xC7DD8` | `bl 0xC7DC8` | Recurse with the same owner and the captured nonnull successor. This is the only back-edge/call into the routine. |
| `0xC7DDC` | `adds r0, r5, #0` | Restore owner argument. |
| `0xC7DDE` | `adds r1, r4, #0` | Set current node argument. |
| `0xC7DE0` | `bl 0x08007170` | Call the verified `AllocatorFree(owner, node)` wrapper after recursion. |
| `0xC7DE4` | `pop {r4, r5}` | Restore saved registers. |
| `0xC7DE6` | `pop {r0}` | Restore saved return address. |
| `0xC7DE8` | `bx r0` | Return; end of the 34-byte function. |
| `0xC7DEA` | `movs r0, r0` | Zero alignment halfword, outside the symbol. |

The source uses the established `struct ListCountNode` and allocator declaration. It retains the observed conditional recursive call instead of making an unconditional call with a possibly null node. No cycle detection is present in the recovered routine.

## Probe and tests

The source is preserved as `src/list_free.c` with its public declaration in `include/list_free.h`. It is integrated into the matching build; no separate copied alias remains. The isolated command was:

```sh
python3 tools/decomp/probe.py src/list_free.c \
    --start 0xC7DC8 --end 0xC7DEC \
    --reference-elf build/verified-272-search-ref.elf
```

The probe selected optimization level 2 and reported 36 of 36 block bytes equal, zero differences, candidate address `0xC7DC8`, and function symbol size 34 bytes. The original function bytes at ROM offset `0xC7DC8` have SHA-256 `3a39806292a1ebe6ff19feff8f5c9e3aeae4072f436632e17ff7870113586d9a`; the complete 36-byte block including padding has SHA-256 `046f4bd9173d5ed0c884532b9bbe97e1ae6eb30cd1ccb72afbb9d2a2d4c1460c`. The carve plan replaced copied interval `[818632, 818744)` with the matching source and a copied suffix at offset `818668`, length `76`; unrelated object order was preserved.

`python3 -m unittest tests.test_list_free_c -v` passes. Its host stub retains allocations and records fixed pointer order and owner identity. The cases check null, one-node, and five-node lists, compare all node bytes for the nonmutating cases, and verify tail-to-head calls. A separate controlled callback case mutates the still-live head's `next` pointer during the tail's free callback; the fixed trace still visits and frees the captured unwind path. This demonstrates candidate behavior under that stub, not side effects of the target allocator.
