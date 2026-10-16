# Tree detach-and-free wrapper research

`DetachAndFreeTreeNode` occupies ROM file interval `[0xC7A5C, 0xC7A74)`,
exactly 24 bytes. It is integrated as matching C. The interval was the complete
former `asm/rom_list_append_prefix.s` incbin and ends immediately before
`AppendListNode` at `0xC7A74`.

## Original instruction evidence

The ROM source identity is SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`. The original 24-byte slice was
saved as `build/tree_remove_original.bin`; SHA-256 of those bytes is
`ba95ba682bb5aab3a46bd2822715b2c244d6e11ce0d86e77238e9667e1ca9208`.
The raw Thumb disassembly command was:

```sh
arm-none-eabi-objdump -D -b binary -m arm -M force-thumb \
  --adjust-vma=0xC7A5C build/tree_remove_original.bin
```

| Address | Instruction/effect |
| --- | --- |
| `C7A5C` | Save `r4`, `r5`, and return address. |
| `C7A5E` | Retain owner in `r4`. |
| `C7A60` | Retain node in `r5`. |
| `C7A62` | Call `DetachTreeNode(owner, node)` at `0xC7858`. |
| `C7A66` | Reload `owner->allocator_owner` after the detach call. |
| `C7A68` | Restore node as the second argument. |
| `C7A6A` | Call `AllocatorFree(allocator_owner, node)` at `0x7170`. |
| `C7A6E..C7A72` | Restore registers and return; the function ends at `C7A74`. |

The allocator reload is observable behavior: detach may change the owner field,
and free uses the changed value. The wrapper returns no value. It does not clear
the node or owner itself.

## Host test contract

`tests/test_tree_remove_c.py` compiles the production wrapper and its host
harness as separate translation units. The harness supplies a compatible
`DetachTreeNode` stub that checks the complete fixed owner and three-node images
at callback entry, then mutates the allocator pointer, every owner field, and
every node field. `AllocatorFree` checks the second event in the exact two-call
trace, the original node address, the reloaded allocator, and every byte of the
fixed post-detach owner/node images. Its stub retains storage for the final
whole-image comparison. The expected images are explicitly initialized
fixtures; they are not derived by rerunning wrapper logic.

The test deliberately stubs only the unrecovered dependency for this wrapper
contract. It does not claim that `DetachTreeNode` itself matches ROM. A separate
real-candidate composition test could establish semantic composition, but is
not part of this primary callback-state oracle.

## Isolated target probe

The dependency probe used the immutable verified ELF
`build/verified-278-tree-layout-ref.elf`, SHA-256
`be0a3ca7edfc3057225e11b125451376d250ea2b3c7cd52d0953a02276fbc2e4`. A unique
ignored build copy was extended with the external Thumb function symbol for the
nonmatching detach dependency. The exact commands were:

```sh
cp build/verified-278-tree-layout-ref.elf \
  build/verified-278-tree-layout-session63-remove-ref.elf
arm-none-eabi-objcopy \
  --add-symbol DetachTreeNode=0x080c7859,global,function \
  build/verified-278-tree-layout-session63-remove-ref.elf \
  build/verified-278-tree-layout-session63-remove-ref.tmp.elf
mv build/verified-278-tree-layout-session63-remove-ref.tmp.elf \
  build/verified-278-tree-layout-session63-remove-ref.elf
```

`readelf -s` reports the added symbol as `0x080C7859`, type `FUNC`, global,
absolute; the odd symbol value preserves Thumb mode. `nm` displays the code
address with the Thumb bit cleared as `0x080C7858`. The extended reference ELF
SHA-256 is
`5089dbce08ea9c7108c80dabc730d129452e7f833fb0e9131292ceaf41b2b3c9`.
The base reference retained its original hash.

Probe command:

```sh
python3 tools/decomp/probe.py src/tree_remove.c \
  --start 0xC7A5C --end 0xC7A74 \
  --reference-elf build/verified-278-tree-layout-session63-remove-ref.elf \
  --disassemble
```

The isolated result was a 24-byte candidate symbol at `0x080C7A5C`, with zero
differences over the 24-byte original interval. The candidate ELF and extracted
candidate bytes have SHA-256 values `1ee6f8d9d7dbcf7a3b121f7be42f7eb96e6247e71fcbef75317ab7235595c78c`
and `ba95ba682bb5aab3a46bd2822715b2c244d6e11ce0d86e77238e9667e1ca9208`,
respectively. The probe reports the function as `UNINVENTORIED` because no
shared manifest had not yet been updated. Main-build integration now adds a
strict function comparison and asserts that `AppendListNode -
DetachAndFreeTreeNode` is 24. `rom_list_unlink_suffix_start` is a linker alias
of the real function symbol. The typed `DetachTreeNode` alias points to copied
code at `0x080C7859`; its existing bytes remain copied provenance and receive
no duplicate recovery credit.

Focused host verification:

```text
python3 -m unittest tests.test_tree_remove_c -v
Ran 1 test ... OK
```
