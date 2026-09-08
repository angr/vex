/*---------------------------------------------------------------*/
/*--- begin                           host_generic_AVX512_F.c ---*/
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

/* Generic helper functions for doing AVX-512 instructions in cases
   where the instruction selectors cannot generate code in-line.
   These are purely back-end entities and cannot be seen/referenced
   from IR. */

/*
 * The file implements instrucitons that should be supported on all AVX-512
machines:
 * AVX-512F (Foundation) subset
 * AVX-512CD (Conflict Detection) subset
*/

#ifdef AVX_512

#include "libvex_basictypes.h"
#include "host_generic_AVX512.h"
#include <emmintrin.h>
#include <immintrin.h>
#include "host_generic_imm8_def.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wattributes"

// NOTE: keep in sync with operand_width from guest_amd64_toIR512.c
enum dstWidth {
   W_8bits = 0,
   W_16bits,
   W_32bits,
   W_64bits,
};
union I64 {
   UChar  w8[8];
   UShort w16[4];
   UInt   w32[2];
   ULong  w64;
};

#define WIDEN_BIT(i) (((mask>>i)&1) ? (-1) : 0 )

VEX_REGPARM(3) void h_Iop_ExpandBitsToInt ( ULong* out, ULong mask, UInt elem_width) {
   Int i=0;
   union I64 dst;
   switch (elem_width) {
      case W_8bits:  for (i=0;i<8;i++) dst.w8[i]  = WIDEN_BIT(i); break;
      case W_16bits: for (i=0;i<4;i++) dst.w16[i] = WIDEN_BIT(i); break;
      case W_32bits: for (i=0;i<2;i++) dst.w32[i] = WIDEN_BIT(i); break;
      case W_64bits:                   dst.w64    = WIDEN_BIT(0); break;
      default: vpanic("h_Iop_ExpandBitsToInt - not implemented\n");
   }
   *out = dst.w64;
}
VEX_REGPARM(3) void h_Iop_ExpandBitsToV128 ( V128* out, ULong mask, UInt elem_width) {
   Int i = 0;
   switch (elem_width) {
      case W_8bits:  for (i=0;i<16;i++) out->w8[i]  = WIDEN_BIT(i); break;
      case W_16bits: for (i=0;i<8; i++) out->w16[i] = WIDEN_BIT(i); break;
      case W_32bits: for (i=0;i<4; i++) out->w32[i] = WIDEN_BIT(i); break;
      case W_64bits: for (i=0;i<2; i++) out->w64[i] = WIDEN_BIT(i); break;
      default: vpanic("h_Iop_ExpandBitsToV128 - not implemented\n");
   }
}
VEX_REGPARM(3) void h_Iop_ExpandBitsToV256 ( V256* out, ULong mask, UInt elem_width) {
   Int i = 0;
   switch (elem_width) {
      case W_8bits:  for (i=0;i<32;i++) out->w8[i]  = WIDEN_BIT(i); break;
      case W_16bits: for (i=0;i<16;i++) out->w16[i] = WIDEN_BIT(i); break;
      case W_32bits: for (i=0;i<8; i++) out->w32[i] = WIDEN_BIT(i); break;
      case W_64bits: for (i=0;i<4; i++) out->w64[i] = WIDEN_BIT(i); break;
      default: vpanic("h_Iop_ExpandBitsToV256 - not implemented\n");
   }
}
VEX_REGPARM(3) void h_Iop_ExpandBitsToV512 ( V512* out, ULong mask, UInt elem_width ) {
   Int i = 0;
   switch (elem_width) {
      case W_8bits:  for (i=0;i<64;i++) out->w8[i]  = WIDEN_BIT(i); break;
      case W_16bits: for (i=0;i<32;i++) out->w16[i] = WIDEN_BIT(i); break;
      case W_32bits: for (i=0;i<16;i++) out->w32[i] = WIDEN_BIT(i); break;
      case W_64bits: for (i=0;i<8; i++) out->w64[i] = WIDEN_BIT(i); break;
      default: vpanic("h_Iop_ExpandBitsToV512 - not implemented\n");
   }
}
#undef WIDEN_BIT

// TODO refactor
typedef union {
   UInt u;
   Int s;
   Float f;
} type32;
typedef union {
   ULong u;
   Long s;
   Double f;
} type64;
#include <limits.h>
VEX_REGPARM(3) void h_Iop_F32toI32U_SKX (UInt* dst, UInt rmode, UInt src_u) {
   type32 src = {.u = src_u};
   if (src.f <= INT_MIN ||src.f > UINT_MAX) {
      *dst = -1;
      return;
   }
   if (src.f <= 0) {
      *dst = 0;
      return;
   }
   *dst = (UInt)src.f;
}
VEX_REGPARM(3) void h_Iop_F64toI32U_SKX (UInt* dst, UInt rmode, ULong src_u) {
   type64 src = {.u = src_u};
   if ( src.f <= INT_MIN || src.f > UINT_MAX) {
      *dst = -1;
      return;
   }
   if (src.f <= 0) {
      *dst = 0;
      return;
   }
   *dst = (UInt)src.f;
}
VEX_REGPARM(3) void h_Iop_F64toI64U_SKX (ULong* dst, UInt rmode, ULong src_u) {
   type64 src = {.u = src_u};
   if (src.f <= LONG_MIN || src.f > ULONG_MAX) {
      *dst = -1;
      return;
   }
   if (src.f <= 0) {
      *dst = 0;
      return;
   }
   *dst = (ULong)src.f;
}
VEX_REGPARM(3) void h_Iop_I32UtoF32_SKX (UInt* dst_u, UInt rmode_fake, UInt src) {
   type32 dst = {.f = (Float)src};
   *dst_u = dst.u;
}
VEX_REGPARM(3) void h_Iop_I64UtoF64_SKX (ULong* dst_u, UInt rmode_fake, ULong src) {
   type64 dst = {.f = (Double)src};
   *dst_u = dst.u;
}

// Use Intel instrinsics.
// I tried using a common wrapper and pass a pointer to intrinsic into the wrapper,
// but:
// 1. it does not link with gcc
// 2. the pointer is not a constant to icc

VEX_REGPARM(3) void h_Iop_Max64Sx8 ( V512* dst, V512* src1, V512* src2 ) {
   register __m512i src1_r = _mm512_loadu_si512((void*)src1);
   register __m512i src2_r = _mm512_loadu_si512((void*)src2);
   _mm512_storeu_si512(dst, _mm512_mask_max_epi64 (src1_r, -1, src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Min64Sx8 ( V512* dst, V512* src1, V512* src2 ) {
   register __m512i src1_r = _mm512_loadu_si512((void*)src1);
   register __m512i src2_r = _mm512_loadu_si512((void*)src2);
   _mm512_storeu_si512(dst, _mm512_mask_min_epi64 (src1_r, -1, src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Max64Ux8 ( V512* dst, V512* src1, V512* src2 ) {
   register __m512i src1_r = _mm512_loadu_si512((void*)src1);
   register __m512i src2_r = _mm512_loadu_si512((void*)src2);
   _mm512_storeu_si512(dst, _mm512_mask_max_epu64 (src1_r, -1, src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Min64Ux8 ( V512* dst, V512* src1, V512* src2 ) {
   register __m512i src1_r = _mm512_loadu_si512((void*)src1);
   register __m512i src2_r = _mm512_loadu_si512((void*)src2);
   _mm512_storeu_si512(dst, _mm512_mask_min_epu64 (src1_r, -1, src1_r, src2_r));
}

VEX_REGPARM(3) void h_Iop_Test32x16 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_test_epi32_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test64x8 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_test_epi64_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN32x16 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_testn_epi32_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN64x8 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_testn_epi64_mask(src1_r, src2_r);
}

VEX_REGPARM(4) void h_Iop_Expand32x16( V512* src_dst, V512* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m512 src1_r = _mm512_loadu_ps(src_dst);
   register __m512 src2_r = _mm512_loadu_ps(vec);
   _mm512_storeu_ps(src_dst,
         zero ? _mm512_maskz_expand_ps(mask, src2_r) :
                _mm512_mask_expand_ps(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Expand64x8( V512* src_dst, V512* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m512d src1_r = _mm512_loadu_pd(src_dst);
   register __m512d src2_r = _mm512_loadu_pd(vec);
   _mm512_storeu_pd(src_dst,
        zero ? _mm512_maskz_expand_pd(mask, src2_r) :
                _mm512_mask_expand_pd(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Compress32x16( V512* src_dst, V512* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m512 src1_r = _mm512_loadu_ps(src_dst);
   register __m512 src2_r = _mm512_loadu_ps(vec);
   _mm512_storeu_ps(src_dst,
         zero ? _mm512_maskz_compress_ps(mask, src2_r) :
                _mm512_mask_compress_ps(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Compress64x8( V512* src_dst, V512* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m512d src1_r = _mm512_loadu_pd(src_dst);
   register __m512d src2_r = _mm512_loadu_pd(vec);
   _mm512_storeu_pd(src_dst,
         zero ? _mm512_maskz_compress_pd(mask, src2_r):
                _mm512_mask_compress_pd(src1_r, mask, src2_r));
}

VEX_REGPARM(2) void h_Iop_ExtractExp32x16(V512* dst, V512* src) {
   register __m512 src_r = _mm512_loadu_ps(src);
   _mm512_storeu_ps(dst, _mm512_getexp_ps(src_r));
}
VEX_REGPARM(2) void h_Iop_ExtractExp64x8(V512* dst, V512* src) {
   register __m512d src_r = _mm512_loadu_pd(src);
   _mm512_storeu_pd(dst, _mm512_getexp_pd(src_r));
}
VEX_REGPARM(3) void h_Iop_ExtractExp32F0x4(V128* dst, V128* src_1, V128* src_2) {
   register __m128 src1_r = _mm_loadu_ps(src_1);
   register __m128 src2_r = _mm_loadu_ps(src_2);
   _mm_storeu_ps(dst, _mm_getexp_ss(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_ExtractExp64F0x2(V128* dst, V128* src_1, V128* src_2) {
   register __m128d src1_r = _mm_loadu_pd(src_1);
   register __m128d src2_r = _mm_loadu_pd(src_2);
   _mm_storeu_pd(dst, _mm_getexp_sd(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_GetMant32x16( V512* dst, V512* src, UInt imm8) {
   Char sc = (imm8 >> 2) & 0x3;
   Char interv = imm8 & 0x3;
   register __m512 src_r = _mm512_loadu_ps(src);
   register __m512 res;
   MANTISSA_SWITCHCASE(_mm512_getmant_ps, src_r);
   _mm512_storeu_ps(dst, res);
}
VEX_REGPARM(3) void h_Iop_GetMant64x8( V512* dst, V512* src, UInt imm8) {
   Char sc = (imm8 >> 2) & 0x3;
   Char interv = imm8 & 0x3;
   register __m512d src_r = _mm512_loadu_pd(src);
   register __m512d res;
   MANTISSA_SWITCHCASE(_mm512_getmant_pd, src_r);
   _mm512_storeu_pd(dst, res);
}
VEX_REGPARM(3) void h_Iop_GetMant32F0x4( V128* src_dst, V128* src, UInt imm8) {
   Char sc = (imm8 >> 2) & 0x3;
   Char interv = imm8 & 0x3;
   register __m128 src_dst_r = _mm_loadu_ps(src_dst);
   register __m128 src_r = _mm_loadu_ps(src);
   register __m128 res;
   MANTISSA_SWITCHCASE(_mm_getmant_ss, src_dst_r, src_r);
   _mm_storeu_ps(src_dst, res);
}
VEX_REGPARM(3) void h_Iop_GetMant64F0x2( V128* src_dst, V128* src, UInt imm8) {
   Char sc = (imm8 >> 2) & 0x3;
   Char interv = imm8 & 0x3;
   register __m128d src_dst_r = _mm_loadu_pd(src_dst);
   register __m128d src_r = _mm_loadu_pd(src);
   register __m128d res;
   MANTISSA_SWITCHCASE(_mm_getmant_sd, src_dst_r, src_r);
   _mm_storeu_pd(src_dst, res);
}

VEX_REGPARM(4) void h_Iop_Ternlog32x16 (V512* src_dst, V512* src2, V512* src3, UInt imm8) {
   register __m512i src_dst_r = _mm512_loadu_si512(src_dst);
   register __m512i src1_r = _mm512_loadu_si512(src3);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   register __m512i res;
   IMM8_SWITCHCASE(_mm512_ternarylogic_epi32, src_dst_r, src2_r, src1_r);
   _mm512_storeu_si512(src_dst, res);
}
VEX_REGPARM(4) void h_Iop_Ternlog64x8 (V512* src_dst, V512* src2, V512* src3, UInt imm8) {
   register __m512i src_dst_r = _mm512_loadu_si512(src_dst);
   register __m512i src1_r = _mm512_loadu_si512(src3);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   register __m512i res;
   IMM8_SWITCHCASE(_mm512_ternarylogic_epi64, src_dst_r, src2_r, src1_r);
   _mm512_storeu_si512(src_dst, res);
}

/* Count leading zeros */
VEX_REGPARM(2) void h_Iop_Clz32( UInt* dst, UInt src) {
   *dst = (src == 0) ? 32 : __builtin_clz(src);
}
/* Conflict detection */
VEX_REGPARM(2) void h_Iop_CfD32x16( V512* dst, V512* src) {
   register __m512i src_r = _mm512_loadu_si512(src);
   _mm512_storeu_si512(dst, _mm512_conflict_epi32(src_r));
}
VEX_REGPARM(2) void h_Iop_CfD64x8( V512* dst, V512* src) {
   register __m512i src_r = _mm512_loadu_si512(src);
   _mm512_storeu_si512(dst, _mm512_conflict_epi64(src_r));
}

VEX_REGPARM(3) void h_Iop_Scale32x16( V512* dst, V512* src1, V512* src2) {
   register __m512 src1_r = _mm512_loadu_ps(src1);
   register __m512 src2_r = _mm512_loadu_ps(src2);
   _mm512_storeu_ps(dst, _mm512_scalef_ps(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Scale64x8( V512* dst, V512* src1, V512* src2) {
   register __m512d src1_r = _mm512_loadu_pd(src1);
   register __m512d src2_r = _mm512_loadu_pd(src2);
   _mm512_storeu_pd(dst, _mm512_scalef_pd(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Scale32F0x4( V128* dst, V128* src1, V128* src2) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 src2_r = _mm_loadu_ps(src2);
   _mm_storeu_ps(dst, _mm_scalef_ss(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Scale64F0x2( V128* dst, V128* src1, V128* src2) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d src2_r = _mm_loadu_pd(src2);
   _mm_storeu_pd(dst, _mm_scalef_sd(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_RoundScale32x16( V512* dst, V512* src, UInt imm8) {
   register __m512 src_r = _mm512_loadu_ps(src);
   register __m512 res;
   IMM8_SWITCHCASE(_mm512_roundscale_ps, src_r);
   _mm512_storeu_ps(dst, res);
}
VEX_REGPARM(3) void h_Iop_RoundScale64x8( V512* dst, V512* src, UInt imm8) {
   register __m512d src_r = _mm512_loadu_pd(src);
   register __m512d res;
   IMM8_SWITCHCASE(_mm512_roundscale_pd, src_r);
   _mm512_storeu_pd(dst, res);
}
VEX_REGPARM(3) void h_Iop_RoundScale32F0x4( V128* dst, V128* src, UInt imm8) {
   register __m128 src_r = _mm_loadu_ps(src);
   register __m128 src_dst_r = _mm_loadu_ps(dst);
   register __m128 res;
   IMM8_SWITCHCASE(_mm_roundscale_ss, src_dst_r, src_r);
   _mm_storeu_ps(dst, res);
}
VEX_REGPARM(3) void h_Iop_RoundScale64F0x2( V128* dst, V128* src, UInt imm8) {
   register __m128d src_r = _mm_loadu_pd(src);
   register __m128d src_dst_r = _mm_loadu_pd(dst);
   register __m128d res;
   IMM8_SWITCHCASE(_mm_roundscale_sd, src_dst_r, src_r);
   _mm_storeu_pd(dst, res);
}

VEX_REGPARM(2) void h_Iop_Recip14_32x16( V512* dst, V512* src ) {
   register __m512 src_r = _mm512_loadu_ps(src);
   _mm512_storeu_ps(dst, _mm512_rcp14_ps(src_r));
}
VEX_REGPARM(2) void h_Iop_Recip14_64x8( V512* dst, V512* src ) {
   register __m512d src_r = _mm512_loadu_pd(src);
   _mm512_storeu_pd(dst, _mm512_rcp14_pd(src_r));
}
VEX_REGPARM(2) void h_Iop_RSqrt14_32x16( V512* dst, V512* src ) {
   register __m512 src_r = _mm512_loadu_ps(src);
    _mm512_storeu_ps(dst, _mm512_rsqrt14_ps(src_r));
}
VEX_REGPARM(2) void h_Iop_RSqrt14_64x8( V512* dst, V512* src ) {
   register __m512d src_r = _mm512_loadu_pd(src);
    _mm512_storeu_pd(dst, _mm512_rsqrt14_pd(src_r));
}
VEX_REGPARM(3) void h_Iop_Recip14_32F0x4( V128* dst, V128* src1, V128* src2 ) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 src2_r = _mm_loadu_ps(src2);
   _mm_storeu_ps(dst, _mm_rcp14_ss(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Recip14_64F0x2( V128* dst, V128* src1, V128* src2 ) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d src2_r = _mm_loadu_pd(src2);
   _mm_storeu_pd(dst, _mm_rcp14_sd(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_RSqrt14_32F0x4 ( V128* dst, V128* src1, V128* src2 ) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 src2_r = _mm_loadu_ps(src2);
   _mm_storeu_ps(dst, _mm_rsqrt14_ss(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_RSqrt14_64F0x2 ( V128* dst, V128* src1, V128* src2 ) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d src2_r = _mm_loadu_pd(src2);
   _mm_storeu_pd(dst, _mm_rsqrt14_sd(src1_r, src2_r));
}

VEX_REGPARM(4) void h_Iop_FixupImm64x8 (V512* dst, V512* src, V512* lookup, UInt imm8 ) {
   register __m512d src1_r = _mm512_loadu_pd(src);
   register __m512i src2_r = _mm512_loadu_si512(lookup);
   register __m512d src_dst_r = _mm512_loadu_pd(dst);
   register __m512d res;
   IMM8_SWITCHCASE(_mm512_fixupimm_pd, src_dst_r, src1_r, src2_r);
   _mm512_storeu_pd(dst, res);
}
VEX_REGPARM(4) void h_Iop_FixupImm32x16 (V512* dst, V512* src, V512* lookup, UInt imm8 ) {
   register __m512  src1_r = _mm512_loadu_ps(src);
   register __m512i src2_r = _mm512_loadu_si512(lookup);
   register __m512  src_dst_r = _mm512_loadu_ps(dst);
   register __m512  res;
   IMM8_SWITCHCASE(_mm512_fixupimm_ps, src_dst_r, src1_r, src2_r);
   _mm512_storeu_ps(dst, res);
}
VEX_REGPARM(4) void h_Iop_FixupImm64F0x2 (V128* dst, V128* src, V128* lookup, UInt imm8 ) {
   register __m128d src1_r = _mm_loadu_pd(src);
   register __m128i src2_r = _mm_loadu_si128(lookup);
   register __m128d src_dst_r = _mm_loadu_pd(dst);
   register __m128d res;
   IMM8_SWITCHCASE(_mm_fixupimm_sd, src_dst_r, src1_r, src2_r);
   _mm_storeu_pd(dst, res);
}
VEX_REGPARM(4) void h_Iop_FixupImm32F0x4 (V128* dst, V128* src, V128* lookup, UInt imm8 ) {
   register __m128 src1_r = _mm_loadu_ps(src);
   register __m128i src2_r = _mm_loadu_si128(lookup);
   register __m128 src_dst_r = _mm_loadu_ps(dst);
   register __m128 res;
   IMM8_SWITCHCASE(_mm_fixupimm_ss, src_dst_r, src1_r, src2_r);
   _mm_storeu_ps(dst, res);
}

int perm_counter = 0;

VEX_REGPARM(3) void h_Iop_Perm32x16 ( V512* dst, V512* index, V512* value ) {
    register __m512i src1_r = _mm512_loadu_si512(index);
    register __m512i src2_r = _mm512_loadu_si512(value);
    _mm512_storeu_si512(dst, _mm512_permutexvar_epi32(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Perm64x8 ( V512* dst, V512* index, V512* value ) {
   register __m512i src1_r = _mm512_loadu_si512(index);
   register __m512i src2_r = _mm512_loadu_si512(value);
   _mm512_storeu_si512(dst, _mm512_permutexvar_epi64(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI32x16 (V512* dst, V512* a, V512* b ) {
   register __m512i src_dst_r = _mm512_loadu_si512(dst);
   register __m512i src1_r = _mm512_loadu_si512(a);
   register __m512i src2_r = _mm512_loadu_si512(b);
   _mm512_storeu_si512(dst, _mm512_permutex2var_epi32(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI64x8 (V512* dst, V512* a, V512* b ) {
   register __m512i src_dst_r = _mm512_loadu_si512(dst);
   register __m512i src1_r = _mm512_loadu_si512(a);
   register __m512i src2_r = _mm512_loadu_si512(b);
   _mm512_storeu_si512(dst, _mm512_permutex2var_epi64(src1_r, src_dst_r, src2_r));
}

// Limitation for all comparisons: ignore the signaling
VEX_REGPARM(4) void h_Iop_Cmp64F0x2( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d src2_r = _mm_loadu_pd(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm_cmp_sd_mask, src1_r, src2_r);
   *dst = res;
}
VEX_REGPARM(4) void h_Iop_Cmp32F0x4 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 src2_r = _mm_loadu_ps(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm_cmp_ss_mask, src1_r, src2_r);
   *dst = res;
}
VEX_REGPARM(4) void h_Iop_Cmp64Fx8( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   register __m512d src1_r = _mm512_loadu_pd(src1);
   register __m512d src2_r = _mm512_loadu_pd(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm512_cmp_pd_mask, src1_r, src2_r);
   *dst = res;
}
VEX_REGPARM(4) void h_Iop_Cmp32Fx16 ( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   register __m512 src1_r = _mm512_loadu_ps(src1);
   register __m512 src2_r = _mm512_loadu_ps(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm512_cmp_ps_mask, src1_r, src2_r);
   *dst = res;
}
VEX_REGPARM(3) void h_Iop_ShrV128 ( V128* dst, V128* src, UInt imm8) {
   register __m128i src_r = _mm_loadu_si128(src);
   register __m128i res;
   IMM8_SWITCHCASE(_mm_srli_si128, src_r);
   _mm_storeu_si128(dst, res);
}
VEX_REGPARM(3) void h_Iop_ShlV128( V128* dst, V128* src, UInt imm8) {
   register __m128i src_r = _mm_loadu_si128(src);
   register __m128i res;
   IMM8_SWITCHCASE(_mm_slli_si128 , src_r);
   _mm_storeu_si128(dst, res);
}


// Limitation for all comparisons: ignore the signaling
// Intel Compiler bug: vpcmpq, vpcmpuq, vpcmpd and vpcmpud instructions with
// predicates 0x3 or 0x7 (_MM_CMPINT_FALSE or _MM_CMPINT_TRUE) are interpreted
// as invalid
// Hence, the handwritten implementation

// Use "define" to do the same check for any data types
#define CMP_WITH_PREDICATE(x, y, result, predicate) \
   switch (predicate) { \
      case 0x00: result = ((x) == (y)); break; \
      case 0x01: result = ((x) <  (y)); break; \
      case 0x02: result = ((x) <= (y)); break; \
      case 0x03: result = 0; break; \
      case 0x04: result = ((x) != (y)); break; \
      case 0x05: result = ((x) >= (y)); break; \
      case 0x06: result = ((x) >  (y)); break; \
      case 0x07: result = 1; break; \
      default:   break; \
   }

VEX_REGPARM(4) void h_Iop_Cmp64Sx8( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 8; i++) {
      CMP_WITH_PREDICATE( (Long)src1->w64[i], (Long)src2->w64[i], bit, imm8)
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp32Sx16 ( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 16; i++) {
      CMP_WITH_PREDICATE( (Int)src1->w32[i], (Int)src2->w32[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp64Ux8( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 8; i++) {
      CMP_WITH_PREDICATE( src1->w64[i], src2->w64[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp32Ux16 ( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 16; i++) {
      CMP_WITH_PREDICATE( src1->w32[i], src2->w32[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
#undef CMP_WITH_PREDICATE

VEX_REGPARM(2) void h_Iop_PrintI64(ULong* dst, ULong src) {
   vex_printf("0x%x\n", src);
   *dst = src;
}

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                             host_generic_AVX512_F.c ---*/
/*---------------------------------------------------------------*/
