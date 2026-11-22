import struct, json, re
from elftools.elf.elffile import ELFFile
from capstone import *
from capstone.arm import *
import os, sys
FN = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("LIBNICKEL_PATH", "libnickel.so.1.0.0")
OUT_JSON = sys.argv[2] if len(sys.argv) > 2 else "translation_inventory.json"
elf=ELFFile(open(FN,'rb')); data=open(FN,'rb').read()
segs=[(s['p_vaddr'],s['p_vaddr']+s['p_filesz'],s['p_offset']) for s in elf.iter_segments() if s['p_type']=='PT_LOAD']
def v2o(v):
    for a,b,o in segs:
        if a<=v<b: return o+(v-a)
def word(v):
    o=v2o(v); return struct.unpack_from('<I',data,o)[0] if o is not None else None
ro=[(elf.get_section_by_name(n)['sh_addr'], elf.get_section_by_name(n)['sh_addr']+elf.get_section_by_name(n)['sh_size']) for n in ('.rodata','.data.rel.ro') if elf.get_section_by_name(n)]
def readstr(v):
    if v is None or not any(a<=v<b for a,b in ro): return None
    o=v2o(v)
    if o is None: return None
    e=data.find(b'\x00',o)
    if e<0 or e-o>600: return None
    b=data[o:e]
    if not b: return None
    try: s=b.decode('utf-8')
    except: return None
    if any(ord(c)<32 and c!='\t' for c in s): return None
    return s
def classof(name):
    m=re.match(r'_ZN(.+)', name)
    if not m: return None
    rest=m.group(1); parts=[]; i=0
    while i<len(rest) and rest[i].isdigit():
        j=i
        while j<len(rest) and rest[j].isdigit(): j+=1
        n=int(rest[i:j]); comp=rest[j:j+n]
        if len(comp)<n: break
        parts.append(comp); i=j+n
    if len(parts)<2: return None
    cls=parts[-2]
    return cls[3:] if cls.startswith('Ui_') else cls
dynsym=elf.get_section_by_name('.dynsym')
mT=Cs(CS_ARCH_ARM,CS_MODE_THUMB); mT.detail=True
mA=Cs(CS_ARCH_ARM,CS_MODE_ARM); mA.detail=True
rt=next(sy for sy in dynsym.iter_symbols() if 'N3SettingsReadingView' in sy.name and 'retranslateUi' in sy.name and sy['st_value'])
va=rt['st_value']&~1; o=v2o(va); addpc=0; tr_target=None
for ins in mT.disasm(data[o:o+300], va):
    if ins.id==ARM_INS_ADD and any(op.type==ARM_OP_REG and ins.reg_name(op.reg)=='pc' for op in ins.operands): addpc+=1
    if ins.id in (ARM_INS_BLX,ARM_INS_BL) and ins.operands and ins.operands[0].type==ARM_OP_IMM and addpc>=2:
        tr_target=ins.operands[0].imm; break
txt=elf.get_section_by_name('.text'); tlo=txt['sh_addr']; thi=tlo+txt['sh_size']
funcs=[(sy['st_value'],sy['st_size'],sy.name) for sy in dynsym.iter_symbols() if sy['st_info']['type']=='STT_FUNC' and sy['st_value'] and sy['st_size'] and tlo<=(sy['st_value']&~1)<thi]
CALLER_SAVED={'r0','r1','r2','r3','r12','ip','lr'}
pairs=[]; unresolved=0; total=0
for a,sz,nm in funcs:
    vf=a&~1; of=v2o(vf); m=mT if (a&1) else mA
    if of is None: continue
    fcls=classof(nm)
    regs={}   # reg -> ('addr',v) or ('off',w) or ('imm',v)
    for ins in m.disasm(data[of:of+sz], vf):
        I=ins.id; ops=ins.operands
        if I in (ARM_INS_BL,ARM_INS_BLX) and ops and ops[0].type==ARM_OP_IMM and ops[0].imm==tr_target:
            total+=1
            c=regs.get('r1'); s=regs.get('r2')
            ss=readstr(s[1]) if s and s[0]=='addr' else None
            cs=readstr(c[1]) if c and c[0]=='addr' else None
            ctx = cs if cs else fcls
            if ctx and ss and re.search('[A-Za-z]',ss): pairs.append((ctx,ss))
            else: unresolved+=1
            for r in CALLER_SAVED: regs.pop(r,None)
            continue
        # ldr rd,[pc,#imm] -> offset word
        if I==ARM_INS_LDR and len(ops)==2 and ops[1].type==ARM_OP_MEM and ins.reg_name(ops[1].mem.base)=='pc':
            pc=(ins.address+4)&~3; w=word(pc+ops[1].mem.disp)
            regs[ins.reg_name(ops[0].reg)]=('off', w if w is not None else 0)
        # add rd,(rd,)pc -> resolve offset to addr
        elif I==ARM_INS_ADD and ops and ops[-1].type==ARM_OP_REG and ins.reg_name(ops[-1].reg)=='pc':
            rd=ins.reg_name(ops[0].reg)
            if rd in regs and regs[rd][0]=='off':
                off=regs[rd][1]; off-= 0x100000000 if off>=0x80000000 else 0
                regs[rd]=('addr',(ins.address+4+off)&0xffffffff)
            else: regs.pop(rd,None)
        # movw rd,#imm
        elif I==ARM_INS_MOVW and len(ops)==2 and ops[1].type==ARM_OP_IMM:
            regs[ins.reg_name(ops[0].reg)]=('imm',ops[1].imm & 0xffff)
        # movt rd,#imm -> combine
        elif I==ARM_INS_MOVT and len(ops)==2 and ops[1].type==ARM_OP_IMM:
            rd=ins.reg_name(ops[0].reg)
            if rd in regs and regs[rd][0]=='imm':
                regs[rd]=('addr',(regs[rd][1] | (ops[1].imm<<16))&0xffffffff)
            else: regs.pop(rd,None)
        # mov rd,rs  copy
        elif I==ARM_INS_MOV and len(ops)==2 and ops[1].type==ARM_OP_REG:
            rd=ins.reg_name(ops[0].reg); rs=ins.reg_name(ops[1].reg)
            if rs in regs: regs[rd]=regs[rs]
            else: regs.pop(rd,None)
        # mov rd,#imm
        elif I==ARM_INS_MOV and len(ops)==2 and ops[1].type==ARM_OP_IMM:
            regs[ins.reg_name(ops[0].reg)]=('imm',ops[1].imm)
        elif I in (ARM_INS_BL,ARM_INS_BLX):
            for r in CALLER_SAVED: regs.pop(r,None)   # only caller-saved clobbered
        elif I in (ARM_INS_B,ARM_INS_BX):
            regs={}
        else:
            # writeback to a dest reg we can't model -> invalidate it
            if ops and ops[0].type==ARM_OP_REG and (ins.regs_access()[1] if hasattr(ins,'regs_access') else True):
                rd=ins.reg_name(ops[0].reg)
                # only invalidate if instruction writes rd (heuristic: first op is dest for data-proc)
                if I in (ARM_INS_ADD,ARM_INS_SUB,ARM_INS_ORR,ARM_INS_AND,ARM_INS_EOR,ARM_INS_LSL,ARM_INS_LSR,ARM_INS_LDR,ARM_INS_LDRB,ARM_INS_LDRH):
                    regs.pop(rd,None)
seen=set(); outp=[]
for c,s in pairs:
    if (c,s) in seen: continue
    seen.add((c,s)); outp.append({"context":c,"source":s})
print(f"call sites seen={total} resolved-pairs(unique)={len(outp)} unresolved={unresolved}")
plur=[p for p in outp if '%n' in p['source']]
print("PLURALS (%n):",len(plur))
for p in plur: print("   ",p['context'],'::',repr(p['source']))
json.dump(outp, open(OUT_JSON,"w"), ensure_ascii=False)
