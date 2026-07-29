/*---------------------------------------------------------------*/
/*--- begin                       host_generic_AVX512_stub.c  ---*/
/*---------------------------------------------------------------*/

/* Portable stand-ins for the AVX-512 host helper functions that
   Valgrind implements with AVX-512 intrinsics in
   host_generic_AVX512_{F,KNL,SKX,ICL}.c.  pyvex only uses the VEX
   front end (lifting), so these helpers can never be called: they
   exist only because IOPS_ARR embeds their addresses.  Using stubs
   keeps the library buildable on every host (arm64, etc.) without
   AVX-512 toolchain support.  The intrinsic originals are kept
   in-tree, unbuilt, for reference and upstream parity. */

#ifdef AVX_512

#include "libvex_basictypes.h"
#include "host_generic_AVX512.h"
#include "main_util.h"

VEX_REGPARM(3) void h_Iop_RoundScale32x16 (V512* a0, V512* a1, UInt a2)
{
   vpanic("h_Iop_RoundScale32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_RoundScale64x8 (V512* a0, V512* a1, UInt a2)
{
   vpanic("h_Iop_RoundScale64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_RoundScale32F0x4 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_RoundScale32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_RoundScale64F0x2 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_RoundScale64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm32x16 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Perm32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm64x4 (V256* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_Perm64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm64x8 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Perm64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Ux4 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Ux4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Ux8 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Ux8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Ux16 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Ux16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Ux2 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Ux2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Ux4 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Ux4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Ux8 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Ux8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Sx4 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Sx4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Sx8 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Sx8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Sx16 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Sx16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Sx2 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Sx2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Sx4 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Sx4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Sx8 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Sx8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_16Sto8x8 (ULong* a0, V128* a1)
{
   vpanic("h_Iop_16Sto8x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Ternlog32x16 (V512* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Ternlog32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Ternlog64x8 (V512* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Ternlog64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test8x16 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Test8x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test8x32 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_Test8x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test8x64 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Test8x64: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test16x8 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Test16x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test16x16 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_Test16x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test16x32 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Test16x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_GetMant32x16 (V512* a0, V512* a1, UInt a2)
{
   vpanic("h_Iop_GetMant32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_GetMant64x8 (V512* a0, V512* a1, UInt a2)
{
   vpanic("h_Iop_GetMant64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN8x16 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_TestN8x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN8x32 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_TestN8x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN8x64 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_TestN8x64: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN16x8 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_TestN16x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN16x16 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_TestN16x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN16x32 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_TestN16x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test32x4 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Test32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test32x8 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_Test32x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test32x16 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Test32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test64x2 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Test64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test64x4 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_Test64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Test64x8 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Test64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_GetMant32F0x4 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_GetMant32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_GetMant64F0x2 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_GetMant64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN32x4 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_TestN32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN32x8 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_TestN32x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN32x16 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_TestN32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN64x2 (ULong* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_TestN64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN64x4 (ULong* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_TestN64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_TestN64x8 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_TestN64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ExpandBitsToV128 (V128* a0, ULong a1, UInt a2)
{
   vpanic("h_Iop_ExpandBitsToV128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ExpandBitsToV256 (V256* a0, ULong a1, UInt a2)
{
   vpanic("h_Iop_ExpandBitsToV256: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ExpandBitsToV512 (V512* a0, ULong a1, UInt a2)
{
   vpanic("h_Iop_ExpandBitsToV512: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Scale32x16 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Scale32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Scale64x8 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Scale64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Scale32F0x4 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Scale32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Scale64F0x2 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Scale64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Min64Sx8 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Min64Sx8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Min64Ux8 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Min64Ux8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Max64Sx8 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Max64Sx8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp8Ux16 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp8Ux16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp8Ux32 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp8Ux32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp8Ux64 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp8Ux64: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp16Ux8 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp16Ux8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp16Ux16 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp16Ux16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp16Ux32 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp16Ux32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp8Sx16 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp8Sx16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp8Sx32 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp8Sx32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp8Sx64 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp8Sx64: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp16Sx8 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp16Sx8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp16Sx16 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp16Sx16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp16Sx32 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp16Sx32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Max64Ux8 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Max64Ux8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_ExtractExp32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_ExtractExp32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_ExtractExp64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_ExtractExp64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VDBPSADBW (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VDBPSADBW: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ExtractExp32F0x4 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_ExtractExp32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ExtractExp64F0x2 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_ExtractExp64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Recip14_32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_Recip14_32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Recip14_64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_Recip14_64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Recip14_32F0x4 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Recip14_32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Recip14_64F0x2 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Recip14_64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_RSqrt14_32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_RSqrt14_32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_RSqrt14_64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_RSqrt14_64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_RSqrt14_32F0x4 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_RSqrt14_32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_RSqrt14_64F0x2 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_RSqrt14_64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Range32x4 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Range32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Range64x2 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Range64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_DPBUSD (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_DPBUSD: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Range32F0x4 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Range32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Range64F0x2 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Range64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_DPBUSDS (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_DPBUSDS: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_DPWSSD (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_DPWSSD: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_DPWSSDS (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_DPWSSDS: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_FixupImm32x16 (V512* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_FixupImm32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_FixupImm64x8 (V512* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_FixupImm64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_PopCount8x16 (V128* a0, V128* a1)
{
   vpanic("h_Iop_PopCount8x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_FixupImm32F0x4 (V128* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_FixupImm32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_FixupImm64F0x2 (V128* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_FixupImm64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Reduce32x4 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Reduce32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Reduce64x2 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Reduce64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Reduce32F0x4 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Reduce32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Reduce64F0x2 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Reduce64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Compress8x16 (V128* a0, V128* a1)
{
   vpanic("h_Iop_Compress8x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Compress16x8 (V128* a0, V128* a1)
{
   vpanic("h_Iop_Compress16x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify32x4 (ULong* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Classify32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify32x8 (ULong* a0, V256* a1, UInt a2)
{
   vpanic("h_Iop_Classify32x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify32x16 (ULong* a0, V512* a1, UInt a2)
{
   vpanic("h_Iop_Classify32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify64x2 (ULong* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Classify64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify64x4 (ULong* a0, V256* a1, UInt a2)
{
   vpanic("h_Iop_Classify64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify64x8 (ULong* a0, V512* a1, UInt a2)
{
   vpanic("h_Iop_Classify64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify32F0x4 (ULong* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Classify32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Classify64F0x2 (ULong* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Classify64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHLDW_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHLDW_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHLDVD_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHLDVD_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHLDVQ_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHLDVQ_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHRDVW_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHRDVW_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHRDW_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHRDW_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ShrV128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_ShrV128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ShlV128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_ShlV128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHRDVD_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHRDVD_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHRDD_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHRDD_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHRDVQ_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHRDVQ_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHRDQ_128 (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_VPSHRDQ_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI8x16 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_PermI8x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI8x32 (V256* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_PermI8x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI8x64 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_PermI8x64: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI16x8 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_PermI16x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI16x16 (V256* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_PermI16x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI16x32 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_PermI16x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI32x4 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_PermI32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI32x8 (V256* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_PermI32x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI32x16 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_PermI32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI64x2 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_PermI64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI64x4 (V256* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_PermI64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PermI64x8 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_PermI64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_MULTISHIFTQB (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_MULTISHIFTQB: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm8x32 (V256* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_Perm8x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm8x64 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Perm8x64: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm16x8 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Perm16x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm16x16 (V256* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_Perm16x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Perm16x32 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_Perm16x32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHUFBITQMB_128 (UInt* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_VPSHUFBITQMB_128: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHUFBITQMB_256 (UInt* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_VPSHUFBITQMB_256: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_VPSHUFBITQMB_512 (ULong* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_VPSHUFBITQMB_512: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PMADD52LUQ (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_PMADD52LUQ: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_PMADD52HUQ (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_PMADD52HUQ: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Fx4 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Fx4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Fx8 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Fx8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp32Fx16 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp32Fx16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Fx2 (ULong* a0, V128* a1, V128* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Fx2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Fx4 (ULong* a0, V256* a1, V256* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Fx4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Cmp64Fx8 (ULong* a0, V512* a1, V512* a2, UInt a3)
{
   vpanic("h_Iop_Cmp64Fx8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CfD32x4 (V128* a0, V128* a1)
{
   vpanic("h_Iop_CfD32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CfD32x8 (V256* a0, V256* a1)
{
   vpanic("h_Iop_CfD32x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CfD32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_CfD32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CfD64x2 (V128* a0, V128* a1)
{
   vpanic("h_Iop_CfD64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CfD64x4 (V256* a0, V256* a1)
{
   vpanic("h_Iop_CfD64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CfD64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_CfD64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Exp32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_Exp32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Exp64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_Exp64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Recip28_32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_Recip28_32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Recip28_64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_Recip28_64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Recip28_32F0x4 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Recip28_32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Recip28_64F0x2 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_Recip28_64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_RSqrt28_32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_RSqrt28_32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_RSqrt28_64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_RSqrt28_64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_RSqrt28_32F0x4 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_RSqrt28_32F0x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_RSqrt28_64F0x2 (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_RSqrt28_64F0x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_Gfni (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_Gfni: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_GfniMult (V128* a0, V128* a1, V128* a2)
{
   vpanic("h_Iop_GfniMult: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_GfniInv (V128* a0, V128* a1, UInt a2)
{
   vpanic("h_Iop_GfniInv: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_OrV512 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_OrV512: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_AndV512 (V512* a0, V512* a1, V512* a2)
{
   vpanic("h_Iop_AndV512: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_NotV512 (V512* a0, V512* a1)
{
   vpanic("h_Iop_NotV512: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CmpNEZ32x16 (V512* a0, V512* a1)
{
   vpanic("h_Iop_CmpNEZ32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_CmpNEZ64x8 (V512* a0, V512* a1)
{
   vpanic("h_Iop_CmpNEZ64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_V256HLtoV512 (V512* a0, V256* a1, V256* a2)
{
   vpanic("h_Iop_V256HLtoV512: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512toV256_0 (V256* a0, V512* a1)
{
   vpanic("h_Iop_V512toV256_0: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512toV256_1 (V256* a0, V512* a1)
{
   vpanic("h_Iop_V512toV256_1: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_0 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_0: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_1 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_1: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_2 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_3 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_3: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_4 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_5 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_5: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_6 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_6: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_V512to64_7 (ULong* a0, V512* a1)
{
   vpanic("h_Iop_V512to64_7: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_ExpandBitsToInt (ULong* a0, ULong a1, UInt a2)
{
   vpanic("h_Iop_ExpandBitsToInt: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Compress32x4 (V128* a0, V128* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Compress32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Compress32x8 (V256* a0, V256* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Compress32x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Compress32x16 (V512* a0, V512* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Compress32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Compress64x2 (V128* a0, V128* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Compress64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Compress64x4 (V256* a0, V256* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Compress64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Compress64x8 (V512* a0, V512* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Compress64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Expand32x4 (V128* a0, V128* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Expand32x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Expand32x8 (V256* a0, V256* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Expand32x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Expand32x16 (V512* a0, V512* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Expand32x16: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Expand64x2 (V128* a0, V128* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Expand64x2: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Expand64x4 (V256* a0, V256* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Expand64x4: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(4) void h_Iop_Expand64x8 (V512* a0, V512* a1, ULong a2, UInt a3)
{
   vpanic("h_Iop_Expand64x8: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_I32UtoF32_SKX (UInt* a0, UInt a1, UInt a2)
{
   vpanic("h_Iop_I32UtoF32_SKX: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_I64UtoF64_SKX (ULong* a0, UInt a1, ULong a2)
{
   vpanic("h_Iop_I64UtoF64_SKX: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_F32toI32U_SKX (UInt* a0, UInt a1, UInt a2)
{
   vpanic("h_Iop_F32toI32U_SKX: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_F64toI32U_SKX (UInt* a0, UInt a1, ULong a2)
{
   vpanic("h_Iop_F64toI32U_SKX: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(3) void h_Iop_F64toI64U_SKX (ULong* a0, UInt a1, ULong a2)
{
   vpanic("h_Iop_F64toI64U_SKX: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_Clz32 (UInt* a0, UInt a1)
{
   vpanic("h_Iop_Clz32: AVX-512 host helpers are not available in pyvex builds");
}

VEX_REGPARM(2) void h_Iop_PrintI64 (ULong* a0, ULong a1)
{
   vpanic("h_Iop_PrintI64: AVX-512 host helpers are not available in pyvex builds");
}

#endif /* ndef AVX_512 */
/*---------------------------------------------------------------*/
/*--- end                         host_generic_AVX512_stub.c  ---*/
/*---------------------------------------------------------------*/
