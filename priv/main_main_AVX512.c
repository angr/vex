/*---------------------------------------------------------------*/
/*--- begin                                main_main_AVX512.c ---*/
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
#include "main_main_AVX512.h"

void check_hwcaps_AVX512 ( VexArch arch, UInt hwcaps ) {
    Bool have_avx  = (hwcaps & VEX_HWCAPS_AMD64_AVX)  != 0;
    Bool have_avx2 = (hwcaps & VEX_HWCAPS_AMD64_AVX2) != 0;
    Bool have_avx512 = (((hwcaps & VEX_HWCAPS_AMD64_AVX512_KNL) != 0 ) || 
                        ((hwcaps & VEX_HWCAPS_AMD64_AVX512_SKX) != 0 ));

    /* AVX-512 without AVX or AVX2 */
    if (have_avx512 && !have_avx)
        invalid_hwcaps(arch, hwcaps,
                        "Support for AVX-512 requires AVX capabilities\n");
    if (have_avx512 && !have_avx2)
        invalid_hwcaps(arch, hwcaps,
                        "Support for AVX-512 requires AVX2 capabilities\n");

}

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                                  main_main_AVX512.c ---*/
/*---------------------------------------------------------------*/
