/*---------------------------------------------------------------*/
/*--- begin                         host_generic_AVX512_ICL.c ---*/
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
/* Ice Lake enabling */

#ifdef AVX_512

#include "libvex_basictypes.h"
#include "host_generic_AVX512.h"
#include <emmintrin.h>
#include <immintrin.h>
#include "host_generic_imm8_def.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

VEX_REGPARM(3) void h_Iop_Perm8x16 ( V128* dst, V128* index, V128* value ) {
   register __m128i src1_r = _mm_loadu_si128(index);
   register __m128i src2_r = _mm_loadu_si128(value);
   _mm_storeu_si128(dst, _mm_permutexvar_epi8(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Perm8x32 ( V256* dst, V256* index, V256* value ) {
   register __m256i src1_r = _mm256_loadu_si256(index);
   register __m256i src2_r = _mm256_loadu_si256(value);
   _mm256_storeu_si256(dst, _mm256_permutexvar_epi8(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Perm8x64 ( V512* dst, V512* index, V512* value ) {
   register __m512i src1_r = _mm512_loadu_si512(index);
   register __m512i src2_r = _mm512_loadu_si512(value);
   _mm512_storeu_si512(dst, _mm512_permutexvar_epi8(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI8x16 (V128* dst, V128* a, V128* b ) {
   register __m128i src_dst_r = _mm_loadu_si128(dst);
   register __m128i src1_r = _mm_loadu_si128(a);
   register __m128i src2_r = _mm_loadu_si128(b);
   _mm_storeu_si128(dst, _mm_permutex2var_epi8(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI8x32 (V256* dst, V256* a, V256* b ) {
   register __m256i src_dst_r = _mm256_loadu_si256(dst);
   register __m256i src1_r = _mm256_loadu_si256(a);
   register __m256i src2_r = _mm256_loadu_si256(b);
   _mm256_storeu_si256(dst, _mm256_permutex2var_epi8(src1_r, src_dst_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_PermI8x64 (V512* dst, V512* a, V512* b ) {
   register __m512i src_dst_r = _mm512_loadu_si512(dst);
   register __m512i src1_r = _mm512_loadu_si512(a);
   register __m512i src2_r = _mm512_loadu_si512(b);
   _mm512_storeu_si512(dst, _mm512_permutex2var_epi8(src1_r, src_dst_r, src2_r));
}

VEX_REGPARM(2) void h_Iop_Expand8x16 (V128* dst, V128* src){};
VEX_REGPARM(2) void h_Iop_Expand16x8 (V128* dst, V128* src){};
VEX_REGPARM(2) void h_Iop_Compress8x16 (V128* dst, V128* src){};
VEX_REGPARM(2) void h_Iop_Compress16x8 (V128* dst, V128* src){};
VEX_REGPARM(3) void h_Iop_MULTISHIFTQB (V128* dst, V128* src1, V128* src2){};
VEX_REGPARM(3) void h_Iop_PMADD52LUQ  (V128* dst, V128* src1, V128* src2){};
VEX_REGPARM(3) void h_Iop_PMADD52HUQ  (V128* dst, V128* src1, V128* src2){};
VEX_REGPARM(2) void h_Iop_PopCount8x16(V128* dst, V128* src) {};
VEX_REGPARM(3) void h_Iop_DPBUSD (V128* dst, V128* src1, V128* src2){};
VEX_REGPARM(3) void h_Iop_DPBUSDS (V128* dst, V128* src1, V128* src2){};
VEX_REGPARM(3) void h_Iop_DPWSSD (V128* dst, V128* src1, V128* src2){};
VEX_REGPARM(3) void h_Iop_DPWSSDS (V128* dst, V128* src1, V128* src2){};
VEX_REGPARM(3) void h_Iop_VPSHUFBITQMB_128 (UInt* dst, V128* src1, V128* src2){};
VEX_REGPARM(3) void h_Iop_VPSHUFBITQMB_256 (UInt* dst, V256* src1, V256* src2){};
VEX_REGPARM(3) void h_Iop_VPSHUFBITQMB_512 (ULong* dst, V512* src1, V512* src2){};


#pragma GCC diagnostic pop

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                           host_generic_AVX512_ICL.c ---*/
/*---------------------------------------------------------------*/
