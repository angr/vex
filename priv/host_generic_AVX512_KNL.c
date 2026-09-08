/*---------------------------------------------------------------*/
/*--- begin                         host_generic_AVX512_KNL.c ---*/
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

/* AVX-512ER (Exponential and reciprocal) subset */

#ifdef AVX_512

#include "libvex_basictypes.h"
#include "host_generic_AVX512.h"
#include <emmintrin.h>
#include <immintrin.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

VEX_REGPARM(2) void h_Iop_Recip28_32x16( V512* dst, V512* src ) {
   register __m512 src_r = _mm512_loadu_ps(src);
   _mm512_storeu_ps(dst, _mm512_rcp28_ps(src_r));
}
VEX_REGPARM(2) void h_Iop_Recip28_64x8( V512* dst, V512* src ) {
   register __m512d src_r = _mm512_loadu_pd(src);
   _mm512_storeu_pd(dst, _mm512_rcp28_pd(src_r));
}
VEX_REGPARM(2) void h_Iop_RSqrt28_32x16( V512* dst, V512* src ) {
   register __m512 src_r = _mm512_loadu_ps(src);
   _mm512_storeu_ps(dst, _mm512_rsqrt28_ps(src_r));
}
VEX_REGPARM(2) void h_Iop_RSqrt28_64x8( V512* dst, V512* src ) {
   register __m512d src_r = _mm512_loadu_pd(src);
   _mm512_storeu_pd(dst, _mm512_rsqrt28_pd(src_r));
}
VEX_REGPARM(3) void h_Iop_Recip28_32F0x4( V128* dst, V128* src1, V128* src2 ) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 src2_r = _mm_loadu_ps(src2);
   _mm_storeu_ps(dst, _mm_rcp28_ss(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_Recip28_64F0x2( V128* dst, V128* src1, V128* src2 ) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d src2_r = _mm_loadu_pd(src2);
   _mm_storeu_pd(dst, _mm_rcp28_sd(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_RSqrt28_32F0x4( V128* dst, V128* src1, V128* src2 ) {
   register __m128 src1_r = _mm_loadu_ps(src1);
   register __m128 src2_r = _mm_loadu_ps(src2);
   _mm_storeu_ps(dst, _mm_rsqrt28_ss(src1_r, src2_r));
}
VEX_REGPARM(3) void h_Iop_RSqrt28_64F0x2( V128* dst, V128* src1, V128* src2 ) {
   register __m128d src1_r = _mm_loadu_pd(src1);
   register __m128d src2_r = _mm_loadu_pd(src2);
   _mm_storeu_pd(dst, _mm_rsqrt28_sd(src1_r, src2_r));
}
VEX_REGPARM(2) void h_Iop_Exp32x16( V512* dst, V512* src ) {
   register __m512 src_r = _mm512_loadu_ps(src);
   _mm512_storeu_ps(dst, _mm512_exp2a23_ps(src_r));
}
VEX_REGPARM(2) void h_Iop_Exp64x8( V512* dst, V512* src ) { 
   register __m512d src_r = _mm512_loadu_pd(src);
   _mm512_storeu_pd(dst, _mm512_exp2a23_pd(src_r));
}

#pragma GCC diagnostic pop

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                           host_generic_AVX512_KNL.c ---*/
/*---------------------------------------------------------------*/
