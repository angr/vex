
/*---------------------------------------------------------------*/
/*--- begin                                  ir_defs_AVX512.c ---*/
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

#include "libvex_basictypes.h"
#include "libvex_ir.h"
#include "libvex.h"

#include "ir_defs_AVX512.h"
#include "host_AVX512.h"
#include "main_util.h"


void ppIRType_AVX512 ( IRType ty )
{
   if (ty == Ity_V512) {
      vex_printf("V512");
      return;
   }
   vpanic("ppIRType_AVX512");
}

void ppIRConst_AVX512 ( const IRConst* con )
{
   if (con->tag == Ico_V512) {
      vex_printf( "V512{0x%08llx}", con->Ico.V512);
      return;
   }
   vpanic("ppIRConst_AVX512");
}

void ppIROp_AVX512 ( IROp op )
{
   if ((op > Iop_LAST_NOT_EVEX) && (op < Iop_LAST)) {
      vex_printf("%s", IOPS_ARR[INDEX(op)].name);
      return;
   }
   vpanic("ppIROp_AVX512");
}

IRConst* IRConst_V512 ( ULong con ) {
   IRConst* c  = LibVEX_Alloc_inline(sizeof(IRConst));
   c->tag      = Ico_V512;
   c->Ico.V512 = con;
   return c;
}

/*---------------------------------------------------------------*/
/*--- Primop types                                            ---*/
/*---------------------------------------------------------------*/

void typeOfPrimop_AVX512 ( IROp op,
      /*OUTs*/
      IRType* t_dst,
      IRType* t_arg1, IRType* t_arg2,
      IRType* t_arg3, IRType* t_arg4 )
{
   *t_dst  = Ity_INVALID;
   *t_arg1 = Ity_INVALID;
   *t_arg2 = Ity_INVALID;
   *t_arg3 = Ity_INVALID;
   *t_arg4 = Ity_INVALID;

   Iop_data iop = IOPS_ARR[INDEX(op)];
   Int argN = 0;
   while ((argN < 5) && (iop.operands[argN] != 0)) {
      argN++;
   }
   *t_dst = iop.operands[0];

   switch (argN) { // fallthrough
      case 5: *t_arg4 = iop.operands[4]; __attribute__ ((fallthrough));
      case 4: *t_arg3 = iop.operands[3]; __attribute__ ((fallthrough));
      case 3: *t_arg2 = iop.operands[2]; __attribute__ ((fallthrough));
      case 2: *t_arg1 = iop.operands[1]; break;
      default:
         ppIROp(op);
         vpanic("typeOfPrimop_AVX512");
   }
}

IRType typeOfIRConst_AVX512 ( const IRConst* con )
{
   if (con->tag == Ico_V512) {
      return Ity_V512;
   }
   vpanic("typeOfIRConst_AVX512");
}
IRConst* deepCopyIRConst_AVX512 ( const IRConst* con )
{
   if (con->tag == Ico_V512)
      return IRConst_V512(con->Ico.V512);
   vpanic("deepCopyIRConst_AVX512");
}

Bool isPlausibleIRType_AVX512 ( IRType ty )
{
   return (ty == Ity_V512);
}

Int sizeofIRType_AVX512 ( IRType ty )
{
   if (ty == Ity_V512)
      return 64;
   vex_printf("\n"); ppIRType(ty); vex_printf("\n");
   vpanic("sizeofIRType_AVX512");
}

Bool primopMightTrap_AVX512 ( IROp op )
{
   if ((op > Iop_LAST_NOT_EVEX) && (op < Iop_LAST))
      return False;
   vpanic("primopMightTrap_AVX512");
}

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                                    ir_defs_AVX512.c ---*/
/*---------------------------------------------------------------*/
