
/*---------------------------------------------------------------*/
/*--- begin                                  ir_defs_AVX512.h ---*/
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
#ifndef __IR_DEFS_AVX512_H
#define __IR_DEFS_AVX512_H
#include "libvex_basictypes.h"
#include "libvex_ir.h"
#include "libvex.h"

#include "main_util.h"

#undef IS_VALID_VECTOR_TYPE
#define IS_VALID_VECTOR_TYPE(x) ((x) == Ity_V128 || (x) == Ity_V256 || (x) == Ity_V512)

void ppIRType_AVX512 ( IRType ty );
void ppIRConst_AVX512 ( const IRConst* con );
void ppIROp_AVX512 ( IROp op );
IRConst* IRConst_V512 ( ULong con );

void typeOfPrimop_AVX512 ( IROp op,
                    /*OUTs*/
                    IRType* t_dst,
                    IRType* t_arg1, IRType* t_arg2,
                    IRType* t_arg3, IRType* t_arg4 );


IRType typeOfIRConst_AVX512 ( const IRConst* con );
IRConst* deepCopyIRConst_AVX512 ( const IRConst* con );
Bool isPlausibleIRType_AVX512 ( IRType ty );
Int sizeofIRType_AVX512 ( IRType ty );
Bool primopMightTrap_AVX512 ( IROp op );

#endif /* ndef __IR_DEFS_AVX512_H */
#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                                    ir_defs_AVX512.h ---*/
/*---------------------------------------------------------------*/
