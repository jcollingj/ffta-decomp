# Container list clear research

`ClearContainerList` occupies ROM interval `[0xC79B4, 0xC79E0)`, a 44-byte
block. The function symbol is 42 bytes; the final two bytes are zero alignment
padding. The probe used the immutable verified reference ELF
`build/verified-273-list-free-ref.elf`, SHA-256
`36f8de2cb35cd5d54bab32be1851db3e9c647e2abacdcca21667da5a87a03eb7`.

## Original control flow

Addresses below are ROM offsets. Raw Thumb instructions were decoded from
`baserom.gba` bytes at the stated offsets.

| Address | Instruction/effect | Control flow |
| --- | --- | --- |
| `C79B4` | Save `r4`, `r5`, `r6`, `lr` | Entry |
| `C79B6` | Copy owner argument `r0` to `r5` | `r5` retains owner |
| `C79B8` | Load word `[r5]` into `r6` | Capture `allocator_owner` |
| `C79BA` | Load word `[r5+4]` into `r4` | Capture `list_head` |
| `C79BC` | Compare `r4` with zero | Initial branch |
| `C79BE` | `beq C79D4` | Null head skips all freeing |
| `C79C0` | Load word `[r4+8]` into `r1` | Read captured head's `next` |
| `C79C2` | Compare `r1` with zero | Conditional recursion |
| `C79C4` | `beq C79CC` | Null successor skips `FreeListNodes` |
| `C79C6` | Copy captured allocator owner to `r0` | First dependency argument |
| `C79C8` | `bl 0xC7DC8` | `FreeListNodes(owner, head->next)` |
| `C79CC` | Copy captured allocator owner to `r0` | Second dependency argument |
| `C79CE` | Copy captured head to `r1` | Original head argument |
| `C79D0` | `bl 0x07170` | `AllocatorFree(owner, head)` |
| `C79D4` | Set `r0` to zero | Shared fallthrough after null-head branch or free |
| `C79D6` | Store zero to `[r5+4]` | Clear owner root after dependency callbacks |
| `C79D8–C79DC` | Restore registers and return | Function ends at `C79DE` |
| `C79DE` | Zero halfword | Alignment to next function at `C79E0` |

This maps both branches and proves the callback order and captured values from
the original instructions. The lower-level `FreeListNodes` performs tail to
head release of the suffix; this wrapper then releases the original head.

## Candidate and evidence

The candidate reuses `struct Container`, `struct ListCountNode`,
`FreeListNodes`, and the existing `AllocatorFree` declaration. No target
pointer casts or duplicate declarations are introduced. The natural C source
probe at `0xC79B4` matched all 44 block bytes: 42 symbol bytes and two zero
padding bytes. The output is in `build/probes/list_clear/`; the immutable
reference snapshot hash is recorded in `comparison.json`.

The host harness links the actual `src/list_free.c` implementation and stubs
only `AllocatorFree`. Fixed empty, one-node, and four-node cases check the
tail-to-head callback sequence, whole-node storage guards, unchanged unrelated
owner fields, and an unchanged `list_head` during every callback. A mutation
case changes both owner fields in the first callback; subsequent calls still
receive the captured allocator owner and original head, and the routine clears
the root only after those calls finish. The callback retains all storage for
the full test.

Focused verification:

```text
python3 -m unittest tests.test_list_clear_c -v
Ran 1 test ... OK
```

At the time of the original list-clear integration, the copied suffix was
`asm/rom_list_count_wrappers_suffix_suffix_suffix.s`, with incbin offset
`0xC79E0` and length `0x128`. That was an integration-time artifact and has
since been superseded: `FreeTreeNodes` now occupies `0xC79E0..0xC7A34`, and
the next copied remainder was `asm/rom_tree_clear_suffix.s`, with incbin
offset `0xC7A34` and length `0xD4`. The earlier list-clear comparison and
`make compare progress` evidence remain historical evidence for that
integration step; the current combined build is verified separately.

The historical tree-clear remainder was subsequently carved for list unlink. Consult `config/decomp_regions.json` for the current copied wrappers; integration-time filenames above are historical evidence.
