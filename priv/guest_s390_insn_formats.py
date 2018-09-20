#!/usr/bin/env python3
import io
import re

STRUCT_R = re.compile(r'^\s+struct {\n$')
FIELD_R = re.compile(r'^\s+unsigned\s+int\s+([a-z\d]*)\s+:\s+(\d+);\n$')
END_R = re.compile(r'\s+}\s+([a-zA-Z\d_]+);\n$')


def work(ss):
    fields = None
    for s in ss:
        if s == '\n':
            continue
        m = STRUCT_R.match(s)
        if m is not None:
            fields = []
            continue
        m = FIELD_R.match(s)
        if m is not None:
            fname, fsize = m.groups()
            fields.append((fname, int(fsize)))
            continue
        m = END_R.match(s)
        if m is not None:
            sname, = m.groups()
            isize = sum(size for _, size in fields)
            if isize == 16 or isize == 32:
                vsize = isize
            elif isize == 48:
                vsize = 64
            else:
                raise Exception(str(isize))
            foffset = 0
            for fname, fsize in fields:
                if len(fname) > 0:
                    voffset = vsize - foffset - fsize
                    vmask = (1 << fsize) - 1
                    insn = '(insn)'
                    if voffset != 0:
                        insn = '(%s >> %d)' % (insn, voffset)
                    insn = '(%s & 0x%x)' % (insn, vmask)
                    print('#define %s_%s(insn) %s' % (sname, fname, insn))
                foffset += fsize
            fields = None
            continue
        raise Exception(s)


work(io.StringIO('''
      struct {
         unsigned int op : 16;
      } E;
      struct {
         unsigned int op :  8;
         unsigned int i  :  8;
      } I;
      struct {
         unsigned int op :  8;
         unsigned int r1 :  4;
         unsigned int r2 :  4;
      } RR;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int op2 :  4;
         unsigned int i2  : 16;
      } RI;
      struct {
         unsigned int op : 16;
         unsigned int    :  8;
         unsigned int r1 :  4;
         unsigned int r2 :  4;
      } RRE;
      struct {
         unsigned int op : 16;
         unsigned int r1 :  4;
         unsigned int    :  4;
         unsigned int r3 :  4;
         unsigned int r2 :  4;
      } RRF;
      struct {
         unsigned int op : 16;
         unsigned int m3 :  4;
         unsigned int m4 :  4;
         unsigned int r1 :  4;
         unsigned int r2 :  4;
      } RRF2;
      struct {
         unsigned int op : 16;
         unsigned int r3 :  4;
         unsigned int    :  4;
         unsigned int r1 :  4;
         unsigned int r2 :  4;
      } RRF3;
      struct {
         unsigned int op : 16;
         unsigned int r3 :  4;
         unsigned int    :  4;
         unsigned int r1 :  4;
         unsigned int r2 :  4;
      } RRR;
      struct {
         unsigned int op : 16;
         unsigned int r3 :  4;
         unsigned int m4 :  4;
         unsigned int r1 :  4;
         unsigned int r2 :  4;
      } RRF4;
      struct {
         unsigned int op : 16;
         unsigned int    :  4;
         unsigned int m4 :  4;
         unsigned int r1 :  4;
         unsigned int r2 :  4;
      } RRF5;
      struct {
         unsigned int op :  8;
         unsigned int r1 :  4;
         unsigned int r3 :  4;
         unsigned int b2 :  4;
         unsigned int d2 : 12;
      } RS;
      struct {
         unsigned int op :  8;
         unsigned int r1 :  4;
         unsigned int r3 :  4;
         unsigned int i2 : 16;
      } RSI;
      struct {
         unsigned int op :  8;
         unsigned int r1 :  4;
         unsigned int x2 :  4;
         unsigned int b2 :  4;
         unsigned int d2 : 12;
      } RX;
      struct {
         unsigned int op : 16;
         unsigned int b2 :  4;
         unsigned int d2 : 12;
      } S;
      struct {
         unsigned int op :  8;
         unsigned int i2 :  8;
         unsigned int b1 :  4;
         unsigned int d1 : 12;
      } SI;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int r3  :  4;
         unsigned int i2  : 16;
         unsigned int     :  8;
         unsigned int op2 :  8;
      } RIE;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int r2  :  4;
         unsigned int i3  :  8;
         unsigned int i4  :  8;
         unsigned int i5  :  8;
         unsigned int op2 :  8;
      } RIE_RRUUU;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int     :  4;
         unsigned int i2  : 16;
         unsigned int m3  :  4;
         unsigned int     :  4;
         unsigned int op2 :  8;
      } RIEv1;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int r2  :  4;
         unsigned int i4  : 16;
         unsigned int m3  :  4;
         unsigned int     :  4;
         unsigned int op2 :  8;
      } RIE_RRPU;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int m3  :  4;
         unsigned int i4  : 16;
         unsigned int i2  :  8;
         unsigned int op2 :  8;
      } RIEv3;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int op2 :  4;
         unsigned int i2  : 32;
      } RIL;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int m3  :  4;
         unsigned int b4  :  4;
         unsigned int d4  : 12;
         unsigned int i2  :  8;
         unsigned int op2 :  8;
      } RIS;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int r2  :  4;
         unsigned int b4  :  4;
         unsigned int d4  : 12;
         unsigned int m3  :  4;
         unsigned int     :  4;
         unsigned int op2 :  8;
      } RRS;
      struct {
         unsigned int op1 :  8;
         unsigned int l1  :  4;
         unsigned int     :  4;
         unsigned int b1  :  4;
         unsigned int d1  : 12;
         unsigned int     :  8;
         unsigned int op2 :  8;
      } RSL;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int r3  :  4;
         unsigned int b2  :  4;
         unsigned int dl2 : 12;
         unsigned int dh2 :  8;
         unsigned int op2 :  8;
      } RSY;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int x2  :  4;
         unsigned int b2  :  4;
         unsigned int d2  : 12;
         unsigned int m3  :  4;
         unsigned int     :  4;
         unsigned int op2 :  8;
      } RXE;
      struct {
         unsigned int op1 :  8;
         unsigned int r3  :  4;
         unsigned int x2  :  4;
         unsigned int b2  :  4;
         unsigned int d2  : 12;
         unsigned int r1  :  4;
         unsigned int     :  4;
         unsigned int op2 :  8;
      } RXF;
      struct {
         unsigned int op1 :  8;
         unsigned int r1  :  4;
         unsigned int x2  :  4;
         unsigned int b2  :  4;
         unsigned int dl2 : 12;
         unsigned int dh2 :  8;
         unsigned int op2 :  8;
      } RXY;
      struct {
         unsigned int op1 :  8;
         unsigned int i2  :  8;
         unsigned int b1  :  4;
         unsigned int dl1 : 12;
         unsigned int dh1 :  8;
         unsigned int op2 :  8;
      } SIY;
      struct {
         unsigned int op :  8;
         unsigned int l  :  8;
         unsigned int b1 :  4;
         unsigned int d1 : 12;
         unsigned int b2 :  4;
         unsigned int d2 : 12;
      } SS;
      struct {
         unsigned int op :  8;
         unsigned int l1 :  4;
         unsigned int l2 :  4;
         unsigned int b1 :  4;
         unsigned int d1 : 12;
         unsigned int b2 :  4;
         unsigned int d2 : 12;
      } SS_LLRDRD;
      struct {
         unsigned int op :  8;
         unsigned int r1 :  4;
         unsigned int r3 :  4;
         unsigned int b2 :  4;
         unsigned int d2 : 12;
         unsigned int b4 :  4;
         unsigned int d4 : 12;
      } SS_RRRDRD2;
      struct {
         unsigned int op : 16;
         unsigned int b1 :  4;
         unsigned int d1 : 12;
         unsigned int b2 :  4;
         unsigned int d2 : 12;
      } SSE;
      struct {
         unsigned int op1 :  8;
         unsigned int r3  :  4;
         unsigned int op2 :  4;
         unsigned int b1  :  4;
         unsigned int d1  : 12;
         unsigned int b2  :  4;
         unsigned int d2  : 12;
      } SSF;
      struct {
         unsigned int op : 16;
         unsigned int b1 :  4;
         unsigned int d1 : 12;
         unsigned int i2 : 16;
      } SIL;
      struct {
         unsigned int op1 : 8;
         unsigned int v1  : 4;
         unsigned int x2  : 4;
         unsigned int b2  : 4;
         unsigned int d2  : 12;
         unsigned int m3  : 4;
         unsigned int rxb : 4;
         unsigned int op2 : 8;
      } VRX;
      struct {
         unsigned int op1 : 8;
         unsigned int v1  : 4;
         unsigned int v2  : 4;
         unsigned int r3  : 4;
         unsigned int     : 4;
         unsigned int m5  : 4;
         unsigned int     : 4;
         unsigned int m4  : 4;
         unsigned int rxb : 4;
         unsigned int op2 : 8;
      } VRR;
      struct {
         unsigned int op1 : 8;
         unsigned int v1  : 4;
         unsigned int v3  : 4;
         unsigned int i2  : 16;
         unsigned int m3  : 4;
         unsigned int rxb : 4;
         unsigned int op2 : 8;
      } VRI;
      struct {
         unsigned int op1 : 8;
         unsigned int v1  : 4;
         unsigned int v3  : 4;
         unsigned int b2  : 4;
         unsigned int d2  : 12;
         unsigned int m4  : 4;
         unsigned int rxb : 4;
         unsigned int op2 : 8;
      } VRS;
      struct {
         unsigned int op1 : 8;
         unsigned int v1  : 4;
         unsigned int v2  : 4;
         unsigned int b2  : 4;
         unsigned int d2  : 12;
         unsigned int m3  : 4;
         unsigned int rxb : 4;
         unsigned int op2 : 8;
     } VRV;
'''))
