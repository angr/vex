/*---------------------------------------------------------------*/
/*--- begin                          host_amd64_isel_AVX512.h ---*/
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
#ifndef __HOST_AMD64_ISEL512_H
#define __HOST_AMD64_ISEL512_H

#include "libvex_basictypes.h"
#include "libvex_ir.h"
#include "libvex.h"
#include "ir_match.h"
#include "host_generic_regs.h"
#include "host_amd64_defs.h"
#include "host_AVX512.h"

#define MULT512 4

typedef
struct {
    /* Constant -- are set at the start and do not change. */
    IRTypeEnv*   type_env;
    HReg*        vregmaps[MULT512];
    Int          n_vregmap;
    UInt         hwcaps;
    Bool         chainingAllowed;
    Addr64       max_ga;
    /* These are modified as we go along. */
    HInstrArray* code;
    Int          vreg_ctr;
}
ISelEnv;

#define vregmap vregmaps[0]
#define vregmapHI vregmaps[1]

void iselExpr_512    ( /*OUT*/HReg *dst, ISelEnv* env, const IRExpr* e );
void iselStmt_512    (ISelEnv* env, IRStmt* stmt);
void doHelperCall_512( /*OUT*/UInt* stackAdjustAfterCall, /*OUT*/RetLoc* retloc,
        ISelEnv* env, IRExpr* guard, IRCallee* cee, IRType retTy, IRExpr** args);

#endif /*__HOST_AMD64_ISEL512_H*/
#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                            host_amd64_isel_AVX512.h ---*/
/*---------------------------------------------------------------*/
