/*---------------------------------------------------------------*/
/*--- begin                         guest_amd64_toIR_AVX512.h ---*/
/*---------------------------------------------------------------*/

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

#ifdef AVX_512
#ifndef __GUEST_AMD64_TOIR_512_H
#define __GUEST_AMD64_TOIR_512_H

#include "libvex_ir.h"
#include "../pub/libvex_guest_amd_AVX512.h"

Long dis__EVEX( const VexAbiInfo*, UInt, Long);
Long nonAVX512_operation_decode(const VexAbiInfo*, UInt, UInt, Long);

const HChar* nameXMMRegEVEX (Int xmmreg);
const HChar* nameYMMRegEVEX (Int ymmreg);
const HChar* nameZMMReg ( Int zmmreg );
Int getEVEXMult( UInt pfx );
UChar get_reg_16_32( UChar index_r );
Int ymmGuestRegOffsetEVEX(Int ymmreg);
UInt getEVexNvvvv ( UInt );
UInt gregOfRexRM32 ( UInt, UChar );
UInt eregOfRexRM32 ( UInt, UChar );
IRExpr* mask_expr( UInt, IRExpr*, IRExpr*, Int);

enum TupleType {
   NoTupleType=0,
   /* affected by embedded broadcast */
   FullVector, HalfVector,
   /* not affected by embedded broadcast */
   FullVectorMem,
   Tuple1Scalar, Tuple1Fixed,
   Tuple2, Tuple4, Tuple8,
   HalfMem, QuarterMem, OctMem,
   Mem128,
   MOVDDUP
};

void setTupleType ( enum TupleType );
void breakupV512toV256s ( IRTemp, IRTemp*,IRTemp* );
void breakupV512to64s ( IRTemp, IRTemp*,IRTemp*,IRTemp*,IRTemp*,IRTemp*,IRTemp*,IRTemp*,IRTemp*);

IRExpr* getZMMReg ( UInt);
IRExpr* getZMMRegLane256 ( UInt, Int );
IRExpr* getZMMRegLane128 ( UInt zmmreg, Int laneno );
IRExpr* getZMMRegLane64 ( UInt zmmreg, Int laneno );
IRExpr* getZMMRegLane64F ( UInt zmmreg, Int laneno );
IRExpr* getZMMRegLane32 ( UInt zmmreg, Int laneno );
IRExpr* getZMMRegLane32F ( UInt zmmreg, Int laneno );
void putZMMReg ( UInt zmmreg, IRExpr* e );
void putZMMRegLane256 ( UInt zmmreg, Int laneno, IRExpr* e );
void putZMMRegLane128 ( UInt zmmreg, Int laneno, IRExpr* e );
void putZMMRegLane64 ( UInt zmmreg, Int laneno, IRExpr* e );
void putZMMRegLane64F ( UInt zmmreg, Int laneno, IRExpr* e );
void putZMMRegLane32 ( UInt zmmreg, Int laneno, IRExpr* e );
void putZMMRegLane32F ( UInt zmmreg, Int laneno, IRExpr* e );
IRExpr* getKReg ( UInt kreg );
IRExpr* getKRegLane16 ( UInt kreg, Int laneno );
IRExpr* getKRegLane8 ( UInt kreg, Int laneno );
void putKReg ( UInt kreg, IRExpr* e );
void putKRegLane32 ( UInt kreg, Int laneno, IRExpr* e );
void putKRegLane16 ( UInt kreg, Int laneno, IRExpr* e );
void putKRegLane8 ( UInt kreg, Int laneno, IRExpr* e );
IRExpr* mkV256 ( UInt val );
IRExpr* mkV512 ( UInt val );
void gen_XSAVE_SEQUENCE_512 ( IRTemp addr, IRTemp rfbm );
void gen_XRSTOR_SEQUENCE_512 ( IRTemp addr, IRTemp xstate_bv, IRTemp rfbm);

#define OFFB_YMM17     offsetof(VexGuestAMD64State,guest_ZMM17)
#define OFFB_YMM18     offsetof(VexGuestAMD64State,guest_ZMM18)
#define OFFB_YMM19     offsetof(VexGuestAMD64State,guest_ZMM19)
#define OFFB_YMM20     offsetof(VexGuestAMD64State,guest_ZMM20)
#define OFFB_YMM21     offsetof(VexGuestAMD64State,guest_ZMM21)
#define OFFB_YMM22     offsetof(VexGuestAMD64State,guest_ZMM22)
#define OFFB_YMM23     offsetof(VexGuestAMD64State,guest_ZMM23)
#define OFFB_YMM24     offsetof(VexGuestAMD64State,guest_ZMM24)
#define OFFB_YMM25     offsetof(VexGuestAMD64State,guest_ZMM25)
#define OFFB_YMM26     offsetof(VexGuestAMD64State,guest_ZMM26)
#define OFFB_YMM27     offsetof(VexGuestAMD64State,guest_ZMM27)
#define OFFB_YMM28     offsetof(VexGuestAMD64State,guest_ZMM28)
#define OFFB_YMM29     offsetof(VexGuestAMD64State,guest_ZMM29)
#define OFFB_YMM30     offsetof(VexGuestAMD64State,guest_ZMM30)
#define OFFB_YMM31     offsetof(VexGuestAMD64State,guest_ZMM31)
#define OFFB_YMM32     offsetof(VexGuestAMD64State,guest_ZMM32)
#define OFFB_ZMM0      offsetof(VexGuestAMD64State,guest_ZMM0)
#define OFFB_ZMM1      offsetof(VexGuestAMD64State,guest_ZMM1)
#define OFFB_ZMM2      offsetof(VexGuestAMD64State,guest_ZMM2)
#define OFFB_ZMM3      offsetof(VexGuestAMD64State,guest_ZMM3)
#define OFFB_ZMM4      offsetof(VexGuestAMD64State,guest_ZMM4)
#define OFFB_ZMM5      offsetof(VexGuestAMD64State,guest_ZMM5)
#define OFFB_ZMM6      offsetof(VexGuestAMD64State,guest_ZMM6)
#define OFFB_ZMM7      offsetof(VexGuestAMD64State,guest_ZMM7)
#define OFFB_ZMM8      offsetof(VexGuestAMD64State,guest_ZMM8)
#define OFFB_ZMM9      offsetof(VexGuestAMD64State,guest_ZMM9)
#define OFFB_ZMM10     offsetof(VexGuestAMD64State,guest_ZMM10)
#define OFFB_ZMM11     offsetof(VexGuestAMD64State,guest_ZMM11)
#define OFFB_ZMM12     offsetof(VexGuestAMD64State,guest_ZMM12)
#define OFFB_ZMM13     offsetof(VexGuestAMD64State,guest_ZMM13)
#define OFFB_ZMM14     offsetof(VexGuestAMD64State,guest_ZMM14)
#define OFFB_ZMM15     offsetof(VexGuestAMD64State,guest_ZMM15)
#define OFFB_ZMM16     offsetof(VexGuestAMD64State,guest_ZMM16)
#define OFFB_ZMM17     offsetof(VexGuestAMD64State,guest_ZMM17)
#define OFFB_ZMM18     offsetof(VexGuestAMD64State,guest_ZMM18)
#define OFFB_ZMM19     offsetof(VexGuestAMD64State,guest_ZMM19)
#define OFFB_ZMM20     offsetof(VexGuestAMD64State,guest_ZMM20)
#define OFFB_ZMM21     offsetof(VexGuestAMD64State,guest_ZMM21)
#define OFFB_ZMM22     offsetof(VexGuestAMD64State,guest_ZMM22)
#define OFFB_ZMM23     offsetof(VexGuestAMD64State,guest_ZMM23)
#define OFFB_ZMM24     offsetof(VexGuestAMD64State,guest_ZMM24)
#define OFFB_ZMM25     offsetof(VexGuestAMD64State,guest_ZMM25)
#define OFFB_ZMM26     offsetof(VexGuestAMD64State,guest_ZMM26)
#define OFFB_ZMM27     offsetof(VexGuestAMD64State,guest_ZMM27)
#define OFFB_ZMM28     offsetof(VexGuestAMD64State,guest_ZMM28)
#define OFFB_ZMM29     offsetof(VexGuestAMD64State,guest_ZMM29)
#define OFFB_ZMM30     offsetof(VexGuestAMD64State,guest_ZMM30)
#define OFFB_ZMM31     offsetof(VexGuestAMD64State,guest_ZMM31)
#define OFFB_ZMM32     offsetof(VexGuestAMD64State,guest_ZMM32)

#define OFFB_K0        offsetof(VexGuestAMD64State,guest_MASKREG[0])
#define OFFB_K1        offsetof(VexGuestAMD64State,guest_MASKREG[1])
#define OFFB_K2        offsetof(VexGuestAMD64State,guest_MASKREG[2])
#define OFFB_K3        offsetof(VexGuestAMD64State,guest_MASKREG[3])
#define OFFB_K4        offsetof(VexGuestAMD64State,guest_MASKREG[4])
#define OFFB_K5        offsetof(VexGuestAMD64State,guest_MASKREG[5])
#define OFFB_K6        offsetof(VexGuestAMD64State,guest_MASKREG[6])
#define OFFB_K7        offsetof(VexGuestAMD64State,guest_MASKREG[7])

#endif // __GUEST_AMD64_TOIR_512_H
#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                           guest_amd64_toIR_AVX512.h ---*/
/*---------------------------------------------------------------*/
