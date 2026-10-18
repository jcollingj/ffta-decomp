# Content entry positions

The routines at ROM offsets `0x99E68` and `0x99C1C` update and read a content
entry's coordinates. The setter uses the low byte of each input twice: as an
unsigned byte for the nested unit fields at `+0xF6` and `+0xF7`, and as a signed
byte for the entry positions at `+0x08` and `+0x0C`. The position formula is
`signed_byte * 32 + 16`. The value returned through the forwarding helper is
shifted left four bits and stored as a 16-bit value at `+0x0A`.

The wrapper at `0x99C1C` loads the entry's object pointer and sign-extends the
low byte of both coordinate arguments before calling `0x131BE0`. It returns
that call's full `u32` value. The copied routine at `0x131BE0` remains an
unrecovered dependency; its Thumb entry is declared as
`GetContentCoordinateQueryValue` in `asm/external_symbols.s`. Neither its
returned value's semantic meaning nor a height interpretation is established
here.

The host harness links the recovered setter and forwarding wrapper as separate
translation units and supplies a controlled stand-in only for the copied
dependency. Its mutation case changes the entry's object pointer during the
call and checks the setter's resulting complete fixture image. It does not
claim host execution of the copied routine. The target content entry has a
`0x60` byte extent; native host layout expands with pointer width while its
opaque extents stay fixed. `UnitFieldView` currently covers bytes through
`+0xFB`, including both setter destinations.

Both C functions now replace their copied ROM bytes in the matching build.
The copied dependency alias does not count as recovered code or establish the
dependency's semantics.
