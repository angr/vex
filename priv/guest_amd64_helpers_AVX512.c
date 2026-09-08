/*---------------------------------------------------------------*/
/*--- begin                      guest_amd64_helpers_AVX512.c ---*/
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
*/

/* This file contains helper functions for amd64 guest code. */

#ifdef AVX_512
#include "libvex_guest_amd64.h"
#include "guest_amd64_defs.h"
#include "libvex_ir.h"
#include "main_util.h"

#  define SET_ABCD(_a,_b,_c,_d)     \
   do { st->guest_RAX = (ULong)(_a);\
      st->guest_RBX = (ULong)(_b);  \
      st->guest_RCX = (ULong)(_c);  \
      st->guest_RDX = (ULong)(_d);  \
   } while (0); \
   break;

// Knight's Landing
/* Claim to be the following CPU (4 x ...), which is AVX512 capable.

   With the following changes: claim that XSaveOpt is not available, by
   cpuid(eax=0xD,ecx=1).eax[0] returns 0, compared to 1 on the real
   CPU.  Consequently, programs that correctly observe these CPUID
   values should only try to use 3 of the 8 XSave-family instructions:
   XGETBV, XSAVE and XRSTOR.  In particular this avoids having to
   implement the compacted or optimised save/restore variants.

   vendor_id       : GenuineIntel
   cpu family      : 6
   model           : 87
   model name      : Intel(R) Genuine Intel(R) CPU 0000 @ 1.30GHz
   stepping        : 1
   microcode       : 0xffff01a0
   cpu MHz         : 999.993
   cache size      : 1024 KB
   physical id     : 0
   siblings        : 256
   core id         : 0
   cpu cores       : 64
   apicid          : 0
   initial apicid  : 0
   fpu             : yes
   fpu_exception   : yes
   cpuid level     : 13
   wp              : yes
   flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca
                   cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht 
                   tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc 
                   arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc 
                   aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl 
                   est tm2 ssse3 fma cx16 xtpr pdcm sse4_1 sse4_2 x2apic 
                   movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand
                   lahf_lm abm 3dnowprefetch epb fsgsbase tsc_adjust bmi1 
                   avx2 smep bmi2 erms avx512f rdseed adx avx512pf avx512er 
                   avx512cd xsaveopt dtherm ida arat pln pts
   bugs            :
   bogomips        : 2593.90
   clflush size    : 64
   cache_alignment : 64
   address sizes   : 46 bits physical, 48 bits virtual
   power management:
*/
void amd64g_dirtyhelper_CPUID_avx512_KNL ( VexGuestAMD64State* st ) {

   UInt old_eax = (UInt)st->guest_RAX;
   UInt old_ecx = (UInt)st->guest_RCX;

   switch (old_eax) {
      case 0x00000000:
         SET_ABCD(0x0000000d, 0X756e6547, 0x6c65746e, 0x49656e69);
      case 0x00000001:
         SET_ABCD(0x00050671, 0xedff0800, 0x7ff8f39f, 0xbfebfbff);
      case 0x00000002:
         SET_ABCD(0x6c6b6a01, 0x00ff616d, 0x00000000, 0x00000000);
      case 0x00000003:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x00000004:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0xfc00c121, 0x01c0003f, 0x0000003f, 0x00000000);
            case 0x00000001:
               SET_ABCD(0xfc00c122, 0x01c0003f, 0x0000003f, 0x00000000);
            case 0x00000002:
               SET_ABCD(0xfc01c143, 0x03c0003f, 0x000003ff, 0x00000013);
            case 0x00000003:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
            default: vpanic("KNL!");
         }
         break;
      case 0x00000005:
         SET_ABCD(0x00000040, 0x00000040, 0x00000003, 0x00000110);
      case 0x00000006:
         SET_ABCD(0x00000077, 0x00000002, 0x00000009, 0x0000000);
      case 0x00000007:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x00000000, 0x1c0d23ab, 0x00000001, 0x00000000);
            default:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x00000008:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x00000009:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x0000000a:
         SET_ABCD(0x07280203, 0x00000000, 0x00000000, 0x00000503);
      case 0x0000000b:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x00000002, 0x00000004, 0x00000100, 0x000000e4);
            case 0x00000001:
               SET_ABCD(0x00000009, 0x00000100, 0x00000201, 0x000000e4);
            default:
               SET_ABCD(0x00000000, 0x00000000, old_ecx, 0x000000e4);
         }
         break;
      case 0x0000000c:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x0000000d:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x000000e7, 0x00000a80, 0x00000a80, 0x00000000);
            case 0x00000001: 
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
            case 0x00000002:
               SET_ABCD(0x00000100, 0x00000240, 0x00000000, 0x00000000);
            default:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x80000000:
         SET_ABCD(0x80000008, 0x00000000, 0x00000000, 0x00000000);
      case 0x80000001:
         SET_ABCD(0x00000000, 0x00000000, 0x00000021, 0x2c100800);
      case 0x80000002:
         SET_ABCD(0x65746e49, 0x2952286c, 0x6e654720, 0x656e6975);
      case 0x80000003:
         SET_ABCD(0x746e4920, 0x52286c65, 0x50432029, 0x30302055);
      case 0x80000005:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x80000006:
         SET_ABCD(0x00000000, 0x00000000, 0x04008040, 0x00000000);
      case 0x80000007:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000100);
      case 0x80000008:
         SET_ABCD(0x0000302e, 0x00000000, 0x00000000, 0x00000000);
      default:
         SET_ABCD(0x000000e7, 0x00000a80, 0x00000a80, 0x00000000);
   }
}


// SkyLake
/* Claim to be the following CPU (4 x ...), which is AVX512 capable.

   With the following changes: claim that XSaveOpt is not available, by
   cpuid(eax=0xD,ecx=1).eax[0] returns 0, compared to 1 on the real
   CPU.  Consequently, programs that correctly observe these CPUID
   values should only try to use 3 of the 8 XSave-family instructions:
   XGETBV, XSAVE and XRSTOR.  In particular this avoids having to
   implement the compacted or optimised save/restore variants.

   vendor_id       : GenuineIntel
   cpu family      : 6
   model           : 85
   model name      : Intel(R) Xeon(R) Platinum 8168 CPU @ 2.70GHz 
   stepping        : 4
   microcode       : 0x2000043
   cpu MHz         : 1000.054
   cache size      : 33792 KB
   physical id     : 0
   siblings        : 48
   core id         : 0
   cpu cores       : 24
   apicid          : 0
   initial apicid  : 0
   fpu             : yes
   fpu_exception   : yes
   cpuid level     : 22
   wp              : yes
   flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca 
   cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht 
   tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art 
   arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc 
   aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl 
   vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid dca sse4_1 
   sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave 
   avx f16c rdrand lahf_lm abm 3dnowprefetch epb cat_l3 
   cdp_l3 intel_pt tpr_shadow vnmi flexpriority ept vpid 
   fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid 
   rtm cqm mpx rdt_a avx512f avx512dq rdseed adx smap 
   clflushopt clwb avx512cd avx512bw avx512vl xsaveopt 
   xsavec xgetbv1 cqm_llc cqm_occup_llc cqm_mbm_total 
   cqm_mbm_local dtherm ida arat pln pts hwp hwp_act_window 
   hwp_epp hwp_pkg_req
   bogomips        : 5400.00
   clflush size    : 64
   cache_alignment : 64
   address sizes   : 46 bits physical, 48 bits virtual
   power management:
   */

void amd64g_dirtyhelper_CPUID_avx512_SKX ( VexGuestAMD64State* st ) {
   UInt old_eax = (UInt)st->guest_RAX;
   UInt old_ecx = (UInt)st->guest_RCX;

   switch (old_eax) {
      case 0x00000000:
         SET_ABCD(0x00000016, 0X756e6547, 0x6c65746e, 0x49656e69);
      case 0x00000001:
         /* Don't advertise RDRAND support, bit 30 in ECX.  */
         SET_ABCD(0x00050654, 0x43400800, 0x3ffafbff, 0xbfebfbff);
      case 0x00000002:
         SET_ABCD(0x76036301, 0x00f0b5ff, 0x00000000, 0x00c30000);
      case 0x00000003:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x00000004:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x7c00c121, 0x01c0003f, 0x0000003f, 0x00000000);
            case 0x00000001:
               SET_ABCD(0x7c00c122, 0x01c0003f, 0x0000003f, 0x00000000);
            case 0x00000002:
               SET_ABCD(0x7c01c143, 0x03c0003f, 0x000003ff, 0x00000000);
            case 0x00000003:
               SET_ABCD(0x7c0fc163, 0x0280003f, 0x0000bfff, 0x00000004);
            default:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x00000005:
         SET_ABCD(0x00000040, 0x00000040, 0x00000003, 0x00002020);
      case 0x00000006:
         SET_ABCD(0x00000ef7, 0x00000002, 0x00000009, 0x00000000);
      case 0x00000007:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x00000000, 0xd39ffffb, 0x00000008, 0xc000000);
            default:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x00000008:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x00000009:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x0000000a:
         SET_ABCD(0x07300404, 0x00000000, 0x00000000, 0x00000603);
      case 0x0000000b:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x00000001, 0x00000002, 0x00000100, 0x00000043);
            case 0x00000001:
               SET_ABCD(0x00000006, 0x00000030, 0x00000201, 0x00000043);
            default: 
               SET_ABCD(0x00000000, 0x00000000, old_ecx, 0x00000043);
         }
         break;
      case 0x0000000c:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x0000000d:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x000002ff, 0x00000a80, 0x00000a88, 0x00000000);
            case 0x00000001:
               // important
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
            case 0x00000002:
               SET_ABCD(0x00000100, 0x00000240, 0x00000000, 0x00000000);
            case 0x00000003:
               SET_ABCD(0x00000040, 0x0003c000, 0x00000000, 0x00000000);
            case 0x00000004:
               SET_ABCD(0x00000040, 0x00000400, 0x00000000, 0x00000000);
            case 0x00000005:
               SET_ABCD(0x00000040, 0x00000440, 0x00000000, 0x00000000);
            case 0x00000006:
               SET_ABCD(0x00000200, 0x00000480, 0x00000000, 0x00000000);
            case 0x00000007:
               SET_ABCD(0x00000400, 0x00000680, 0x00000000, 0x00000000);
            case 0x00000008:
               SET_ABCD(0x00000080, 0x00000000, 0x00000001, 0x00000000);
            case 0x00000009:
               SET_ABCD(0x00000008, 0x00000a80, 0x00000000, 0x00000000);
            default:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x0000000e:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x0000000f:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x00000000, 0x000000bf, 0x00000000, 0x00000002);
            case 0x00000001:
               SET_ABCD(0x00000000, 0x00018000, 0x000000bf, 0x00000007);
            default:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x00000010:
         switch (old_ecx) {
             case 0x00000000:
                 SET_ABCD(0x00000000, 0x0000000a, 0x00000000, 0x00000000);
             case 0x00000001:
                 SET_ABCD(0x0000000a, 0x00000600, 0x00000004, 0x0000000f);
             case 0x00000002:
                 SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
             case 0x00000003:
                 SET_ABCD(0x00000059, 0x00000000, 0x00000004, 0x00000007);
             default:
                 SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x00000011:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x00000012:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x00000013:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x00000014:
         switch (old_ecx) {
            case 0x00000000:
               SET_ABCD(0x00000001, 0x0000000f, 0x00000007, 0x00000000);
            case 0x00000001:
               SET_ABCD(0x02490002, 0x003f3fff, 0x00000000, 0x00000000);
            default:
               SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
         }
         break;
      case 0x00000015:
         SET_ABCD(0x00000002, 0x000000d8, 0x00000000, 0x00000000);

      case 0x80000000:
         SET_ABCD(0x80000008, 0x00000000, 0x00000000, 0x00000000);
      case 0x80000001:
         SET_ABCD(0x00000000, 0x00000000, 0x00000121, 0x2c100800);
      case 0x80000002:
         SET_ABCD(0x65746e49, 0x2952286c, 0x6f655820, 0x2952286e);
      case 0x80000003:
         SET_ABCD(0x616c5020, 0x756e6974, 0x3138206d, 0x43203836);
      case 0x80000004:
         SET_ABCD(0x40205550, 0x372e3220, 0x7a484730, 0x00000000);
      case 0x80000005:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000000);
      case 0x80000006:
         SET_ABCD(0x00000000, 0x00000000, 0x01006040, 0x00000000);
      case 0x80000007:
         SET_ABCD(0x00000000, 0x00000000, 0x00000000, 0x00000100);
      case 0x80000008:
         SET_ABCD(0x0000302e, 0x00000000, 0x00000000, 0x00000000);

      default:
         SET_ABCD(0x00000a8c, 0x00000e74, 0x00000064, 0x00000000);
   }
}

#  undef SET_ABCD

/* Initialise ZMM registers */
void LibVEX_GuestAMD64_initialise_ZMM ( /*OUT*/VexGuestAMD64State* vex_state )
{
#  define AVXZERO(_zmm) \
   do { \
      for (Int i=0; i<16; i++) { \
         _zmm[i]=0; \
      } \
   } while (0)
   vex_state->guest_SSEROUND = (ULong)Irrm_NEAREST;
   AVXZERO(vex_state->guest_ZMM0);
   AVXZERO(vex_state->guest_ZMM1);
   AVXZERO(vex_state->guest_ZMM2);
   AVXZERO(vex_state->guest_ZMM3);
   AVXZERO(vex_state->guest_ZMM4);
   AVXZERO(vex_state->guest_ZMM5);
   AVXZERO(vex_state->guest_ZMM6);
   AVXZERO(vex_state->guest_ZMM7);
   AVXZERO(vex_state->guest_ZMM8);
   AVXZERO(vex_state->guest_ZMM9);
   AVXZERO(vex_state->guest_ZMM10);
   AVXZERO(vex_state->guest_ZMM11);
   AVXZERO(vex_state->guest_ZMM12);
   AVXZERO(vex_state->guest_ZMM13);
   AVXZERO(vex_state->guest_ZMM14);
   AVXZERO(vex_state->guest_ZMM15);
   AVXZERO(vex_state->guest_ZMM16);
   AVXZERO(vex_state->guest_ZMM17);
   AVXZERO(vex_state->guest_ZMM18);
   AVXZERO(vex_state->guest_ZMM19);
   AVXZERO(vex_state->guest_ZMM20);
   AVXZERO(vex_state->guest_ZMM21);
   AVXZERO(vex_state->guest_ZMM22);
   AVXZERO(vex_state->guest_ZMM23);
   AVXZERO(vex_state->guest_ZMM24);
   AVXZERO(vex_state->guest_ZMM25);
   AVXZERO(vex_state->guest_ZMM26);
   AVXZERO(vex_state->guest_ZMM27);
   AVXZERO(vex_state->guest_ZMM28);
   AVXZERO(vex_state->guest_ZMM29);
   AVXZERO(vex_state->guest_ZMM30);
   AVXZERO(vex_state->guest_ZMM31);
   AVXZERO(vex_state->guest_ZMM32);
#  undef AVXZERO
}

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                        guest_amd64_helpers_AVX512.c ---*/
/*---------------------------------------------------------------*/
