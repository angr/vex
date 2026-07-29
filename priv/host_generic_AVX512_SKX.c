/*---------------------------------------------------------------*/
/*--- begin                         host_generic_AVX512_SKX.c ---*/
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

/* Generic helper functions for doing AVX-512 instrucitons in cases
   where the instruction selectors cannot generate code in-line.
   These are purely back-end entities and cannot be seen/referenced
   from IR. */

/* The file impplenets instructions introduced in SkyLake machines:
 * AVX-512BW (Byte and Word) subset
 * AVX-512DQ (Doubleword and Quadword) subset
 * AVX-512VL (Vector length) subset
*/

#ifdef AVX_512

#include "libvex_basictypes.h"
#include "host_generic_AVX512.h"
#include <emmintrin.h>
#include <immintrin.h>
#include "host_generic_imm8_def.h"

// Use Intel instrinsics.
// I tried using a common wrapper and pass a pointer to intrinsic into the wrapper,
// but:
// 1. it does not link with gcc
// 2. the pointer is not a constant to icc

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wattributes"

VEX_REGPARM(3) void h_Iop_Test8x16 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_test_epi8_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test8x32 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_test_epi8_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test8x64 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_test_epi8_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test16x8 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_test_epi16_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test16x16 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_test_epi16_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test16x32 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_test_epi16_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test32x4 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_test_epi32_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test32x8 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_test_epi32_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test64x2 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_test_epi64_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_Test64x4 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_test_epi64_mask(src1_r, src2_r);
}

VEX_REGPARM(3) void h_Iop_TestN8x16 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_testn_epi8_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN8x32 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_testn_epi8_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN8x64 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_testn_epi8_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN16x8 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_testn_epi16_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN16x16 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_testn_epi16_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN16x32 ( ULong* dst, V512* src1, V512* src2) {
   register __m512i src1_r = _mm512_loadu_si512(src1);
   register __m512i src2_r = _mm512_loadu_si512(src2);
   *dst = _mm512_testn_epi16_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN32x4 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_testn_epi32_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN32x8 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_testn_epi32_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN64x2 ( ULong* dst, V128* src1, V128* src2) {
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i src2_r = _mm_loadu_si128(src2);
   *dst = _mm_testn_epi64_mask(src1_r, src2_r);
}
VEX_REGPARM(3) void h_Iop_TestN64x4 ( ULong* dst, V256* src1, V256* src2) {
   register __m256i src1_r = _mm256_loadu_si256(src1);
   register __m256i src2_r = _mm256_loadu_si256(src2);
   *dst = _mm256_testn_epi64_mask(src1_r, src2_r);
}

VEX_REGPARM(4) void h_Iop_Expand32x4( V128* src_dst, V128* vec, ULong mask, UInt z) {
   if (mask == 0) mask = -1;
   register __m128 src1_r = _mm_loadu_ps(src_dst);
   register __m128 src2_r = _mm_loadu_ps(vec);
   _mm_storeu_ps(src_dst,
         z ? _mm_maskz_expand_ps(mask, src2_r) :
             _mm_mask_expand_ps(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Expand32x8( V256* src_dst, V256* vec, ULong mask, UInt z) {
   if (mask == 0) mask = -1;
   register __m256 src1_r = _mm256_loadu_ps(src_dst);
   register __m256 src2_r = _mm256_loadu_ps(vec);
   _mm256_storeu_ps(src_dst,
         z ? _mm256_maskz_expand_ps(mask, src2_r) :
             _mm256_mask_expand_ps(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Expand64x2( V128* src_dst, V128* vec, ULong mask, UInt z) {
   if (mask == 0) mask = -1;
   register __m128d src1_r = _mm_loadu_pd(src_dst);
   register __m128d src2_r = _mm_loadu_pd(vec);
   _mm_storeu_pd(src_dst,
         z ? _mm_maskz_expand_pd(mask, src2_r) :
             _mm_mask_expand_pd(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Expand64x4( V256* src_dst, V256* vec, ULong mask, UInt z) {
   if (mask == 0) mask = -1;
   register __m256d src1_r = _mm256_loadu_pd(src_dst);
   register __m256d src2_r = _mm256_loadu_pd(vec);
   _mm256_storeu_pd(src_dst,
         z ? _mm256_maskz_expand_pd(mask, src2_r) :
             _mm256_mask_expand_pd(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Compress32x4( V128* src_dst, V128* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m128 src1_r = _mm_loadu_ps(src_dst);
   register __m128 src2_r = _mm_loadu_ps(vec);
   _mm_storeu_ps(src_dst, zero ?
         _mm_maskz_compress_ps(mask, src2_r):
         _mm_mask_compress_ps(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Compress32x8( V256* src_dst, V256* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m256 src1_r = _mm256_loadu_ps(src_dst);
   register __m256 src2_r = _mm256_loadu_ps(vec);
   _mm256_storeu_ps(src_dst, zero ?
         _mm256_maskz_compress_ps(mask, src2_r) :
         _mm256_mask_compress_ps(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Compress64x2( V128* src_dst, V128* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m128d src1_r = _mm_loadu_pd(src_dst);
   register __m128d src2_r = _mm_loadu_pd(vec);
   _mm_storeu_pd(src_dst, zero ?
         _mm_maskz_compress_pd(mask, src2_r):
         _mm_mask_compress_pd(src1_r, mask, src2_r));
}
VEX_REGPARM(4) void h_Iop_Compress64x4( V256* src_dst, V256* vec, ULong mask, UInt zero) {
   if (mask == 0) mask = -1;
   register __m256d src1_r = _mm256_loadu_pd(src_dst);
   register __m256d src2_r = _mm256_loadu_pd(vec);
   _mm256_storeu_pd(src_dst, zero ?
         _mm256_maskz_compress_pd(mask, src2_r) :
         _mm256_mask_compress_pd(src1_r, mask, src2_r));
}

/* Conflict detection */
VEX_REGPARM(2) void h_Iop_CfD32x4( V128* dst, V128* src) {
   register __m128i src_r = _mm_loadu_si128(src);
   _mm_storeu_si128(dst, _mm_conflict_epi32(src_r));
}
VEX_REGPARM(2) void h_Iop_CfD32x8( V256* dst, V256* src) {
   register __m256i src_r = _mm256_loadu_si256(src);
   _mm256_storeu_si256(dst, _mm256_conflict_epi32(src_r));
}
VEX_REGPARM(2) void h_Iop_CfD64x2( V128* dst, V128* src) {
   register __m128i src_r = _mm_loadu_si128(src);
   _mm_storeu_si128(dst, _mm_conflict_epi64(src_r));
}
VEX_REGPARM(2) void h_Iop_CfD64x4( V256* dst, V256* src) {
   register __m256i src_r = _mm256_loadu_si256(src);
   _mm256_storeu_si256(dst, _mm256_conflict_epi64(src_r));
}

VEX_REGPARM(3) void h_Iop_Perm16x8 ( V128* dst, V128* index, V128* value ) {
   register __m128i src1_r = _mm_loadu_si128(index);
   register __m128i src2_r = _mm_loadu_si128(value);
   _mm_storeu_si128(dst, _mm_permutexvar_epi16(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Perm16x16 ( V256* dst, V256* index, V256* value ) {
   register __m256i src1_r = _mm256_loadu_si256(index);
   register __m256i src2_r = _mm256_loadu_si256(value);
   _mm256_storeu_si256(dst, _mm256_permutexvar_epi16(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Perm64x4 ( V256* dst, V256* index, V256* value ) {
   register __m256i src1_r = _mm256_loadu_si256(index);
   register __m256i src2_r = _mm256_loadu_si256(value);
   _mm256_storeu_si256(dst, _mm256_permutexvar_epi64(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Perm16x32 ( V512* dst, V512* index, V512* value ) {
   register __m512i src1_r = _mm512_loadu_si512(index);
   register __m512i src2_r = _mm512_loadu_si512(value);
   _mm512_storeu_si512(dst, _mm512_permutexvar_epi16(src1_r, src2_r));
}

VEX_REGPARM(3) void h_Iop_PermI16x8 (V128* dst, V128* a, V128* b ) {
   register __m128i src_dst_r = _mm_loadu_si128(dst);
   register __m128i src1_r = _mm_loadu_si128(a);
   register __m128i src2_r = _mm_loadu_si128(b);
   _mm_storeu_si128(dst, _mm_permutex2var_epi16(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI32x4 (V128* dst, V128* a, V128* b ) {
   register __m128i src_dst_r = _mm_loadu_si128(dst);
   register __m128i src1_r = _mm_loadu_si128(a);
   register __m128i src2_r = _mm_loadu_si128(b);
   _mm_storeu_si128(dst, _mm_permutex2var_epi32(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI64x2 (V128* dst, V128* a, V128* b ) {
   register __m128i src_dst_r = _mm_loadu_si128(dst);
   register __m128i src1_r = _mm_loadu_si128(a);
   register __m128i src2_r = _mm_loadu_si128(b);
   _mm_storeu_si128(dst, _mm_permutex2var_epi64(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI16x16 (V256* dst, V256* a, V256* b ) {
   register __m256i src_dst_r = _mm256_loadu_si256(dst);
   register __m256i src1_r = _mm256_loadu_si256(a);
   register __m256i src2_r = _mm256_loadu_si256(b);
   _mm256_storeu_si256(dst, _mm256_permutex2var_epi16(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI32x8 (V256* dst, V256* a, V256* b ) {
   register __m256i src_dst_r = _mm256_loadu_si256(dst);
   register __m256i src1_r = _mm256_loadu_si256(a);
   register __m256i src2_r = _mm256_loadu_si256(b);
   _mm256_storeu_si256(dst, _mm256_permutex2var_epi32(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI64x4 (V256* dst, V256* a, V256* b ) {
   register __m256i src_dst_r = _mm256_loadu_si256(dst);
   register __m256i src1_r = _mm256_loadu_si256(a);
   register __m256i src2_r = _mm256_loadu_si256(b);
   _mm256_storeu_si256(dst, _mm256_permutex2var_epi64(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI16x32 (V512* dst, V512* a, V512* b ) {
   register __m512i src_dst_r = _mm512_loadu_si512(dst);
   register __m512i src1_r = _mm512_loadu_si512(a);
   register __m512i src2_r = _mm512_loadu_si512(b);
   _mm512_storeu_si512(dst, _mm512_permutex2var_epi16(src1_r, src_dst_r, src2_r));
}

// Limitation for all comparisons: ignore the signaling
VEX_REGPARM(4) void h_Iop_Cmp64Fx2( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d src2_r = _mm_loadu_pd(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm_cmp_pd_mask, src1_r, src2_r);
   *dst = res;
}
VEX_REGPARM(4) void h_Iop_Cmp32Fx4 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 src2_r = _mm_loadu_ps(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm_cmp_ps_mask, src1_r, src2_r);
   *dst = res;
}
VEX_REGPARM(4) void h_Iop_Cmp64Fx4( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   register __m256d src1_r = _mm256_loadu_pd(src1);
   register __m256d src2_r = _mm256_loadu_pd(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm256_cmp_pd_mask, src1_r, src2_r);
   *dst = res;
}
VEX_REGPARM(4) void h_Iop_Cmp32Fx8 ( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   register __m256 src1_r = _mm256_loadu_ps(src1);
   register __m256 src2_r = _mm256_loadu_ps(src2);
   ULong res = 0;
   IMM8_SWITCHCASE_5(_mm256_cmp_ps_mask, src1_r, src2_r);
   *dst = res;
}

VEX_REGPARM(3) void h_Iop_Classify32F0x4 ( ULong* dst, V128* src, UInt imm8) {
   register __m128 src1_r = _mm_loadu_ps(src);
   ULong res = 0;
   IMM8_SWITCHCASE(_mm_fpclass_ss_mask, src1_r);
   *dst = res;
}
VEX_REGPARM(3) void h_Iop_Classify64F0x2 ( ULong* dst, V128* src, UInt imm8) {
   register __m128d src1_r = _mm_loadu_pd(src);
   ULong res = 0;
   IMM8_SWITCHCASE(_mm_fpclass_sd_mask, src1_r);
   *dst = res;
}
VEX_REGPARM(3) void h_Iop_Classify32x4 ( ULong* dst, V128* src, UInt imm8) {
   register __m128 src1_r = _mm_loadu_ps(src);
   ULong res = 0;
   IMM8_SWITCHCASE(_mm_fpclass_ps_mask, src1_r);
   *dst = res;
}
VEX_REGPARM(3) void h_Iop_Classify64x2 ( ULong* dst, V128* src, UInt imm8) {
   register __m128d src1_r = _mm_loadu_pd(src);
   ULong res = 0;
   IMM8_SWITCHCASE(_mm_fpclass_pd_mask, src1_r);
   *dst = res;
}
VEX_REGPARM(3) void h_Iop_Classify32x8 ( ULong* dst, V256* src, UInt imm8) {
   register __m256 src1_r = _mm256_loadu_ps(src);
   ULong res = 0;
   IMM8_SWITCHCASE(_mm256_fpclass_ps_mask, src1_r);
   *dst = res;
}
VEX_REGPARM(3) void h_Iop_Classify64x4 ( ULong* dst, V256* src, UInt imm8) {
   register __m256d src1_r = _mm256_loadu_pd(src);
   ULong res = 0;
   IMM8_SWITCHCASE(_mm256_fpclass_pd_mask, src1_r);
   *dst = res;
}
VEX_REGPARM(3) void h_Iop_Classify32x16 ( ULong* dst, V512* src, UInt imm8) {
      register __m512 src1_r = _mm512_loadu_ps(src);
      ULong res = 0;
      IMM8_SWITCHCASE(_mm512_fpclass_ps_mask, src1_r);
      *dst = res;
}
VEX_REGPARM(3) void h_Iop_Classify64x8 ( ULong* dst, V512* src, UInt imm8) {
   register __m512d src1_r = _mm512_loadu_pd(src);
   ULong res = 0;
   IMM8_SWITCHCASE(_mm512_fpclass_pd_mask, src1_r);
   *dst = res;
}

VEX_REGPARM(3) void h_Iop_Reduce64x2 (V128* dst, V128* src1, UInt imm8 ) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d res;
   IMM8_SWITCHCASE(_mm_reduce_pd, src1_r);
   _mm_storeu_pd(dst, res);
}
VEX_REGPARM(3) void h_Iop_Reduce32x4 ( V128* dst, V128* src1, UInt imm8 ) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 res;
   IMM8_SWITCHCASE(_mm_reduce_ps, src1_r);
   _mm_storeu_ps(dst, res);
}
VEX_REGPARM(3) void h_Iop_Reduce64F0x2 (V128* src_dst, V128* src1, UInt imm8 ) {
   register __m128d src_dst_r = _mm_loadu_pd(src_dst);
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d res;
   IMM8_SWITCHCASE(_mm_reduce_sd, src_dst_r, src1_r);
   _mm_storeu_pd(src_dst, res);
}
VEX_REGPARM(3) void h_Iop_Reduce32F0x4 ( V128* src_dst, V128* src1, UInt imm8 ) {
   register __m128 src_dst_r = _mm_loadu_ps(src_dst);
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 res;
   IMM8_SWITCHCASE(_mm_reduce_ss, src_dst_r, src1_r);
   _mm_storeu_ps(src_dst, res);
}
VEX_REGPARM(3) void h_Iop_Range64x2 ( V128* src_dst, V128* src1, UInt imm8 ) {
   register __m128d src_dst_r = _mm_loadu_pd(src_dst);
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d res;
   imm8 &= 0xF;
   IMM8_SWITCHCASE_4(_mm_range_pd, src_dst_r, src1_r);
   _mm_storeu_pd(src_dst, res);
}
VEX_REGPARM(3) void h_Iop_Range32x4 ( V128* src_dst, V128* src1, UInt imm8 ) {
   register __m128 src_dst_r = _mm_loadu_ps(src_dst);
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 res;
   imm8 &= 0xF;
   IMM8_SWITCHCASE_4(_mm_range_ps, src_dst_r, src1_r);
   _mm_storeu_ps(src_dst, res);
}
VEX_REGPARM(3) void h_Iop_Range64F0x2 ( V128* src_dst, V128* src1, UInt imm8 ) {
   register __m128d src_dst_r = _mm_loadu_pd(src_dst);
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d res;
   imm8 &= 0xF;
   IMM8_SWITCHCASE_4(_mm_range_sd, src_dst_r, src1_r);
   _mm_storeu_pd(src_dst, res);
}
VEX_REGPARM(3) void h_Iop_Range32F0x4 ( V128* src_dst, V128* src1, UInt imm8 ) {
   register __m128 src_dst_r = _mm_loadu_ps(src_dst);
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 res;
   imm8 &= 0xF;
   IMM8_SWITCHCASE_4(_mm_range_ss, src_dst_r, src1_r);
   _mm_storeu_ps(src_dst, res);
}
VEX_REGPARM(3) void h_Iop_VDBPSADBW ( V128* src_dst, V128* src1, UInt imm8 ) {
   register __m128i src_dst_r = _mm_loadu_si128(src_dst);
   register __m128i src1_r = _mm_loadu_si128(src1);
   register __m128i res;
   IMM8_SWITCHCASE(_mm_dbsad_epu8, src_dst_r, src1_r);
   _mm_storeu_si128(src_dst, res);
}

VEX_REGPARM(2) void h_Iop_16Sto8x8 ( ULong* dst, V128* src ) {
   register __m128i src_r = _mm_loadu_si128(src);
   _mm_storel_epi64(dst, _mm_cvtsepi16_epi8(src_r));
}


// Limitation for all comparisons: ignore the signaling
// Intel Compiler bug: vpcmpq, vpcmpuq, vpcmpd, vpcmpud, vpcmpw, vpcmpuw,
// vpcmpb and vpcmpub instructions with predicates 0x3 or 0x7
// (_MM_CMPINT_FALSE or _MM_CMPINT_TRUE) are interpreted as invalid
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

VEX_REGPARM(4) void h_Iop_Cmp64Sx2( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 2; i++) {
      CMP_WITH_PREDICATE( (Long)src1->w64[i], (Long)src2->w64[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp64Sx4( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 4; i++) {
      CMP_WITH_PREDICATE( (Long)src1->w64[i], (Long)src2->w64[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp32Sx4 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 4; i++) {
      CMP_WITH_PREDICATE( (Int)src1->w32[i], (Int)src2->w32[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp32Sx8 ( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 8; i++) {
      CMP_WITH_PREDICATE( (Int)src1->w32[i], (Int)src2->w32[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp16Sx8 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 8; i++) {
      CMP_WITH_PREDICATE( (Short)src1->w16[i], (Short)src2->w16[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp16Sx16 ( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 16; i++) {
      CMP_WITH_PREDICATE( (Short)src1->w16[i], (Short)src2->w16[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp16Sx32 ( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 32; i++) {
      CMP_WITH_PREDICATE( (Short)src1->w16[i], (Short)src2->w16[i], bit, imm8);
      result |= ((UInt) bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp8Sx16 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 16; i++) {
      CMP_WITH_PREDICATE( (Char)src1->w8[i], (Char)src2->w8[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp8Sx32 ( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 32; i++) {
      CMP_WITH_PREDICATE( (Char)src1->w8[i], (Char)src2->w8[i], bit, imm8);
      result |= ((UInt) bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp8Sx64 ( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 64; i++) {
      CMP_WITH_PREDICATE( (Char)src1->w8[i], (Char)src2->w8[i], bit, imm8);
      result |= ((ULong) bit << i);
   }
   *dst = result;
}

VEX_REGPARM(4) void h_Iop_Cmp64Ux2( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 2; i++) {
      CMP_WITH_PREDICATE( src1->w64[i], src2->w64[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp64Ux4( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 4; i++) {
      CMP_WITH_PREDICATE( src1->w64[i], src2->w64[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp32Ux4 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 4; i++) {
      CMP_WITH_PREDICATE( src1->w32[i], src2->w32[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp32Ux8 ( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 8; i++) {
      CMP_WITH_PREDICATE( src1->w32[i], src2->w32[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp16Ux8 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 8; i++) {
      CMP_WITH_PREDICATE( src1->w16[i], src2->w16[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp16Ux16 ( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 16; i++) {
      CMP_WITH_PREDICATE( src1->w16[i], src2->w16[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp16Ux32 ( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 32; i++) {
      CMP_WITH_PREDICATE( src1->w16[i], src2->w16[i], bit, imm8);
      result |= ((UInt) bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp8Ux16 ( ULong* dst, V128* src1, V128* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 16; i++) {
      CMP_WITH_PREDICATE( src1->w8[i], src2->w8[i], bit, imm8);
      result |= (bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp8Ux32 ( ULong* dst, V256* src1, V256* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 32; i++) {
      CMP_WITH_PREDICATE( src1->w8[i], src2->w8[i], bit, imm8);
      result |= ((UInt) bit << i);
   }
   *dst = result;
}
VEX_REGPARM(4) void h_Iop_Cmp8Ux64 ( ULong* dst, V512* src1, V512* src2, UInt imm8) {
   ULong result = 0x0;
   Bool bit = False;
   imm8 &= 0x7;
   for (Int i = 0; i < 64; i++) {
      CMP_WITH_PREDICATE( src1->w8[i], src2->w8[i], bit, imm8);
      result |= ((ULong) bit << i);
   }
   *dst = result;
}
#undef CMP_WITH_PREDICATE

#pragma GCC diagnostic pop

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                           host_generic_AVX512_SKX.c ---*/
/*---------------------------------------------------------------*/
