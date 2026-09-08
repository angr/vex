/*---------------------------------------------------------------*/
/*--- begin                                main_main_AVX512.h ---*/
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
#include "libvex.h"

const HChar* show_hwcaps_amd64 ( UInt hwcaps );
void invalid_hwcaps ( VexArch arch, UInt hwcaps, const HChar *message );
void check_hwcaps_AVX512 ( VexArch arch, UInt hwcaps );

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                                  main_main_AVX512.h ---*/
/*---------------------------------------------------------------*/
