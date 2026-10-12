"""Lossless decoder/encoder and GNU as renderer for the original 52-byte job records."""
from __future__ import annotations
import argparse
from pathlib import Path

COUNT = 116
RECORD_SIZE = 52
START = 0x521A14
SIZE = COUNT * RECORD_SIZE
# The names pNN match GetJobProperty selectors; unknown bytes remain explicitly unknown.
BYTE_FIELDS = {
    0: ('p00', 2, 'u16'), 2: ('unknown02', 2, 'bytes'),
    4: ('p01', 1, 'u8'), 5: ('inherit', 1, 'u8'), 6: ('p03', 1, 'u8'),
    7: ('p04', 2, 'u16'), 9: ('p05', 2, 'u16'),
    12: ('p08', 1, 'u8'), 13: ('p09', 1, 'u8'), 14: ('p10', 1, 'u8'),
    15: ('p11', 1, 'u8'), 16: ('p12', 1, 'u8'), 17: ('p13', 1, 'u8'),
    22: ('p22', 1, 'u8'), 23: ('p23', 1, 'u8'), 24: ('p24', 1, 'u8'), 25: ('p25', 1, 'u8'),
    32: ('p41', 1, 'u8'), 33: ('p42', 1, 'u8'), 34: ('p43', 1, 'u8'),
    35: ('p44', 1, 'u8'), 36: ('p45', 1, 'u8'), 37: ('p46', 1, 'u8'),
    38: ('p47', 1, 'u8'), 39: ('unknown27', 1, 'u8'),
    40: ('p30', 1, 'u8'), 41: ('p31', 1, 'u8'), 42: ('p32', 1, 'u8'),
    44: ('p33', 1, 'u8'), 45: ('p36', 1, 'u8'), 46: ('p37', 1, 'u8'), 47: ('p38', 1, 'u8'), 48: ('p39', 1, 'u8'), 49: ('p40', 1, 'u8'), 50: ('unknown32', 2, 'bytes'),
}
BIT_GROUPS = [
    (11, [('p06',4), ('p07',4)]),
    (18, [('unknown18',3), ('p14',3), ('p15',3), ('p16',3), ('p17',3), ('p18',3), ('p19',3), ('p20',3), ('p21',3), ('unknown21',5)]),
    (26, [('p26',12), ('p27',12), ('p28',12), ('p29',12)]),
    (43, [('p34',4), ('p35',4)]),
]

def _check_record(r):
    if not isinstance(r, dict) or set(r) != {n for _, fields in BIT_GROUPS for n,_ in fields} | {v[0] for v in BYTE_FIELDS.values()}:
        raise ValueError('record fields do not match the documented 52-byte schema')

def decode_record(data: bytes) -> dict:
    if len(data) != RECORD_SIZE: raise ValueError('job record must be 52 bytes')
    r={}
    for off,(name,width,kind) in BYTE_FIELDS.items():
        chunk=data[off:off+width]
        r[name]=chunk.hex() if kind=='bytes' else int.from_bytes(chunk,'little')
    for off,fields in BIT_GROUPS:
        word=int.from_bytes(data[off:off+(sum(b for _,b in fields)+7)//8],'little'); shift=0
        for name,bits in fields:
            r[name]=(word>>shift)&((1<<bits)-1); shift+=bits
    return r

def encode_record(r: dict) -> bytes:
    _check_record(r); out=bytearray(RECORD_SIZE)
    for off,(name,width,kind) in BYTE_FIELDS.items():
        value=r[name]
        if kind=='bytes':
            if not isinstance(value,str) or len(value)!=width*2:
                raise ValueError(f'{name} must be {width} raw bytes as hex')
            try: raw=bytes.fromhex(value)
            except ValueError as e: raise ValueError(f'{name} must be hexadecimal') from e
            if len(raw) != width:
                raise ValueError(f'{name} must decode to exactly {width} bytes')
            for byte in raw: _range(name, byte, 8)
            out[off:off+width]=raw
        else:
            _range(name,value,8*width); out[off:off+width]=value.to_bytes(width,'little')
    for off,fields in BIT_GROUPS:
        word=0; shift=0
        for name,bits in fields:
            value=r[name]; _range(name,value,bits); word |= value<<shift; shift+=bits
        width=(sum(b for _,b in fields)+7)//8; out[off:off+width]=word.to_bytes(width,'little')
    return bytes(out)

def _range(name,value,bits):
    if type(value) is not int or not 0 <= value < (1<<bits):
        raise ValueError(f'{name} must fit unsigned {bits}-bit field')

def decode_table(data: bytes) -> list[dict]:
    if len(data)!=SIZE: raise ValueError(f'job table must be exactly {SIZE} bytes')
    return [decode_record(data[i:i+RECORD_SIZE]) for i in range(0,len(data),RECORD_SIZE)]

def encode_table(records):
    if len(records)!=COUNT: raise ValueError(f'job table must contain {COUNT} records')
    return b''.join(map(encode_record,records))

def render_asm(records):
    if len(records)!=COUNT: raise ValueError(f'job table must contain {COUNT} records')
    for record in records: encode_record(record)
    # Stable source field order grouped by byte offset.
    names=[]
    for off in range(RECORD_SIZE):
        for bo,fs in BIT_GROUPS:
            if bo==off: names.extend(n for n,_ in fs)
        if off in BYTE_FIELDS: names.append(BYTE_FIELDS[off][0])
    lines=['@ Authored 116-record job table. pNN are GetJobProperty selectors.', '.section .text', '.global gJobs', '.type gJobs, %object',
'.macro job_u8 value', '    .if (\\value < 0) || (\\value > 255)', '        .error "job_u8 out of range"', '    .endif', '    .byte \\value', '.endm',
'.macro job_u16 value', '    .if (\\value < 0) || (\\value > 65535)', '        .error "job_u16 out of range"', '    .endif', '    .hword \\value', '.endm',
'.macro job_bits bits, value', '    .if (\\value < 0) || (\\value >= (1 << \\bits))', '        .error "job packed field out of range"', '    .endif', '.endm',
'.macro job_raw2 a, b', '    .byte \\a, \\b', '.endm',
'.macro job_raw1 value', '    job_u8 \\value', '.endm',
'.macro job_record id']
    # Macro parameters comma-separated, then emit grouped fields according to physical layout.
    macro_names=[]
    for n in names:
        macro_names.extend([n+'_0', n+'_1'] if n in ('unknown02','unknown32') else [n])
    lines[-1]='.macro job_record id, '+', '.join(macro_names)
    lines += ['    .if (. - gJobs) != (\\id * 52)', '        .error "job IDs must be consecutive from zero"', '    .endif']
    for off in range(RECORD_SIZE):
        bg=next((fs for bo,fs in BIT_GROUPS if bo==off),None)
        if bg:
            for n,b in bg: lines.append(f'    job_bits {b}, \\{n}')
            # Emit all fields packed into little-endian 32-bit word as bytes by assembler expression.
            expr=' | '.join(f'(\\{n} << {sum(x[1] for x in bg[:i])})' for i,(n,_) in enumerate(bg))
            for sh in range(0, sum(x[1] for x in bg), 8): lines.append(f'    .byte (({expr}) >> {sh}) & 255')
            continue
        if off in BYTE_FIELDS:
            n,w,k=BYTE_FIELDS[off]
            if k=='bytes': lines.extend([f'    job_u8 \\{n}_0', f'    job_u8 \\{n}_1'])
            elif w==2: lines.append(f'    job_u16 \\{n}')
            else: lines.append(f'    job_u8 \\{n}')
    lines += ['.endm','gJobs:']
    for i,r in enumerate(records):
        vals=[]
        for n in names:
            if n in ('unknown02','unknown32'): vals.extend(bytes.fromhex(r[n]))
            else: vals.append(r[n])
        lines.append(f'    job_record {i}, '+', '.join(map(str,vals)))
        lines.append('')
    lines += [f'.if (. - gJobs) != {SIZE}', '.error "job table must contain 116 records"', '.endif']
    return '\n'.join(lines)+'\n'

def main():
    p=argparse.ArgumentParser(); p.add_argument('rom',type=Path); p.add_argument('output',type=Path)
    a=p.parse_args(); rom=a.rom.read_bytes()
    if len(rom)<START+SIZE: raise SystemExit('input ROM is truncated before job table')
    a.output.write_text(render_asm(decode_table(rom[START:START+SIZE])))
if __name__=='__main__': main()
