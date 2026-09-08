/*--------------------------------------------------------------------*/
/*--- begin                              guest_amd64_toIR_AVX512.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Valgrind, a dynamic binary instrumentation
   framework.

   Copyright (C) 2021 Intel Corporation
      tatyana.a.volnina@intel.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

/* Translates AVX-512 AMD64 code to IR. */
#ifdef AVX_512

#include "guest_amd64_toIR_AVX512.h"

/* Largest lane count of any EVEX operand: 64 x 8-bit lanes in a ZMM. */
#define MAX_ELEMENTS 64

/* guest_amd64_toIR.c #undefs DIP/DIS before #including this file; redefine
   them in the standard-variadic form that both GCC and MSVC accept. */
#ifndef _MSC_VER
#define DIP(format, ...)               \
   if (vex_traceflags & VEX_TRACE_FE)  \
      vex_printf(format, ## __VA_ARGS__)

#define DIS(buf, format, ...)          \
   if (vex_traceflags & VEX_TRACE_FE)  \
      vex_sprintf(buf, format, ## __VA_ARGS__)
#else
#define DIP(format, ...)               \
   if (vex_traceflags & VEX_TRACE_FE)  \
      vex_printf(format, __VA_ARGS__)

#define DIS(buf, format, ...)          \
   if (vex_traceflags & VEX_TRACE_FE)  \
      vex_sprintf(buf, format, __VA_ARGS__)
#endif

typedef UInt Prefix_EVEX;
// Have to implement it as a global variable: some VEX functions have to refer
// to EVEX prefix, and changing their signatures requires too many changes
Prefix_EVEX evex = 0;

#define EVEX_R1     (1<<0)    /* R1 bit */
#define EVEX_VEXnV4 (1<<1)    /* EVEX vvvv[4] bit */
#define EVEX_EVEXb  (1<<2)    /* EVEX b bit */
#define EVEX_WIG    (1<<3)    /* Ignore element width */
#define EVEX_MASKZ  (1<<4)    /* 0 for merging, 1 for zeroing */
#define EVEX_MASKR_OFFSET  (5)  /* 3 bits, mask register number */
#define EVEX_DSTW_OFFSET   (8)  /* 3 bits, see enum operand_width */
#define EVEX_EVEXL_OFFSET  (11) /* 2 bits, source vector length. 0 for 128, ..., 2 for 512 */
#define EVEX_EVEXTT_OFFSET (13) /* 4 bits, EVEX tuple type */
#define EVEX_VL_OVERRIDE   (1<<17)
#define PFX_NA (0)

// NOTE: keep in sync with dstWidth from host_generic_AVX512_F.c
enum operand_width {
   W_8  = 0,
   W_16 = 1,
   W_32 = 2,
   W_64 = 3
};
enum encoded_w { W0=0, W1, WIG };

// For now, only used to tell if we can read masked memory elements
// (we can if there is no fault suppression)
enum exception_class {
   EXC_UNDEF,
   E1,         // Explicitly aligned vector moves/load/ltores, with fault suppression
   E1NF,       // Explicitly aligned vector non-temporal stores, no fault suppression
   E2,         // FP Vector load+op, fault suppression
   E3, E3NF,   // FP Scalar/Partial vector load+ops
   E4, E4NF,   // Integer vector load+ops
   E4nb, E4NFnb,  // Integer vector load+ops, embedded broadcast not supported
   E5, E5NF,   // Legacy-like promotions
   E6, E6NF,   // Post-AVX promotions
   E6BC,       // Post-AVX promotions; broadcast implemented in VG as "no fault suppression" for the simplicity
   E7NM,       // Register-to-register op
   E7NM128,    // Register-to-register op, M128 tuple type
   E9NF,       // Miscellaneous 128-bit, no fault suppression
   E10, E10NF, // Non-XF Scalars, Vector Length ignored
   E11,        // VCVTPH2PS, VCVTPS2PH, fault suppression
   E12,        // Gather and Scatter family, fault suppression
   E12NP,      // Gather and Scatter Prefetch family, no page faults
};

static void setZeroMode( Bool mode_z ) {
   if (mode_z) evex |= EVEX_MASKZ;
   else evex &= ~EVEX_MASKZ;
}
static Bool getZeroMode( void ) {
   return toBool(evex & EVEX_MASKZ);
}
static void setEvexMask (UInt mask) {
   evex &= ~(0x7 << EVEX_MASKR_OFFSET);
   evex |= (mask << EVEX_MASKR_OFFSET);
}
static UInt getEvexMask( void ) {
   return (evex >> EVEX_MASKR_OFFSET) & 0x7;
}
UInt getEVexNvvvv ( Prefix pfx ) {
   UInt r = (UInt)pfx;
   r /= (UInt)PFX_VEXnV0; /* pray this turns into a shift */
   r &= 0xF;
   if (evex & EVEX_VEXnV4) r += 16;
   return r;
}
static void setWIG( void ) {
   evex |= EVEX_WIG;
}
static Int getWIG( void ) {
   return ((evex & EVEX_WIG) ? 1 : 0);
}

static void setDstW ( enum operand_width width ) {
   evex &= ~(0x7 << EVEX_DSTW_OFFSET);
   evex |= ((UInt)width << EVEX_DSTW_OFFSET);
}
static Int getDstW( void ) {
   return ((evex>>EVEX_DSTW_OFFSET) & 0x7);
}

static void setEVexL ( UInt vl ) {
   evex &= ~(0x3 << EVEX_EVEXL_OFFSET);
   evex |= ((UInt)vl << EVEX_EVEXL_OFFSET);
}

// PLEASE READ
// If instruction is FP register-register, the broadcast bit (getEvexb())
// overrides instruction length to 512 or serial, and vl means rmode
// No way to tell it here so handle it after this function manually
static Int getEvexb( void ) {
   return (evex & EVEX_EVEXb) ? 1 : 0;
}
static Int getEVexL ( void ) {
   Int vl = (evex >> EVEX_EVEXL_OFFSET) & 0x3;
   return vl;
}

UInt gregOfRexRM32 ( Prefix pfx, UChar mod_reg_rm ) {
   UInt reg = (Int)((mod_reg_rm >> 3) & 7);
   reg += (pfx & PFX_REXR) ? 8 : 0;
   reg += (evex & EVEX_R1) ? 16 : 0;
   return reg;
}
UInt eregOfRexRM32 ( Prefix pfx, UChar mod_reg_rm )  {
   vassert(epartIsReg(mod_reg_rm));
   UInt reg = (Int)((mod_reg_rm) & 7);
   reg += (pfx & PFX_REXB) ? 8 : 0;
   reg += (pfx & PFX_REXX) ? 16 : 0;
   return reg;
}
UChar get_reg_16_32(UChar index_r) {
   return (evex & EVEX_VEXnV4) ? (index_r+16) : (index_r);
}

void setTupleType ( enum TupleType type ) {
   evex &= ~(0xF << EVEX_EVEXTT_OFFSET);
   evex |= (((UInt)type & 0xF) << EVEX_EVEXTT_OFFSET);
}
static Int getTupleType ( void ) {
   return ((evex >> EVEX_EVEXTT_OFFSET) & 0xF);
}

static Int ipow(Int x, Int n) {
   UInt res = 1;
   for (Int i = 0; i < n; ++i)
      res *= x;
   return res;
}

Int getEVEXMult( Prefix pfx ) {
   if (evex == 0)
      return 1;

   Int type = getTupleType();
   if (getEvexb()) { // use embedded broadcast
      DIP("use embedded broadcast\n");
      switch (type) {
         case FullVector: return (getRexW(pfx)) ? 8 : 4;
         case HalfVector: return 4;
         default: vpanic("unsupported tuple embedded broadcast");
      }
   } else {
      switch (type) {
         case NoTupleType:   return 1;
         case FullVector:
         case FullVectorMem: return ipow(2, getEVexL()+4);
         case HalfVector:
         case HalfMem:       return ipow(2, getEVexL()+3);
         case QuarterMem:    return ipow(2, getEVexL()+2);
         case OctMem:        return ipow(2, getEVexL()+1);
         case Tuple1Fixed:   return (getDstW() == W_64) ? 8 : 4;
         case Tuple2:        return (getDstW() == W_64) ? 16 : 8;
         case Tuple4:        return (getDstW() == W_64) ? 32 : 16;
         case Tuple8:        return 32;
         case Mem128:        return 16;
         // Tuple1Scalar relies on src width, but src does not have 8- and 16-bit cases
         case Tuple1Scalar:  return getWIG() ? ipow(2, getDstW()) : ipow(2, getRexW(pfx)+2);
         case MOVDDUP: {
            switch (getEVexL()) {
               case 0: return 8;
               case 1: return 32;
               case 2: return 64;
               default: vpanic("invalid vector length");
            }
            break;
         }
         default: vpanic("invalid tuple type");
      }
   }
}

const HChar* nameXMMRegEVEX (Int xmmreg) {
   static const HChar* xmm_names[16]
      = { "%xmm16", "%xmm17", "%xmm18", "%xmm19",
         "%xmm20", "%xmm21", "%xmm22", "%xmm23",
         "%xmm24", "%xmm25", "%xmm26", "%xmm27",
         "%xmm28", "%xmm29", "%xmm30", "%xmm31" };
   if (xmmreg < 16 || xmmreg > 31) {
      vpanic("nameXMMReg(avx512)");
   }
   return xmm_names[xmmreg-16];
}

const HChar* nameYMMRegEVEX (Int ymmreg) {
   static const HChar* ymm_names[16]
      = { "%ymm16", "%ymm17", "%ymm18", "%ymm19",
         "%ymm20", "%ymm21", "%ymm22", "%ymm23",
         "%ymm24", "%ymm25", "%ymm26", "%ymm27",
         "%ymm28", "%ymm29", "%ymm30", "%ymm31" };
   if (ymmreg < 16 || ymmreg > 31) {
      vpanic("nameYMMReg(avx512)");
   }
   return ymm_names[ymmreg-16];
}

const HChar* nameZMMReg ( Int zmmreg ) {
   static const HChar* zmm_names[32]
      = { "%zmm0",  "%zmm1",  "%zmm2",  "%zmm3",
         "%zmm4",  "%zmm5",  "%zmm6",  "%zmm7",
         "%zmm8",  "%zmm9",  "%zmm10", "%zmm11",
         "%zmm12", "%zmm13", "%zmm14", "%zmm15",
         "%zmm16", "%zmm17", "%zmm18", "%zmm19",
         "%zmm20", "%zmm21", "%zmm22", "%zmm23",
         "%zmm24", "%zmm25", "%zmm26", "%zmm27",
         "%zmm28", "%zmm29", "%zmm30", "%zmm31", };
   if (zmmreg < 0 || zmmreg > 31) vpanic("namezmmReg(amd64)");
   return zmm_names[zmmreg];
}

static const HChar* nameKReg ( Int kreg ) {
   static const HChar* k_names[8]
      = { "%k0", "%k1", "%k2", "%k3",
         "%k4", "%k5", "%k6", "%k7", };
   if (kreg < 0 || kreg > 7) {
      vpanic("namekReg(amd64)");
   }
   return k_names[kreg];
}

Int ymmGuestRegOffsetEVEX ( Int ymmreg )
{
   switch (ymmreg) {
      case 16: return OFFB_YMM16;
      case 17: return OFFB_YMM17;
      case 18: return OFFB_YMM18;
      case 19: return OFFB_YMM19;
      case 20: return OFFB_YMM20;
      case 21: return OFFB_YMM21;
      case 22: return OFFB_YMM22;
      case 23: return OFFB_YMM23;
      case 24: return OFFB_YMM24;
      case 25: return OFFB_YMM25;
      case 26: return OFFB_YMM26;
      case 27: return OFFB_YMM27;
      case 28: return OFFB_YMM28;
      case 29: return OFFB_YMM29;
      case 30: return OFFB_YMM30;
      case 31: return OFFB_YMM31;
      default: {
         vpanic("ymmGuestRegOffset(avx512)");
      }
   }
}

static Int zmmGuestRegOffset ( UInt zmmreg ) {
   switch (zmmreg) {
      case 0: return OFFB_ZMM0;
      case 1: return OFFB_ZMM1;
      case 2: return OFFB_ZMM2;
      case 3: return OFFB_ZMM3;
      case 4: return OFFB_ZMM4;
      case 5: return OFFB_ZMM5;
      case 6: return OFFB_ZMM6;
      case 7: return OFFB_ZMM7;
      case 8: return OFFB_ZMM8;
      case 9: return OFFB_ZMM9;
      case 10: return OFFB_ZMM10;
      case 11: return OFFB_ZMM11;
      case 12: return OFFB_ZMM12;
      case 13: return OFFB_ZMM13;
      case 14: return OFFB_ZMM14;
      case 15: return OFFB_ZMM15;
      case 16: return OFFB_ZMM16;
      case 17: return OFFB_ZMM17;
      case 18: return OFFB_ZMM18;
      case 19: return OFFB_ZMM19;
      case 20: return OFFB_ZMM20;
      case 21: return OFFB_ZMM21;
      case 22: return OFFB_ZMM22;
      case 23: return OFFB_ZMM23;
      case 24: return OFFB_ZMM24;
      case 25: return OFFB_ZMM25;
      case 26: return OFFB_ZMM26;
      case 27: return OFFB_ZMM27;
      case 28: return OFFB_ZMM28;
      case 29: return OFFB_ZMM29;
      case 30: return OFFB_ZMM30;
      case 31: return OFFB_ZMM31;
      default: vex_printf("%x\n", zmmreg);
               vpanic("zmmGuestRegOffset(amd64)");
   }
}
static Int kGuestRegOffset ( UInt kreg ) {
   switch (kreg) {
      case 0: return OFFB_K0;
      case 1: return OFFB_K1;
      case 2: return OFFB_K2;
      case 3: return OFFB_K3;
      case 4: return OFFB_K4;
      case 5: return OFFB_K5;
      case 6: return OFFB_K6;
      case 7: return OFFB_K7;
      default: vpanic("kGuestRegOffset(amd64)");
   }
}

static IRExpr* qop ( IROp op, IRExpr* a1, IRExpr* a2, IRExpr* a3, IRExpr* a4) {
   return IRExpr_Qop(op, a1, a2, a3, a4);
}


static Int zmmGuestRegLane32offset ( UInt zmmreg, Int laneno ) {
   /* Correct for little-endian host only. */
   vassert(host_endness == VexEndnessLE);
   vassert(laneno >= 0 && laneno < 16);
   return zmmGuestRegOffset( zmmreg ) + 4 * laneno;
}


static Int zmmGuestRegLane64offset ( UInt zmmreg, Int laneno ) {
   /* Correct for little-endian host only. */
   vassert(host_endness == VexEndnessLE);
   vassert(laneno >= 0 && laneno < 8);
   return zmmGuestRegOffset( zmmreg ) + 8 * laneno;
}



static Int zmmGuestRegLane128offset ( UInt zmmreg, Int laneno ) {
   /* Correct for little-endian host only. */
   vassert(host_endness == VexEndnessLE);
   vassert(laneno >= 0 && laneno < 4);
   return zmmGuestRegOffset( zmmreg ) + 16 * laneno;
}

static Int zmmGuestRegLane256offset ( UInt zmmreg, Int laneno ) {
   /* Correct for little-endian host only. */
   vassert(host_endness == VexEndnessLE);
   vassert(laneno >= 0 && laneno < 2);
   return zmmGuestRegOffset( zmmreg ) + 32 * laneno;
}

static Int kGuestRegLane32Offset( UInt kreg, Int laneno ) {
   /* Correct for little-endian host only. */
   vassert(host_endness == VexEndnessLE);
   vassert(laneno >= 0 && laneno < 2);
   return kGuestRegOffset( kreg ) + 4 * laneno;
}

static Int kGuestRegLane16Offset( UInt kreg, Int laneno ) {
   /* Correct for little-endian host only. */
   vassert(host_endness == VexEndnessLE);
   vassert(laneno >= 0 && laneno < 4);
   return kGuestRegOffset( kreg ) + 2 * laneno;
}

static Int kGuestRegLane8Offset( UInt kreg, Int laneno ) {
   /* Correct for little-endian host only. */
   vassert(host_endness == VexEndnessLE);
   vassert(laneno >= 0 && laneno < 8);
   return kGuestRegOffset( kreg ) + laneno;
}

IRExpr* mkV256 ( UInt val ) {
   return IRExpr_Const(IRConst_V256(val));
}

static IRExpr* get_evex_roundingmode (void) {
   if (getEvexb())
      return mkU32(getEVexL() + 0x08);
   else
      return get_sse_roundingmode();
}

IRExpr* getZMMReg ( UInt zmmreg ) {
   return IRExpr_Get( zmmGuestRegOffset(zmmreg), Ity_V512 );
}

IRExpr* getZMMRegLane256 ( UInt zmmreg, Int laneno ) {
   return IRExpr_Get( zmmGuestRegLane256offset(zmmreg,laneno), Ity_V256 );
}

IRExpr* getZMMRegLane128 ( UInt zmmreg, Int laneno ) {
   return IRExpr_Get( zmmGuestRegLane128offset(zmmreg,laneno), Ity_V128 );
}

IRExpr* getZMMRegLane64 ( UInt zmmreg, Int laneno ) {
   return IRExpr_Get( zmmGuestRegLane64offset(zmmreg,laneno), Ity_I64 );
}

IRExpr* getZMMRegLane64F ( UInt zmmreg, Int laneno ) {
   return IRExpr_Get( zmmGuestRegLane64offset(zmmreg,laneno), Ity_F64 );
}

IRExpr* getZMMRegLane32 ( UInt zmmreg, Int laneno ) {
   return IRExpr_Get( zmmGuestRegLane32offset(zmmreg,laneno), Ity_I32 );
}

IRExpr* getZMMRegLane32F ( UInt zmmreg, Int laneno ) {
   return IRExpr_Get( zmmGuestRegLane32offset(zmmreg,laneno), Ity_F32 );
}

void putZMMReg ( UInt zmmreg, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_V512);
   stmt( IRStmt_Put( zmmGuestRegOffset(zmmreg), e ) );
}

void putZMMRegLane256 ( UInt zmmreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_V256);
   stmt( IRStmt_Put( zmmGuestRegLane256offset(zmmreg,laneno), e ) );
}

void putZMMRegLane128 ( UInt zmmreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_V128);
   stmt( IRStmt_Put( zmmGuestRegLane128offset(zmmreg,laneno), e ) );
}

void putZMMRegLane64 ( UInt zmmreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_I64);
   stmt( IRStmt_Put( zmmGuestRegLane64offset(zmmreg,laneno), e ) );
}

void putZMMRegLane64F ( UInt zmmreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_F64);
   stmt( IRStmt_Put( zmmGuestRegLane64offset(zmmreg,laneno), e ) );
}

void putZMMRegLane32 ( UInt zmmreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_I32);
   stmt( IRStmt_Put( zmmGuestRegLane32offset(zmmreg,laneno), e ) );
}

void putZMMRegLane32F ( UInt zmmreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_F32);
   stmt( IRStmt_Put( zmmGuestRegLane32offset(zmmreg,laneno), e ) );
}

IRExpr* getKReg ( UInt kreg ) {
   return IRExpr_Get( kGuestRegOffset(kreg), Ity_I64 );
}

static IRExpr* getKRegLane32 ( UInt kreg, Int laneno ) {
   return IRExpr_Get( kGuestRegLane32Offset(kreg, laneno), Ity_I32 );
}

IRExpr* getKRegLane16 ( UInt kreg, Int laneno ) {
   return IRExpr_Get( kGuestRegLane16Offset(kreg, laneno), Ity_I16 );
}

IRExpr* getKRegLane8 ( UInt kreg, Int laneno ) {
   return IRExpr_Get( kGuestRegLane8Offset(kreg, laneno), Ity_I8 );
}

void putKReg ( UInt kreg, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_I64);
   stmt( IRStmt_Put( kGuestRegOffset(kreg), e ) );
}

void putKRegLane32 ( UInt kreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_I32);
   stmt( IRStmt_Put( kGuestRegLane32Offset(kreg, laneno), e ) );
}

void putKRegLane16 ( UInt kreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_I16);
   stmt( IRStmt_Put( kGuestRegLane16Offset(kreg, laneno), e ) );
}

void putKRegLane8 ( UInt kreg, Int laneno, IRExpr* e ) {
   vassert(typeOfIRExpr(irsb->tyenv,e) == Ity_I8);
   stmt( IRStmt_Put( kGuestRegLane8Offset(kreg, laneno), e ) );
}

IRExpr* mkV512 ( UInt val ) {
   return IRExpr_Const(IRConst_V512(val));
}


void breakupV512toV256s ( IRTemp t512,
      /*OUTs*/ IRTemp* t1, IRTemp* t0 )
{
   vassert(t0 && *t0 == IRTemp_INVALID);
   vassert(t1 && *t1 == IRTemp_INVALID);
   *t0 = newTemp(Ity_V256);
   *t1 = newTemp(Ity_V256);
   assign(*t1, unop(Iop_V512toV256_1, mkexpr(t512)));
   assign(*t0, unop(Iop_V512toV256_0, mkexpr(t512)));
}
static void breakupV512toV128s ( IRTemp t512,
      /*OUTs*/ IRTemp* t3, IRTemp* t2, IRTemp* t1, IRTemp* t0 )
{
   IRTemp t256_1 = IRTemp_INVALID;
   IRTemp t256_0 = IRTemp_INVALID;
   breakupV512toV256s( t512, &t256_1, &t256_0 );
   breakupV256toV128s( t256_1, t3, t2 );
   breakupV256toV128s( t256_0, t1, t0 );
}

void breakupV512to64s ( IRTemp t512,
      /*OUTs*/
      IRTemp* t7, IRTemp* t6,
      IRTemp* t5, IRTemp* t4,
      IRTemp* t3, IRTemp* t2,
      IRTemp* t1, IRTemp* t0 )
{
   IRTemp t256_1 = IRTemp_INVALID;
   IRTemp t256_0 = IRTemp_INVALID;
   breakupV512toV256s( t512, &t256_1, &t256_0 );
   breakupV256to64s( t256_1, t7, t6, t5, t4 );
   breakupV256to64s( t256_0, t3, t2, t1, t0 );
}


// helpers start
static void Merge_dst( IRTemp dst, IRTemp* s, Int multiplier);
static void Split_arg (IRTemp src, IRTemp* s, Int multiplier);


static IRTemp m_Copy ( IRTemp src1, IRTemp unused, UChar unused_i ) {
   return src1;
}

// Right rotation: res = (src >> rotate) | (src << (width-rotate))
// Left rotation: res = (src << rotate) | (src >> (width-rotate))
static IRTemp m_RotateRight32(IRTemp src, IRTemp rotate, UChar unused) {
   IRExpr* rotate_m32;
   if (typeOfIRTemp(irsb->tyenv, rotate) == Ity_I8) { // imm version
      rotate_m32 = binop(Iop_And8, mkexpr(rotate), mkU8(0x1F));
   } else { // vector version
      rotate_m32 = binop(Iop_And8, unop(Iop_32to8, mkexpr(rotate)), mkU8(0x1F));
   }
   IRTemp res = newTemp(Ity_I32);
   assign( res, binop(Iop_Or32,
            binop(Iop_Shr32, mkexpr(src), rotate_m32),
            binop(Iop_Shl32, mkexpr(src), binop(Iop_Sub8, mkU8(0x20), rotate_m32))) );
   return res;
}
static IRTemp m_RotateLeft32(IRTemp src, IRTemp rotate, UChar unused) {
   IRExpr* rotate_m32;
   if (typeOfIRTemp(irsb->tyenv, rotate) == Ity_I8) {
      rotate_m32 = binop(Iop_And8, mkexpr(rotate), mkU8(0x1F));
   } else {
      rotate_m32 = binop(Iop_And8, unop(Iop_32to8, mkexpr(rotate)), mkU8(0x1F));
   }
   IRTemp res = newTemp(Ity_I32);
   assign( res, binop(Iop_Or32,
            binop(Iop_Shl32, mkexpr(src), rotate_m32),
            binop(Iop_Shr32, mkexpr(src), binop(Iop_Sub8, mkU8(0x20), rotate_m32))) );
   return res;
}
static IRTemp m_RotateRight64(IRTemp src, IRTemp rotate, UChar unused) {
   IRExpr* rotate_m64;
   if (typeOfIRTemp(irsb->tyenv, rotate) == Ity_I8) {
      rotate_m64 = binop(Iop_And8, mkexpr(rotate), mkU8(0x3F));
   } else {
      rotate_m64 = binop(Iop_And8, unop(Iop_64to8, mkexpr(rotate)), mkU8(0x3F));
   }
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop(Iop_Or64,
            binop(Iop_Shr64, mkexpr(src), rotate_m64),
            binop(Iop_Shl64, mkexpr(src), binop(Iop_Sub8, mkU8(0x40), rotate_m64))) );
   return res;
}
static IRTemp m_RotateLeft64(IRTemp src, IRTemp rotate, UChar unused) {
   IRExpr* rotate_m64;
   if (typeOfIRTemp(irsb->tyenv, rotate) == Ity_I8) {
      rotate_m64 = binop(Iop_And8, mkexpr(rotate), mkU8(0x3F));
   } else {
      rotate_m64 = binop(Iop_And8, unop(Iop_64to8, mkexpr(rotate)), mkU8(0x3F));
   }
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop(Iop_Or64,
            binop(Iop_Shl64, mkexpr(src), rotate_m64),
            binop(Iop_Shr64, mkexpr(src), binop(Iop_Sub8, mkU8(0x40), rotate_m64))) );
   return res;
}

static IRTemp m_PMULDQ (IRTemp src1, IRTemp src2, UChar unused) {
   return math_PMULDQ_128(src1, src2);
}
static IRTemp m_PMULUDQ (IRTemp src1, IRTemp src2, UChar unused) {
   return math_PMULUDQ_128(src1, src2);
}
static IRTemp m_VPERMILPS ( IRTemp sV, IRTemp unused, UChar imm8 ) {
   return math_VPERMILPS_128(sV, imm8);
}
static IRTemp m_PERMILPS_VAR ( IRTemp dataV, IRTemp ctrlV, UChar unused) {
   return math_PERMILPS_VAR_128(dataV, ctrlV);
}
static IRTemp m_PERMILPD_VAR ( IRTemp dataV, IRTemp ctrlV, UChar unused) {
   return math_PERMILPD_VAR_128(dataV, ctrlV);
}
static IRTemp m_VPERMILPD_128 ( IRTemp sV, IRTemp unused, UChar imm8 ) {
   IRTemp res = newTemp(Ity_V128);
   IRTemp s0 = IRTemp_INVALID, s1 = IRTemp_INVALID;
   breakupV128to64s( sV, &s1, &s0 );
   assign(res, binop(Iop_64HLtoV128,
            mkexpr((imm8 & (1<<1)) ? s1 : s0),
            mkexpr((imm8 & (1<<0)) ? s1 : s0)));
   return res;
}
static IRTemp m_VPERMILPD_256 ( IRTemp sV, IRTemp unused, UChar imm8 ) {
   IRTemp res = newTemp(Ity_V256);
   IRTemp s_val[4], s_res[4];
   for (Int j = 0; j < 4; j++) {
      s_val[j] = IRTemp_INVALID;
      s_res[j] = newTemp(Ity_I64);
   }
   Split_arg(sV, s_val, 4);
   for (Int j = 0; j < 4; j++) {
      assign(s_res[j], (imm8 & (1<<j)) ?
            mkexpr(s_val[1+2*(Int)(j/2)]) :
            mkexpr(s_val[2*(Int)(j/2)]));
   }
   Merge_dst(res, s_res, 4);
   return res;
}
static IRTemp m_VPERMILPD_512 ( IRTemp sV, IRTemp unused, UChar imm8 ) {
   IRTemp res = newTemp(Ity_V512);
   IRTemp s_val[8], s_res[8];
   for (Int j = 0; j < 8; j++) {
      s_val[j] = IRTemp_INVALID;
      s_res[j] = newTemp(Ity_I64);
   }
   Split_arg(sV, s_val, 8);
   for (Int j = 0; j < 8; j++) {
      assign(s_res[j], (imm8 & (1<<j)) ?
            mkexpr(s_val[1+2*(Int)(j/2)]) :
            mkexpr(s_val[2*(Int)(j/2)]));
   }
   Merge_dst(res, s_res, 8);
   return res;
}

static IRTemp m_NAND (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign( res, binop(Iop_AndV128, unop(Iop_NotV128, mkexpr(src1)), mkexpr(src2)) );
   return res;
}

static IRTemp m_EXTRACT_256(IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_V256);
   assign(res, unop( (imm8 & 1) ? Iop_V512toV256_1 : Iop_V512toV256_0, mkexpr(src)));
   return res;
}
static IRTemp m_EXTRACT_128(IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_V128);
   if (typeOfIRTemp(irsb->tyenv, src) == Ity_V256) // 256-bit src
      assign(res, unop( (imm8 & 1) ? Iop_V256toV128_1 : Iop_V256toV128_0, mkexpr(src)));
   else { // 512-bit src
      IRTemp s[4] = {IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID};
      Split_arg(src, s, 4);
      assign(res, mkexpr(s[imm8 & 3]));
   }
   return res;
}

static IRTemp m_EXTRACT_64(IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_I64);
   assign(res, unop( (imm8 & 1) ? Iop_V128HIto64 : Iop_V128to64, mkexpr(src)));
   return res;
}
static IRTemp m_EXTRACT_32(IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_I32);
   IRTemp s[4] = {IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID};
   Split_arg(src, s, 4);
   assign(res, mkexpr(s[imm8 & 3]));
   return res;
}

static IRTemp m_INSERT_256(IRTemp src1, IRTemp src2, UChar imm8) {
   IRTemp s[2] = {IRTemp_INVALID, IRTemp_INVALID};
   breakupV512toV256s(src1, &s[1], &s[0]);
   s[imm8 & 1] = src2;
   IRTemp res = newTemp(Ity_V512);
   assign( res, binop(Iop_V256HLtoV512, mkexpr(s[1]), mkexpr(s[0])) );
   return res;
}
static IRTemp m_INSERT_128(IRTemp src1, IRTemp src2, UChar imm8) {
   IRTemp res = IRTemp_INVALID;
   IRTemp s[4] = {IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID};
   if (typeOfIRTemp(irsb->tyenv, src1) == Ity_V256) { // 256-bit dst
      breakupV256toV128s(src1, &s[1], &s[0]);
      s[imm8 & 1] = src2;
      res = newTemp(Ity_V256);
      assign( res, binop(Iop_V128HLtoV256, mkexpr(s[1]), mkexpr(s[0])) );
   } else { // 512-bit dst
      breakupV512toV128s( src1, &s[3], &s[2], &s[1], &s[0] );
      s[imm8 & 3] = src2;
      res = newTemp(Ity_V512);
      assign( res, binop(Iop_V256HLtoV512,
               binop(Iop_V128HLtoV256, mkexpr(s[3]), mkexpr(s[2])),
               binop(Iop_V128HLtoV256, mkexpr(s[1]), mkexpr(s[0]))) );
   }
   return res;
}

static IRTemp m_Broadcast ( IRTemp src, IRTemp unused, UChar ntimes ) {
   setTupleType( FullVectorMem );

   IRTemp s[MAX_ELEMENTS];
   vassert(ntimes <= MAX_ELEMENTS);
   for (Int i = 0; i < ntimes; i++) {
      s[i] = src;
   }
   IRTemp res = IRTemp_INVALID;
   switch ( sizeofIRType(typeOfIRTemp(irsb->tyenv, src)) * ntimes ) { // dst size in bytes
      case 64: res = newTemp(Ity_V512); break;
      case 32: res = newTemp(Ity_V256); break;
      case 16: res = newTemp(Ity_V128); break;
      case 8:  res = newTemp(Ity_I64);  break;
      default: vpanic("unsupported broadcast dst size");
   }
   Merge_dst(res, s, ntimes);
   return res;
}
static IRTemp m_BROADCASTMB2Q (IRTemp src, IRTemp unused, UChar ntimes ) {
   IRTemp mask_low_8_to_64 = newTemp(Ity_I64);
   assign(mask_low_8_to_64, unop(Iop_8Uto64, unop(Iop_64to8, mkexpr(src))) );
   return m_Broadcast(mask_low_8_to_64, unused, ntimes);
}
static IRTemp m_BROADCASTMW2D (IRTemp src, IRTemp unused, UChar ntimes ) {
   IRTemp mask_low_16_to_32 = newTemp(Ity_I32);
   assign(mask_low_16_to_32, unop(Iop_16Uto32, unop(Iop_64to16, mkexpr(src))) );
   return m_Broadcast(mask_low_16_to_32, unused, ntimes);
}

static IRTemp m_SQRT64 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, unop(Iop_Sqrt64F0x2,
         binop(Iop_SetV128lo64,
            mkexpr(src1),
            unop(Iop_V128to64, mkexpr(src2)))));
   return res;
}
static IRTemp m_SQRT32 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, unop(Iop_Sqrt32F0x4,
         binop(Iop_SetV128lo32,
            mkexpr(src1),
            unop(Iop_V128to32, mkexpr(src2)))));
   return res;
}


/* Conversions */

static IRTemp m_I32S_to_F64(IRTemp src, IRTemp unused_ir, UChar unused) {
   IRTemp res = newTemp(Ity_I64);
   assign(res, unop(Iop_ReinterpF64asI64, unop(Iop_I32StoF64, mkexpr(src))));
   return res;
}
static IRTemp m_I32U_to_F32(IRTemp src, IRTemp unused_ir, UChar unused) {
   IRTemp res = newTemp(Ity_I32);
   assign(res, binop(Iop_I32UtoF32_SKX,
               get_FAKE_roundingmode(),
               mkexpr(src)) );
   return res;
}
static IRTemp m_I32U_to_F64 (IRTemp src, IRTemp unused_ir, UChar unused) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop(Iop_I64UtoF64_SKX,
               get_FAKE_roundingmode(),
               unop(Iop_32Uto64, mkexpr(src))) );
   return res;
}
static IRTemp m_I64S_to_F32 (IRTemp src, IRTemp unused_ir, UChar unused) {
   IRTemp res = newTemp(Ity_I32);
   assign( res, unop(Iop_ReinterpF32asI32, binop(Iop_F64toF32,
               get_FAKE_roundingmode(),
               binop(Iop_I64StoF64,
                  get_FAKE_roundingmode(),
                  mkexpr(src)))) );
   return res;
}
static IRTemp m_I64S_to_F64 (IRTemp src, IRTemp unused_ir, UChar unused) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, unop(Iop_ReinterpF64asI64, binop(Iop_I64StoF64,
               get_FAKE_roundingmode(),
               mkexpr(src))) );
   return res;
}
static IRTemp m_I64U_to_F32 (IRTemp src, IRTemp unused_ir, UChar unused) {
   IRTemp res = newTemp(Ity_I32);
   assign( res, unop(Iop_ReinterpF32asI32, binop( Iop_F64toF32,
               get_FAKE_roundingmode(),
               unop(Iop_ReinterpI64asF64, binop(Iop_I64UtoF64_SKX,
                     get_FAKE_roundingmode(),
                     mkexpr(src))))) );
   return res;
}
static IRTemp m_I64U_to_F64 (IRTemp src, IRTemp unused_ir, UChar unused) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop( Iop_I64UtoF64_SKX,
               get_FAKE_roundingmode(),
               mkexpr(src)) );
   return res;
}
static IRTemp m_F32_to_I32S( IRTemp rmode, IRTemp src, UChar truncate) {
   IRTemp res = newTemp(Ity_I32);
   assign( res, binop( Iop_F64toI32S,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            unop(Iop_F32toF64, unop(Iop_ReinterpI32asF32, mkexpr(src)))) );
   return res;
}
static IRTemp m_F32_to_I32U( IRTemp rmode, IRTemp src, UChar truncate) {
   IRTemp res = newTemp(Ity_I32);
   assign( res, binop( Iop_F32toI32U_SKX,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            mkexpr(src)) );
   return res;
}
static IRTemp m_F32_to_I64S (IRTemp rmode, IRTemp src, UChar truncate) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop( Iop_F64toI64S,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            unop(Iop_F32toF64, unop(Iop_ReinterpI32asF32, mkexpr(src)))) );
   return res;
}
static IRTemp m_F32_to_I64U (IRTemp rmode, IRTemp src, UChar truncate ) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop(Iop_F64toI64U_SKX,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            unop(Iop_ReinterpF64asI64, unop(Iop_F32toF64, unop(Iop_ReinterpI32asF32, mkexpr(src))))) );
   return res;
}
static IRTemp m_F32_to_F64 (IRTemp rmode, IRTemp src, UChar unused) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, unop(Iop_ReinterpF64asI64,
            unop(Iop_F32toF64, unop(Iop_ReinterpI32asF32, mkexpr(src)))) );
   return res;
}
static IRTemp m_F64_to_I32S (IRTemp rmode, IRTemp src, UChar truncate) {
   IRTemp res = newTemp(Ity_I32);
   assign( res, binop( Iop_F64toI32S,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            unop(Iop_ReinterpI64asF64, mkexpr(src))) );
   return res;
}
static IRTemp m_F64_to_I32U (IRTemp rmode, IRTemp src, UChar truncate) {
   IRTemp res = newTemp(Ity_I32);
   assign( res, binop(Iop_F64toI32U_SKX,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            mkexpr(src)));
   return res;
}
static IRTemp m_F64_to_I64S( IRTemp rmode, IRTemp src, UChar truncate) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop(Iop_F64toI64S,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            unop(Iop_ReinterpI64asF64, mkexpr(src))) );
   return res;
}
static IRTemp m_F64_to_I64U (IRTemp rmode, IRTemp src, UChar truncate) {
   IRTemp res = newTemp(Ity_I64);
   assign( res, binop(Iop_F64toI64U_SKX,
            truncate ? mkU32(Irrm_ZERO) : mkexpr(rmode),
            mkexpr(src)) );
   return res;
}
static IRTemp m_F64_to_F32 (IRTemp rmode, IRTemp src, UChar unused) {
   IRTemp res = newTemp(Ity_I32);
   assign( res, unop(Iop_ReinterpF32asI32, binop(Iop_F64toF32,
               mkexpr(rmode),
               unop(Iop_ReinterpI64asF64, mkexpr(src)))) );
   return res;
}

/* serial conversions */
static IRTemp m_I32S_to_F32_F0x4(IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo32,
            mkexpr(src1),
            unop(Iop_ReinterpF32asI32, binop(Iop_F64toF32,
                  get_FAKE_roundingmode(),
                  unop(Iop_I32StoF64, mkexpr(src2))))));
   return res;
}
static IRTemp m_I32S_to_F64_F0x2(IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo64,
            mkexpr(src1),
            unop(Iop_ReinterpF64asI64, unop(Iop_I32StoF64, mkexpr(src2)))));
   return res;
}
static IRTemp m_I32U_to_F32_F0x4(IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo32,
            mkexpr(src1),
            binop(Iop_I32UtoF32_SKX,
               get_FAKE_roundingmode(),
               mkexpr(src2))));
   return res;
}
static IRTemp m_I32U_to_F64_F0x2 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo64,
            mkexpr(src1),
            binop(Iop_I64UtoF64_SKX,
               get_FAKE_roundingmode(),
               unop(Iop_32Uto64, mkexpr(src2)))));
   return res;
}
static IRTemp m_I64S_to_F32_F0x4(IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo32,
            mkexpr(src1),
            unop(Iop_ReinterpF32asI32, binop(Iop_F64toF32,
                  get_FAKE_roundingmode(),
                  binop(Iop_I64StoF64,
                     get_FAKE_roundingmode(),
                     mkexpr(src2))))));
   return res;
}
static IRTemp m_I64S_to_F64_F0x2 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo64,
            mkexpr(src1),
            unop(Iop_ReinterpF64asI64,
               binop(Iop_I64StoF64,
                  get_FAKE_roundingmode(),
                  mkexpr(src2)))));
   return res;
}
static IRTemp m_I64U_to_F32_F0x4 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo32,
            mkexpr(src1),
            unop(Iop_ReinterpF32asI32, binop( Iop_F64toF32,
               get_FAKE_roundingmode(),
               unop(Iop_ReinterpI64asF64, binop(Iop_I64UtoF64_SKX,
                     get_FAKE_roundingmode(),
                     mkexpr(src2)))))));
   return res;
}
static IRTemp m_I64U_to_F64_F0x2 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo64,
            mkexpr(src1),
            binop( Iop_I64UtoF64_SKX,
               get_FAKE_roundingmode(),
               mkexpr(src2))));
   return res;
}
static IRTemp m_F32_to_F64_F0x2 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo64,
            mkexpr(src1),
            unop(Iop_ReinterpF64asI64, unop(Iop_F32toF64,
                  unop(Iop_ReinterpI32asF32, unop(Iop_V128to32, mkexpr(src2)))))));
   return res;
}
static IRTemp m_F64_to_F32_F0x4 (IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_V128);
   assign(res, binop(Iop_SetV128lo32,
            mkexpr(src1),
            unop(Iop_ReinterpF32asI32, binop(Iop_F64toF32,
                  get_FAKE_roundingmode(),
                  unop(Iop_ReinterpI64asF64, unop(Iop_V128to64, mkexpr(src2)))))));
   return res;
}

/* stauration conversions */
static IRTemp m_I16U_to_I8U_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I8);
   IRExpr* upper_bound = binop(Iop_CmpLE32U, mkU32(0xFF), unop(Iop_16Uto32, mkexpr(src)) );
   assign( res, IRExpr_ITE( upper_bound, mkU8(0xFF), // if 255 <= src return 255
            unop(Iop_16to8, mkexpr(src))) );
   return res;
}

static IRTemp m_I32S_to_I8S_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I8);
   IRExpr* lower_bound = binop(Iop_CmpLE32S, mkexpr(src), unop(Iop_8Sto32, mkU8(0x80)) );
   IRExpr* upper_bound = binop(Iop_CmpLE32S, unop(Iop_8Sto32, mkU8(0x7F)), mkexpr(src) );
   assign( res, IRExpr_ITE( lower_bound, mkU8(0x80), // if src <= -128 return -128
                IRExpr_ITE( upper_bound, mkU8(0x7F), // if 127 <= src  return 127
                   unop(Iop_32to8, mkexpr(src)))) );
   return res;
}
static IRTemp m_I32U_to_I8U_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I8);
   IRExpr* upper_bound = binop(Iop_CmpLE32U, unop(Iop_8Uto32, mkU8(0xFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( upper_bound, mkU8(0xFF), // if 255 <= src return 255
            unop(Iop_32to8, mkexpr(src))) );
   return res;
}
static IRTemp m_I64S_to_I8S_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I8);
   IRExpr* lower_bound = binop(Iop_CmpLE64S, mkexpr(src), unop(Iop_8Sto64, mkU8(0x80)) );
   IRExpr* upper_bound = binop(Iop_CmpLE64S, unop(Iop_8Sto64, mkU8(0x7F)), mkexpr(src) );
   assign( res, IRExpr_ITE( lower_bound, mkU8(0x80),
                IRExpr_ITE( upper_bound, mkU8(0x7F),
                   unop(Iop_64to8, mkexpr(src)))) );
   return res;
}
static IRTemp m_I64U_to_I8U_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I8);
   IRExpr* upper_bound = binop(Iop_CmpLE64U, unop(Iop_8Uto64, mkU8(0xFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( upper_bound, mkU8(0xFF),
            unop(Iop_64to8, mkexpr(src))) );
   return res;
}
static IRTemp m_I32S_to_I16S_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I16);
   IRExpr* lower_bound = binop(Iop_CmpLE32S, mkexpr(src), unop(Iop_16Sto32, mkU16(0x8000)) );
   IRExpr* upper_bound = binop(Iop_CmpLE32S, unop(Iop_16Sto32, mkU16(0x7FFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( lower_bound, mkU16(0x8000),
                IRExpr_ITE( upper_bound, mkU16(0x7FFF),
                   unop(Iop_32to16, mkexpr(src)))) );
   return res;
}
static IRTemp m_I32U_to_I16U_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I16);
   IRExpr* upper_bound = binop(Iop_CmpLE32U, unop(Iop_16Uto32, mkU16(0xFFFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( upper_bound, mkU16(0xFFFF), unop(Iop_32to16, mkexpr(src))) );
   return res;
}
static IRTemp m_I64S_to_I16S_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I16);
   IRExpr* lower_bound = binop(Iop_CmpLE64S, mkexpr(src), unop(Iop_16Sto64, mkU16(0x8000)) );
   IRExpr* upper_bound = binop(Iop_CmpLE64S, unop(Iop_16Sto64, mkU16(0x7FFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( lower_bound, mkU16(0x8000),
                IRExpr_ITE( upper_bound, mkU16(0x7FFF),
                   unop(Iop_64to16, mkexpr(src)))) );
   return res;
}
static IRTemp m_I64U_to_I16U_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I16);
   IRExpr* upper_bound = binop(Iop_CmpLE64U, unop(Iop_16Uto64, mkU16(0xFFFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( upper_bound, mkU16(0xFFFF), unop(Iop_64to16, mkexpr(src))) );
   return res;
}
static IRTemp m_I64S_to_I32S_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I32);
   IRExpr* lower_bound = binop(Iop_CmpLE64S, mkexpr(src), unop(Iop_32Sto64, mkU32(0x80000000)) );
   IRExpr* upper_bound = binop(Iop_CmpLE64S, unop(Iop_32Sto64, mkU32(0x7FFFFFFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( lower_bound, mkU32(0x80000000),
                IRExpr_ITE( upper_bound, mkU32(0x7FFFFFFF),
                   unop(Iop_64to32, mkexpr(src)))) );
   return res;
}
static IRTemp m_I64U_to_I32U_sat( IRTemp src, IRTemp unused, UChar unused_i) {
   IRTemp res = newTemp(Ity_I32);
   IRExpr* upper_bound = binop(Iop_CmpLE64U, unop(Iop_32Uto64, mkU32(0xFFFFFFFF)), mkexpr(src) );
   assign( res, IRExpr_ITE( upper_bound, mkU32(0xFFFFFFFF), unop(Iop_64to32, mkexpr(src))) );
   return res;
}

static IRTemp m_ANDN (IRTemp src1, IRTemp src2, UChar unused ) {
   IROp and_op, not_op;
   switch (typeOfIRTemp(irsb->tyenv, src1)) {
      case Ity_V512: and_op = (IROp)Iop_AndV512; not_op = (IROp)Iop_NotV512; break;
      case Ity_V256: and_op = Iop_AndV256; not_op = Iop_NotV256; break;
      case Ity_V128: and_op = Iop_AndV128; not_op = Iop_NotV128; break;
      case Ity_I64:  and_op = Iop_And64;   not_op = Iop_Not64;   break;
      case Ity_I32:  and_op = Iop_And32;   not_op = Iop_Not32;   break;
      case Ity_I16:  and_op = Iop_And16;   not_op = Iop_Not16;   break;
      case Ity_I8:   and_op = Iop_And8;    not_op = Iop_Not8;    break;
      default: vpanic("invalid ANDN length");
   }
   IRTemp res = newTemp(typeOfIRTemp(irsb->tyenv, src1));
   assign( res, binop(and_op, mkexpr(src2), unop(not_op, mkexpr(src1))) );
   return res;
}
static IRTemp m_XNOR (IRTemp src1, IRTemp src2, UChar unused ) {
   IROp xor_op, not_op;
   switch (typeOfIRTemp(irsb->tyenv, src1)) {
      case Ity_I64: xor_op = Iop_Xor64; not_op = Iop_Not64; break;
      case Ity_I32: xor_op = Iop_Xor32; not_op = Iop_Not32; break;
      case Ity_I16: xor_op = Iop_Xor16; not_op = Iop_Not16; break;
      case Ity_I8:  xor_op = Iop_Xor8;  not_op = Iop_Not8;  break;
      default: vpanic("invalid XNOR length");
   }
   IRTemp res = newTemp(typeOfIRTemp(irsb->tyenv, src1));
   assign( res, unop(not_op, binop(xor_op, mkexpr(src2), mkexpr(src1))) );
   return res;
}
static IRTemp m_ORTEST (IRTemp src1, IRTemp src2, UChar unused ) {
   IRType type = typeOfIRTemp(irsb->tyenv, src2);
   IRExpr* or_temp = NULL;
   switch (type) {
      case Ity_I64:
         or_temp = binop(Iop_Or64, mkexpr(src1), mkexpr(src2));
         break;
      case Ity_I32:
         or_temp = unop(Iop_32Uto64, binop(Iop_Or32, unop(Iop_64to32, mkexpr(src1)), mkexpr(src2)));
         break;
      case Ity_I16:
         or_temp = unop(Iop_16Uto64, binop(Iop_Or16, unop(Iop_64to16, mkexpr(src1)), mkexpr(src2)));
         break;
      case Ity_I8:
         or_temp = unop(Iop_8Uto64, binop(Iop_Or8, unop(Iop_64to8, mkexpr(src1)), mkexpr(src2)));
         break;
      default: vpanic("invalid ORTEST length");
   }
   ULong size = sizeofIRType(type);
   IRExpr* carry_flag = IRExpr_ITE(
         binop(Iop_CmpEQ64, or_temp, mkU64(size*8-1) /* 0xFF..FF */ ),
         mkU64(AMD64G_CC_MASK_C),
         mkU64(0));
   IRExpr* zero_flag = IRExpr_ITE(
         binop(Iop_CmpEQ64, or_temp, mkU64(0)),
         mkU64(AMD64G_CC_MASK_Z),
         mkU64(0));

   IRTemp old_rflags = newTemp(Ity_I64);
   assign(old_rflags, mk_amd64g_calculate_rflags_all());
   const ULong maskOSAP = AMD64G_CC_MASK_O | AMD64G_CC_MASK_S | AMD64G_CC_MASK_A | AMD64G_CC_MASK_P;
   const ULong maskCZ = AMD64G_CC_MASK_C | AMD64G_CC_MASK_Z;

   IRTemp new_rflags = newTemp(Ity_I64);
   assign(new_rflags, binop(Iop_Or64,
            binop(Iop_And64, mkexpr(old_rflags), mkU64(maskOSAP)),
            binop(Iop_And64,
               binop(Iop_Or64, carry_flag, zero_flag),
               mkU64(maskCZ))));

   /* Set all fields even though only OFFB_CC_DEP1 is used.
    * For some reason it does not work otherwise */
   stmt( IRStmt_Put( OFFB_CC_OP,   mkU64(AMD64G_CC_OP_COPY) ));
   stmt( IRStmt_Put( OFFB_CC_DEP1, mkexpr(new_rflags) ));
   stmt( IRStmt_Put( OFFB_CC_DEP2, mkU64(0) ));
   stmt( IRStmt_Put( OFFB_CC_NDEP, mkU64(0) ));

   return src1;
}
static IRTemp m_TEST (IRTemp src1, IRTemp src2, UChar unused ) {
   IRExpr *and_temp = NULL, *andN_temp = NULL;
   switch (typeOfIRTemp(irsb->tyenv, src2)) {
      case Ity_I64:
         and_temp  = binop(Iop_And64, mkexpr(src1), mkexpr(src2));
         andN_temp = binop(Iop_And64, unop(Iop_Not64, mkexpr(src1)), mkexpr(src2));
         break;
      case Ity_I32:
         and_temp  = unop(Iop_32Uto64, binop(Iop_And32, unop(Iop_64to32, mkexpr(src1)), mkexpr(src2)));
         andN_temp = unop(Iop_32Uto64, binop(Iop_And32, unop(Iop_Not32, unop(Iop_64to32, mkexpr(src1))), mkexpr(src2)));
         break;
      case Ity_I16:
         and_temp  = unop(Iop_16Uto64, binop(Iop_And16, unop(Iop_64to16, mkexpr(src1)), mkexpr(src2)));
         andN_temp = unop(Iop_16Uto64, binop(Iop_And16, unop(Iop_Not16, unop(Iop_64to16, mkexpr(src1))), mkexpr(src2)));
         break;
      case Ity_I8:
         and_temp  = unop(Iop_8Uto64, binop(Iop_And8, unop(Iop_64to8, mkexpr(src1)), mkexpr(src2)));
         andN_temp = unop(Iop_8Uto64, binop(Iop_And8, unop(Iop_Not8, unop(Iop_64to8, mkexpr(src1))), mkexpr(src2)));
         break;
      default: vpanic("invalid ORTEST length");
   }
   IRExpr* carry_flag = IRExpr_ITE(
         binop(Iop_CmpEQ64, andN_temp, mkU64(0)),
         mkU64(AMD64G_CC_MASK_C),
         mkU64(0));
   IRExpr* zero_flag = IRExpr_ITE(
         binop(Iop_CmpEQ64, and_temp, mkU64(0)),
         mkU64(AMD64G_CC_MASK_Z),
         mkU64(0));

   IRTemp old_rflags = newTemp(Ity_I64);
   assign(old_rflags, mk_amd64g_calculate_rflags_all());
   const ULong maskOSAP = AMD64G_CC_MASK_O | AMD64G_CC_MASK_S | AMD64G_CC_MASK_A | AMD64G_CC_MASK_P;
   const ULong maskCZ = AMD64G_CC_MASK_C | AMD64G_CC_MASK_Z;

   IRTemp new_rflags = newTemp(Ity_I64);
   assign(new_rflags, binop(Iop_Or64,
            binop(Iop_And64, mkexpr(old_rflags), mkU64(maskOSAP)),
            binop(Iop_And64,
               binop(Iop_Or64, carry_flag, zero_flag),
               mkU64(maskCZ))));

   /* Set all fields even though only OFFB_CC_DEP1 is used.
    * For some reason it does not work otherwise */
   stmt( IRStmt_Put( OFFB_CC_OP,   mkU64(AMD64G_CC_OP_COPY) ));
   stmt( IRStmt_Put( OFFB_CC_DEP1, mkexpr(new_rflags) ));
   stmt( IRStmt_Put( OFFB_CC_DEP2, mkU64(0) ));
   stmt( IRStmt_Put( OFFB_CC_NDEP, mkU64(0) ));

   return src1;
}

static IRTemp m_Shuffe128 (IRTemp src_to_low, IRTemp src_to_high, UChar imm8 ) {
   IRTemp sl[4], sh[4], res = IRTemp_INVALID;
   for (Int i=0; i<4; i++) {
      sh[i] = IRTemp_INVALID;
      sl[i] = IRTemp_INVALID;
   }
   if (typeOfIRTemp(irsb->tyenv, src_to_low) == Ity_V512) {
      res = newTemp(Ity_V512);
      breakupV512toV128s( src_to_high, &sh[3], &sh[2], &sh[1], &sh[0] );
      breakupV512toV128s( src_to_low,  &sl[3], &sl[2], &sl[1], &sl[0] );
      assign( res, binop(Iop_V256HLtoV512,
               binop(Iop_V128HLtoV256, mkexpr(sh[(imm8>>6)&3]),  mkexpr(sh[(imm8>>4)&3])),
               binop(Iop_V128HLtoV256, mkexpr(sl[(imm8>>2)&3]),  mkexpr(sl[(imm8>>0)&3]))) );
   } else {
      res = newTemp(Ity_V256);
      breakupV256toV128s( src_to_high, &sh[1], &sh[0] );
      breakupV256toV128s( src_to_low,  &sl[1], &sl[0] );
      assign( res, binop(Iop_V128HLtoV256, mkexpr(sh[(imm8>>1)&1]), mkexpr(sl[(imm8>>0)&1])));
   }
   return res;
}

static IRTemp m_perm_imm_64x4 (IRTemp values, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_V256);
   IRTemp s_val[4], s_res[4];
   for (Int j = 0; j < 4; j++) {
      s_val[j] = IRTemp_INVALID;
      s_res[j] = newTemp(Ity_I64);
   }
   Split_arg(values, s_val, 4);
   for (Int j = 0; j < 4; j++)
      assign(s_res[j], mkexpr( s_val[ (imm8>>(j*2)) & 3 ] ));
   Merge_dst(res, s_res, 4);
   return res;
}

static IRTemp m_PABS_64 (IRTemp src, IRTemp unused, UChar laneszB) {
   if (laneszB < 8)
      return math_PABS_MMX(src, laneszB);

   IRTemp res     = newTemp(Ity_I64);
   IRTemp srcNeg  = newTemp(Ity_I64);
   IRTemp negMask = newTemp(Ity_I64);
   IRTemp posMask = newTemp(Ity_I64);

   assign( negMask, binop(Iop_Sar64, mkexpr(src), mkU8(8*laneszB-1)) );
   assign( posMask, unop(Iop_Not64, mkexpr(negMask)) );
   assign( srcNeg,  binop(Iop_Sub64, mkU64(0), mkexpr(src)) );
   assign( res, binop(Iop_Or64,
            binop(Iop_And64, mkexpr(src),    mkexpr(posMask)),
            binop(Iop_And64, mkexpr(srcNeg), mkexpr(negMask)) ));
   return res;
}

static IRTemp m_SHUFPD_512 (IRTemp src1, IRTemp src2, UChar imm8) {
   IRTemp sVhi = IRTemp_INVALID, sVlo = IRTemp_INVALID;
   IRTemp dVhi = IRTemp_INVALID, dVlo = IRTemp_INVALID;
   breakupV512toV256s( src1, &sVhi, &sVlo );
   breakupV512toV256s( src2, &dVhi, &dVlo );
   IRTemp rVhi = math_SHUFPD_256(sVhi, dVhi, (imm8 >> 4) & 0xF);
   IRTemp rVlo = math_SHUFPD_256(sVlo, dVlo, imm8 & 0xF);
   IRTemp res  = newTemp(Ity_V512);
   assign(res, binop(Iop_V256HLtoV512, mkexpr(rVhi), mkexpr(rVlo)));
   return res;
}

static IRTemp m_PSHUFB (IRTemp src1, IRTemp src2, UChar unused) {
   return math_PSHUFB_XMM(src1, src2);
}
static IRTemp m_PMADDWD (IRTemp src1, IRTemp src2, UChar unused) {
   return math_PMADDWD_128(src1, src2);
}
static IRTemp m_PSADBW (IRTemp src1, IRTemp src2, UChar unused) {
   return math_PSADBW_128(src1, src2);
}

static IRTemp m_PSHUFD (IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_V128);
   IRTemp s[4] = { IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID};
   breakupV128to32s( src, &s[3], &s[2], &s[1], &s[0] );
   assign( res, mkV128from32s( s[(imm8>>6)&3], s[(imm8>>4)&3],
                              s[(imm8>>2)&3], s[(imm8>>0)&3] ) );
   return res;
}
static IRTemp m_PSHUFHW (IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_V128);
   IRTemp srcMut = newTemp(Ity_I64);
   assign(srcMut, unop(Iop_V128HIto64, mkexpr(src)));
   IRTemp s[4] = { IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID};
   breakup64to16s( srcMut, &s[3], &s[2], &s[1], &s[0] );
   IRTemp resMut = newTemp(Ity_I64);
   assign( resMut, mk64from16s( s[(imm8>>6)&3], s[(imm8>>4)&3],
                                s[(imm8>>2)&3], s[(imm8>>0)&3] ) );
   assign(res, binop(Iop_64HLtoV128, mkexpr(resMut), unop(Iop_V128to64, mkexpr(src))));
   return res;
}
static IRTemp m_PSHUFLW (IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_V128);
   IRTemp srcMut = newTemp(Ity_I64);
   assign(srcMut, unop(Iop_V128to64, mkexpr(src)));
   IRTemp s[4] = { IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID};
   breakup64to16s( srcMut, &s[3], &s[2], &s[1], &s[0] );
   IRTemp resMut = newTemp(Ity_I64);
   assign( resMut, mk64from16s( s[(imm8>>6)&3], s[(imm8>>4)&3],
            s[(imm8>>2)&3], s[(imm8>>0)&3] ) );
   assign(res, binop(Iop_64HLtoV128, unop(Iop_V128HIto64, mkexpr(src)), mkexpr(resMut)));
   return res;
}

static IRTemp m_PMULHRSW(IRTemp src1, IRTemp src2, UChar unused) {
   IRTemp res = newTemp(Ity_I64);
   assign(res, dis_PMULHRSW_helper(mkexpr(src1), mkexpr(src2)));
   return res;
}

static IRTemp m_VPMOVB2M(IRTemp src, IRTemp unused, UChar vl) {
   IRTemp res = IRTemp_INVALID, cmp = IRTemp_INVALID;
   IRTemp elem = newTemp(Ity_I8);
   assign(elem, mkU8(1<<7));
   IROp cmp_op = Iop_INVALID;
   UInt mult = 0;
   switch (vl) {
      case 0: res = newTemp(Ity_V128); cmp_op = (IROp)Iop_Cmp8Ux16; mult = 16; break;
      case 1: res = newTemp(Ity_V256); cmp_op = (IROp)Iop_Cmp8Ux32; mult = 32; break;
      case 2: res = newTemp(Ity_V512); cmp_op = (IROp)Iop_Cmp8Ux64; mult = 64; break;
      default: vpanic("Invalid VL");
   }
   cmp = m_Broadcast(elem, unused, mult);
   IRTemp dummy = newTemp(Ity_I64);
   assign(dummy, mkU64(0));
   assign(res, qop(cmp_op, mkexpr(dummy), mkexpr(cmp), mkexpr(src), mkU8(0x2)));
   return res;
}
static IRTemp m_VPMOVW2M(IRTemp src, IRTemp unused, UChar vl) {
   IRTemp res = IRTemp_INVALID, cmp = IRTemp_INVALID;
   IRTemp elem = newTemp(Ity_I16);
   assign(elem, mkU16(1<<15));
   IROp cmp_op = Iop_INVALID;
   UInt mult = 0;
   switch (vl) {
      case 0: res = newTemp(Ity_V128); cmp_op = (IROp)Iop_Cmp16Ux8;  mult = 8;  break;
      case 1: res = newTemp(Ity_V256); cmp_op = (IROp)Iop_Cmp16Ux16; mult = 16; break;
      case 2: res = newTemp(Ity_V512); cmp_op = (IROp)Iop_Cmp16Ux32; mult = 32; break;
      default: vpanic("Invalid VL");
   }
   cmp = m_Broadcast(elem, unused, mult);
   IRTemp dummy = newTemp(Ity_I64);
   assign(dummy, mkU64(0));
   assign(res, qop(cmp_op, mkexpr(dummy), mkexpr(cmp), mkexpr(src), mkU8(0x2)));
   return res;
}
static IRTemp m_VPMOVD2M(IRTemp src, IRTemp unused, UChar vl) {
   IRTemp res = IRTemp_INVALID, cmp = IRTemp_INVALID;
   IRTemp elem = newTemp(Ity_I32);
   assign(elem, mkU32(1ULL<<31));
   IROp cmp_op = Iop_INVALID;
   switch (vl) {
      case 0:
         res = newTemp(Ity_V128);
         cmp_op = (IROp)Iop_Cmp32Ux4;
         cmp = m_Broadcast(elem, unused, 4);
         break;
      case 1:
         res = newTemp(Ity_V256);
         cmp_op = (IROp)Iop_Cmp32Ux8;
         cmp = m_Broadcast(elem, unused, 8);
         break;
      case 2:
         res = newTemp(Ity_V512);
         cmp_op = (IROp)Iop_Cmp32Ux16;
         cmp = m_Broadcast(elem, unused, 16);
         break;
      default: vpanic("Invalid VL");
   }
   IRTemp dummy = newTemp(Ity_I64);
   assign(dummy, mkU64(0));
   assign(res, qop(cmp_op, mkexpr(dummy), mkexpr(cmp), mkexpr(src), mkU8(0x2)));
   return res;
}
static IRTemp m_VPMOVQ2M(IRTemp src, IRTemp unused, UChar vl) {
   IRTemp res = IRTemp_INVALID, cmp = IRTemp_INVALID;
   IRTemp elem = newTemp(Ity_I64);
   assign(elem, mkU64(1ULL<<63));
   IROp cmp_op = Iop_INVALID;
   switch (vl) {
      case 0:
         res = newTemp(Ity_V128);
         cmp_op = (IROp)Iop_Cmp64Ux2;
         cmp = m_Broadcast(elem, unused, 2);
         break;
      case 1:
         res = newTemp(Ity_V256);
         cmp_op = (IROp)Iop_Cmp64Ux4;
         cmp = m_Broadcast(elem, unused, 4);
         break;
      case 2:
         res = newTemp(Ity_V512);
         cmp_op = (IROp)Iop_Cmp64Ux8;
         cmp = m_Broadcast(elem, unused, 8);
         break;
      default: vpanic("Invalid VL");
   }
   IRTemp dummy = newTemp(Ity_I64);
   assign(dummy, mkU64(0));
   assign(res, qop(cmp_op, mkexpr(dummy), mkexpr(cmp), mkexpr(src), mkU8(0x2)));
   return res;
}

static IRTemp m_kShiftR(IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(typeOfIRTemp(irsb->tyenv, src));
   IROp shiftR = Iop_INVALID;
   IRExpr *zero = NULL;
   UInt max = 0;
   switch (typeOfIRTemp(irsb->tyenv, src)) {
      case Ity_I8:  shiftR = Iop_Shr8;  max = 0x7;  zero = mkU8(0);  break;
      case Ity_I16: shiftR = Iop_Shr16; max = 0xF;  zero = mkU16(0); break;
      case Ity_I32: shiftR = Iop_Shr32; max = 0x1F; zero = mkU32(0); break;
      case Ity_I64: shiftR = Iop_Shr64; max = 0x3F; zero = mkU64(0); break;
      default: vpanic("wrong mask shift");
   }
   assign( res, (imm8 > max) ? zero : binop(shiftR, mkexpr(src), mkU8(imm8)) );
   return res;
}
static IRTemp m_kShiftL(IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(typeOfIRTemp(irsb->tyenv, src));
   IROp shiftL = Iop_INVALID;
   IRExpr *zero = NULL;
   UInt max = 0;
   switch (typeOfIRTemp(irsb->tyenv, src)) {
      case Ity_I8:  shiftL = Iop_Shl8;  max = 0x7;  zero = mkU8(0);  break;
      case Ity_I16: shiftL = Iop_Shl16; max = 0xF;  zero = mkU16(0); break;
      case Ity_I32: shiftL = Iop_Shl32; max = 0x1F; zero = mkU32(0); break;
      case Ity_I64: shiftL = Iop_Shl64; max = 0x3F; zero = mkU64(0); break;
      default: vpanic("wrong mask shift");
   }
   assign( res, (imm8 > max) ? zero : binop(shiftL, mkexpr(src), mkU8(imm8)) );
   return res;
}

static IRTemp m_F32toF16x4(IRTemp src, IRTemp unused, UChar imm8) {
   IRTemp res = newTemp(Ity_I64);
   assign(res, binop(Iop_F32toF16x4, mkU32(imm8), mkexpr(src)));
   return res;
}

static IRTemp m_POPCOUNT_16(IRTemp src, IRTemp unused, UChar unused_i) {
   return gen_POPCOUNT(Ity_I16, src);
}
static IRTemp m_POPCOUNT_32(IRTemp src, IRTemp unused, UChar unused_i) {
   return gen_POPCOUNT(Ity_I32, src);
}
static IRTemp m_POPCOUNT_64(IRTemp src, IRTemp unused, UChar unused_i) {
   return gen_POPCOUNT(Ity_I64, src);
}

static IRTemp m_Align_32 (IRTemp src1, IRTemp src2, UChar imm8) {
   IRType src_t = typeOfIRTemp(irsb->tyenv, src2);
   UInt n = (sizeofIRType(src_t)) / (sizeofIRType(Ity_I32));
   switch (src_t) {
      case Ity_V512: imm8 &= 0x0F; break;
      case Ity_V256: imm8 &= 0x07; break;
      case Ity_V128: imm8 &= 0x03; break;
      default: vpanic("Invalid VL");
   }

   IRTemp s1_elem[MAX_ELEMENTS], s2_elem[MAX_ELEMENTS], dst_elem[MAX_ELEMENTS];
   vassert(n <= MAX_ELEMENTS);
   UInt j = 0;
   for (j = 0; j < n; j++) {
      s1_elem[j] = IRTemp_INVALID;
      s2_elem[j] = IRTemp_INVALID;
      dst_elem[j] = newTemp(Ity_I32);
   }
   Split_arg(src1, s1_elem, n);
   Split_arg(src2, s2_elem, n);
   for (j = 0; j < n-imm8; j++) {
      assign(dst_elem[j], mkexpr(s2_elem[j+imm8]));
   }
   for (j = n-imm8; j < n; j++) {
      assign(dst_elem[j], mkexpr(s1_elem[j+imm8-n]));
   }
   IRTemp dst = newTemp(src_t);
   Merge_dst(dst, dst_elem, n);
   return dst;
}
static IRTemp m_Align_64 (IRTemp src1, IRTemp src2, UChar imm8) {
   IRType src_t = typeOfIRTemp(irsb->tyenv, src1);
   UInt n = sizeofIRType(src_t) / sizeofIRType(Ity_I64);
   switch (src_t) {
      case Ity_V512: imm8 &= 0x07; break;
      case Ity_V256: imm8 &= 0x03; break;
      case Ity_V128: imm8 &= 0x01; break;
      default: vpanic("Invalid VL");
   }

   Int j=0;
   IRTemp s1_elem[MAX_ELEMENTS], s2_elem[MAX_ELEMENTS], dst_elem[MAX_ELEMENTS];
   vassert(n <= MAX_ELEMENTS);
   for (j = 0; j < n; j++) {
      s1_elem[j] = IRTemp_INVALID;
      s2_elem[j] = IRTemp_INVALID;
      dst_elem[j] = newTemp(Ity_I64);
   }
   Split_arg(src1, s1_elem, n);
   Split_arg(src2, s2_elem, n);
   for (j = 0; j < n-imm8; j++) {
      assign(dst_elem[j], mkexpr(s2_elem[j+imm8]));
   }
   for (j = n-imm8; j < n; j++) {
      assign(dst_elem[j], mkexpr(s1_elem[j+imm8-n]));
   }
   IRTemp dst = newTemp(src_t);
   Merge_dst(dst, dst_elem, n);
   return dst;
}

static IRTemp m_ConvertF16x2toF32(IRTemp src1, IRTemp src2, UChar imm8) {
   vpanic("ConvertF16x2toF32 not implemented yet");
   return IRTemp_INVALID; /* not reached; keeps MSVC (no noreturn) quiet */
}

static IRTemp m_vcmpss(IRTemp src1, IRTemp src2, UChar imm8) {
   Bool preZero = False;
   Bool preSwap = False;
   IROp op      = Iop_INVALID;
   Bool postNot = False;

   Bool ok = findSSECmpOp(&preZero, &preSwap, &op, &postNot, imm8,
         False /*one lane*/, 4 /*32 bits*/);
   if (!ok) { 
      vpanic("Comparison not found\n");
   }

   IRTemp argL = newTemp(Ity_V128);
   IRTemp argR = newTemp(Ity_V128);
   IRTemp cmp = newTemp(Ity_I64);
   IRTemp res = newTemp(Ity_I64);

   if (preZero) {
      assign(argL, mkV128(0xFFF0)); 
      assign(argR, mkV128(0xFFF0));
   } else {
      if (preSwap) {
         assign(argL, unop(Iop_32UtoV128, mkexpr(src2)));
         assign(argR, unop(Iop_32UtoV128, mkexpr(src1)));
      } else {
         assign(argL, unop(Iop_32UtoV128, mkexpr(src1)));
         assign(argR, unop(Iop_32UtoV128, mkexpr(src2)));
      }
   }
   
   assign(cmp, unop(Iop_V128to64, binop(op, mkexpr(argL), mkexpr(argR))));
   if (postNot) {
      assign (res, binop(Iop_And64,
               binop(Iop_Xor64, mkexpr(cmp), mkU64(0x000F)),
               mkU64(0x1)));
   } else {
      assign (res, binop(Iop_And64, mkexpr(cmp), mkU64(0x1)));
   }
   return res;
}

static IRTemp m_vcmpsd(IRTemp src1, IRTemp src2, UChar imm8) {
   Bool preZero = False;
   Bool preSwap = False;
   IROp op      = Iop_INVALID;
   Bool postNot = False;

   Bool ok = findSSECmpOp(&preZero, &preSwap, &op, &postNot, imm8,
         False /*one lane*/, 8 /*64 bits*/);
   if (!ok) { 
      vpanic("Comparison not found\n");
   }

   IRTemp argL = newTemp(Ity_V128);
   IRTemp argR = newTemp(Ity_V128);
   IRTemp cmp = newTemp(Ity_I64);
   IRTemp res = newTemp(Ity_I64);

   if (preZero) {
      assign(argL, mkV128(0xFF00)); 
      assign(argR, mkV128(0xFF00));
   } else {
      if (preSwap) {
         assign(argL, unop(Iop_64UtoV128, mkexpr(src2)));
         assign(argR, unop(Iop_64UtoV128, mkexpr(src1)));
      } else {
         assign(argL, unop(Iop_64UtoV128, mkexpr(src1)));
         assign(argR, unop(Iop_64UtoV128, mkexpr(src2)));
      }
   }
   assign(cmp, unop(Iop_V128to64, binop(op, mkexpr(argL), mkexpr(argR))));
   if (postNot) {
      assign (res, binop(Iop_And64,
               binop(Iop_Xor64, mkexpr(cmp), mkU64(0x00FF)),
               mkU64(0x1)));
   } else {
      assign (res, binop(Iop_And64, mkexpr(cmp), mkU64(0x1)));
   }
   return res;
}
// helpers end

#define MAX_MULTIPLIER (64)
#define MAX_ARITY (5)

static void breakup32to8s ( IRTemp t32,
      IRTemp* t3, IRTemp* t2, IRTemp* t1, IRTemp* t0 )
{
   IRTemp hi16 = newTemp(Ity_I16);
   IRTemp lo16 = newTemp(Ity_I16);
   assign( hi16, unop(Iop_32HIto16, mkexpr(t32)) );
   assign( lo16, unop(Iop_32to16,   mkexpr(t32)) );

   vassert(t0 && *t0 == IRTemp_INVALID);
   vassert(t1 && *t1 == IRTemp_INVALID);
   vassert(t2 && *t2 == IRTemp_INVALID);
   vassert(t3 && *t3 == IRTemp_INVALID);

   *t0 = newTemp(Ity_I8);
   *t1 = newTemp(Ity_I8);
   *t2 = newTemp(Ity_I8);
   *t3 = newTemp(Ity_I8);
   assign( *t0, unop(Iop_16to8,   mkexpr(lo16)) );
   assign( *t1, unop(Iop_16HIto8, mkexpr(lo16)) );
   assign( *t2, unop(Iop_16to8,   mkexpr(hi16)) );
   assign( *t3, unop(Iop_16HIto8, mkexpr(hi16)) );
}
static void breakup64to32s ( IRTemp t64, IRTemp* t1, IRTemp* t0 )
{
   vassert(t0 && *t0 == IRTemp_INVALID);
   vassert(t1 && *t1 == IRTemp_INVALID);
   *t0 = newTemp(Ity_I32);
   *t1 = newTemp(Ity_I32);
   assign( *t0, unop(Iop_64to32,   mkexpr(t64)) );
   assign( *t1, unop(Iop_64HIto32, mkexpr(t64)) );
}

static void breakup32to16s ( IRTemp t32, IRTemp* t1, IRTemp* t0 )
{
   vassert(t0 && *t0 == IRTemp_INVALID);
   vassert(t1 && *t1 == IRTemp_INVALID);
   *t0 = newTemp(Ity_I16);
   *t1 = newTemp(Ity_I16);
   assign( *t0, unop(Iop_32to16,   mkexpr(t32)) );
   assign( *t1, unop(Iop_32HIto16, mkexpr(t32)) );
}
static void breakup16to8s ( IRTemp t16, IRTemp* t1, IRTemp* t0 )
{
   vassert(t0 && *t0 == IRTemp_INVALID);
   vassert(t1 && *t1 == IRTemp_INVALID);
   *t0 = newTemp(Ity_I8);
   *t1 = newTemp(Ity_I8);
   assign( *t0, unop(Iop_16to8,   mkexpr(t16)) );
   assign( *t1, unop(Iop_16HIto8, mkexpr(t16)) );
}


/* The INS_ARR opFn slot takes a UChar imm8; the upstream math_* helpers take
   UInt.  Same values, but MSVC (C4113) rejects the pointer-type mismatch. */
static IRTemp m_PALIGNR_XMM(IRTemp a, IRTemp b, UChar imm8) { return math_PALIGNR_XMM(a, b, imm8); }
static IRTemp m_PCLMULQDQ(IRTemp a, IRTemp b, UChar imm8) { return math_PCLMULQDQ(a, b, imm8); }
static IRTemp m_PINSRB_128(IRTemp a, IRTemp b, UChar imm8) { return math_PINSRB_128(a, b, imm8); }
static IRTemp m_PINSRD_128(IRTemp a, IRTemp b, UChar imm8) { return math_PINSRD_128(a, b, imm8); }
static IRTemp m_PINSRQ_128(IRTemp a, IRTemp b, UChar imm8) { return math_PINSRQ_128(a, b, imm8); }
static IRTemp m_PINSRW_128(IRTemp a, IRTemp b, UChar imm8) { return math_PINSRW_128(a, b, imm8); }
static IRTemp m_SHUFPD_128(IRTemp a, IRTemp b, UChar imm8) { return math_SHUFPD_128(a, b, imm8); }
static IRTemp m_SHUFPD_256(IRTemp a, IRTemp b, UChar imm8) { return math_SHUFPD_256(a, b, imm8); }
static IRTemp m_SHUFPS_128(IRTemp a, IRTemp b, UChar imm8) { return math_SHUFPS_128(a, b, imm8); }
static IRTemp m_SHUFPS_256(IRTemp a, IRTemp b, UChar imm8) { return math_SHUFPS_256(a, b, imm8); }

#include <guest_AVX512.h>

IRExpr* mask_expr(Prefix pfx, IRExpr* unmasked, IRExpr* original, Int ins_id)
{
   IRType dst_type   = typeOfIRExpr(irsb->tyenv, unmasked);
   vassert(dst_type == typeOfIRExpr(irsb->tyenv, original));

   UInt mask = getEvexMask();
   if (!mask) {
      return unmasked;
   }
   DIP("{k%u}", mask);

   IRTemp res        = newTemp(dst_type);
   UInt   tuple_type = getTupleType();

   if ( INS_ARR[ins_id].args[0] == _kG ) {
      // Destination is a full mask register: each opmask bit corresponds
      // to a result bit; only zeroing mode is available
      return (binop(Iop_And64, unmasked, getKReg(mask)));
   }

   /* Serial case. Result is a vector, but only the last element is masked. */
   if (tuple_type == Tuple1Scalar) {
      // Only the lowest mask bit and vector elements are used
      IRTemp cond = newTemp(Ity_I1);
      assign( cond, unop(Iop_64to1, getKReg(mask)) );

      IRExpr* masked = NULL;
      switch (dst_type) {
         case Ity_V128: {
            if (getDstW() == W_32)
               masked = binop(Iop_SetV128lo32, unmasked, getZeroMode() ? mkU32(0): unop(Iop_V128to32, original));
            else
               masked = binop(Iop_SetV128lo64, unmasked, getZeroMode() ? mkU64(0): unop(Iop_V128to64, original));
            break;
         }
         case Ity_I64: masked = getZeroMode() ? mkU64(0): original; break;
         case Ity_I32: masked = getZeroMode() ? mkU32(0): original; break;
         default: vpanic("unknown IR type");
      }
      assign(res, IRExpr_ITE(mkexpr(cond), unmasked, masked));
      return (mkexpr(res));
   }

   /* Vector cases */
   /* Set auxiliary IRs depending on the destination width */
   IROp bitAnd = Iop_INVALID;
   IROp bitOr  = Iop_INVALID;
   IROp bitNot = Iop_INVALID;
   switch (dst_type) {
      case Ity_V512: bitAnd=Iop_AndV512; bitOr=Iop_OrV512; bitNot=Iop_NotV512; break;
      case Ity_V256: bitAnd=Iop_AndV256; bitOr=Iop_OrV256; bitNot=Iop_NotV256; break;
      case Ity_V128: bitAnd=Iop_AndV128; bitOr=Iop_OrV128; bitNot=Iop_NotV128; break;
      case Ity_I64:  bitAnd=Iop_And64;   bitOr=Iop_Or64;   bitNot=Iop_Not64;   break;
      case Ity_I32:  bitAnd=Iop_And32;   bitOr=Iop_Or32;   bitNot=Iop_Not32;   break;
      case Ity_I16:  bitAnd=Iop_And16;   bitOr=Iop_Or16;   bitNot=Iop_Not16;   break;
      default: vpanic("unsupported mask length");
   }

   /* Do the masking */
   /* Determine granularity. mask_vec expands each mask bit to the element size (0->0, 1->0xFF..FF) */
   IRTemp mask_vec = newTemp(dst_type);
   switch (dst_type) {
      case Ity_V512:
         assign(mask_vec, binop(Iop_ExpandBitsToV512, getKReg(mask), mkU8(getDstW())));
         break;
      case Ity_V256:
         assign(mask_vec, binop(Iop_ExpandBitsToV256, getKReg(mask), mkU8(getDstW())));
         break;
      case Ity_V128:
         assign(mask_vec, binop(Iop_ExpandBitsToV128, getKReg(mask), mkU8(getDstW())));
         break;
      case Ity_I64:
         assign(mask_vec, binop(Iop_ExpandBitsToInt, getKReg(mask), mkU8(getDstW())));
         break;
      case Ity_I32:
         assign(mask_vec, unop (Iop_64to32,
                  binop(Iop_ExpandBitsToInt, getKReg(mask), mkU8(getDstW()))));
         break;
      case Ity_I16:
         assign(mask_vec, unop (Iop_64to16,
                  binop(Iop_ExpandBitsToInt, getKReg(mask), mkU8(getDstW()))));
         break;
      default: vpanic("weird mask");
   }

   if (getZeroMode()) {
      // zero out unmasked elements
      assign(res, binop(bitAnd, unmasked, mkexpr(mask_vec)));
   }
   else {
      // zero out unmasked elements, fill the gaps with the original values
      assign(res, binop(bitOr,
               binop(bitAnd, unmasked, mkexpr(mask_vec)),
               binop(bitAnd, original, unop(bitNot, mkexpr(mask_vec)))));
   }
   return (mkexpr(res));
}


static void Split_arg (IRTemp src, IRTemp* s, Int multiplier) {
   switch (multiplier) {
      case 1: s[0] = src; return;
      case 2:
              switch( typeOfIRTemp(irsb->tyenv, src) ) {
                 case Ity_V512: breakupV512toV256s( src, &s[1], &s[0] ); return;
                 case Ity_V256: breakupV256toV128s( src, &s[1], &s[0] ); return;
                 case Ity_V128: breakupV128to64s  ( src, &s[1], &s[0] ); return;
                 case Ity_I64:  breakup64to32s    ( src, &s[1], &s[0] ); return;
                 case Ity_I32:  breakup32to16s    ( src, &s[1], &s[0] ); return;
                 case Ity_I16:  breakup16to8s     ( src, &s[1], &s[0] ); return;
                 default: vpanic(" cannot split in 2");
              }
              break;
      case 4:
              switch( typeOfIRTemp(irsb->tyenv, src) ) {
                 case Ity_V512: breakupV512toV128s( src, &s[3], &s[2], &s[1], &s[0] ); return;
                 case Ity_V256: breakupV256to64s  ( src, &s[3], &s[2], &s[1], &s[0] ); return;
                 case Ity_V128: breakupV128to32s  ( src, &s[3], &s[2], &s[1], &s[0] ); return;
                 case Ity_I64:  breakup64to16s    ( src, &s[3], &s[2], &s[1], &s[0] ); return;
                 case Ity_I32:  breakup32to8s     ( src, &s[3], &s[2], &s[1], &s[0] ); return;
                 default: vpanic(" cannot split in 4");
              }
              break;
      case 8:
              switch( typeOfIRTemp(irsb->tyenv, src) ) {
                 case Ity_V512: breakupV512to64s ( src, &s[7], &s[6], &s[5], &s[4], &s[3], &s[2], &s[1], &s[0] ); return;
                 case Ity_V256: breakupV256to32s ( src, &s[7], &s[6], &s[5], &s[4], &s[3], &s[2], &s[1], &s[0] ); return;
                 case Ity_V128: {
                    IRTemp tmpHi = IRTemp_INVALID;
                    IRTemp tmpLo = IRTemp_INVALID;
                    breakupV128to64s(src, &tmpHi, &tmpLo);
                    breakup64to16s( tmpHi, &s[7], &s[6], &s[5], &s[4] );
                    breakup64to16s( tmpLo, &s[3], &s[2], &s[1], &s[0] );
                    return;
                 }
                 case Ity_I64: {
                    IRTemp tmpHi = IRTemp_INVALID;
                    IRTemp tmpLo = IRTemp_INVALID;
                    breakup64to32s(src, &tmpHi, &tmpLo);
                    breakup32to8s( tmpHi, &s[7], &s[6], &s[5], &s[4] );
                    breakup32to8s( tmpLo, &s[3], &s[2], &s[1], &s[0] );
                    return;
                 }
                 default: vpanic(" cannot split in 8");
              }
              break;
      case 16:
              switch( typeOfIRTemp(irsb->tyenv, src) ) {
                 case Ity_V512: {
                    IRTemp tmpHi = IRTemp_INVALID;
                    IRTemp tmpLo = IRTemp_INVALID;
                    breakupV512toV256s(src, &tmpHi, &tmpLo);
                    breakupV256to32s (tmpHi, &s[15], &s[14], &s[13], &s[12], &s[11], &s[10], &s[9], &s[8] );
                    breakupV256to32s (tmpLo, &s[7], &s[6], &s[5], &s[4], &s[3], &s[2], &s[1], &s[0] );
                    return;
                 }
                 case Ity_V256: {
                    IRTemp tmp1, tmp2, tmp3, tmp4;
                    tmp1 = tmp2 = tmp3 = tmp4 = IRTemp_INVALID;
                    breakupV256to64s( src, &tmp4, &tmp3, &tmp2, &tmp1);
                    breakup64to16s( tmp4, &s[15], &s[14], &s[13], &s[12] );
                    breakup64to16s( tmp3, &s[11], &s[10], &s[9],  &s[8]  );
                    breakup64to16s( tmp2, &s[7],  &s[6],  &s[5],  &s[4]  );
                    breakup64to16s( tmp1, &s[3],  &s[2],  &s[1],  &s[0]  );
                    return;
                 }
                 case Ity_V128: {
                    IRTemp tmp1, tmp2, tmp3, tmp4;
                    tmp1 = tmp2 = tmp3 = tmp4 = IRTemp_INVALID;
                    breakupV128to32s( src, &tmp4, &tmp3, &tmp2, &tmp1);
                    breakup32to8s( tmp4, &s[15], &s[14], &s[13], &s[12] );
                    breakup32to8s( tmp3, &s[11], &s[10], &s[9],  &s[8]  );
                    breakup32to8s( tmp2, &s[7],  &s[6],  &s[5],  &s[4]  );
                    breakup32to8s( tmp1, &s[3],  &s[2],  &s[1],  &s[0]  );
                    return;
                 }
                 default: vpanic(" cannot split in 16");
              }
              break;
      case 32:
              switch( typeOfIRTemp(irsb->tyenv, src) ) {
                 case Ity_V512: {
                    IRTemp t[8];
                    for (Int i=0; i<8; i++)
                       t[i] = IRTemp_INVALID;
                    breakupV512to64s ( src, &t[7], &t[6], &t[5], &t[4], &t[3], &t[2], &t[1], &t[0] );
                    breakup64to16s( t[7], &s[31], &s[30], &s[29], &s[28] );
                    breakup64to16s( t[6], &s[27], &s[26], &s[25], &s[24] );
                    breakup64to16s( t[5], &s[23], &s[22], &s[21], &s[20] );
                    breakup64to16s( t[4], &s[19], &s[18], &s[17], &s[16] );
                    breakup64to16s( t[3], &s[15], &s[14], &s[13], &s[12] );
                    breakup64to16s( t[2], &s[11], &s[10], &s[9],  &s[8]  );
                    breakup64to16s( t[1], &s[7],  &s[6],  &s[5],  &s[4]  );
                    breakup64to16s( t[0], &s[3],  &s[2],  &s[1],  &s[0]  );
                    return;
                 }
                 case Ity_V256: {
                    IRTemp t[8];
                    for (Int i=0; i<8; i++)
                       t[i] = IRTemp_INVALID;
                    breakupV256to32s ( src, &t[7], &t[6], &t[5], &t[4], &t[3], &t[2], &t[1], &t[0] );
                    breakup32to8s( t[7], &s[31], &s[30], &s[29], &s[28] );
                    breakup32to8s( t[6], &s[27], &s[26], &s[25], &s[24] );
                    breakup32to8s( t[5], &s[23], &s[22], &s[21], &s[20] );
                    breakup32to8s( t[4], &s[19], &s[18], &s[17], &s[16] );
                    breakup32to8s( t[3], &s[15], &s[14], &s[13], &s[12] );
                    breakup32to8s( t[2], &s[11], &s[10], &s[9],  &s[8]  );
                    breakup32to8s( t[1], &s[7],  &s[6],  &s[5],  &s[4]  );
                    breakup32to8s( t[0], &s[3],  &s[2],  &s[1],  &s[0]  );
                    return;
                 }
                 default: vpanic("You don't want to split in 32");
              }
              break;
      case 64:
      {
         vassert (typeOfIRTemp(irsb->tyenv, src) == Ity_V512);
         IRTemp tmpHi = IRTemp_INVALID;
         IRTemp tmpLo = IRTemp_INVALID;
         breakupV512toV256s(src, &tmpHi, &tmpLo);
         IRTemp t[16];
         for (Int i=0; i<16; i++)
            t[i] = IRTemp_INVALID;
         breakupV256to32s ( tmpHi, &t[15], &t[14], &t[13], &t[12], &t[11], &t[10], &t[9], &t[8] );
         breakupV256to32s ( tmpLo, &t[7], &t[6], &t[5], &t[4], &t[3], &t[2], &t[1], &t[0] );

         breakup32to8s( t[15], &s[63], &s[62], &s[61], &s[60] );
         breakup32to8s( t[14], &s[59], &s[58], &s[57], &s[56] );
         breakup32to8s( t[13], &s[55], &s[54], &s[53], &s[52] );
         breakup32to8s( t[12], &s[51], &s[50], &s[49], &s[48] );
         breakup32to8s( t[11], &s[47], &s[46], &s[45], &s[44] );
         breakup32to8s( t[10], &s[43], &s[42], &s[41], &s[40] );
         breakup32to8s( t[9],  &s[39], &s[38], &s[37], &s[36] );
         breakup32to8s( t[8],  &s[35], &s[34], &s[33], &s[32] );
         breakup32to8s( t[7],  &s[31], &s[30], &s[29], &s[28] );
         breakup32to8s( t[6],  &s[27], &s[26], &s[25], &s[24] );
         breakup32to8s( t[5],  &s[23], &s[22], &s[21], &s[20] );
         breakup32to8s( t[4],  &s[19], &s[18], &s[17], &s[16] );
         breakup32to8s( t[3],  &s[15], &s[14], &s[13], &s[12] );
         breakup32to8s( t[2],  &s[11], &s[10], &s[9],  &s[8]  );
         breakup32to8s( t[1],  &s[7],  &s[6],  &s[5],  &s[4]  );
         breakup32to8s( t[0],  &s[3],  &s[2],  &s[1],  &s[0]  );
         return;
      }
      default: vex_printf("%d ", multiplier);
               vpanic("unsupported split multiplier");
   }
}

static void Merge_dst( IRTemp dst, IRTemp* s, Int multiplier) {
   switch (multiplier) {
      case 1:
         assign(dst, mkexpr(s[0])); break;
      case 2:
         switch( typeOfIRTemp(irsb->tyenv, dst) ) {
            case Ity_V512: assign(dst, binop(Iop_V256HLtoV512, mkexpr(s[1]), mkexpr(s[0]))); return;
            case Ity_V256: assign(dst, binop(Iop_V128HLtoV256, mkexpr(s[1]), mkexpr(s[0]))); return;
            case Ity_V128: assign(dst, binop(Iop_64HLtoV128,   mkexpr(s[1]), mkexpr(s[0]))); return;
            case Ity_I64:  assign(dst, binop(Iop_32HLto64,     mkexpr(s[1]), mkexpr(s[0]))); return;
            case Ity_I32:  assign(dst, binop(Iop_16HLto32,     mkexpr(s[1]), mkexpr(s[0]))); return;
            case Ity_I16:  assign(dst, binop(Iop_8HLto16,      mkexpr(s[1]), mkexpr(s[0]))); return;
            default: vpanic("unsupported 2x multiplier");
         }
      case 4:
#define merge_4( iop_half, iop_quarter ) \
         assign( dst, binop( iop_half, \
                  binop(iop_quarter, mkexpr(s[3]), mkexpr(s[2])), \
                  binop(iop_quarter, mkexpr(s[1]), mkexpr(s[0]))) );
         switch( typeOfIRTemp(irsb->tyenv, dst) ) {
            case Ity_V512: merge_4( Iop_V256HLtoV512, Iop_V128HLtoV256); return;
            case Ity_V256: merge_4( Iop_V128HLtoV256, Iop_64HLtoV128);   return;
            case Ity_V128: merge_4( Iop_64HLtoV128,   Iop_32HLto64);     return;
            case Ity_I64:  merge_4( Iop_32HLto64,     Iop_16HLto32);     return;
            case Ity_I32:  merge_4( Iop_16HLto32,     Iop_8HLto16);      return;
            default: vpanic("unsupported 4x multiplier");
#undef merge_4
         }
      case 8:
#define merge_8( iop_half, iop_quarter, iop_oct ) \
         assign( dst, binop(iop_half,\
                  binop(iop_quarter, \
                     binop(iop_oct, mkexpr(s[7]), mkexpr(s[6])), \
                     binop(iop_oct, mkexpr(s[5]), mkexpr(s[4]))), \
                  binop(iop_quarter, \
                     binop(iop_oct, mkexpr(s[3]), mkexpr(s[2])), \
                     binop(iop_oct, mkexpr(s[1]), mkexpr(s[0])))) );
         switch( typeOfIRTemp(irsb->tyenv, dst) ) {
            case Ity_V512: merge_8(Iop_V256HLtoV512, Iop_V128HLtoV256, Iop_64HLtoV128); return;
            case Ity_V256: merge_8(Iop_V128HLtoV256, Iop_64HLtoV128,   Iop_32HLto64);   return;
            case Ity_V128: merge_8(Iop_64HLtoV128,   Iop_32HLto64,     Iop_16HLto32);   return;
            case Ity_I64:  merge_8(Iop_32HLto64,     Iop_16HLto32,     Iop_8HLto16);    return;
            default: vpanic("unsupported 8x multiplier");
#undef merge_8
         }
      case 16:
#define merge_16( iop_half, iop_quarter, iop_oct, iop_sed) \
         assign( dst, binop(iop_half,\
                  binop(iop_quarter, \
                     binop(iop_oct, \
                        binop(iop_sed, mkexpr(s[15]), mkexpr(s[14])), \
                        binop(iop_sed, mkexpr(s[13]), mkexpr(s[12]))), \
                     binop(iop_oct, \
                        binop(iop_sed, mkexpr(s[11]), mkexpr(s[10])), \
                        binop(iop_sed, mkexpr(s[ 9]), mkexpr(s[8])))), \
                  binop(iop_quarter, \
                     binop(iop_oct, \
                        binop(iop_sed, mkexpr(s[7]), mkexpr(s[6])), \
                        binop(iop_sed, mkexpr(s[5]), mkexpr(s[4]))), \
                     binop(iop_oct, \
                        binop(iop_sed, mkexpr(s[3]), mkexpr(s[2])), \
                        binop(iop_sed, mkexpr(s[1]), mkexpr(s[0]))))));
         switch( typeOfIRTemp(irsb->tyenv, dst) ) {
            case Ity_V512: merge_16(Iop_V256HLtoV512, Iop_V128HLtoV256, Iop_64HLtoV128, Iop_32HLto64); return;
            case Ity_V256: merge_16(Iop_V128HLtoV256, Iop_64HLtoV128,   Iop_32HLto64,   Iop_16HLto32); return;
            case Ity_V128: merge_16(Iop_64HLtoV128,   Iop_32HLto64,     Iop_16HLto32,   Iop_8HLto16);  return;
            default: vpanic("unsupported 16x multiplier");
         }
      case 32:
         switch( typeOfIRTemp(irsb->tyenv, dst) ) {
            case Ity_V512:
               assign( dst, binop(Iop_V256HLtoV512,
                        binop(Iop_V128HLtoV256, binop(Iop_64HLtoV128, binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[31]), mkexpr(s[30])),
                                 binop(Iop_16HLto32, mkexpr(s[29]), mkexpr(s[28]))), binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[27]), mkexpr(s[26])),
                                 binop(Iop_16HLto32, mkexpr(s[25]), mkexpr(s[24])))), binop(Iop_64HLtoV128, binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[23]), mkexpr(s[22])),
                                 binop(Iop_16HLto32, mkexpr(s[21]), mkexpr(s[20]))), binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[19]), mkexpr(s[18])),
                                 binop(Iop_16HLto32, mkexpr(s[17]), mkexpr(s[16]))))),
                        binop(Iop_V128HLtoV256, binop(Iop_64HLtoV128, binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[15]), mkexpr(s[14])),
                                 binop(Iop_16HLto32, mkexpr(s[13]), mkexpr(s[12]))), binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[11]), mkexpr(s[10])),
                                 binop(Iop_16HLto32, mkexpr(s[9]),  mkexpr(s[8])))), binop(Iop_64HLtoV128, binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[7]),  mkexpr(s[6])),
                                 binop(Iop_16HLto32, mkexpr(s[5]),  mkexpr(s[4]))), binop(Iop_32HLto64,
                                 binop(Iop_16HLto32, mkexpr(s[3]),  mkexpr(s[2])),
                                 binop(Iop_16HLto32, mkexpr(s[1]),  mkexpr(s[0])))))) );
               return;
            case Ity_V256:
               assign( dst, binop(Iop_V128HLtoV256,
                        binop(Iop_64HLtoV128, binop(Iop_32HLto64, binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[31]), mkexpr(s[30])),
                                 binop(Iop_8HLto16, mkexpr(s[29]), mkexpr(s[28]))), binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[27]), mkexpr(s[26])),
                                 binop(Iop_8HLto16, mkexpr(s[25]), mkexpr(s[24])))), binop(Iop_32HLto64, binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[23]), mkexpr(s[22])),
                                 binop(Iop_8HLto16, mkexpr(s[21]), mkexpr(s[20]))), binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[19]), mkexpr(s[18])),
                                 binop(Iop_8HLto16, mkexpr(s[17]), mkexpr(s[16]))))),
                        binop(Iop_64HLtoV128, binop(Iop_32HLto64, binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[15]), mkexpr(s[14])),
                                 binop(Iop_8HLto16, mkexpr(s[13]), mkexpr(s[12]))), binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[11]), mkexpr(s[10])),
                                 binop(Iop_8HLto16, mkexpr(s[9]),  mkexpr(s[8])))), binop(Iop_32HLto64, binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[7]),  mkexpr(s[6])),
                                 binop(Iop_8HLto16, mkexpr(s[5]),  mkexpr(s[4]))), binop(Iop_16HLto32,
                                 binop(Iop_8HLto16, mkexpr(s[3]),  mkexpr(s[2])),
                                 binop(Iop_8HLto16, mkexpr(s[1]),  mkexpr(s[0])))))) );
               return;
            default: vpanic("unsupported 32x multiplier");
         }
      case 64: {
         vassert (typeOfIRTemp(irsb->tyenv, dst) == Ity_V512);
         assign( dst, binop(Iop_V256HLtoV512,
                  binop(Iop_V128HLtoV256,
                     binop(Iop_64HLtoV128, binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[63]), mkexpr(s[62])),
                              binop(Iop_8HLto16, mkexpr(s[61]), mkexpr(s[60]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[59]), mkexpr(s[58])),
                              binop(Iop_8HLto16, mkexpr(s[57]), mkexpr(s[56])))), binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[55]), mkexpr(s[54])),
                              binop(Iop_8HLto16, mkexpr(s[53]), mkexpr(s[52]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[51]), mkexpr(s[50])),
                              binop(Iop_8HLto16, mkexpr(s[49]), mkexpr(s[48]))))),
                     binop(Iop_64HLtoV128, binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[47]), mkexpr(s[46])),
                              binop(Iop_8HLto16, mkexpr(s[45]), mkexpr(s[44]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[43]), mkexpr(s[42])),
                              binop(Iop_8HLto16, mkexpr(s[41]), mkexpr(s[40])))), binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[39]), mkexpr(s[38])),
                              binop(Iop_8HLto16, mkexpr(s[37]), mkexpr(s[36]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[35]), mkexpr(s[34])),
                              binop(Iop_8HLto16, mkexpr(s[33]), mkexpr(s[32])))))),
                  binop(Iop_V128HLtoV256,
                     binop(Iop_64HLtoV128, binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[31]), mkexpr(s[30])),
                              binop(Iop_8HLto16, mkexpr(s[29]), mkexpr(s[28]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[27]), mkexpr(s[26])),
                              binop(Iop_8HLto16, mkexpr(s[25]), mkexpr(s[24])))), binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[23]), mkexpr(s[22])),
                              binop(Iop_8HLto16, mkexpr(s[21]), mkexpr(s[20]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[19]), mkexpr(s[18])),
                              binop(Iop_8HLto16, mkexpr(s[17]), mkexpr(s[16]))))),
                     binop(Iop_64HLtoV128, binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[15]), mkexpr(s[14])),
                              binop(Iop_8HLto16, mkexpr(s[13]), mkexpr(s[12]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[11]), mkexpr(s[10])),
                              binop(Iop_8HLto16, mkexpr(s[9]),  mkexpr(s[8])))), binop(Iop_32HLto64, binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[7]),  mkexpr(s[6])),
                              binop(Iop_8HLto16, mkexpr(s[5]),  mkexpr(s[4]))), binop(Iop_16HLto32,
                              binop(Iop_8HLto16, mkexpr(s[3]),  mkexpr(s[2])),
                              binop(Iop_8HLto16, mkexpr(s[1]),  mkexpr(s[0]))))))) );
         return;
      }
      default: vex_printf("%d ", multiplier);
               vpanic("unsupported merge multiplier");
   }
}

static Bool suppressMemFaults (enum exception_class exc) {
   switch (exc) {
      case EXC_UNDEF: case E1: case E2: case E3: case E4: case E4nb:
      case E5: case E6: case E10: case E11: case E12: case E12NP:
         return True;
      case E1NF: case E3NF: case E4NF: case E4NFnb:
      case E5NF: case E6NF: case E9NF: case E10NF:
      case E6BC:
         return False;
      case E7NM: case E7NM128:
         vpanic("Loading memory in a register-only instruciton");
      default: vpanic("Unknown instruction exception type");
   }
}

/* Only access what instruction mask allows to access, to avoid Memcheck confusion */
static IRExpr* loadLE_masked (
      IRType src_ty, IRExpr* addr, IRExpr* mask, enum operand_width width )
{
   IRType elem_ty = Ity_I8 + width; // well enough
   IRExpr *dummy = NULL;
   Int addr_inc = 0;
   IRLoadGOp load_sz = ILGop_INVALID;
   switch (elem_ty) {
      case Ity_I8:  dummy = mkU8(0);  addr_inc = 0x1; load_sz = ILGop_Ident8;  break;
      case Ity_I16: dummy = mkU16(0); addr_inc = 0x2; load_sz = ILGop_Ident16; break;
      case Ity_I32: dummy = mkU32(0); addr_inc = 0x4; load_sz = ILGop_Ident32; break;
      case Ity_I64: dummy = mkU64(0); addr_inc = 0x8; load_sz = ILGop_Ident64; break;
      default: vpanic("Unimplemented masked load type");
   }

   IRTemp res[MAX_MULTIPLIER];
   Int count = sizeofIRType(src_ty) / sizeofIRType(elem_ty);
   // For each element, calculate its address and generate a guarded load
   for (Int i = 0; i < count; i++) {
      res[i] = newTemp(elem_ty);
      IRExpr* guard = unop(Iop_64to1, binop(Iop_Shr64, mask, mkU8(i)));
      stmt( IRStmt_LoadG(Iend_LE, load_sz,
                         res[i],
                         binop(Iop_Add64, addr, mkU64(i*addr_inc)),
                         dummy,
                         guard));
   }
   IRTemp dst = newTemp(src_ty);
   Merge_dst(dst, res, count);
   return mkexpr(dst);
}

static void storeLE_masked (
      IRExpr* addr, IRExpr* data, IRExpr* mask, enum operand_width width )
{
   IRType dst_ty = typeOfIRExpr(irsb->tyenv, data);
   IRType elem_ty = Ity_I8 + width; // well enough
   Int addr_inc = 0;
   switch (width) {
      case W_8:  addr_inc = 0x1;  break;
      case W_16: addr_inc = 0x2; break;
      case W_32: addr_inc = 0x4; break;
      case W_64: addr_inc = 0x8; break;
      default: vpanic("Unsupported masked store width");
   }

   // Split the data
   Int count = sizeofIRType(dst_ty) / sizeofIRType(elem_ty);
   IRTemp src = newTemp(dst_ty);
   assign(src, data);
   IRTemp res[MAX_MULTIPLIER];
   for (Int i = 0; i < count; i++) {
      res[i] = IRTemp_INVALID;
   }
   Split_arg(src, res, count);

   // For each element, calculate its address and generate a guarded store
   for (Int i = 0; i < count; i++) {
      IRExpr* guard = unop(Iop_64to1, binop(Iop_Shr64, mask, mkU8(i)));
      stmt( IRStmt_StoreG(
               Iend_LE,
               binop(Iop_Add64, addr, mkU64(i*addr_inc)),
               mkexpr(res[i]),
               guard));
   }
}

static Long Get_Instr_Args(
      const VexAbiInfo* vbi, Prefix pfx, Long delta,
      const UInt ins_id, IRTemp* arg, UInt* arity, UChar* int_val,
      UInt rG, UInt rV, UInt *rE_back, IRTemp* addr, UChar modrm)
{
   HChar dis_buf[50];
   Int alen = 0;
   UInt rE = 0;

   if (epartIsReg(modrm)) {
      rE = eregOfRexRM32(pfx, modrm);
      *rE_back = rE;
      delta += 1;
   } else {
      Int imm8_is_present = 0;
      for (Int i=0; i<5; i++)
         if (INS_ARR[ins_id].args[i] == _imm8)
            imm8_is_present = 1;
      *addr = disAMode( &alen, vbi, pfx, delta, dis_buf, imm8_is_present );
      delta += alen;
   }

   // Read the destination (args[0]) as well, to use it for merge-masking
#define REG_ARG(get, name, reg)  {DIP("%s ",name(reg)); assign(arg[i],get(reg));}
   for (Int i=0; i<5; i++) {
      enum op_encoding op_code = INS_ARR[ins_id].args[i];
      if (op_code == _none) {
         (*arity)--; // not an argument
         continue;
      }

      IRType op_type = INS_ARR[ins_id].arg_type[i];
      arg[i] = newTemp(op_type);

      // imm8 is a special case
      if (op_code == _imm8) {
         *int_val = getUChar(delta++);
         assign(arg[i], mkU8(*int_val));
         DIP("imm8 %d", *int_val);
         continue;
      }

      if (op_code == _rmode) {
         assign(arg[i], get_evex_roundingmode());
         DIP("rmode");
         continue;
      }

      // memory reference
      switch (op_code) {
         case _rmE: case _imE: case _kmE:
            if (epartIsReg(modrm)) 
               break;
            /* fall through */
         case _mG: case _mE: {
            if (getEvexb()) { // embedded memory broadcast
               //TODO test 8 and 16 bit versions
               IRType width = getRexW(pfx) ? Ity_I64 : Ity_I32;
               Int broadcast_times = sizeofIRType(op_type)/sizeofIRType(width);
               IRTemp elem = newTemp(width);
               assign (elem, loadLE(width, mkexpr(*addr)));
               arg[i] = m_Broadcast(elem, elem, broadcast_times);
               DIP("%s (1to%d)", dis_buf, broadcast_times);
               continue;
            } else {
               Int mask = getEvexMask();
               if (mask && (i == 0)) // masked memory destination
                  assign(arg[i], loadLE_masked(op_type, mkexpr(*addr), getKReg(mask), getDstW()));
               else
                  if (mask && suppressMemFaults(INS_ARR[ins_id].exc)) {
                     // memory source, and we should not read masked off elements
                     // calculate memory element size
                     Int width = getDstW();
                     switch (getTupleType()) {
                        case OctMem:      width--; /* fall through */
                        case QuarterMem:  width--; /* fall through */
                        case HalfVector:
                        case HalfMem:     width--; break;
                        default: break;
                     }
                     assign(arg[i], loadLE_masked( op_type, mkexpr(*addr), getKReg(mask), width));
                  }
                  else // full memory read - no mask or no fault suppression
                     assign(arg[i], loadLE( op_type, mkexpr(*addr)));
               DIP("%s ", dis_buf);
               continue;
            }
         }
         default: break; // continue to register operands
      }

      UInt reg = 0;
      switch (op_code) {
         case _rG: case _iG: case _kG: reg = rG; break;
         case _rV: case _kV:           reg = rV; break;
         case _rmE: case _imE: case _kmE:
         case _rE: case _iE: case _kE: reg = rE; break;
         default: vpanic("Unrecognizes register operand");
      }
      // scalar register
      if ((op_code == _iG) || (op_code == _iE) || (op_code == _imE)) {
         switch (op_type) {
            case Ity_I8:
               assign(arg[i], unop(Iop_16to8, getIReg16(reg)));
               DIP("%s ", nameIReg16(reg));
               break;
            case Ity_I16: REG_ARG (getIReg16, nameIReg16, reg); break;
            case Ity_I32: REG_ARG (getIReg32, nameIReg32, reg); break;
            case Ity_I64: REG_ARG (getIReg64, nameIReg64, reg); break;
            default: vpanic("\nunimplemented scalar register size");
         }
         continue;
      }
      // mask register
      if ((op_code == _kE) || (op_code == _kG) || (op_code == _kV) || (op_code == _kmE)) {
         DIP("%s ", nameKReg(reg));
         switch (op_type) {
            case Ity_I64: assign(arg[i], getKReg(reg)); break;
            case Ity_I32: assign(arg[i], getKRegLane32(reg, 0)); break;
            case Ity_I16: assign(arg[i], getKRegLane16(reg, 0)); break;
            case Ity_I8:  assign(arg[i], getKRegLane8(reg, 0)); break;
            default:      vpanic ("unknown mask length");
         }
         continue;
      }

      // vector register as the only remaining option
      switch (op_type) {
         case Ity_V512: REG_ARG (getZMMReg, nameZMMReg, reg); break;
         case Ity_V256: REG_ARG (getYMMReg, nameYMMReg, reg); break;
         case Ity_V128: REG_ARG (getXMMReg, nameXMMReg, reg); break;
         case Ity_I64:  assign(arg[i], getXMMRegLane64(reg, 0));
                        DIP("%s ", nameXMMReg(reg));
                        break;
         case Ity_I32:  assign(arg[i], getXMMRegLane32(reg, 0));
                        DIP("%s ", nameXMMReg(reg));
                        break;
         case Ity_I16:  assign(arg[i], getXMMRegLane16(reg, 0));
                        DIP("%s ", nameXMMReg(reg));
                        break;
         case Ity_I8:   assign(arg[i], unop(Iop_16to8, getXMMRegLane16(reg, 0)));
                        DIP("%s ", nameXMMReg(reg));
                        break;
         default:       vpanic ("unknown vector length");
      }
   }
#undef REG_ARG
   return delta;
}


static void Put_Instr_Result(Int ins_id, IRExpr* masked,
      UInt rG, UInt rV, UInt rE, IRTemp addr, UChar modrm)
{
   enum op_encoding op_code = INS_ARR[ins_id].args[0];

   // memory store
   switch (INS_ARR[ins_id].args[0]) {
      case _rmE: case _imE: case _kmE:
         if (epartIsReg(modrm)) 
            break;
         /* fall through */
      case _mG: case _mE: {
         Int mask = getEvexMask();
         if (mask) 
            storeLE_masked( mkexpr(addr), masked, getKReg(mask), getDstW());
         else
            storeLE( mkexpr(addr), masked );
         return;
      }
      default: break; // continue to register operands
   }

   // register store
   UInt reg = 0;
   switch (INS_ARR[ins_id].args[0]) {
      case _rG: case _iG: case _kG:  reg = rG; break;
      case _rmE: case _imE: case _kmE:
      case _rE: case _iE: case _kE:  reg = rE; break;
      case _rV:                      reg = rV; break;
      default: vpanic("unknown destination register\n");
   }

   IRType op_type = INS_ARR[ins_id].arg_type[0];

   // scalar register
   if ((op_code == _iG) || (op_code == _iE) || (op_code == _imE)) {
      switch (op_type) { // put the data
         case Ity_I8:  putIReg16( reg, unop(Iop_8Uto16, masked) ); break;
         case Ity_I16: putIReg16( reg, masked ); break;
         case Ity_I32: putIReg32( reg, masked ); break;
         case Ity_I64: putIReg64( reg, masked ); break;
         default: vpanic("Unrecognized scalar register size");
      }
      return;
   }
   // mask register
   if ((op_code == _kE) || (op_code == _kG) || (op_code == _kmE)) {
      switch (op_type) { // put the data
         case Ity_I8:  putKRegLane8(reg, 0, masked);  break;
         case Ity_I16: putKRegLane16(reg, 0, masked); break;
         case Ity_I32: putKRegLane32(reg, 0, masked); break;
         case Ity_I64: putKReg(reg, masked);          break;
         default: vpanic("Unrecognized mask register size");
      }
      return;
   }
   // vector register

   putZMMReg(reg, mkV512(0)); // zero out the entire register beforehand
   switch (op_type) { // put the data
      case Ity_I16:  stmt(IRStmt_Put( xmmGuestRegLane16offset(reg, 0), masked)); break;
      case Ity_I32:  putZMMRegLane32 (reg, 0, masked); break;
      case Ity_I64:  putZMMRegLane64 (reg, 0, masked); break;
      case Ity_V128: putZMMRegLane128(reg, 0, masked); break;
      case Ity_V256: putZMMRegLane256(reg, 0, masked); break;
      case Ity_V512: putZMMReg( reg, masked ); break;
      default: vpanic("Unrecognized vector register size");
   }

   // It is a tempting idea to put rV in the remainder of xmm in serial cases
   // But it does not work for FMA instructions, so handle it at IR construction instead
   return;
}



static Long dis_FMA_512 ( const VexAbiInfo* vbi, Prefix pfx, Long delta, Int ins_id )
{
   /* determine instruction type and parameters */
   UChar opc = INS_ARR[ins_id].opcode;
   Bool scalar = ((opc & 0xF) > 7 && (opc & 1));

   Bool negateRes   = False;
   Bool negateZeven = False;
   Bool negateZodd  = False;
   switch (opc & 0xF) {
      case 0x6:           negateZeven = True; break;
      case 0x7:           negateZodd = True;  break;
      case 0x8: case 0x9: break;
      case 0xA: case 0xB: negateZeven = True; negateZodd = True; break;
      case 0xC: case 0xD: negateRes = True; negateZeven = True; negateZodd = True; break;
      case 0xE: case 0xF: negateRes = True; break;
      default:  vpanic("dis_FMA_512(amd64)");
   }
   Bool is_64 = (getDstW() == W_64);
   IRType original_type = is_64 ? Ity_I64     : Ity_I32;
   IROp   negate       = is_64 ? Iop_NegF64  : Iop_NegF32;
   IROp   mult_add     = is_64 ? Iop_MAddF64 : Iop_MAddF32;
   IROp   reinterprI2F = is_64 ? Iop_ReinterpI64asF64 : Iop_ReinterpI32asF32;
   IROp   reinterprF2I = is_64 ? Iop_ReinterpF64asI64 : Iop_ReinterpF32asI32;
   IROp   get_lowest   = is_64 ? Iop_V128to64 : Iop_V128to32;
   IROp   merge_lowest = is_64 ? Iop_SetV128lo64 : Iop_SetV128lo32;
   UInt element_count = sizeofIRType(INS_ARR[ins_id].arg_type[0]) / sizeofIRType(original_type);
   DIP(" %s ", INS_ARR[ins_id].name);

   /* retrieve arguments. Assuming the master file has them in dst, 1, 2, 3 order */
   IRTemp arg[MAX_ARITY];
   IRTemp s[MAX_ARITY][MAX_ELEMENTS];
   IRExpr* expr_s[MAX_ARITY][MAX_ELEMENTS];
   UInt arity = 4; // MUST be so!
   IRTemp res[MAX_ELEMENTS];
   vassert(element_count <= MAX_ELEMENTS);

   Long delta_out = delta;
   UChar modrm = getUChar(delta_out);
   UInt rG = gregOfRexRM32(pfx, modrm);
   UInt rV = getEVexNvvvv(pfx);
   UInt rE = 0;
   IRTemp addr = IRTemp_INVALID;
   UChar int_val = 0;
   delta_out = Get_Instr_Args(vbi, pfx, delta_out, ins_id, arg, &arity, &int_val, rG, rV, &rE, &addr, modrm);

   IRTemp unmasked = newTemp(INS_ARR[ins_id].arg_type[0]);
   if (scalar)
   {
      for (Int i = 1; i <= arity; i++)
         expr_s[i][0] = unop(reinterprI2F, unop(get_lowest, mkexpr(arg[i])));
      if (negateZodd)
         expr_s[3][0] = unop(negate, expr_s[3][0]);
      expr_s[0][0] = IRExpr_Qop(mult_add, get_evex_roundingmode(), expr_s[1][0], expr_s[2][0], expr_s[3][0]);
      if (negateRes)
         expr_s[0][0] = unop(negate, expr_s[0][0]);
      assign(unmasked, binop(merge_lowest, mkexpr(arg[0]), unop(reinterprF2I, expr_s[0][0])));
   }
   else
   {
      // NOTE arity is 3 now
      /* Split the sources */
      for (Int i = 1; i <= arity; i++) {
         for (Int j = 0; j < element_count; j++)
            s[i][j] = IRTemp_INVALID;
         Split_arg(arg[i], s[i], element_count);
         for (Int j = 0; j < element_count; j++)
            expr_s[i][j] = unop(reinterprI2F, mkexpr(s[i][j]));
      }

      /* Multiply-add and negate values */
      for (Int j = 0; j < element_count; j++) {
         if ((j & 1) ? negateZodd : negateZeven)
            expr_s[3][j] = unop(negate, expr_s[3][j]);
         expr_s[0][j] = IRExpr_Qop(mult_add, get_evex_roundingmode(), expr_s[1][j], expr_s[2][j], expr_s[3][j]);
         if (negateRes)
            expr_s[0][j] = unop(negate, expr_s[0][j]);
         res[j] = newTemp(original_type);
         assign(res[j], unop(reinterprF2I, expr_s[0][j]));
      }
      /* Merge back */
      Merge_dst(unmasked, res, element_count);
   }
   IRExpr* masked = mask_expr(pfx, mkexpr(unmasked), mkexpr(arg[0]), ins_id);
   Put_Instr_Result(ins_id, masked, rG, rV, rE, addr, modrm);
   return delta_out;
}


static ULong dis_GATHER_512 ( const VexAbiInfo* vbi, Prefix pfx, Long delta,
      UInt rG, UInt ins_id)
{
   HChar  dis_buf[50] = {0};
   Int    alen=0, i=0, vscale=0;
   IRTemp addr = IRTemp_INVALID;
   UInt   rI = 0;
   UInt   mask = getEvexMask();
   IRExpr* address = NULL;

   addr = disAVSIBMode ( &alen, vbi, pfx, delta, dis_buf,
         &rI, INS_ARR[ins_id].arg_type[1], &vscale );
   delta += alen;
   DIP (" %s %s,", INS_ARR[ins_id].name, dis_buf);
   switch (INS_ARR[ins_id].arg_type[0]) {
      case Ity_V512: DIP("%s", nameZMMReg(rG)); break;
      case Ity_V256: DIP("%s", nameYMMReg(rG)); break;
      case Ity_V128:
      case Ity_I64:  DIP("%s", nameXMMReg(rG)); break;
      default: vpanic("unimplemented VGATHER type");
   }
   DIP (",%s\n", nameKReg(mask));

   IRType reg_w = (getDstW() == W_64) ? Ity_I64 : Ity_I32;
   IRType ind_w = (INS_ARR[ins_id].args[1] == _vm64) ? Ity_I64 : Ity_I32;
   Int count = sizeofIRType(INS_ARR[ins_id].arg_type[0]) / sizeofIRType(reg_w);

   for (i = 0; i < count; i++) {
      address = (ind_w == Ity_I64) ?
         getZMMRegLane64(rI, i) :
         unop(Iop_32Sto64, getZMMRegLane32(rI, i));
      switch (vscale) {
         case 1: break;
         case 2: address = binop(Iop_Shl64, address, mkU8(1)); break;
         case 4: address = binop(Iop_Shl64, address, mkU8(2)); break;
         case 8: address = binop(Iop_Shl64, address, mkU8(3)); break;
         default: vpanic("unimplemented gather vscale");
      }
      address = binop(Iop_Add64, mkexpr(addr), address);
      address = handleAddrOverrides(vbi, pfx, address);

      // Difference form AVX2 gathers: use guarded loads instead of conditionals
      IRExpr* guard = unop(Iop_64to1, binop(Iop_Shr64, getKReg(mask), mkU8(i)));
      if (reg_w == Ity_I64) {
         IRTemp loaded_val = newTemp(Ity_I64);
         stmt( IRStmt_LoadG(Iend_LE, ILGop_Ident64,
                  loaded_val,
                  address,
                  getZMMRegLane64(rG, i),
                  guard));
         putZMMRegLane64(rG, i, mkexpr(loaded_val));
      } else {
         IRTemp loaded_val = newTemp(Ity_I32);
         stmt( IRStmt_LoadG(Iend_LE, ILGop_Ident32,
                  loaded_val,
                  address,
                  getZMMRegLane32(rG, i),
                  guard));
         putZMMRegLane32(rG, i, mkexpr(loaded_val));
      }
   }

   // Fill the upper part of result vector register.
   switch (INS_ARR[ins_id].arg_type[0]) {
      case Ity_I64:  putZMMRegLane64 ( rG, 1, mkU64(0) );  /*fallthru*/
      case Ity_V128: putZMMRegLane128( rG, 1, mkV128(0) ); /*fallthru*/
      case Ity_V256: putZMMRegLane256( rG, 1, mkV256(0) );
      default: break;
   }

   putKReg(mask, mkU64(0));
   return delta;
}

static ULong dis_SCATTER_512 ( const VexAbiInfo* vbi, Prefix pfx, Long delta,
      UInt rG, UInt ins_id)
{
   HChar  dis_buf[50] = {0};
   Int    alen=0, i=0, vscale=0;
   IRTemp addr = IRTemp_INVALID;
   UInt   rI = 0;
   UInt   mask = getEvexMask();
   IRExpr* address = NULL;

   addr = disAVSIBMode ( &alen, vbi, pfx, delta, dis_buf,
         &rI, INS_ARR[ins_id].arg_type[0], &vscale );
   delta += alen;

   DIP (" %s \n", INS_ARR[ins_id].name);
   switch (INS_ARR[ins_id].arg_type[1]) {
      case Ity_V512: DIP("%s", nameZMMReg(rG)); break;
      case Ity_V256: DIP("%s", nameYMMReg(rG)); break;
      case Ity_V128:
      case Ity_I64:  DIP("%s", nameXMMReg(rG)); break;
      default: vpanic("unimplemented VSCATTER type");
   }
   DIP (",%s,%s\n", dis_buf, nameKReg(mask));

   IRType ind_w = (INS_ARR[ins_id].args[0] == _vm64) ? Ity_I64 : Ity_I32;
   IRType reg_w = (getDstW() == W_64) ? Ity_I64 : Ity_I32;
   Int count = sizeofIRType(INS_ARR[ins_id].arg_type[1]) / sizeofIRType(reg_w);

   for (i = 0; i < count; i++) {
      address = (ind_w == Ity_I64) ?
         getZMMRegLane64(rI, i) :
         unop(Iop_32Sto64, getZMMRegLane32(rI, i));
      switch (vscale) {
         case 1: break;
         case 2: address = binop(Iop_Shl64, address, mkU8(1)); break;
         case 4: address = binop(Iop_Shl64, address, mkU8(2)); break;
         case 8: address = binop(Iop_Shl64, address, mkU8(3)); break;
         default: vpanic("unimplemented scatter vscale");
      }
      address = binop(Iop_Add64, mkexpr(addr), address);
      address = handleAddrOverrides(vbi, pfx, address);

      // Difference form AVX2 gathers: use guarded stores instead of conditionals
      IRExpr* guard = unop(Iop_64to1, binop(Iop_Shr64, getKReg(mask), mkU8(i)));
      stmt( IRStmt_StoreG( Iend_LE, address,
               (reg_w == Ity_I64) ? getZMMRegLane64(rG, i) : getZMMRegLane32(rG, i),
               guard));
   }

   putKReg(mask, mkU64(0));
   return delta;
}

enum shift_type {SHL=0, SHR=1, SAR=2};

/* Vector shift by a constant.
 * The constant might be imm8 or the lowest bits of an xmm vector register */
static Long dis_shift_ser_512 ( const VexAbiInfo* vbi, Prefix pfx, Long delta, Int ins_id )
{
   /* retrieve arguments */
   UInt element_count = sizeofIRType(INS_ARR[ins_id].arg_type[0]) / sizeofIRType(Ity_V128);
   IRTemp arg[MAX_ARITY];
   IRTemp s[MAX_ARITY][MAX_ELEMENTS];
   IRTemp res[MAX_ELEMENTS];
   vassert(element_count <= MAX_ELEMENTS);
   Long delta_out = delta;
   UChar modrm = getUChar(delta_out);
   UInt rG = gregOfRexRM32(pfx, modrm);
   UInt rV = getEVexNvvvv(pfx);
   UInt rE = 0;
   IRTemp addr = IRTemp_INVALID;
   UChar int_val = 0;
   UInt arity = 3;
   DIP(" %s ", INS_ARR[ins_id].name);
   delta_out = Get_Instr_Args(vbi, pfx, delta_out, ins_id, arg, &arity, &int_val, rG, rV, &rE, &addr, modrm);

   /* determine a matching 128-bit IR */
   UInt width = getDstW();
   UInt size = ipow(2, (width+3));
   IROp shift_op = Iop_INVALID;
   switch (INS_ARR[ins_id].parameter) {
      case SHL:
         switch (width) {
            case W_8:  shift_op = Iop_ShlN8x16; break;
            case W_16: shift_op = Iop_ShlN16x8; break;
            case W_32: shift_op = Iop_ShlN32x4; break;
            case W_64: shift_op = Iop_ShlN64x2; break;
            default: vpanic("unsupported shift width");
         }
         break;
      case SHR:
         switch (width) {
            case W_8:  shift_op = Iop_ShrN8x16; break;
            case W_16: shift_op = Iop_ShrN16x8; break;
            case W_32: shift_op = Iop_ShrN32x4; break;
            case W_64: shift_op = Iop_ShrN64x2; break;
            default: vpanic("unsupported shift width");
         }
         break;
      case SAR:
         switch (width) {
            case W_8:  shift_op = Iop_SarN8x16; break;
            case W_16: shift_op = Iop_SarN16x8; break;
            case W_32: shift_op = Iop_SarN32x4; break;
            case W_64: shift_op = Iop_SarN64x2; break;
            default: vpanic("unsupported shift width");
         }
         break;
      default: vpanic("unsupported shift type");
   }

   /* Split the 1st source into 128-bit parts */
   for (Int j=0; j<element_count; j++) {
      s[1][j] = IRTemp_INVALID;
      res[j]  = newTemp(Ity_V128);
   }
   Split_arg(arg[1], s[1], element_count);
   
   if (int_val > 0) {
      /* imm8 case */ 
      if (INS_ARR[ins_id].parameter == SAR) {
         for (Int j=0; j<element_count; j++) {
            assign( res[j], IRExpr_ITE(binop(Iop_CmpLT64U, mkU64(int_val), mkU64(size)),
                     binop(shift_op, mkexpr(s[1][j]), mkU8(int_val)),
                     binop(shift_op, mkexpr(s[1][j]), mkU8(size-1))) );
         }
      } else {
         for (Int j=0; j<element_count; j++) {
            assign( res[j], IRExpr_ITE( binop(Iop_CmpLT64U, mkU64(int_val), mkU64(size)),
                     binop(shift_op, mkexpr(s[1][j]), mkU8(int_val)),
                     mkV128(0)) );
         }
      }
   } else {
      /* Lowest vector bits */
      IRTemp amt64 = newTemp(Ity_I64);
      assign(amt64, unop(Iop_V128to64, mkexpr(arg[2])));
      if (INS_ARR[ins_id].parameter == SAR) {
         for (Int j=0; j<element_count; j++) {
            assign( res[j], IRExpr_ITE(binop(Iop_CmpLT64U, mkexpr(amt64), mkU64(size)),
                     binop(shift_op, mkexpr(s[1][j]), unop(Iop_64to8, mkexpr(amt64))),
                     binop(shift_op, mkexpr(s[1][j]), mkU8(size-1))) );
         }
      } else {
         for (Int j=0; j<element_count; j++) {
            assign( res[j], IRExpr_ITE( binop(Iop_CmpLT64U, mkexpr(amt64), mkU64(size)),
                     binop(shift_op, mkexpr(s[1][j]), unop(Iop_64to8, mkexpr(amt64))),
                     mkV128(0)) );
         }
      }
   }

   /* Merge back and mask the result */
   IRTemp unmasked = newTemp(INS_ARR[ins_id].arg_type[0]);
   Merge_dst(unmasked, res, element_count);
   IRExpr* masked = mask_expr(pfx, mkexpr(unmasked), mkexpr(arg[0]), ins_id);

   Put_Instr_Result(ins_id, masked, rG, rV, rE, addr, modrm);
   return delta_out;
}

/* Vector by vector shift */
static Long dis_shift_vec_512 ( const VexAbiInfo* vbi, Prefix pfx, Long delta, Int ins_id )
{
   /* determine instruction type and parameters */
   IRType part_ty = Ity_INVALID;
   IRExpr* shift_max = NULL;
   UInt width = getDstW();
   switch (width) {
      case W_8:  part_ty = Ity_I8;  shift_max = mkU64(0x8);  break;
      case W_16: part_ty = Ity_I16; shift_max = mkU64(0x10); break;
      case W_32: part_ty = Ity_I32; shift_max = mkU64(0x20); break;
      case W_64: part_ty = Ity_I64; shift_max = mkU64(0x40); break;
      default: vpanic("unsupported shift width");
   }
   IROp shift_op = Iop_INVALID;
   switch (INS_ARR[ins_id].parameter) {
      case SHL:
         switch (width) {
            case W_8:  shift_op = Iop_Shl8;  break;
            case W_16: shift_op = Iop_Shl16; break;
            case W_32: shift_op = Iop_Shl32; break;
            case W_64: shift_op = Iop_Shl64; break;
         }
         break;
      case SHR:
         switch (width) {
            case W_8:  shift_op = Iop_Shr8;  break;
            case W_16: shift_op = Iop_Shr16; break;
            case W_32: shift_op = Iop_Shr32; break;
            case W_64: shift_op = Iop_Shr64; break;
         }
         break;
      case SAR:
         switch (width) {
            case W_8:  shift_op = Iop_Sar8;  break;
            case W_16: shift_op = Iop_Sar16; break;
            case W_32: shift_op = Iop_Sar32; break;
            case W_64: shift_op = Iop_Sar64; break;
         }
         break;
      default: vpanic("unsupported shift type");
   }

   /* retrieve arguments */
   UInt element_count = sizeofIRType(INS_ARR[ins_id].arg_type[0]) / sizeofIRType(part_ty);
   IRTemp arg[MAX_ARITY];
   IRTemp s[MAX_ARITY][MAX_ELEMENTS];
   UInt arity = 3;
   IRTemp res[MAX_ELEMENTS];
   vassert(element_count <= MAX_ELEMENTS);

   Long delta_out = delta;
   UChar modrm = getUChar(delta_out);
   UInt rG = gregOfRexRM32(pfx, modrm);
   UInt rV = getEVexNvvvv(pfx);
   UInt rE = 0;
   IRTemp addr = IRTemp_INVALID;
   UChar int_val = 0;
   DIP(" %s ", INS_ARR[ins_id].name);
   delta_out = Get_Instr_Args(vbi, pfx, delta_out, ins_id, arg, &arity, &int_val, rG, rV, &rE, &addr, modrm);

   /* Split the 1st and 2nd sources */
   for (Int j=0; j<element_count; j++) {
      s[1][j] = IRTemp_INVALID;
      s[2][j] = IRTemp_INVALID;
      res[j]  = newTemp(part_ty);
   }
   Split_arg(arg[1], s[1], element_count);
   Split_arg(arg[2], s[2], element_count);

   // Generating lot of redundant values here; fortunately, IR optimization handles it
   /* Shift each element */
   for (Int j=0; j<element_count; j++) {
      /* We need the second src as a 64-bit int for the comparison,
       * and as a 8-bit int for shift */
      IRExpr *amt_cmp, *amt_shift;
      switch (part_ty) {
         case Ity_I64:
            amt_cmp = mkexpr(s[2][j]);
            amt_shift = unop(Iop_64to8, mkexpr(s[2][j]));
            break;
         case Ity_I32:
            amt_cmp = unop(Iop_32Uto64, mkexpr(s[2][j]));
            amt_shift = unop(Iop_32to8, mkexpr(s[2][j]));
            break;
         case Ity_I16:
            amt_cmp = unop(Iop_16Uto64, mkexpr(s[2][j]));
            amt_shift = unop(Iop_16to8, mkexpr(s[2][j]));
            break;
         case Ity_I8:
            amt_cmp = unop(Iop_8Uto64, mkexpr(s[2][j]));
            amt_shift = mkexpr(s[2][j]);
            break;
         default: vpanic("invalid type of shift amount");
      }

      IRExpr *default_val = NULL;
      switch (INS_ARR[ins_id].parameter) {
         case 0: /* SHL */
         case 1: /* SHR */
            switch (width) {
               case W_8:  default_val = mkU8(0);  break;
               case W_16: default_val = mkU16(0); break;
               case W_32: default_val = mkU32(0); break;
               case W_64: default_val = mkU64(0); break;
            }
            break;
         case 2: /* SAR */
            switch (width) {
               case W_8:  default_val = binop(shift_op, mkexpr(s[1][j]), mkU8(0x8));  break;
               case W_16: default_val = binop(shift_op, mkexpr(s[1][j]), mkU8(0xF));  break;
               case W_32: default_val = binop(shift_op, mkexpr(s[1][j]), mkU8(0x1F)); break;
               case W_64: default_val = binop(shift_op, mkexpr(s[1][j]), mkU8(0x3F)); break;
            }
            break;
      }
      assign( res[j], IRExpr_ITE(
               binop(Iop_CmpLT64U, amt_cmp, shift_max),
               binop(shift_op, mkexpr(s[1][j]), amt_shift),
               default_val));
   }

   /* Merge back and mask the result */
   IRTemp unmasked = newTemp(INS_ARR[ins_id].arg_type[0]);
   Merge_dst(unmasked, res, element_count);
   IRExpr* masked = mask_expr(pfx, mkexpr(unmasked), mkexpr(arg[0]), ins_id);

   Put_Instr_Result(ins_id, masked, rG, rV, rE, addr, modrm);
   return delta_out;
}

static ULong dis_compress ( const VexAbiInfo* vbi, Prefix pfx, Long delta, UInt ins_id)
{
   HChar  dis_buf[50] = {0};
   Int    alen=0, i=0;
   UInt   mask = getEvexMask();
   UChar  modrm = getUChar(delta);
   Int    rG = gregOfRexRM32(pfx, modrm);
   IRTemp addr = IRTemp_INVALID;
   Bool is_64 = getRexW(pfx);

   const HChar* (*name_reg)(int);
   IRExpr* (*get_reg)(unsigned int);
   IROp compress_op = Iop_INVALID;
   switch (getEVexL()) {
      case 2: name_reg = nameZMMReg;
              get_reg = getZMMReg;
              compress_op = (IROp)(is_64 ? Iop_Compress64x8 : Iop_Compress32x16);
              break;
      case 1: name_reg = nameYMMReg;
              get_reg = getYMMReg;
              compress_op = (IROp)(is_64 ? Iop_Compress64x4 : Iop_Compress32x8);
              break;
      case 0: name_reg = nameXMMReg;
              get_reg = getXMMReg;
              compress_op = (IROp)(is_64 ? Iop_Compress64x2 : Iop_Compress32x4);
              break;
      default: vpanic("invalid dis_compress");
   }
   DIP (" %s %s ", INS_ARR[ins_id].name, name_reg(rG));

   // Separating register and memory cases
   // In register case, we can store the entire result in destination register
   // without causing any invalid writes
   // In memory case, Memcheck can complain on invalid memory access if we
   // store the entire result, so store the elements one by one
   if (epartIsReg(modrm)) {
      UInt rE = eregOfRexRM(pfx, modrm);
      delta += 1;
      DIP("%s", name_reg(rE));
      IRTemp rE_original = newTemp(INS_ARR[ins_id].arg_type[0]);
      assign(rE_original, get_reg(rE));

      // sewardj 2021Dec30: believed to be wrong in the same way as dis_expand
      // (see comment there)
      //IRExpr* result = qop(compress_op, mkexpr(rE_original), get_reg(rG), getKReg(mask), mkU8(getZeroMode()));
      IRExpr* result = qop(compress_op, mkexpr(rE_original), get_reg(rG), mask ? getKReg(mask) : mkU64(0), mkU8(getZeroMode()));

      UInt rV = 0;
      Put_Instr_Result(ins_id, result, rG, rV, rE, addr, modrm);
   } else {
      addr = disAMode(&alen, vbi, pfx, delta, dis_buf, 0);
      delta += alen;
      DIP (",%s,%s\n", dis_buf, nameKReg(mask));

      Int count = sizeofIRType(INS_ARR[ins_id].arg_type[1]) / ( is_64 ? 8 : 4);
      IRTemp elem_addr[MAX_ELEMENTS + 1];
      vassert(count + 1 <= MAX_ELEMENTS + 1);
      for (i = 0; i < count+1; i++) {
         elem_addr[i] = newTemp(Ity_I64);
      }
      assign (elem_addr[0], mkexpr(addr));

      for (i = 0; i < count; i++) {
         IRExpr* guard = unop(Iop_64to1, binop(Iop_Shr64, getKReg(mask), mkU8(i)));
         stmt( IRStmt_StoreG(Iend_LE,
                  mkexpr(elem_addr[i]),
                  is_64 ? getZMMRegLane64(rG, i) : getZMMRegLane32(rG, i),
                  guard) );
         assign (elem_addr[i+1], IRExpr_ITE(guard,
                  binop(Iop_Add64, mkexpr(elem_addr[i]), mkU64(is_64 ? 8 : 4 )),
                  mkexpr(elem_addr[i])));
      }
   }
   return delta;
}

static ULong dis_expand ( const VexAbiInfo* vbi, Prefix pfx, Long delta, UInt ins_id)
{
   HChar  dis_buf[50] = {0};
   Int    alen=0, i=0;
   UInt   mask = getEvexMask();
   UChar  modrm = getUChar(delta);
   Int    rG = gregOfRexRM32(pfx, modrm);
   IRTemp addr = IRTemp_INVALID;
   Bool is_64 = getRexW(pfx);

   const HChar* (*name_reg)(int);
   IRExpr* (*get_reg)(unsigned int);
   IROp expand_op = Iop_INVALID;
   switch (getEVexL()) {
      case 2: name_reg = nameZMMReg;
              get_reg = getZMMReg;
              expand_op = (IROp)(is_64 ? Iop_Expand64x8 : Iop_Expand32x16);
              break;
      case 1: name_reg = nameYMMReg;
              get_reg = getYMMReg;
              expand_op = (IROp)(is_64 ? Iop_Expand64x4 : Iop_Expand32x8);
              break;
      case 0: name_reg = nameXMMReg;
              get_reg = getXMMReg;
              expand_op = (IROp)(is_64 ? Iop_Expand64x2 : Iop_Expand32x4);
              break;
      default: vpanic("invalid dis_expand");
   }
   DIP (" %s %s ", INS_ARR[ins_id].name, name_reg(rG));

   // Separating register and memory cases
   // In register case, we can store the entire result in destination register
   // without causing any invalid reads
   // In memory case, Memcheck can complain on invalid memory access if we
   // load the entire source, so load the elements one by one
   if (epartIsReg(modrm)) {
      UInt rE = eregOfRexRM(pfx, modrm);
      delta += 1;
      DIP("%s", name_reg(rE));
      IRTemp rG_original = newTemp(INS_ARR[ins_id].arg_type[0]);
      assign(rG_original, get_reg(rG));

      // sewardj 2021Dec30: I think this is wrong; it causes the unconditional
      // (no-mask) version of this instruction to instead use k0 for the mask.
      //IRExpr* result = qop(expand_op, mkexpr(rG_original), get_reg(rE), getKReg(mask), mkU8(getZeroMode()));
      IRExpr* result = qop(expand_op, mkexpr(rG_original), get_reg(rE), mask ? getKReg(mask) : mkU64(0), mkU8(getZeroMode()));

      Put_Instr_Result(ins_id, result, rG, 0, rE, addr, modrm);
   } else {
      addr = disAMode(&alen, vbi, pfx, delta, dis_buf, 0);
      delta += alen;
      DIP (",%s,%s\n", dis_buf, nameKReg(mask));

      Int count = sizeofIRType(INS_ARR[ins_id].arg_type[1]) / (is_64 ? 8:4);
      IRTemp elem_addr[MAX_ELEMENTS + 1];
      vassert(count + 1 <= MAX_ELEMENTS + 1);
      for (i = 0; i < count+1; i++) {
         elem_addr[i] = newTemp(Ity_I64);
      }
      assign (elem_addr[0], mkexpr(addr));

      IRExpr* mask_0asFF;
      if (mask) 
         mask_0asFF = getKReg(mask);
      else 
         mask_0asFF = mkU64(-1);

      for (i = 0; i < count; i++) {
         IRExpr* guard = unop(Iop_64to1, binop(Iop_Shr64, mask_0asFF, mkU8(i)));
         if (is_64)
            putZMMRegLane64(rG, i, IRExpr_ITE(guard, 
                     loadLE( Ity_I64, mkexpr(elem_addr[i])),
                     getZeroMode() ? mkU64(0) : getZMMRegLane64(rG, i)));
         else
            putZMMRegLane32(rG, i, IRExpr_ITE(guard, 
                     loadLE( Ity_I32, mkexpr(elem_addr[i])),
                     getZeroMode() ? mkU32(0) : getZMMRegLane32(rG, i)));
         assign (elem_addr[i+1], IRExpr_ITE(guard,
                  binop(Iop_Add64, mkexpr(elem_addr[i]), mkU64(is_64 ? 8:4)),
                  mkexpr(elem_addr[i])));
      }
      // Fill the upper part of result vector register.
      switch (INS_ARR[ins_id].arg_type[0]) {
         case Ity_V128: putZMMRegLane128( rG, 1, mkV128(0) ); /*fallthru*/
         case Ity_V256: putZMMRegLane256( rG, 1, mkV256(0) );
         default: break;
      }
   }
   return delta;
}

static Long dis_EVEX_exceptions (const VexAbiInfo* vbi, Prefix pfx, Escape esc, Long delta, Int ins_id) {
   UChar opcode = getUChar(delta);
   delta += 1;
   UChar modrm = getUChar(delta);
   UInt rG = gregOfRexRM32(pfx, modrm);
   UInt rV = getEVexNvvvv(pfx);
   UInt mask = getEvexMask();
   HChar dis_buf[50];
   Int alen = 0;
   UChar imm8;

   switch (opcode) {
      case 0x10:
         /* VMOVSD */
         if (haveF2no66noF3(pfx) && (esc == ESC_0F) && getRexW(pfx)) {
            if (epartIsReg(modrm)) {
               UInt rE = eregOfRexRM(pfx, modrm);
               delta += 1;
               DIP(" VMOVSD %s,%s,%s\n", nameXMMReg(rE), nameXMMReg(rV), nameXMMReg(rG));
               putXMMReg(rG, mask_expr(pfx,
                        binop(Iop_SetV128lo64, getXMMReg(rV), getXMMRegLane64(rE, 0)),
                        getXMMReg(rG), ins_id) );
            }
            else {
               IRTemp addr = disAMode(&alen, vbi, pfx, delta, dis_buf, 0);
               delta += alen;
               DIP(" VMOVSD %s,%s\n", dis_buf, nameXMMReg(rG));
               putXMMReg(rG, mask_expr(pfx,
                        binop(Iop_SetV128lo64, mkV128(0), loadLE(Ity_I64, mkexpr(addr))),
                        getXMMReg(rG), ins_id) );
            }
            putZMMRegLane128(rG, 1, mkV128(0));
            putZMMRegLane256(rG, 1, mkV256(0));
         }
         /* VMOVSS */
         if (haveF3no66noF2(pfx) && (esc == ESC_0F) && !getRexW(pfx)) {
            if (epartIsReg(modrm)) {
               UInt rE = eregOfRexRM(pfx, modrm);
               delta += 1;
               DIP(" VMOVSS %s,%s,%s\n", nameXMMReg(rE), nameXMMReg(rV), nameXMMReg(rG));
               putXMMReg(rG, mask_expr(pfx,
                        binop(Iop_SetV128lo32, getXMMReg(rV), getXMMRegLane32(rE, 0)),
                        getXMMReg(rG), ins_id) );
            }
            else {
               IRTemp addr = disAMode ( &alen, vbi, pfx, delta, dis_buf, 0 );
               delta += alen;
               DIP(" VMOVSS %s,%s\n", dis_buf, nameXMMReg(rG));
               putXMMReg(rG, mask_expr(pfx,
                        binop(Iop_SetV128lo32, mkV128(0), loadLE(Ity_I32, mkexpr(addr))),
                        getXMMReg(rG), ins_id) );
            }
            putZMMRegLane128(rG, 1, mkV128(0));
            putZMMRegLane256(rG, 1, mkV256(0));
         }
         /* VPSRLVW */
         if (have66noF2noF3(pfx) && (esc == ESC_0F38) && getRexW(pfx))
            delta = dis_shift_vec_512( vbi, pfx, delta, ins_id );
         break;
      case 0x11:
          /* VMOVSD */
         if (haveF2no66noF3(pfx) && (esc == ESC_0F) && getRexW(pfx)) {
            if (epartIsReg(modrm)) {
               UInt rE = eregOfRexRM(pfx, modrm);
               delta += 1;
               DIP(" VMOVSD %s,%s,%s\n", nameXMMReg(rE), nameXMMReg(rV), nameXMMReg(rG));
               putXMMReg(rE, mask_expr(pfx,
                        binop(Iop_SetV128lo64, getXMMReg(rV), getXMMRegLane64(rG, 0)),
                        getXMMReg(rE), ins_id) );
            }
            else {
               IRTemp addr = disAMode (&alen, vbi, pfx, delta, dis_buf, 0);
               delta += alen;
               DIP(" VMOVSD %s,%s", nameXMMReg(rG), dis_buf);
               if (mask) {
                  DIP("{%s}\n", nameKReg(mask));
                  storeLE( mkexpr(addr), IRExpr_ITE( unop(Iop_64to1, getKReg(mask)),
                           getXMMRegLane64(rG, 0),
                        loadLE( Ity_I64, mkexpr(addr))));
               } else {
                  DIP("\n");
                  storeLE( mkexpr(addr), getXMMRegLane64(rG, 0));
               }
            }
         }
         /* VMOVSS */
         if (haveF3no66noF2(pfx) && (esc == ESC_0F) && !getRexW(pfx)) {
            if (epartIsReg(modrm)) {
               UInt rE = eregOfRexRM(pfx, modrm);
               delta += 1;
               DIP(" VMOVSS %s,%s,%s\n", nameXMMReg(rE), nameXMMReg(rV), nameXMMReg(rG));
               putXMMReg(rE, mask_expr(pfx,
                        binop(Iop_SetV128lo32, getXMMReg(rV), getXMMRegLane32(rG, 0)),
                        getXMMReg(rE), ins_id) );
            }
            else {
               IRTemp addr = disAMode ( &alen, vbi, pfx, delta, dis_buf, 0 );
               delta += alen;
               DIP(" VMOVSS %s,%s", nameXMMReg(rG), dis_buf);
               if (mask) {
                  DIP("{%s}\n", nameKReg(mask));
                  storeLE( mkexpr(addr), IRExpr_ITE( unop(Iop_64to1, getKReg(mask)),
                        getXMMRegLane32(rG, 0),
                        loadLE( Ity_I32, mkexpr(addr))));
               } else {
                  DIP("\n");
                  storeLE( mkexpr(addr), getXMMRegLane32(rG, 0));
               }
            }
         }
         /* VPSRAVW */
         if (have66noF2noF3(pfx) && (esc == ESC_0F38) && getRexW(pfx))
            delta = dis_shift_vec_512( vbi, pfx, delta, ins_id );
         break;
      case 0x12:
         /* VMOVHLPS */
         if (haveNo66noF2noF3(pfx) && (esc == ESC_0F) && !getRexW(pfx) && epartIsReg(modrm)) {
            setTupleType( FullVectorMem );
            UInt rE = eregOfRexRM(pfx, modrm);
            delta += 1;
            DIP(" VMOVHLPS %s,%s,%s\n", nameXMMReg(rE), nameXMMReg(rV), nameXMMReg(rG));
            putYMMRegLoAndZU(rG, binop(Iop_64HLtoV128,
                     getXMMRegLane64(rV, 1),
                     getXMMRegLane64(rE, 1)));
            putZMMRegLane256(rG, 1, mkV256(0));
         }
         /* VMOVLPS */
         if ( haveNo66noF2noF3(pfx) && (esc == ESC_0F) && !getRexW(pfx) && !epartIsReg(modrm) ) {
            setTupleType( Tuple2 );
            IRTemp addr = disAMode ( &alen, vbi, pfx, delta, dis_buf, 0 );
            delta += alen;
            DIP(" VMOVLPS %s,%s,%s\n", dis_buf, nameXMMReg(rV), nameXMMReg(rG));
            putYMMRegLoAndZU(rG, binop(Iop_64HLtoV128,
                     getXMMRegLane64(rV, 1),
                     loadLE(Ity_I64, mkexpr(addr))));
            putZMMRegLane256(rG, 1, mkV256(0));
         }
         /* VPSLLVW */
         if (have66noF2noF3(pfx) && (esc == ESC_0F38) && getRexW(pfx))
            delta = dis_shift_vec_512( vbi, pfx, delta, ins_id );
         break;
      case 0x14: {
         /* VPEXTRB */
         IRTemp xmm_vec = newTemp(Ity_V128);
         assign(xmm_vec, getXMMReg(rG));
         IRTemp s[16];
         for (Int i=0; i<16; i++)
            s[i] = IRTemp_INVALID;
         Split_arg(xmm_vec, s, 16);
         if (epartIsReg(modrm)) {
            UInt rE = eregOfRexRM(pfx, modrm);
            imm8 = (Int)getUChar(delta+1);
            DIP(" VPEXTRB $0x%x,%s,%s\n", imm8, nameXMMReg(rG), nameIReg64(rE));
            delta += 1+1;
            putIReg64(rE, unop(Iop_8Uto64, mkexpr(s[ imm8&0xF ])));
         } else {
            IRTemp addr = IRTemp_INVALID;
            addr = disAMode( &alen, vbi, pfx, delta, dis_buf, 1 );
            imm8 = (Int)getUChar(delta+alen);
            DIP(" VPEXTRB $0x%x,%s,%s\n", imm8, nameXMMReg(rG), dis_buf );
            delta += alen+1;
            storeLE(mkexpr(addr), mkexpr(s[ imm8&0xF ]));
         }
         break;
      }
      case 0x15: {
         /* VPEXTRW */
         IRTemp xmm_vec = newTemp(Ity_V128);
         assign(xmm_vec, getXMMReg(rG));
         IRTemp s[8];
         for (Int i=0; i<8; i++)
            s[i] = IRTemp_INVALID;
         Split_arg(xmm_vec, s, 8);
         if (epartIsReg(modrm)) {
            UInt rE = eregOfRexRM(pfx, modrm);
            imm8 = (Int)getUChar(delta+1);
            DIP(" VPEXTRW $0x%x,%s,%s\n", imm8, nameXMMReg(rG), nameIReg64(rE));
            delta += 1+1;
            putIReg64(rE, unop(Iop_16Uto64, mkexpr(s[ imm8&0x7 ])));
         } else {
            IRTemp addr = IRTemp_INVALID;
            addr = disAMode( &alen, vbi, pfx, delta, dis_buf, 1 );
            imm8 = (Int)getUChar(delta+alen);
            DIP(" VPEXTRW $0x%x,%s,%s\n", imm8, nameXMMReg(rG), dis_buf );
            delta += alen+1;
            storeLE(mkexpr(addr), mkexpr(s[ imm8&0x7 ]));
         }
         break;
      }
      case 0x16:
         /* VMOVHPS */
         if ( haveNo66noF2noF3(pfx) && (esc == ESC_0F) && !getRexW(pfx) && !epartIsReg(modrm) ) {
            setTupleType( Tuple2 );
            IRTemp addr = disAMode ( &alen, vbi, pfx, delta, dis_buf, 0 );
            delta += alen;
            DIP(" VMOVHPS %s, %s, %s\n", dis_buf, nameXMMReg(rV), nameXMMReg(rG));
            IRTemp res = newTemp(Ity_V128);
            assign(res, binop(Iop_64HLtoV128,
                     loadLE(Ity_I64, mkexpr(addr)),
                     getXMMRegLane64(rV, 0)));
            putYMMRegLoAndZU(rG, mkexpr(res));
            putZMMRegLane256(rG, 1, mkV256(0));
         }
         /* VMOVLHPS */
         if (haveNo66noF2noF3(pfx) && (esc == ESC_0F) && !getRexW(pfx) && epartIsReg(modrm)) {
            setTupleType( FullVectorMem );
            UInt rE = eregOfRexRM32(pfx, modrm);
            delta += 1;
            DIP(" VMOVLHPS %s,%s,%s\n", nameXMMReg(rE), nameXMMReg(rV), nameXMMReg(rG));
            IRTemp res = newTemp(Ity_V128);
            assign(res, binop(Iop_64HLtoV128,
                     getXMMRegLane64(rE, 0),
                     getXMMRegLane64(rV, 0)));
            putYMMRegLoAndZU(rG, mkexpr(res));
            putZMMRegLane256(rG, 1, mkV256(0));
         }
         break;
      case 0x21: {
         /*VINSERTPS*/
         IRTemp d2ins = newTemp(Ity_I32); /* comes from the E part */
         if ( epartIsReg( modrm ) ) {
            UInt   rE = eregOfRexRM(pfx, modrm);
            IRTemp vE = newTemp(Ity_V128);
            assign( vE, getXMMReg(rE) );
            IRTemp dsE[4] = { IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID, IRTemp_INVALID};
            breakupV128to32s( vE, &dsE[3], &dsE[2], &dsE[1], &dsE[0] );
            imm8 = getUChar(delta+1);
            d2ins = dsE[(imm8 >> 6) & 3]; /* "imm8_count_s" */
            delta += 1+1;
            DIP( " INSERTPS $0x%x, %s,%s\n", imm8, nameXMMReg(rE), nameXMMReg(rG) );
         } else {
            IRTemp addr = disAMode( &alen, vbi, pfx, delta, dis_buf, 1 );
            assign( d2ins, loadLE( Ity_I32, mkexpr(addr) ) );
            imm8 = getUChar(delta+alen);
            delta += alen+1;
            DIP( "INSERTPS $0x%x, %s,%s\n", imm8, dis_buf, nameXMMReg(rG) );
         }
         IRTemp vV = newTemp(Ity_V128);
         assign( vV, getXMMReg(rV) );
         putYMMRegLoAndZU( rG, mkexpr(math_INSERTPS( vV, d2ins, imm8 )) );
         break;
      }
      case 0x2E:
      case 0x2F: /* VUCOMISS, VUCOMISD, VCOMISS, VCOMISD*/
         DIP(" VCOMIS ");
         stmt( IRStmt_Put( OFFB_CC_OP,   mkU64(AMD64G_CC_OP_COPY) ));
         stmt( IRStmt_Put( OFFB_CC_DEP2, mkU64(0) ));

         IRTemp lhs = newTemp(Ity_F64);
         assign(lhs, getRexW(pfx) ?
               getXMMRegLane64F(rG, 0) :
               unop(Iop_F32toF64, getXMMRegLane32F(rG, 0)));
         IRTemp rhs = newTemp(Ity_F64);
         if (epartIsReg(modrm)) {
            UInt rE = eregOfRexRM(pfx, modrm);
            delta += 1;
            assign(rhs, getRexW(pfx) ?
                  getXMMRegLane64F(rE, 0) :
                  unop(Iop_F32toF64, getXMMRegLane32F(rE, 0)));
         } else {
            IRTemp addr = disAMode(&alen, vbi, pfx, delta, dis_buf, 0);
            delta += alen;
            assign(rhs, getRexW(pfx) ?
                  loadLE(Ity_F64, mkexpr(addr)) :
                  unop(Iop_F32toF64, loadLE(Ity_F32, mkexpr(addr))));
         }
         stmt( IRStmt_Put( OFFB_CC_DEP1, binop(Iop_And64,
                     unop( Iop_32Uto64, binop(Iop_CmpF64, mkexpr(lhs), mkexpr(rhs))),
                     mkU64(0x45))) );
         break;
      case 0x45: case 0x46: case 0x47:
         delta = dis_shift_vec_512( vbi, pfx, delta, ins_id );
         break;
      case 0x64: case 0x65: case 0x66: {
         /* VPBLENDMB, VPBLENDMW, VPBLENDMD, VPBLENDMQ, VBLENDMPS, VBLENDMPD */
         IRTemp arg[MAX_ARITY];
         UInt arity = 3, rE = 0;
         UChar int_val = 0;
         IRTemp addr = IRTemp_INVALID;
         DIP(" VBLEND ");
         delta = Get_Instr_Args(vbi, pfx, delta, ins_id, arg, &arity, &int_val, rG, rV, &rE, &addr, modrm);
         IRExpr* masked = mask_expr( pfx, mkexpr(arg[2]), mkexpr(arg[1]), ins_id);
         Put_Instr_Result(ins_id, masked, rG, rV, rE, addr, modrm);
         break;
      }
      case 0x71: case 0x72: case 0x73:
         delta = dis_shift_ser_512( vbi, pfx, delta, ins_id );
         break;
      case 0x88: case 0x89: /* VEXPANDPD */
         delta = dis_expand(vbi, pfx, delta, ins_id);
         break;
      case 0x8A:  /* VCOMPRESSPS, VCOMPRESSPD */
      case 0x8B: /* VPCOMPRESSD, VPCOMPRESSQ */
         delta = dis_compress(vbi, pfx, delta, ins_id);
         break;
      case 0x90: case 0x91: case 0x92: case 0x93:
         delta = dis_GATHER_512(vbi, pfx, delta, rG, ins_id);
         break;
      case 0xA0: case 0xA1: case 0xA2: case 0xA3:
         delta = dis_SCATTER_512(vbi, pfx, delta, rG, ins_id);
         break;
      case 0xAE: { /* CLWB or CLFLUSHOPT, do as NOOP */
         if (esc == ESC_0F) {
            (void) disAMode(&alen, vbi, pfx, delta, dis_buf, 0);
            delta += alen;
            break;
         }
         /* fall through */
      }
      case 0x96: case 0x97: case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
      case 0xA6: case 0xA7: case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
      case 0xB6: case 0xB7: case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:  /* VFMAS */
         if (have66noF2noF3(pfx))
            delta = dis_FMA_512( vbi, pfx, delta, ins_id );
         break;
      case 0xC5: {
         /* VPEXTRW */
         UInt rE = eregOfRexRM(pfx, modrm);
         IRTemp xmm_vec = newTemp(Ity_V128);
         assign(xmm_vec, getXMMReg(rE));
         IRTemp s[8];
         for (Int i=0; i<8; i++)
            s[i] = IRTemp_INVALID;
         Split_arg(xmm_vec, s, 8);
         imm8 = (Int)getUChar(delta+1);
         DIP(" VPEXTRW $0x%x,%s,%s\n", imm8, nameXMMReg(rE), nameIReg64(rG));
         delta += 1+1;
         putIReg64(rG, unop(Iop_16Uto64, mkexpr(s[ imm8&0x7 ])));
         break;
      }
      case 0xC6: case 0xC7: { /* KNL prefetches, do not emulate */
         (void) disAMode(&alen, vbi, pfx, delta, dis_buf, 0);
         delta += alen;
         break;
      }

      case 0xD1: case 0xD2: case 0xD3:
      case 0xE1: case 0xE2: // not a typo - 0xE3 is VPAVGW, not a shift
      case 0xF1: case 0xF2: case 0xF3:
         delta = dis_shift_ser_512( vbi, pfx, delta, ins_id );
         break;
      default:
         vex_printf("0x%x\n", opcode);
         vpanic("AVX-512 exception not implemented");
   }
   return delta;
}


static Long ParseEVEX (Prefix* pfx, Escape* esc, const Long deltaIn)
{
   evex = 0;
   Long delta = deltaIn;
   UChar evex1 = getUChar(delta);
   UChar evex2 = getUChar(delta+1);
   UChar evex3 = getUChar(delta+2);
   delta += 3;
   DIP("62 %02x %02x %02x\t", evex1, evex2, evex3);

   /* Snarf contents of byte 1 */

   /* ! These four are stored in reverse despite the documentation claiming otherwise ! */
   /* R */  *pfx |= (evex1 & (1<<7)) ? 0 : PFX_REXR;
   /* X */  *pfx |= (evex1 & (1<<6)) ? 0 : PFX_REXX;
   /* B */  *pfx |= (evex1 & (1<<5)) ? 0 : PFX_REXB;
   /* R' */ evex |= (evex1 & (1<<4)) ? 0 : EVEX_R1;

   /* mm */
   switch (evex1 & 0x3) {
      case 1: *esc = ESC_0F;   break;
      case 2: *esc = ESC_0F38; break;
      case 3: *esc = ESC_0F3A; break;
      default: vassert(0);
   }

   /* Snarf contents of byte 2. vvvv is stored in reverse despite the documentation claiming otherwise */
   /* W */  *pfx |= (evex2 & (1<<7)) ? PFX_REXW : 0;
   /* v3 */ *pfx |= (evex2 & (1<<6)) ? 0 : PFX_VEXnV3;
   /* v2 */ *pfx |= (evex2 & (1<<5)) ? 0 : PFX_VEXnV2;
   /* v1 */ *pfx |= (evex2 & (1<<4)) ? 0 : PFX_VEXnV1;
   /* v0 */ *pfx |= (evex2 & (1<<3)) ? 0 : PFX_VEXnV0;
   /* pp */
   switch (evex2 & 3) {
      case 0: break;
      case 1: *pfx |= PFX_66; break;
      case 2: *pfx |= PFX_F3; break;
      case 3: *pfx |= PFX_F2; break;
      default: vassert(0);
   }

   /* Snarf contents of byte 3 */
   /* z */   setZeroMode((evex3 & (1<<7)) >> 7);
   /* L' */  setEVexL((evex3 & (3<<5)) >> 5);
   /* L  */  *pfx  |= (evex3 & (1<<5)) ? PFX_VEXL  : 0;
   /* b  */  evex |= (evex3 & (1<<4)) ? EVEX_EVEXb : 0;
   /* ~v4*/  evex |= (evex3 & (1<<3)) ? 0: EVEX_VEXnV4;
   /* ~m */  setEvexMask( evex3 & 0x7 );

   return delta;
}



static const char* get_pfx_name(Prefix pfx) {
   if (haveF2(pfx)) return "F2";
   if (haveF3(pfx)) return "F3";
   if (have66(pfx)) return "66";
   return "NA";
}
static const char* get_esc_name(Escape esc) {
   switch (esc) {
      case ESC_NONE: return "ESC_NONE";
      case ESC_0F:   return "ESC_0F";
      case ESC_0F38: return "ESC_0F38";
      case ESC_0F3A: return "ESC_0F3A";
      default: return "no esc";
   }
}
static Int IRTypeVL(IRType ty) {
   switch (ty) {
      case Ity_I8:
      case Ity_I16: case Ity_F16:
      case Ity_I32: case Ity_F32: case Ity_D32:
      case Ity_I64: case Ity_F64: case Ity_D64:
      case Ity_F128: case Ity_I128: case Ity_V128:
         return 0;
      case Ity_V256: return 1;
      case Ity_V512: return 2;
      default: vpanic("invalid VL");
   }
}

Long dis__EVEX( const VexAbiInfo* vbi, Prefix pfx, Long delta ) {

   /* Returning deltaIN unchanged signals decode failure to the caller. */
   Long deltaIN = delta;

   /* Parse instruction prefix */
   Escape esc = ESC_NONE;
   delta++;
   delta = ParseEVEX(&pfx, &esc, delta);

   /* Find array entry with a matching opcode, prefix and vector length */
   UChar opcode = getUChar(delta);
   UChar modrm = getUChar(delta+1);
   Long prefix = (pfx & PFX_66) | (pfx & PFX_F2) | (pfx & PFX_F3);
   UInt width = getRexW(pfx);

   UInt ins_id=0;
   while (ins_id < N_INS_ARR && INS_ARR[ins_id].opcode != opcode) { // match opcode
      ins_id++;
   }
   while (ins_id < N_INS_ARR &&
         INS_ARR[ins_id].opcode == opcode && // match esc and prefix
         (INS_ARR[ins_id].pfx != prefix || INS_ARR[ins_id].esc != esc)) {
      ins_id++;
   }

   /* Not in the table: an AVX-512 subset this lifter does not implement, or
      simply not a valid instruction.  Report a decode failure rather than
      panicking -- the caller turns that into Ijk_NoDecode. */
   if (ins_id >= N_INS_ARR || INS_ARR[ins_id].opcode != opcode) {
      evex = 0;
      return deltaIN;
   }

   if (INS_ARR[ins_id].src_w!= WIG) { // match width
      while (ins_id < N_INS_ARR &&
            (INS_ARR[ins_id].opcode == opcode) && (INS_ARR[ins_id].src_w != WIG) &&
            (INS_ARR[ins_id].src_w != width )) {
         ins_id++;
      }
      if (ins_id >= N_INS_ARR || INS_ARR[ins_id].opcode != opcode) {
         evex = 0;
         return deltaIN;
      }
   } else {
      setWIG();
   }


   // Find matching "/2" or "/4" specifier, if it exists
   if (INS_ARR[ins_id].misc != NULL) {
      if ((char)INS_ARR[ins_id].misc[0] == '/') {
         while (ins_id < N_INS_ARR && INS_ARR[ins_id].misc != NULL &&
               (char)INS_ARR[ins_id].misc[1] != gregLO3ofRM(modrm) + '0') {
            ins_id++;
         }
         if (ins_id >= N_INS_ARR || INS_ARR[ins_id].misc == NULL) {
            evex = 0;
            return deltaIN;
         }
      }
   }

   if (INS_ARR[ins_id].misc != "noVL") {
      Int first_arg = 1;
      if ((INS_ARR[ins_id].args[1] == INS_ARR[ins_id].args[0]) //  the 1st src is dst
            || (INS_ARR[ins_id].args[1] == _rmode)) // the 1st source is rounding mode
         first_arg = 2;
      Int vl = getEVexL();
      if ( epartIsReg(modrm) && (getEvexb())) {
         // VL means rmode, actual length is 512 or serial
         vl = 2;
      }
      while (( vl != IRTypeVL(INS_ARR[ins_id].arg_type[0])) &&
            (vl != IRTypeVL(INS_ARR[ins_id].arg_type[first_arg]) )) { // match dst or src1 length
         ins_id++;
      }
   }

   if (INS_ARR[ins_id].opcode != opcode ||
         INS_ARR[ins_id].pfx  != prefix ||
         INS_ARR[ins_id].esc  != esc    ) {
      vex_printf("id %u - pfx %s esc %s w %u, opcode 0x%x, g %d\n",
            ins_id, get_pfx_name(prefix), get_esc_name(esc), width, opcode, gregLO3ofRM(modrm));
      vpanic("width, length or /N not implemented");
   }

   IROp irop = (IROp)INS_ARR[ins_id].irop;
   setTupleType( INS_ARR[ins_id].type );
   setDstW( INS_ARR[ins_id].dst_w );
   if (INS_ARR[ins_id].mask == MASK_MERGE)
      setZeroMode( False );
   if (INS_ARR[ins_id].mask == MASK_ZERO)
      setZeroMode( True );


   IRTemp arg[MAX_ARITY];
   IRTemp s[MAX_ARITY][MAX_MULTIPLIER];
   IRTemp res[MAX_MULTIPLIER];
   UInt arity = MAX_ARITY;

   /* Handle exceptions */
   if ((irop == Iop_INVALID) && INS_ARR[ins_id].opFn == NULL ) {
      delta = dis_EVEX_exceptions ( vbi, pfx, esc, delta, ins_id );
      DIP("\n");
      evex = 0;
      return delta;
   }

   /* retrieve arguments */
   delta++;
   UInt rG = gregOfRexRM32(pfx, modrm);
   UInt rV = getEVexNvvvv(pfx);
   UInt rE = 0;
   IRTemp addr = IRTemp_INVALID;
   UChar int_val = 0;
   DIP(" %s ", INS_ARR[ins_id].name);
   delta = Get_Instr_Args(vbi, pfx, delta, ins_id, arg, &arity, &int_val, rG, rV, &rE, &addr, modrm);
   if (INS_ARR[ins_id].parameter != -1)
      int_val = INS_ARR[ins_id].parameter;

   IRTemp unmasked = newTemp(INS_ARR[ins_id].arg_type[0]);

   /* Now, we have all information on the instruction.
    * There are several general ways we can translate it:
    *    1. If there is an existent IR doing the right job, just shorter -
    *       for example, Iop_Add64Fx2 fo 512-bit VADDPD - split the sources
    *       into several parts, call the IR and merge the results back
    *    2. If the instruciton needs access to full vectors and cannot be split -
    *       for example, VALIGND - just use a new IR for it
    *    3. If the instruction implements a new functionality with VL variants,
    *       and can be split - for example, VFIXUPIMMPS, there are options:
    *       3.1 Implement a 128-bit IR, call it two of four times for VL versions
    *           Use when:
    *           - The IR could be later translated to assembly without intrinsics, OR
    *           - 512-bit version of the instruction is unavailable on Knights Landing
    *       3.2 Implement a 512-bit IR, pad VL variants to the right length
    *          Use when the IR will be translated to assembly with intrinsics and
    *          could be called on Knights Landing (or perhaps Knights Mill)
    */

   IRType original_type;
   /* Positive multiplier - split into N parts, negative - pad to 512 bits */
   if (INS_ARR[ins_id].multiplier > 0) {
      switch ((UInt) (sizeofIRType(INS_ARR[ins_id].arg_type[0]) / INS_ARR[ins_id].multiplier)) {
         case 64: original_type = Ity_V512; break;
         case 32: original_type = Ity_V256; break;
         case 16: original_type = Ity_V128; break;
         case 8:  original_type = Ity_I64;  break;
         case 4:  original_type = Ity_I32;  break;
         case 2:  original_type = Ity_I16;  break;
         case 1:  original_type = Ity_I8;   break;
         default: vpanic("partial width not implemented");
      }

      for (Int i=1; i<arity; i++) {
         // Copy rmode or imm8, split the rest
         if ((INS_ARR[ins_id].args[i] == _rmode) || (INS_ARR[ins_id].args[i] == _imm8)) {
            for (Int j=0; j<INS_ARR[ins_id].multiplier; j++)
               s[i][j] = arg[i];
         } else {
            for (Int j=0; j<INS_ARR[ins_id].multiplier; j++) {
               s[i][j] = IRTemp_INVALID;
            }
            Split_arg(arg[i], s[i], INS_ARR[ins_id].multiplier);
         }
      }
   } else {
      original_type = Ity_V512;
      for (Int i=1; i<arity; i++) {
         // Copy rmode or imm8, pad the rest to 512
         if ((INS_ARR[ins_id].args[i] == _rmode) || (INS_ARR[ins_id].args[i] == _imm8)) {
            s[i][0] = arg[i];
         } else {
            s[i][0] = newTemp(Ity_V512);
            switch (INS_ARR[ins_id].multiplier) {
               case -2:
                  assign(s[i][0], binop(Iop_V256HLtoV512, mkV256(0), mkexpr(arg[i])));
                  break;
               case -4:
                  assign(s[i][0], binop(Iop_V256HLtoV512,
                           binop(Iop_V128HLtoV256, mkV128(0), mkV128(0)),
                           binop(Iop_V128HLtoV256, mkV128(0), mkexpr(arg[i]))));
                  break;
               default: vpanic("unknown negative multiplier");
            }
         }
      }
   }

   // Construct the IR(s)
   Int i=0;
   do {
      res[i] = newTemp(original_type);
      if (irop == Iop_INVALID) { // Function
         res[i] = INS_ARR[ins_id].opFn(s[1][i], s[2][i], int_val);
      } else { // IRop
         switch (arity) {
            case 2:
               if (INS_ARR[ins_id].parameter != -1 ) {
                  assign(res[i], binop(irop, mkexpr(s[1][i]), mkU8(int_val)));
               } else {
                  assign(res[i], unop(irop, mkexpr(s[1][i])));
               }
               break;
            case 3:
               if (INS_ARR[ins_id].parameter != -1 ) {
                  assign(res[i], triop(irop, mkexpr(s[1][i]), mkexpr(s[2][i]), mkU8(int_val)));
               } else {
                  assign(res[i], binop (irop, mkexpr(s[1][i]), mkexpr(s[2][i])));
               }
               break;
            case 4:
               if (INS_ARR[ins_id].parameter != -1 ) {
                  assign(res[i], qop(irop, mkexpr(s[1][i]), mkexpr(s[2][i]), mkexpr(s[3][i]), mkU8(int_val)));
               } else {
                  assign(res[i], triop(irop, mkexpr(s[1][i]), mkexpr(s[2][i]), mkexpr(s[3][i])));
               }
               break;
            case 5: assign(res[i], qop(irop, mkexpr(s[1][i]), mkexpr(s[2][i]), mkexpr(s[3][i]), mkexpr(s[4][i]))); break;
            default: vpanic("invalid arity");
         }
      }
      i++;
   } while (i < INS_ARR[ins_id].multiplier);

   if (INS_ARR[ins_id].multiplier > 0) {
      Merge_dst(unmasked, res, INS_ARR[ins_id].multiplier);
   } else {
      switch (INS_ARR[ins_id].multiplier) {
         case -2:
            assign(unmasked, unop(Iop_V512toV256_0, mkexpr(res[0])));
            break;
         case -4:
            assign(unmasked, unop(Iop_V256toV128_0, unop(Iop_V512toV256_0, mkexpr(res[0]))));
            break;
         default: vpanic("unknown negative multiplier");
      }
   }

   IRExpr* masked;
   if (INS_ARR[ins_id].mask != MASK_NONE) {
      masked = mask_expr(pfx, mkexpr(unmasked), mkexpr(arg[0]), ins_id);
   } else {
      masked = mkexpr(unmasked);
   }
   Put_Instr_Result(ins_id, masked, rG, rV, rE, addr, modrm);
   DIP("\n");
   evex = 0;
   return delta;
}


// Opmask and other non-AVX512 insrucitons specific to Skylake machines
Long nonAVX512_operation_decode ( const VexAbiInfo* vbi, UInt esc, Prefix pfx, Long deltaIN )
{
   // NOTE: NOT EVEX prefixed. Do not use the "evex" variable!
   Long delta = deltaIN;

   /* Find array entry with a matching opcode, prefix and vector length */
   UChar  opcode = getUChar(delta-1);
   UChar  modrm = getUChar(delta);

   Long prefix = (pfx & PFX_66) | (pfx & PFX_F2) | (pfx & PFX_F3);
   Int width = getRexW(pfx);

   UInt ins_id=0;
   while (ins_id < N_INS_ARR && INS_ARR[ins_id].opcode != opcode) // match opcode
      ins_id++;
   while (ins_id < N_INS_ARR &&
         INS_ARR[ins_id].opcode == opcode && // match esc and prefix
         (INS_ARR[ins_id].pfx != prefix || INS_ARR[ins_id].esc != esc))
      ins_id++;
   if (ins_id < N_INS_ARR && INS_ARR[ins_id].src_w != WIG) // match width
      while (ins_id < N_INS_ARR &&
            (INS_ARR[ins_id].opcode == opcode) && (INS_ARR[ins_id].src_w != width ))
         ins_id++;
   else
      setWIG();

   /* Not in the table: report a decode failure (deltaIN unchanged) rather
      than panicking, so the caller can emit Ijk_NoDecode. */
   if (ins_id >= N_INS_ARR             ||
         INS_ARR[ins_id].opcode != opcode ||
         INS_ARR[ins_id].pfx    != prefix ||
         INS_ARR[ins_id].esc    != esc    ) {
      evex = 0;
      return deltaIN;
   }

   IROp irop = (IROp)INS_ARR[ins_id].irop;
   setDstW( INS_ARR[ins_id].dst_w );
   
   /* Handle exceptions */
   if ((irop == Iop_INVALID) && INS_ARR[ins_id].opFn == NULL ) {
      delta --;
      delta = dis_EVEX_exceptions ( vbi, pfx, esc, delta, ins_id );
      DIP("\n");
      evex = 0;
      return delta;
   }
   

   /* retrieve arguments */
   IRTemp arg[MAX_ARITY];
   UInt arity = 5;
   UInt rG = gregOfRexRM(pfx, modrm);
   UInt rV = getVexNvvvv(pfx);
   UInt rE = 0;
   IRTemp addr = IRTemp_INVALID;
   UChar int_val = 0;
   DIP(" %s ", INS_ARR[ins_id].name);
   delta = Get_Instr_Args(vbi, pfx, delta, ins_id, arg, &arity, &int_val, rG, rV, &rE, &addr, modrm);
   if (INS_ARR[ins_id].parameter != -1)
      int_val = INS_ARR[ins_id].parameter;

   IRTemp res = newTemp(INS_ARR[ins_id].arg_type[0]);
   if (irop == Iop_INVALID) { // Function
      res = INS_ARR[ins_id].opFn(arg[1], arg[2], int_val);
   } else { // IRop
      switch (arity) {
         case 2:
            if (INS_ARR[ins_id].parameter != -1 )
               assign(res, binop(irop, mkexpr(arg[1]), mkU8(int_val)));
            else
               assign(res, unop(irop, mkexpr(arg[1])));
            break;
         case 3:
            if (INS_ARR[ins_id].parameter != -1 )
               assign(res, triop(irop, mkexpr(arg[1]), mkexpr(arg[2]), mkU8(int_val)));
            else
               assign(res, binop (irop, mkexpr(arg[1]), mkexpr(arg[2])));
            break;
         default: vpanic("invalid arity");
      }
   }

   // if result is mask register, zero it beforehand
   enum op_encoding op_code = INS_ARR[ins_id].args[0];
   if (op_code == _kG)
      putKReg(rG, mkU64(0));
   if ( (op_code == _kE) || ((op_code == _kmE) && epartIsReg(modrm)) )
      putKReg(rE, mkU64(0));

   Put_Instr_Result(ins_id, mkexpr(res), rG, rV, rE, addr, modrm);
   DIP("\n");
   evex = 0;
   return delta;
}



// Save area offsets are taken from the ISE
#define K0_OFFSET         0x440
#define ZMM0_Hi256_OFFSET 0x480
#define ZMM16_OFFSET      0x680
void gen_XSAVE_SEQUENCE_512 (IRTemp addr, IRTemp rfbm)
{
   IRTemp rfbm_3 = newTemp(Ity_I64);
   assign(rfbm_3, binop(Iop_And64, mkexpr(rfbm), mkU64(0xE0)));
   IRExpr* guard_3 = binop(Iop_CmpNE64, mkexpr(rfbm_3), mkU64(0x0));
   UInt reg;
   // Opmask registers
   for (reg = 0; reg < 8; reg++) {
      stmt( IRStmt_StoreG(
               Iend_LE,
               binop(Iop_Add64, mkexpr(addr), mkU64(K0_OFFSET + reg * 8)),
               getKReg(reg),
               guard_3
               ));
   }
   // High 256 bits of ZMM0-ZMM15 registers
   for (reg = 0; reg < 16; reg++) {
      stmt( IRStmt_StoreG(
               Iend_LE,
               binop(Iop_Add64, mkexpr(addr), mkU64(ZMM0_Hi256_OFFSET + reg * 32)),
               getZMMRegLane128(reg, 2),
               guard_3
               ));
      stmt( IRStmt_StoreG(
               Iend_LE,
               binop(Iop_Add64, mkexpr(addr), mkU64(ZMM0_Hi256_OFFSET + 16 + reg * 32)),
               getZMMRegLane128(reg, 3),
               guard_3
               ));
   }
   // ZMM16-ZMM31 registers; lane granularity for IRStmt_StoreG
   for (reg = 0; reg < 16; reg++) {
      for (Int lane = 0; lane < 4; lane++) {
         stmt( IRStmt_StoreG(
                  Iend_LE,
                  binop(Iop_Add64, mkexpr(addr), mkU64(ZMM16_OFFSET + lane*16 + reg*64)),
                  getZMMRegLane128(reg+16, lane),
                  guard_3
                  ));
      }
   }
}

void gen_XRSTOR_SEQUENCE_512 (IRTemp addr, IRTemp xstate_bv, IRTemp rfbm)
{
   IRTemp rfbm_3 = newTemp(Ity_I64);
   assign (rfbm_3, binop(Iop_And64, mkexpr(rfbm), mkU64(0xE0)));
   IRExpr* rfbm_3e = binop(Iop_CmpNE64, mkexpr(rfbm_3), mkU64(0));

   IRTemp xstate_bv_3 = newTemp(Ity_I64);
   assign (xstate_bv_3, binop(Iop_And64, mkexpr(xstate_bv), mkU64(0xF0)));

   IRTemp restore_3 = newTemp(Ity_I64);
   assign (restore_3, binop(Iop_And64, mkexpr(rfbm_3), mkexpr(xstate_bv_3)));
   IRExpr* restore_3e = binop(Iop_CmpNE64, mkexpr(restore_3), mkU64(0));

   UInt reg;
   /* Same scheme as AVX: first zero it out, and then possibly restore from the memory area. */
   // Opmask registers
   for (reg = 0; reg < 8; reg++) {
      putGuarded(kGuestRegOffset(reg), rfbm_3e, mkU64(0));
   }
   // High 256 bits of ZMM0-ZMM15 registers and ZMM16-ZMM31 registers
   for (reg = 0; reg < 16; reg++) {
      putGuarded(zmmGuestRegLane256offset(reg, 1), rfbm_3e, mkV256(0));
      putGuarded(zmmGuestRegOffset(reg+16), rfbm_3e, mkV512(0));
   }

   // Opmask registers
   for (reg = 0; reg < 8; reg++) {
      IRExpr* ea  = binop(Iop_Add64, mkexpr(addr), mkU64(K0_OFFSET + reg * 8));
      IRExpr* alt = getKReg(reg);
      IRTemp  loadedValue = newTemp(Ity_I64);
      stmt( IRStmt_LoadG(Iend_LE,
                         ILGop_Ident64,
                         loadedValue, ea, alt, restore_3e) );
      putKReg(reg, mkexpr(loadedValue));
   }
   // High 256 bits of ZMM0-ZMM15 registers
   for (reg = 0; reg < 16; reg++) {
      IRExpr* ea  = binop(Iop_Add64, mkexpr(addr), mkU64(ZMM0_Hi256_OFFSET + reg * 32));
      IRExpr* alt = getZMMRegLane128(reg, 2);
      IRTemp  loadedValuehi = newTemp(Ity_V128);
      stmt( IRStmt_LoadG(Iend_LE,
                         ILGop_IdentV128,
                         loadedValuehi, ea, alt, restore_3e) );
      putZMMRegLane128(reg, 2, mkexpr(loadedValuehi));

      IRTemp  loadedValuelo = newTemp(Ity_V128);
      ea  = binop(Iop_Add64, mkexpr(addr), mkU64(ZMM0_Hi256_OFFSET + 16 + reg * 32));
      alt = getZMMRegLane128(reg, 3);
      stmt( IRStmt_LoadG(Iend_LE,
                         ILGop_IdentV128,
                         loadedValuelo, ea, alt, restore_3e) );
      putZMMRegLane128(reg, 3, mkexpr(loadedValuelo));
   }
   // ZMM16-ZMM31 registers. Lane granularity for ILGop_IdentV128
   for (reg = 0; reg < 16; reg++) {
       for (Int lane = 0; lane < 4; lane++) {
           IRExpr* ea  = binop(Iop_Add64, mkexpr(addr), mkU64(ZMM16_OFFSET + lane*16 + reg * 64));
           IRExpr* alt = getZMMRegLane128(reg+16, lane);
           IRTemp  loadedValue = newTemp(Ity_V128);
           stmt( IRStmt_LoadG(Iend_LE,
                       ILGop_IdentV128,
                       loadedValue, ea, alt, restore_3e) );
           putZMMRegLane128(reg+16, lane, mkexpr(loadedValue));
       }
   }
}
#endif /* ndef AVX_512 */
/*--------------------------------------------------------------------*/
/*--- end                                guest_amd64_toIR_AVX512.c ---*/
/*--------------------------------------------------------------------*/
