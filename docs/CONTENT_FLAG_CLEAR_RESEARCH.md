# `ClearContentFlag21FIfSet` recovery research

The routine occupies the candidate interval `0x9FF8–0xA018` (exclusive end),
32 bytes total. Thumb instructions start with `push {lr}` at `0x9FF8`. The
literal load at `0x9FFA` resolves to the word at `0xA014`, whose little-endian
value is `0x0000021F`. The call at `0x9FFC` is `GetGameFlag(0x21F)`. The code
shifts its `u8` result left by 24 and compares with zero; the branch at
`0xA004` skips the setter when the result is zero. Otherwise it reloads
`0x21F`, passes zero as the second argument, and calls `SetGameFlag` at
`0xA00A`. Both paths reach `pop {r0}; bx r0` at `0xA00E–0xA011`.

Byte accounting for the interval is 26 bytes of executed instructions,
2 zero alignment bytes at `0xA012–0xA013`, and the 4-byte literal pool word at
`0xA014–0xA017`. Thus the block is 32 bytes; the final 6 bytes are not
executable instructions. The original block comparison reports a 32-byte
expected interval. The literal's numeric value is checked from its resolved
address and decoded word, independently of the test fixture.

The controlled harness compiles the candidate as its own translation unit
and supplies `GetGameFlag`/`SetGameFlag` stubs. It tests every possible `u8`
getter result (0 through 255), requires exactly one getter call with `0x21F`,
and checks the ordered `G` or `GS` trace and setter arguments. A full initialized
`gGameFlags` backing image is compared before and after, including from inside
the setter, to assert that this wrapper itself does not mutate storage. A
second build links the actual `src/game_flags.c` in its own translation unit;
it checks the real boolean getter/setter composition over every possible byte
value and compares the whole backing image, including neighboring flag bits.
Controlled stubs cover return values 2–255, which the production getter
cannot return because its contract is boolean.

## Natural source-shape trials

Each trial used the same declarations and function wrapper below, replacing
only the body with the listed text. SHA-256 hashes identify the exact complete
trial source. All probes used `python3 tools/decomp/probe.py` for
`0x9FF8–0xA018`, optimization `-O2`, and the immutable reference ELF whose
SHA-256 was
`be0a3ca7edfc3057225e11b125451376d250ea2b3c7cd52d0953a02276fbc2e4`.
Every trial retained the entry address. The comparison reports a 36-byte
candidate function symbol against the 32-byte original comparison interval.
Within the shared 32-byte interval, 27 bytes differ; the candidate has another
4 bytes after that interval. The probe's `functions[0].differing_bytes` is 31:
27 differing bytes in the overlap plus the 4-byte length difference. These
metrics keep the candidate symbol size, original block extent, overlap
differences, and probe length-adjusted difference count distinct. No trial
improved the source, so the original readable candidate is retained.

This executable generator reconstructs every full trial source and asserts
its SHA-256. It does not contain placeholder comments or omit source newlines:

```python
from hashlib import sha256
from pathlib import Path

prefix = (
    '#include "types.h"\n'
    'extern u8 GetGameFlag(u16 flag);\n'
    'extern void SetGameFlag(u16 flag, u16 value);\n'
    'void ClearContentFlag21FIfSet(void)\n'
    '{\n'
)
suffix = '\n}\n'
trials = {
    'if_call': ('if (GetGameFlag(0x21F)) SetGameFlag(0x21F, 0);',
                '333f89c824f25c94b187498bbc56b11d7987f35779e557ee5a07a97f82a3db63'),
    'if_neg_empty': ('if (!GetGameFlag(0x21F)) { } else { SetGameFlag(0x21F, 0); }',
                     '1ba4abbe063adb9cd67678ab3d24fd0c4a38259e2dc727257c1a5a9cce729727'),
    'local_byte': ('u8 flag = GetGameFlag(0x21F); if (flag != 0) SetGameFlag(0x21F, 0);',
                   '39a3ce058c5e9c322df84e3a658c650d95d36be47c9c04627cf87d6a024950ea'),
    'local_halfword': ('u16 flag = GetGameFlag(0x21F); if (flag) SetGameFlag(0x21F, 0);',
                       '55e5cd6fdf5682409af5070b3cf09193dc488d4f0e3ec3c1409f049731a764dd'),
    'local_invert': ('u8 flag = GetGameFlag(0x21F); if (flag == 0) return; SetGameFlag(0x21F, 0);',
                     '375db4a3ca688ee4272f0cd78b5fe422b14109727d9bdc8d84473a6b38dc9790'),
    'conditional_statement': ('GetGameFlag(0x21F) ? SetGameFlag(0x21F, 0) : (void)0;',
                              '3735a175b0b5724448ad3345721b2a9559b000725251016cbfae2b773e994244'),
    'do_once': ('do { if (GetGameFlag(0x21F) == 0) break; SetGameFlag(0x21F, 0); } while (0);',
                '8126d24df07c1298d19f5296a754bfa88f9f46df405e9166ed61444182e09b35'),
    'logical_guard': ('if (GetGameFlag(0x21F) && 1) SetGameFlag(0x21F, 0);',
                      'fff33a18a08d6fc56a054096c67a647008daf3a355f54873c33c38e288fa9f5c'),
    'compare_one': ('if (GetGameFlag(0x21F) != 0) SetGameFlag(0x21F, 0);',
                    '99173bb03dea16635c4b8b976e2e9a1b2feb9f8c6f2a44fbac9b3cf04066ef3d'),
    'nested': ('if (GetGameFlag(0x21F) != 0) { SetGameFlag(0x21F, 0); }',
               '332776571904952385dde7afb84d4e654dbc626bbb585a36b666d91f07944bdc'),
}

output = Path('/tmp/content_flag_clear_trials')
output.mkdir(exist_ok=True)
for name, (body, expected_hash) in trials.items():
    source = (prefix + body + suffix).encode()
    actual_hash = sha256(source).hexdigest()
    assert actual_hash == expected_hash, (name, actual_hash, expected_hash)
    (output / (name + '.c')).write_bytes(source)
print('verified', len(trials), 'trial source hashes in', output)
```

| Trial | Body | Source SHA-256 | Candidate function symbol | Original interval | Overlap byte differences | Probe length-adjusted differences |
|---|---|---|---:|---:|---:|---:|
| `if_call` | `if (GetGameFlag(0x21F)) SetGameFlag(0x21F, 0);` | `333f89c824f25c94b187498bbc56b11d7987f35779e557ee5a07a97f82a3db63` | 36 | 32 | 27 | 31 |
| `if_neg_empty` | `if (!GetGameFlag(0x21F)) { } else { SetGameFlag(0x21F, 0); }` | `1ba4abbe063adb9cd67678ab3d24fd0c4a38259e2dc727257c1a5a9cce729727` | 36 | 32 | 27 | 31 |
| `local_byte` | `u8 flag = GetGameFlag(0x21F); if (flag != 0) SetGameFlag(0x21F, 0);` | `39a3ce058c5e9c322df84e3a658c650d95d36be47c9c04627cf87d6a024950ea` | 36 | 32 | 27 | 31 |
| `local_halfword` | `u16 flag = GetGameFlag(0x21F); if (flag) SetGameFlag(0x21F, 0);` | `55e5cd6fdf5682409af5070b3cf09193dc488d4f0e3ec3c1409f049731a764dd` | 36 | 32 | 27 | 31 |
| `local_invert` | `u8 flag = GetGameFlag(0x21F); if (flag == 0) return; SetGameFlag(0x21F, 0);` | `375db4a3ca688ee4272f0cd78b5fe422b14109727d9bdc8d84473a6b38dc9790` | 36 | 32 | 27 | 31 |
| `conditional_statement` | `GetGameFlag(0x21F) ? SetGameFlag(0x21F, 0) : (void)0;` | `3735a175b0b5724448ad3345721b2a9559b000725251016cbfae2b773e994244` | 36 | 32 | 27 | 31 |
| `do_once` | `do { if (GetGameFlag(0x21F) == 0) break; SetGameFlag(0x21F, 0); } while (0);` | `8126d24df07c1298d19f5296a754bfa88f9f46df405e9166ed61444182e09b35` | 36 | 32 | 27 | 31 |
| `logical_guard` | `if (GetGameFlag(0x21F) && 1) SetGameFlag(0x21F, 0);` | `fff33a18a08d6fc56a054096c67a647008daf3a355f54873c33c38e288fa9f5c` | 36 | 32 | 27 | 31 |
| `compare_one` | `if (GetGameFlag(0x21F) != 0) SetGameFlag(0x21F, 0);` | `99173bb03dea16635c4b8b976e2e9a1b2feb9f8c6f2a44fbac9b3cf04066ef3d` | 36 | 32 | 27 | 31 |
| `nested` | `if (GetGameFlag(0x21F) != 0) { SetGameFlag(0x21F, 0); }` | `332776571904952385dde7afb84d4e654dbc626bbb585a36b666d91f07944bdc` | 36 | 32 | 27 | 31 |

These are compiler-shape experiments only. The candidate remains nonmatching
and is not counted as recovered code.
