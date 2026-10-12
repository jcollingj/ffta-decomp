"""Lossless authored job records and packed field boundary checks."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from tools.decomp import job_data

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('arm-none-eabi-as') and shutil.which('arm-none-eabi-objcopy'),
                     'requires matching-build binutils')
class JobDataTests(unittest.TestCase):
    def assemble(self, source):
        with tempfile.TemporaryDirectory() as directory:
            folder=Path(directory); (folder/'jobs.s').write_text(source)
            result=subprocess.run(['arm-none-eabi-as','-o',str(folder/'jobs.o'),str(folder/'jobs.s')],capture_output=True,text=True)
            if result.returncode: return result,None
            subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(folder/'jobs.o'),str(folder/'jobs.bin')],check=True)
            return result,(folder/'jobs.bin').read_bytes()

    @classmethod
    def setUpClass(cls):
        cls.raw=(ROOT/'baserom.gba').read_bytes()[job_data.START:job_data.START+job_data.SIZE]
        cls.records=job_data.decode_table(cls.raw)
        cls.source=(ROOT/'data/jobs.s').read_text()

    def test_schema_and_generated_assembly_round_trip_exact_table(self):
        self.assertEqual(len(self.raw),116*52)
        self.assertEqual(job_data.encode_table(self.records),self.raw)
        self.assertEqual(job_data.render_asm(self.records),self.source)
        result,actual=self.assemble(self.source)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(len(actual),6032)
        self.assertEqual(actual,self.raw)

    def test_target_selector_layout_matches_decoded_fields(self):
        # Independently transcribed from ARM objdump of GetJobProperty: LDRB/LSRS
        # for selectors 6-7 and 14-21, bytewise unaligned loads for selectors 0/4/5,
        # and bytewise 12-bit assembly for selectors 26-29. See parent task evidence.
        b=bytes((i*37+11)&255 for i in range(52))
        decoded=job_data.decode_record(b)
        byte_reads={1:(4,8),3:(6,8),8:(12,8),9:(13,8),10:(14,8),11:(15,8),
                    12:(16,8),13:(17,8),22:(22,8),23:(23,8),24:(24,8),25:(25,8),
                    30:(40,8),31:(41,8),32:(42,8),33:(44,8),36:(45,8),37:(46,8),
                    38:(47,8),39:(48,8),40:(49,8),41:(32,8),42:(33,8),43:(34,8),
                    44:(35,8),45:(36,8),46:(37,8),47:(38,8)}
        selector_field={0:'p00',1:'p01',2:'inherit',3:'p03',4:'p04',5:'p05',6:'p06',7:'p07',
                        8:'p08',9:'p09',10:'p10',11:'p11',12:'p12',13:'p13',
                        14:'p14',15:'p15',16:'p16',17:'p17',18:'p18',19:'p19',20:'p20',21:'p21',
                        22:'p22',23:'p23',24:'p24',25:'p25',26:'p26',27:'p27',28:'p28',29:'p29',
                        30:'p30',31:'p31',32:'p32',33:'p33',34:'p34',35:'p35',36:'p36',37:'p37',
                        38:'p38',39:'p39',40:'p40',41:'p41',42:'p42',43:'p43',44:'p44',45:'p45',46:'p46',47:'p47'}
        direct={0:(0,16),4:(7,16),5:(9,16)}
        for selector,(offset,width) in byte_reads.items():
            self.assertEqual(decoded[selector_field[selector]], int.from_bytes(b[offset:offset+width//8],'little'))
        for selector,(offset,width) in direct.items():
            self.assertEqual(decoded[selector_field[selector]],int.from_bytes(b[offset:offset+width//8],'little'))
        self.assertEqual(decoded['p06'],b[11]&15); self.assertEqual(decoded['p07'],b[11]>>4)
        packed=int.from_bytes(b[18:22],'little')
        for selector in range(14,22):
            self.assertEqual(decoded[selector_field[selector]],(packed >> (3*(selector-13))) & 7)
        packed=int.from_bytes(b[26:32],'little')
        for selector in range(26,30):
            self.assertEqual(decoded[selector_field[selector]],(packed >> (12*(selector-26))) & 0xfff)
        self.assertEqual(decoded['p34'],b[43]&15); self.assertEqual(decoded['p35'],b[43]>>4)

    def test_bitfield_edits_preserve_all_other_bits(self):
        fields=[(n,bits,offset,shift) for offset,group in job_data.BIT_GROUPS
                for n,bits in group for shift in [sum(bits0 for _,bits0 in group[:group.index((n,bits))])]]
        for field,bits,offset,shift in fields:
            with self.subTest(field=field):
                changed=[dict(r) for r in self.records]; changed[0][field]=(1<<bits)-1
                data=job_data.encode_table(changed)
                width=(sum(size for _,size in next(g for off,g in job_data.BIT_GROUPS if off==offset))+7)//8
                before=int.from_bytes(self.raw[offset:offset+width],'little')
                mask=((1<<bits)-1)<<shift
                expected=(before&~mask)|(((1<<bits)-1)<<shift)
                self.assertEqual(data[offset:offset+width],expected.to_bytes(width,'little'))
                self.assertEqual(data[:offset]+data[offset+width:],self.raw[:offset]+self.raw[offset+width:])

    def test_invalid_ranges_and_table_shapes_are_rejected(self):
        for field,value in [('p06',16),('p07',-1),('p14',8),('unknown21',32),('p26',4096),('p34',16),('p00',65536),('p08',256)]:
            with self.subTest(field=field,value=value):
                changed=dict(self.records[0]); changed[field]=value
                with self.assertRaises(ValueError): job_data.encode_record(changed)
        with self.assertRaises(ValueError): job_data.encode_table(self.records[:-1])
        with self.assertRaises(ValueError): job_data.decode_table(self.raw[:-1])
        malformed=dict(self.records[0]); malformed['unknown02']='00  ';
        with self.assertRaises(ValueError): job_data.encode_record(malformed)
        with self.assertRaises(ValueError): job_data.render_asm([malformed,*self.records[1:]])

    def test_assembly_record_count_and_field_limits_are_guarded(self):
        signature=next(x for x in self.source.splitlines() if x.startswith('.macro job_record '))
        params=signature.removeprefix('.macro job_record ').split(', ')
        original=next(x for x in self.source.splitlines() if x.startswith('    job_record 0,'))
        for field,value in [('p06','16'),('p14','8'),('p26','4096'),('p35','16'),('unknown02_0','256')]:
            with self.subTest(field=field):
                values=original.split(', '); values[params.index(field)]=value
                lines=self.source.splitlines(); idx=next(i for i,x in enumerate(lines) if x.startswith('    job_record 0,'))
                lines[idx]=', '.join(values)
                result,_=self.assemble('\n'.join(lines)+'\n')
                self.assertNotEqual(result.returncode,0)
        removed=self.source.replace(next(x for x in self.source.splitlines() if x.startswith('    job_record 115,'))+'\n','',1)
        result,_=self.assemble(removed)
        self.assertNotEqual(result.returncode,0)
