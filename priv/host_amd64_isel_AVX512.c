
/*---------------------------------------------------------------*/
/*--- begin                          host_amd64_isel_AVX512.c ---*/
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

#include "ir_match.h"
#include "host_generic_simd64.h"
#include "host_generic_simd128.h"
#include "host_generic_simd256.h"
#include <host_generic_AVX512.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

void doHelperCall_512 (
      /*OUT*/UInt*   stackAdjustAfterCall,
      /*OUT*/RetLoc* retloc,
      ISelEnv* env,
      IRExpr* guard,
      IRCallee* cee, IRType retTy, IRExpr** args )
{
   AMD64CondCode cc = Acc_ALWAYS;
   HReg argregs[6] = {0};
   HReg tmpregs[6] = {0};
   UInt n_args=0, i=0;

   /* Set default returns.  We'll update them later if needed. */
   *stackAdjustAfterCall = 0;
   *retloc               = mk_RetLoc_INVALID();

   /* These are used for cross-checking that IR-level constraints on
      the use of IRExpr_VECRET() and IRExpr_GSPTR() are observed. */
   UInt nVECRETs = 0;
   UInt nGSPTRs  = 0;

   if (retTy != Ity_V512) {
      vassert(0);
   }

   /* Marshal args for a call and do the call.

      Do the slow shceme foe 512-bit vecor code.

      In the slow scheme, all args are first computed into vregs, and
      once they are all done, they are moved to the relevant real
      regs.  This always gives correct code, but it also gives a bunch
      of vreg-to-rreg moves which are usually redundant but are hard
      for the register allocator to get rid of.
      */

   /* Note that the cee->regparms field is meaningless on AMD64 host
      (since there is only one calling convention) and so we always
      ignore it. */
   n_args = 0;
   for (i = 0; args[i]; i++) {
      n_args++;
   }
   vassert(n_args <= 6);

   argregs[0] = hregAMD64_RDI();
   argregs[1] = hregAMD64_RSI();
   argregs[2] = hregAMD64_RDX();
   argregs[3] = hregAMD64_RCX();
   argregs[4] = hregAMD64_R8();
   argregs[5] = hregAMD64_R9();

   tmpregs[0] = tmpregs[1] = tmpregs[2] =
      tmpregs[3] = tmpregs[4] = tmpregs[5] = INVALID_HREG;

#  if 0 /* debug only */
   if (n_args > 0) {for (i = 0; args[i]; i++) {
      ppIRExpr(args[i]); vex_printf(" "); }
   vex_printf("\n");}
#  endif

   /* Allocate a place for it on the stack and record its address. */
   HReg r_vecRetAddr = INVALID_HREG;
   r_vecRetAddr = newVRegI(env);
   sub_from_rsp(env, 64);
   addInstr(env, mk_iMOVsd_RR( hregAMD64_RSP(), r_vecRetAddr ));

   for (i = 0; i < n_args; i++) {
      IRExpr* arg = args[i];
      if (UNLIKELY(arg->tag == Iex_GSPTR)) {
         tmpregs[i] = newVRegI(env);
         addInstr(env, mk_iMOVsd_RR( hregAMD64_RBP(), tmpregs[i]));
         nGSPTRs++;
      }
      else if (UNLIKELY(arg->tag == Iex_VECRET)) {
         /* We stashed the address of the return slot earlier, so just
            retrieve it now. */
         vassert(!hregIsInvalid(r_vecRetAddr));
         tmpregs[i] = r_vecRetAddr;
         nVECRETs++;
      }
      else {
         vassert(typeOfIRExpr(env->type_env, args[i]) == Ity_I64);
         tmpregs[i] = iselIntExpr_R(env, args[i]);
      }
   }

   /* Now we can compute the condition.  We can't do it earlier
      because the argument computations could trash the condition
      codes.  Be a bit clever to handle the common case where the
      guard is 1:Bit. */
   if (guard) {
      if (guard->tag == Iex_Const
            && guard->Iex.Const.con->tag == Ico_U1
            && guard->Iex.Const.con->Ico.U1 == True) {
         /* unconditional -- do nothing */
      } else {
         cc = iselCondCode_C( env, guard );
      }
   }

   /* Move the args to their final destinations. */
   for (i = 0; i < n_args; i++) {
      /* None of these insns, including any spill code that might
         be generated, may alter the condition codes. */
      addInstr( env, mk_iMOVsd_RR( tmpregs[i], argregs[i] ) );
   }

   /* Do final checks, set the return values, and generate the call
      instruction proper. */
   vassert(nVECRETs == 1);
   vassert(nGSPTRs == 0 || nGSPTRs == 1);
   vassert(*stackAdjustAfterCall == 0);
   vassert(is_RetLoc_INVALID(*retloc));

   *retloc = mk_RetLoc_spRel(RLPri_V512SpRel, 0);
   *stackAdjustAfterCall = 64;
   /* Finally, generate the call itself.  This needs the *retloc value
      set in the switch above, which is why it's at the end. */
   addInstr(env, AMD64Instr_Call(cc, (Addr)cee->addr, n_args, *retloc));
}


static void lookupIRTempQuartet ( HReg* vec, ISelEnv* env, IRTemp tmp )
{
   vassert(tmp < env->n_vregmap);
   for (int i=0; i<MULT512; i++) {
      vassert(! hregIsInvalid(env->vregmaps[i][tmp]));
   }
   for (int i=0; i<MULT512; i++) {
      vec[i] = env->vregmaps[i][tmp];
   }
}


void iselStmt_512(ISelEnv* env, IRStmt* stmt)
{
   switch (stmt->tag) {
      case Ist_Store:
         if (Ity_V512 == typeOfIRExpr(env->type_env, stmt->Ist.Store.data)) {
            HReg vec[MULT512] = {0};
            AMD64AMode* am = NULL;
            HReg rA = iselIntExpr_R(env, stmt->Ist.Store.addr);
            iselExpr_512(vec, env, stmt->Ist.Store.data);
            for (Int i = 0; i < MULT512; i++) {
               am = AMD64AMode_IR(i*16, rA);
               addInstr(env, AMD64Instr_SseLdSt(False, 16, vec[i], am));
            }
            return;
         }
         break;
      case Ist_Put:
         if (Ity_V512 == typeOfIRExpr(env->type_env, stmt->Ist.Put.data)) {
            HReg vec[MULT512] = {0};
            AMD64AMode* am = NULL;
            HReg rbp = hregAMD64_RBP();
            iselExpr_512(vec, env, stmt->Ist.Put.data);
            for (Int i = 0; i < MULT512; i++) {
               am = AMD64AMode_IR(stmt->Ist.Put.offset + i*16, rbp);
               addInstr(env, AMD64Instr_SseLdSt(False, 16, vec[i], am));
            }
            return;
         }
         break;
      case Ist_WrTmp:
         if (Ity_V512 == typeOfIRTemp(env->type_env, stmt->Ist.WrTmp.tmp)) {
            HReg res[MULT512] = {0}, dst[MULT512] = {0};
            iselExpr_512(res, env, stmt->Ist.WrTmp.data);
            lookupIRTempQuartet(dst, env, stmt->Ist.WrTmp.tmp);
            for (Int i = 0; i < MULT512; i++) {
               addInstr(env, mk_vMOVsd_RR(res[i], dst[i]));
            }
            return;
         }
         break;
      case Ist_Dirty: {
         IRDirty* d = stmt->Ist.Dirty.details;
         if (Ity_V512 == typeOfIRTemp(env->type_env, d->tmp)) {
            /* Marshal args, do the call, and set the return value to
               0x555..555 if this is a conditional call that returns a value
               and the call is skipped. */
            UInt   addToSp = 0;
            RetLoc rloc    = mk_RetLoc_INVALID();
            doHelperCall_512( &addToSp, &rloc, env, d->guard, d->cee, Ity_V512, d->args );
            vassert(is_sane_RetLoc(rloc));

            /* Now deal with the returned value */
            vassert(rloc.pri == RLPri_V512SpRel);
            vassert(addToSp >= 64);
            HReg dst[MULT512];
            AMD64AMode* am;
            lookupIRTempQuartet(dst, env, d->tmp);

            for (Int i = 0; i < MULT512; i++) {
               am = AMD64AMode_IR(rloc.spOff+i*16, hregAMD64_RSP());
               addInstr(env, AMD64Instr_SseLdSt( True, 16, dst[i], am));
            }
            add_to_rsp(env, addToSp);
            return;
         }
         break;
      }
      default: break;
   }
   ppIRStmt(stmt);
   vpanic("iselStmt(amd64)_512");
}


typedef AMD64Instr* (*asm_call)(AMD64SseOp, HReg, HReg);

static void select_IRExpr_argument (ISelEnv* env, const IRExpr* ir, HReg* arg) {
   switch (typeOfIRExpr(env->type_env, ir)) {
      case Ity_V512: iselExpr_512(arg, env, ir);               break;
      case Ity_V256: iselDVecExpr(&arg[1], &arg[0], env, ir);  break;
      case Ity_V128: arg[0] = iselVecExpr(env, ir);            break;
      case Ity_I64: 
      case Ity_I32:
      case Ity_I16:
      case Ity_I8:   arg[0] = iselIntExpr_R(env, ir);          break;
      default:       vpanic("select_IRExpr_argument\n");
   }
}

static void store_IRExpr_at_register(ISelEnv* env, IRType type, HReg* arg, HReg reg) {
   switch (type) {
      case Ity_V512: 
         for (Int i = 0; i < MULT512; i++) 
            addInstr(env, AMD64Instr_SseLdSt(False, 16, arg[i], AMD64AMode_IR(i*16, reg)));
         break;
      case Ity_V256:
         addInstr(env, AMD64Instr_SseLdSt(False, 16, arg[1], AMD64AMode_IR(16, reg)));
         addInstr(env, AMD64Instr_SseLdSt(False, 16, arg[0], AMD64AMode_IR(0, reg)));
         break;
      case Ity_V128:
         addInstr(env, AMD64Instr_SseLdSt(False, 16, arg[0], AMD64AMode_IR(0, reg)));
         break;
      case Ity_I64: 
      case Ity_I32:
      case Ity_I16:
      case Ity_I8: 
         addInstr(env, AMD64Instr_Alu64R(Aalu_MOV, AMD64RMI_Reg(arg[0]), reg));
         break;
      default:
         vpanic("store_IRExpr_at_register\n");
   }
}

static void load_HReg_from_register(ISelEnv* env, const IRExpr* ir, HReg* dst, HReg reg) {
   switch (typeOfIRExpr(env->type_env, ir)) {
      case Ity_V512:
         for (Int i = 0; i < MULT512; i++) {
            addInstr(env, AMD64Instr_SseLdSt(True, 16, dst[i], AMD64AMode_IR(i*16, reg)));
         }
         break;
      case Ity_V256:
         addInstr(env, AMD64Instr_SseLdSt(True, 16, dst[1], AMD64AMode_IR(16, reg)));
         addInstr(env, AMD64Instr_SseLdSt(True, 16, dst[0], AMD64AMode_IR(0, reg)));
         break;
      case Ity_V128:
         addInstr(env, AMD64Instr_SseLdSt(True, 16, *dst, AMD64AMode_IR(0, reg)));
         break;
      case Ity_I64:
      case Ity_I32:
      case Ity_I16:
      case Ity_I8:
         addInstr(env, AMD64Instr_Alu64R(Aalu_MOV, AMD64RMI_Mem(AMD64AMode_IR(0, reg)), *dst));
         break;
      default:
         vpanic("load_HReg_from_register\n");
   }
}

static void handle_unop(HReg *dst, ISelEnv* env, const IRExpr* e)
{
   HWord fn = 0;
   // Special cases:
   switch (e->Iex.Unop.op) {
      case (IROp)Iop_NotV512: {
         HReg arg[MULT512] = {0};
         iselExpr_512(arg, env, e->Iex.Unop.arg);
         for (Int i = 0; i < MULT512; i++) {
            dst[i] = do_sse_NotV128(env, arg[i]);
         }
         return;
      }
      case (IROp)Iop_V512toV256_0: {
         HReg arg[MULT512] = {0};
         iselExpr_512(arg, env, e->Iex.Unop.arg);
         dst[0] = arg[0];
         dst[1] = arg[1];
         return;
      }
      case (IROp)Iop_V512toV256_1: {
         HReg arg[MULT512] = {0};
         iselExpr_512(arg, env, e->Iex.Unop.arg);
         dst[0] = arg[2];
         dst[1] = arg[3];
         return;
      }
      case (IROp)Iop_V512to64_0: case (IROp)Iop_V512to64_1:
      case (IROp)Iop_V512to64_2: case (IROp)Iop_V512to64_3:
      case (IROp)Iop_V512to64_4: case (IROp)Iop_V512to64_5:
      case (IROp)Iop_V512to64_6: case (IROp)Iop_V512to64_7: {
         HReg temp[MULT512] = {0};
         HReg vec = newVRegV(env);
         iselExpr_512(temp, env, e->Iex.Unop.arg);
         UInt octet = e->Iex.Unop.op - Iop_V512to64_0;
         vec = temp[octet/2];
         Int off = 0;
         switch (octet) {
            case 0: case 2: case 4: case 6:
               off = -16; break;
            default:
               off = -8; break;
         }
         HReg        rsp     = hregAMD64_RSP();
         AMD64AMode* m16_rsp = AMD64AMode_IR(-16, rsp);
         AMD64AMode* off_rsp = AMD64AMode_IR(off, rsp);
         addInstr(env, AMD64Instr_SseLdSt(False, 16, vec, m16_rsp));
         addInstr(env, AMD64Instr_Alu64R( Aalu_MOV, AMD64RMI_Mem(off_rsp), *dst ));
         return;
      }
      case (IROp)Iop_CmpNEZ32x16: {
         HReg arg[MULT512] = {0}, tmp[MULT512] = {0};
         HReg zero = generate_zeroes_V128(env);
         iselExpr_512(arg, env, e->Iex.Unop.arg);
         for (Int i = 0; i < MULT512; i++) {
            tmp[i] = newVRegV(env);
            addInstr(env, mk_vMOVsd_RR(arg[i], tmp[i]));
            addInstr(env, AMD64Instr_SseReRg(Asse_CMPEQ32, zero, tmp[i]));
            dst[i] = do_sse_NotV128(env, tmp[i]);
         }
         return;
      }
      case (IROp)Iop_CmpNEZ64x8: {
         HReg arg[MULT512] = {0}, tmp[MULT512] = {0};
         iselExpr_512(arg, env, e->Iex.Unop.arg);
         for (Int i = 0; i < MULT512; i++) {
            tmp[i] = generate_zeroes_V128(env);
            addInstr(env, AMD64Instr_SseReRg(Asse_CMPEQ32, arg[i], tmp[i]));
            tmp[i]=do_sse_NotV128(env, tmp[i]);
            addInstr(env, AMD64Instr_SseShuf(0xB1, tmp[i], dst[i]));
            addInstr(env, AMD64Instr_SseReRg(Asse_OR, tmp[i], dst[i]));
         }
         return;
      }
      case Iop_ClzNat32:      fn = (HWord) h_Iop_Clz32;     break;
      default: break;
   }

   if (fn == 0) {
      // Common case:
      IROp ir_op = e->Iex.Unop.op - Iop_LAST_NOT_EVEX;
      fn = (HWord) IOPS_ARR[ir_op].function_call;
   }
   /* Select and translate inner IRs first, then start translating this IR*/
   HReg arg[MULT512] = {0};
   select_IRExpr_argument(env, e->Iex.Unop.arg, arg);
   
   HReg argp = newVRegI(env);
   sub_from_rsp(env, 240);
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(48, hregAMD64_RSP()), argp));
   addInstr(env, AMD64Instr_Alu64R(Aalu_AND, AMD64RMI_Imm( ~(UInt)15 ), argp));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(0, argp), hregAMD64_RDI()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(64, argp), hregAMD64_RSI()));

   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Unop.arg), arg, hregAMD64_RSI());
   addInstr(env, AMD64Instr_Call( Acc_ALWAYS, (ULong)fn, 2, mk_RetLoc_simple(RLPri_None) ));
   load_HReg_from_register(env, e, dst, argp);

   add_to_rsp(env, 240);
}

static void handle_binop(HReg *dst, ISelEnv* env, const IRExpr* e) {
   HWord fn = 0;
   // Special cases:
   switch (e->Iex.Binop.op) {
      case (IROp)Iop_V256HLtoV512:
         iselDVecExpr(&dst[3], &dst[2], env, e->Iex.Binop.arg1);
         iselDVecExpr(&dst[1], &dst[0], env, e->Iex.Binop.arg2);
         return;
      case (IROp)Iop_OrV512: {
         HReg argL[MULT512] = {0}, argR[MULT512] = {0};
         iselExpr_512(argL, env, e->Iex.Binop.arg1);
         iselExpr_512(argR, env, e->Iex.Binop.arg2);
         for (Int i = 0; i < MULT512; i++) {
            addInstr(env, mk_vMOVsd_RR(argL[i], dst[i]));
            addInstr(env, AMD64Instr_SseReRg(Asse_OR, argR[i], dst[i]));
         }
         return;
      }
      case (IROp)Iop_AndV512: {
         HReg argL[MULT512] = {0}, argR[MULT512] = {0};
         iselExpr_512(argL, env, e->Iex.Binop.arg1);
         iselExpr_512(argR, env, e->Iex.Binop.arg2);
         for (Int i = 0; i < MULT512; i++) {
            addInstr(env, mk_vMOVsd_RR(argL[i], dst[i]));
            addInstr(env, AMD64Instr_SseReRg(Asse_AND, argR[i], dst[i]));
         }
         return;
      }
      case Iop_ShrV128:  fn = (HWord) h_Iop_ShrV128;  break;
      case Iop_ShlV128:  fn = (HWord) h_Iop_ShlV128;  break;
      default: break;
   }

   if (fn == 0) {
      // Common case:
      IROp ir_op = e->Iex.Binop.op - Iop_LAST_NOT_EVEX;
      fn = (HWord) IOPS_ARR[ir_op].function_call;
   }
   
   HReg arg1[MULT512] = {0};
   HReg arg2[MULT512] = {0};
   select_IRExpr_argument(env, e->Iex.Binop.arg1, arg1);
   select_IRExpr_argument(env, e->Iex.Binop.arg2, arg2);

   HReg argp = newVRegI(env);
   sub_from_rsp(env, 240);
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(48, hregAMD64_RSP()), argp));
   addInstr(env, AMD64Instr_Alu64R(Aalu_AND, AMD64RMI_Imm( ~(UInt)15 ), argp));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(0, argp), hregAMD64_RDI()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(64, argp), hregAMD64_RSI()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(128, argp), hregAMD64_RDX()));

   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Binop.arg1), arg1, hregAMD64_RSI());
   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Binop.arg2), arg2, hregAMD64_RDX());
   addInstr(env, AMD64Instr_Call( Acc_ALWAYS, (ULong)fn, 3, mk_RetLoc_simple(RLPri_None) ));
   load_HReg_from_register(env, e, dst, argp);

   add_to_rsp(env, 240);
   return;
}

static void handle_triop(HReg *dst, ISelEnv* env, const IRExpr* e) {
   UInt ir_op = e->Iex.Triop.details->op - Iop_LAST_NOT_EVEX;
   vassert(ir_op <= (Iop_LAST - Iop_LAST_NOT_EVEX));

   HWord fn = (HWord) IOPS_ARR[ir_op].function_call;
   if (fn == 0) {
      ppIRExpr(e); vex_printf("\n");
      vpanic("no helper!");
   }

   // We use the same register for the first source and destination, to save space
   // If the first source is integer, pass its value through pointer
   Bool arg1_is_int = typeOfIRExpr(env->type_env, e->Iex.Triop.details->arg1) == Ity_I64;
   AMD64RI* arg1_p = NULL;
   HReg arg1[MULT512] = {0};
   if (arg1_is_int) {
      arg1_p = iselIntExpr_RI(env, e->Iex.Triop.details->arg1);
   } else {
      select_IRExpr_argument(env, e->Iex.Triop.details->arg1, arg1);
   }
   HReg arg2[MULT512] = {0};
   HReg arg3[MULT512] = {0};
   select_IRExpr_argument(env, e->Iex.Triop.details->arg2, arg2);
   select_IRExpr_argument(env, e->Iex.Triop.details->arg3, arg3);
   
   HReg argp = newVRegI(env);
   sub_from_rsp(env, 240);
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(48, hregAMD64_RSP()), argp));
   addInstr(env, AMD64Instr_Alu64R(Aalu_AND, AMD64RMI_Imm( ~(UInt)15 ), argp));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(0, argp), hregAMD64_RDI()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(64, argp), hregAMD64_RSI()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(128, argp), hregAMD64_RDX()));

   if (arg1_is_int) {
      addInstr(env, AMD64Instr_Alu64M(Aalu_MOV, arg1_p, AMD64AMode_IR(0, hregAMD64_RDI())));
   } else {
      store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Triop.details->arg1), arg1, hregAMD64_RDI());
   }
   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Triop.details->arg2), arg2, hregAMD64_RSI());
   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Triop.details->arg3), arg3, hregAMD64_RDX());
   
   addInstr(env, AMD64Instr_Call( Acc_ALWAYS, (ULong)fn, 3, mk_RetLoc_simple(RLPri_None) ));
   load_HReg_from_register(env, e, dst, argp);

   add_to_rsp(env, 240);
   return;
}

static void handle_qop(HReg *dst, ISelEnv* env, const IRExpr* e) {
   UInt ir_op = e->Iex.Qop.details->op - Iop_LAST_NOT_EVEX;
   vassert(ir_op <= (Iop_LAST - Iop_LAST_NOT_EVEX));
   
   HWord fn = (HWord) IOPS_ARR[ir_op].function_call;
   if (fn == 0) {
      ppIRExpr(e); vex_printf("\n");
      vpanic("no helper!");
   }
   
   // We use the same register for the first source and destination, to save space
   // If the first source is integer, pass its value through pointer
   Bool arg1_is_int = typeOfIRExpr(env->type_env, e->Iex.Qop.details->arg1) == Ity_I64;
   AMD64RI* arg1_p = NULL;
   HReg arg1[MULT512] = {0};
   if (arg1_is_int) {
      arg1_p = iselIntExpr_RI(env, e->Iex.Qop.details->arg1);
   } else {
      select_IRExpr_argument(env, e->Iex.Qop.details->arg1, arg1);
   }
   HReg arg2[MULT512] = {0};
   HReg arg3[MULT512] = {0};
   HReg arg4[MULT512] = {0};
   select_IRExpr_argument(env, e->Iex.Qop.details->arg2, arg2);
   select_IRExpr_argument(env, e->Iex.Qop.details->arg3, arg3);
   select_IRExpr_argument(env, e->Iex.Qop.details->arg4, arg4);
   
   HReg argp = newVRegI(env);
   sub_from_rsp(env, 240);
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(48, hregAMD64_RSP()), argp));
   addInstr(env, AMD64Instr_Alu64R(Aalu_AND, AMD64RMI_Imm( ~(UInt)15 ), argp));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(0, argp), hregAMD64_RDI()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(64, argp), hregAMD64_RSI()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(128, argp), hregAMD64_RDX()));
   addInstr(env, AMD64Instr_Lea64(AMD64AMode_IR(192, argp), hregAMD64_RCX()));

   if (arg1_is_int) {
      addInstr(env, AMD64Instr_Alu64M(Aalu_MOV, arg1_p, AMD64AMode_IR(0, hregAMD64_RDI())));
   } else {
      store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Qop.details->arg1), arg1, hregAMD64_RDI());
   }
   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Qop.details->arg2), arg2, hregAMD64_RSI());
   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Qop.details->arg3), arg3, hregAMD64_RDX());
   store_IRExpr_at_register(env, typeOfIRExpr(env->type_env, e->Iex.Qop.details->arg4), arg4, hregAMD64_RCX());
   addInstr(env, AMD64Instr_Call( Acc_ALWAYS, (ULong)fn, 4, mk_RetLoc_simple(RLPri_None) ));
   load_HReg_from_register(env, e, dst, argp);

   add_to_rsp(env, 240);
   return;
}

void iselExpr_512 ( /*OUT*/HReg *dst, ISelEnv* env, const IRExpr* e )
{
   vassert(e);

   switch (typeOfIRExpr(env->type_env, e)) {
      case Ity_I64: case Ity_I32: case Ity_I16: case Ity_I8:
         *dst = newVRegI(env);
         break;
      case Ity_V128: case Ity_F64: case Ity_F32:
         *dst = newVRegV(env);
         break;
      case Ity_V256:
         dst[0] = newVRegV(env);
         dst[1] = newVRegV(env);
         break;
      case Ity_V512:
         for (Int i = 0; i < MULT512; i++)
            dst[i] = newVRegV(env);
         break;
      default:
         vassert(0);
   }

   switch (e->tag) {
      case Iex_RdTmp:
         lookupIRTempQuartet(dst, env, e->Iex.RdTmp.tmp);
         break;
      case Iex_Get: {
         AMD64AMode* am;
         HReg rbp = hregAMD64_RBP();
         for (Int i = 0; i < MULT512; i++) {
            am = AMD64AMode_IR(e->Iex.Get.offset + i*16, rbp);
            addInstr(env, AMD64Instr_SseLdSt(True, 16, dst[i], am));
         }
         break;
      }
      case Iex_Load: {
         AMD64AMode* am;
         HReg rA = iselIntExpr_R(env, e->Iex.Load.addr);
         for (Int i = 0; i < MULT512; i++) {
            am = AMD64AMode_IR(i*16, rA);
            addInstr(env, AMD64Instr_SseLdSt(True, 16, dst[i], am));
         }
         break;
      }
      case Iex_Const:
         vassert(e->Iex.Const.con->tag == Ico_V512);
         if ((e->Iex.Const.con->Ico.V512) == 0x0 ) {
            for (Int i = 0; i < MULT512; i++)
               dst[i] = generate_zeroes_V128(env);
         }
         break;
      case Iex_ITE: {
         HReg r1[MULT512], r0[MULT512];
         iselExpr_512(r1, env, e->Iex.ITE.iftrue);
         iselExpr_512(r0, env, e->Iex.ITE.iffalse);
         AMD64CondCode cc = iselCondCode_C(env, e->Iex.ITE.cond);
         for (Int i = 0; i < MULT512; i++) {
            addInstr(env, mk_vMOVsd_RR(r1[i], dst[i]));
            addInstr(env, AMD64Instr_SseCMov(cc ^ 1, r0[i], dst[i]));
         }
         break;
      }
      case Iex_Unop:
         handle_unop(dst, env, e);
         break;
      case Iex_Binop:
         handle_binop(dst, env, e);
         break;
      case Iex_Triop:
         handle_triop(dst, env, e);
         break;
      case Iex_Qop:
         handle_qop(dst, env, e);
         break;
      default:
         vex_printf("iselVecExpr_512 - unknown e->tag %u\n", e->tag);
         vpanic("iselVecExpr__512");
   }
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                          host_amd64_isel_AVX512.c ---*/
/*---------------------------------------------------------------*/
