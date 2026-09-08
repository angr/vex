/*---------------------------------------------------------------*/
/*--- begin                           host_generic_imm8_def.c ---*/
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

/* This code is horrific but it seems to be the least worst option.
 * A compiler must know imm8 value for intrinsic in compile time.
 * We only get it in runtime, while emulating some IR with intrinsics.
 * So, we generate all the possibilities beforehand and switch to the right one 
 * in runtime. */

/* If there is any way to rewrite it, please go ahead. */

#define IMM8_SWITCHCASE_3( _fname, ...) \
switch (imm8) { \
   case   0: res = _fname (__VA_ARGS__,   0); break; \
   case   1: res = _fname (__VA_ARGS__,   1); break; \
   case   2: res = _fname (__VA_ARGS__,   2); break; \
   case   3: res = _fname (__VA_ARGS__,   3); break; \
   case   4: res = _fname (__VA_ARGS__,   4); break; \
   case   5: res = _fname (__VA_ARGS__,   5); break; \
   case   6: res = _fname (__VA_ARGS__,   6); break; \
   case   7: res = _fname (__VA_ARGS__,   7); break; \
   default: break; \
}

#define IMM8_SWITCHCASE_4( _fname, ...) \
switch (imm8) { \
   case   0: res = _fname (__VA_ARGS__,   0); break; \
   case   1: res = _fname (__VA_ARGS__,   1); break; \
   case   2: res = _fname (__VA_ARGS__,   2); break; \
   case   3: res = _fname (__VA_ARGS__,   3); break; \
   case   4: res = _fname (__VA_ARGS__,   4); break; \
   case   5: res = _fname (__VA_ARGS__,   5); break; \
   case   6: res = _fname (__VA_ARGS__,   6); break; \
   case   7: res = _fname (__VA_ARGS__,   7); break; \
   case   8: res = _fname (__VA_ARGS__,   8); break; \
   case   9: res = _fname (__VA_ARGS__,   9); break; \
   case  10: res = _fname (__VA_ARGS__,  10); break; \
   case  11: res = _fname (__VA_ARGS__,  11); break; \
   case  12: res = _fname (__VA_ARGS__,  12); break; \
   case  13: res = _fname (__VA_ARGS__,  13); break; \
   case  14: res = _fname (__VA_ARGS__,  14); break; \
   case  15: res = _fname (__VA_ARGS__,  15); break; \
   default: break; \
}

#define IMM8_SWITCHCASE_5( _fname, ...) \
switch (imm8) { \
   case   0: res = _fname (__VA_ARGS__,   0); break; \
   case   1: res = _fname (__VA_ARGS__,   1); break; \
   case   2: res = _fname (__VA_ARGS__,   2); break; \
   case   3: res = _fname (__VA_ARGS__,   3); break; \
   case   4: res = _fname (__VA_ARGS__,   4); break; \
   case   5: res = _fname (__VA_ARGS__,   5); break; \
   case   6: res = _fname (__VA_ARGS__,   6); break; \
   case   7: res = _fname (__VA_ARGS__,   7); break; \
   case   8: res = _fname (__VA_ARGS__,   8); break; \
   case   9: res = _fname (__VA_ARGS__,   9); break; \
   case  10: res = _fname (__VA_ARGS__,  10); break; \
   case  11: res = _fname (__VA_ARGS__,  11); break; \
   case  12: res = _fname (__VA_ARGS__,  12); break; \
   case  13: res = _fname (__VA_ARGS__,  13); break; \
   case  14: res = _fname (__VA_ARGS__,  14); break; \
   case  15: res = _fname (__VA_ARGS__,  15); break; \
   case  16: res = _fname (__VA_ARGS__,  16); break; \
   case  17: res = _fname (__VA_ARGS__,  17); break; \
   case  18: res = _fname (__VA_ARGS__,  18); break; \
   case  19: res = _fname (__VA_ARGS__,  19); break; \
   case  20: res = _fname (__VA_ARGS__,  20); break; \
   case  21: res = _fname (__VA_ARGS__,  21); break; \
   case  22: res = _fname (__VA_ARGS__,  22); break; \
   case  23: res = _fname (__VA_ARGS__,  23); break; \
   case  24: res = _fname (__VA_ARGS__,  24); break; \
   case  25: res = _fname (__VA_ARGS__,  25); break; \
   case  26: res = _fname (__VA_ARGS__,  26); break; \
   case  27: res = _fname (__VA_ARGS__,  27); break; \
   case  28: res = _fname (__VA_ARGS__,  28); break; \
   case  29: res = _fname (__VA_ARGS__,  29); break; \
   case  30: res = _fname (__VA_ARGS__,  30); break; \
   case  31: res = _fname (__VA_ARGS__,  31); break; \
   default: break; \
}

#define IMM8_SWITCHCASE( _fname, ...) \
switch (imm8) { \
   case   0: res = _fname (__VA_ARGS__,   0); break; \
   case   1: res = _fname (__VA_ARGS__,   1); break; \
   case   2: res = _fname (__VA_ARGS__,   2); break; \
   case   3: res = _fname (__VA_ARGS__,   3); break; \
   case   4: res = _fname (__VA_ARGS__,   4); break; \
   case   5: res = _fname (__VA_ARGS__,   5); break; \
   case   6: res = _fname (__VA_ARGS__,   6); break; \
   case   7: res = _fname (__VA_ARGS__,   7); break; \
   case   8: res = _fname (__VA_ARGS__,   8); break; \
   case   9: res = _fname (__VA_ARGS__,   9); break; \
   case  10: res = _fname (__VA_ARGS__,  10); break; \
   case  11: res = _fname (__VA_ARGS__,  11); break; \
   case  12: res = _fname (__VA_ARGS__,  12); break; \
   case  13: res = _fname (__VA_ARGS__,  13); break; \
   case  14: res = _fname (__VA_ARGS__,  14); break; \
   case  15: res = _fname (__VA_ARGS__,  15); break; \
   case  16: res = _fname (__VA_ARGS__,  16); break; \
   case  17: res = _fname (__VA_ARGS__,  17); break; \
   case  18: res = _fname (__VA_ARGS__,  18); break; \
   case  19: res = _fname (__VA_ARGS__,  19); break; \
   case  20: res = _fname (__VA_ARGS__,  20); break; \
   case  21: res = _fname (__VA_ARGS__,  21); break; \
   case  22: res = _fname (__VA_ARGS__,  22); break; \
   case  23: res = _fname (__VA_ARGS__,  23); break; \
   case  24: res = _fname (__VA_ARGS__,  24); break; \
   case  25: res = _fname (__VA_ARGS__,  25); break; \
   case  26: res = _fname (__VA_ARGS__,  26); break; \
   case  27: res = _fname (__VA_ARGS__,  27); break; \
   case  28: res = _fname (__VA_ARGS__,  28); break; \
   case  29: res = _fname (__VA_ARGS__,  29); break; \
   case  30: res = _fname (__VA_ARGS__,  30); break; \
   case  31: res = _fname (__VA_ARGS__,  31); break; \
   case  32: res = _fname (__VA_ARGS__,  32); break; \
   case  33: res = _fname (__VA_ARGS__,  33); break; \
   case  34: res = _fname (__VA_ARGS__,  34); break; \
   case  35: res = _fname (__VA_ARGS__,  35); break; \
   case  36: res = _fname (__VA_ARGS__,  36); break; \
   case  37: res = _fname (__VA_ARGS__,  37); break; \
   case  38: res = _fname (__VA_ARGS__,  38); break; \
   case  39: res = _fname (__VA_ARGS__,  39); break; \
   case  40: res = _fname (__VA_ARGS__,  40); break; \
   case  41: res = _fname (__VA_ARGS__,  41); break; \
   case  42: res = _fname (__VA_ARGS__,  42); break; \
   case  43: res = _fname (__VA_ARGS__,  43); break; \
   case  44: res = _fname (__VA_ARGS__,  44); break; \
   case  45: res = _fname (__VA_ARGS__,  45); break; \
   case  46: res = _fname (__VA_ARGS__,  46); break; \
   case  47: res = _fname (__VA_ARGS__,  47); break; \
   case  48: res = _fname (__VA_ARGS__,  48); break; \
   case  49: res = _fname (__VA_ARGS__,  49); break; \
   case  50: res = _fname (__VA_ARGS__,  50); break; \
   case  51: res = _fname (__VA_ARGS__,  51); break; \
   case  52: res = _fname (__VA_ARGS__,  52); break; \
   case  53: res = _fname (__VA_ARGS__,  53); break; \
   case  54: res = _fname (__VA_ARGS__,  54); break; \
   case  55: res = _fname (__VA_ARGS__,  55); break; \
   case  56: res = _fname (__VA_ARGS__,  56); break; \
   case  57: res = _fname (__VA_ARGS__,  57); break; \
   case  58: res = _fname (__VA_ARGS__,  58); break; \
   case  59: res = _fname (__VA_ARGS__,  59); break; \
   case  60: res = _fname (__VA_ARGS__,  60); break; \
   case  61: res = _fname (__VA_ARGS__,  61); break; \
   case  62: res = _fname (__VA_ARGS__,  62); break; \
   case  63: res = _fname (__VA_ARGS__,  63); break; \
   case  64: res = _fname (__VA_ARGS__,  64); break; \
   case  65: res = _fname (__VA_ARGS__,  65); break; \
   case  66: res = _fname (__VA_ARGS__,  66); break; \
   case  67: res = _fname (__VA_ARGS__,  67); break; \
   case  68: res = _fname (__VA_ARGS__,  68); break; \
   case  69: res = _fname (__VA_ARGS__,  69); break; \
   case  70: res = _fname (__VA_ARGS__,  70); break; \
   case  71: res = _fname (__VA_ARGS__,  71); break; \
   case  72: res = _fname (__VA_ARGS__,  72); break; \
   case  73: res = _fname (__VA_ARGS__,  73); break; \
   case  74: res = _fname (__VA_ARGS__,  74); break; \
   case  75: res = _fname (__VA_ARGS__,  75); break; \
   case  76: res = _fname (__VA_ARGS__,  76); break; \
   case  77: res = _fname (__VA_ARGS__,  77); break; \
   case  78: res = _fname (__VA_ARGS__,  78); break; \
   case  79: res = _fname (__VA_ARGS__,  79); break; \
   case  80: res = _fname (__VA_ARGS__,  80); break; \
   case  81: res = _fname (__VA_ARGS__,  81); break; \
   case  82: res = _fname (__VA_ARGS__,  82); break; \
   case  83: res = _fname (__VA_ARGS__,  83); break; \
   case  84: res = _fname (__VA_ARGS__,  84); break; \
   case  85: res = _fname (__VA_ARGS__,  85); break; \
   case  86: res = _fname (__VA_ARGS__,  86); break; \
   case  87: res = _fname (__VA_ARGS__,  87); break; \
   case  88: res = _fname (__VA_ARGS__,  88); break; \
   case  89: res = _fname (__VA_ARGS__,  89); break; \
   case  90: res = _fname (__VA_ARGS__,  90); break; \
   case  91: res = _fname (__VA_ARGS__,  91); break; \
   case  92: res = _fname (__VA_ARGS__,  92); break; \
   case  93: res = _fname (__VA_ARGS__,  93); break; \
   case  94: res = _fname (__VA_ARGS__,  94); break; \
   case  95: res = _fname (__VA_ARGS__,  95); break; \
   case  96: res = _fname (__VA_ARGS__,  96); break; \
   case  97: res = _fname (__VA_ARGS__,  97); break; \
   case  98: res = _fname (__VA_ARGS__,  98); break; \
   case  99: res = _fname (__VA_ARGS__,  99); break; \
   case 100: res = _fname (__VA_ARGS__, 100); break; \
   case 101: res = _fname (__VA_ARGS__, 101); break; \
   case 102: res = _fname (__VA_ARGS__, 102); break; \
   case 103: res = _fname (__VA_ARGS__, 103); break; \
   case 104: res = _fname (__VA_ARGS__, 104); break; \
   case 105: res = _fname (__VA_ARGS__, 105); break; \
   case 106: res = _fname (__VA_ARGS__, 106); break; \
   case 107: res = _fname (__VA_ARGS__, 107); break; \
   case 108: res = _fname (__VA_ARGS__, 108); break; \
   case 109: res = _fname (__VA_ARGS__, 109); break; \
   case 110: res = _fname (__VA_ARGS__, 110); break; \
   case 111: res = _fname (__VA_ARGS__, 111); break; \
   case 112: res = _fname (__VA_ARGS__, 112); break; \
   case 113: res = _fname (__VA_ARGS__, 113); break; \
   case 114: res = _fname (__VA_ARGS__, 114); break; \
   case 115: res = _fname (__VA_ARGS__, 115); break; \
   case 116: res = _fname (__VA_ARGS__, 116); break; \
   case 117: res = _fname (__VA_ARGS__, 117); break; \
   case 118: res = _fname (__VA_ARGS__, 118); break; \
   case 119: res = _fname (__VA_ARGS__, 119); break; \
   case 120: res = _fname (__VA_ARGS__, 120); break; \
   case 121: res = _fname (__VA_ARGS__, 121); break; \
   case 122: res = _fname (__VA_ARGS__, 122); break; \
   case 123: res = _fname (__VA_ARGS__, 123); break; \
   case 124: res = _fname (__VA_ARGS__, 124); break; \
   case 125: res = _fname (__VA_ARGS__, 125); break; \
   case 126: res = _fname (__VA_ARGS__, 126); break; \
   case 127: res = _fname (__VA_ARGS__, 127); break; \
   case 128: res = _fname (__VA_ARGS__, 128); break; \
   case 129: res = _fname (__VA_ARGS__, 129); break; \
   case 130: res = _fname (__VA_ARGS__, 130); break; \
   case 131: res = _fname (__VA_ARGS__, 131); break; \
   case 132: res = _fname (__VA_ARGS__, 132); break; \
   case 133: res = _fname (__VA_ARGS__, 133); break; \
   case 134: res = _fname (__VA_ARGS__, 134); break; \
   case 135: res = _fname (__VA_ARGS__, 135); break; \
   case 136: res = _fname (__VA_ARGS__, 136); break; \
   case 137: res = _fname (__VA_ARGS__, 137); break; \
   case 138: res = _fname (__VA_ARGS__, 138); break; \
   case 139: res = _fname (__VA_ARGS__, 139); break; \
   case 140: res = _fname (__VA_ARGS__, 140); break; \
   case 141: res = _fname (__VA_ARGS__, 141); break; \
   case 142: res = _fname (__VA_ARGS__, 142); break; \
   case 143: res = _fname (__VA_ARGS__, 143); break; \
   case 144: res = _fname (__VA_ARGS__, 144); break; \
   case 145: res = _fname (__VA_ARGS__, 145); break; \
   case 146: res = _fname (__VA_ARGS__, 146); break; \
   case 147: res = _fname (__VA_ARGS__, 147); break; \
   case 148: res = _fname (__VA_ARGS__, 148); break; \
   case 149: res = _fname (__VA_ARGS__, 149); break; \
   case 150: res = _fname (__VA_ARGS__, 150); break; \
   case 151: res = _fname (__VA_ARGS__, 151); break; \
   case 152: res = _fname (__VA_ARGS__, 152); break; \
   case 153: res = _fname (__VA_ARGS__, 153); break; \
   case 154: res = _fname (__VA_ARGS__, 154); break; \
   case 155: res = _fname (__VA_ARGS__, 155); break; \
   case 156: res = _fname (__VA_ARGS__, 156); break; \
   case 157: res = _fname (__VA_ARGS__, 157); break; \
   case 158: res = _fname (__VA_ARGS__, 158); break; \
   case 159: res = _fname (__VA_ARGS__, 159); break; \
   case 160: res = _fname (__VA_ARGS__, 160); break; \
   case 161: res = _fname (__VA_ARGS__, 161); break; \
   case 162: res = _fname (__VA_ARGS__, 162); break; \
   case 163: res = _fname (__VA_ARGS__, 163); break; \
   case 164: res = _fname (__VA_ARGS__, 164); break; \
   case 165: res = _fname (__VA_ARGS__, 165); break; \
   case 166: res = _fname (__VA_ARGS__, 166); break; \
   case 167: res = _fname (__VA_ARGS__, 167); break; \
   case 168: res = _fname (__VA_ARGS__, 168); break; \
   case 169: res = _fname (__VA_ARGS__, 169); break; \
   case 170: res = _fname (__VA_ARGS__, 170); break; \
   case 171: res = _fname (__VA_ARGS__, 171); break; \
   case 172: res = _fname (__VA_ARGS__, 172); break; \
   case 173: res = _fname (__VA_ARGS__, 173); break; \
   case 174: res = _fname (__VA_ARGS__, 174); break; \
   case 175: res = _fname (__VA_ARGS__, 175); break; \
   case 176: res = _fname (__VA_ARGS__, 176); break; \
   case 177: res = _fname (__VA_ARGS__, 177); break; \
   case 178: res = _fname (__VA_ARGS__, 178); break; \
   case 179: res = _fname (__VA_ARGS__, 179); break; \
   case 180: res = _fname (__VA_ARGS__, 180); break; \
   case 181: res = _fname (__VA_ARGS__, 181); break; \
   case 182: res = _fname (__VA_ARGS__, 182); break; \
   case 183: res = _fname (__VA_ARGS__, 183); break; \
   case 184: res = _fname (__VA_ARGS__, 184); break; \
   case 185: res = _fname (__VA_ARGS__, 185); break; \
   case 186: res = _fname (__VA_ARGS__, 186); break; \
   case 187: res = _fname (__VA_ARGS__, 187); break; \
   case 188: res = _fname (__VA_ARGS__, 188); break; \
   case 189: res = _fname (__VA_ARGS__, 189); break; \
   case 190: res = _fname (__VA_ARGS__, 190); break; \
   case 191: res = _fname (__VA_ARGS__, 191); break; \
   case 192: res = _fname (__VA_ARGS__, 192); break; \
   case 193: res = _fname (__VA_ARGS__, 193); break; \
   case 194: res = _fname (__VA_ARGS__, 194); break; \
   case 195: res = _fname (__VA_ARGS__, 195); break; \
   case 196: res = _fname (__VA_ARGS__, 196); break; \
   case 197: res = _fname (__VA_ARGS__, 197); break; \
   case 198: res = _fname (__VA_ARGS__, 198); break; \
   case 199: res = _fname (__VA_ARGS__, 199); break; \
   case 200: res = _fname (__VA_ARGS__, 200); break; \
   case 201: res = _fname (__VA_ARGS__, 201); break; \
   case 202: res = _fname (__VA_ARGS__, 202); break; \
   case 203: res = _fname (__VA_ARGS__, 203); break; \
   case 204: res = _fname (__VA_ARGS__, 204); break; \
   case 205: res = _fname (__VA_ARGS__, 205); break; \
   case 206: res = _fname (__VA_ARGS__, 206); break; \
   case 207: res = _fname (__VA_ARGS__, 207); break; \
   case 208: res = _fname (__VA_ARGS__, 208); break; \
   case 209: res = _fname (__VA_ARGS__, 209); break; \
   case 210: res = _fname (__VA_ARGS__, 210); break; \
   case 211: res = _fname (__VA_ARGS__, 211); break; \
   case 212: res = _fname (__VA_ARGS__, 212); break; \
   case 213: res = _fname (__VA_ARGS__, 213); break; \
   case 214: res = _fname (__VA_ARGS__, 214); break; \
   case 215: res = _fname (__VA_ARGS__, 215); break; \
   case 216: res = _fname (__VA_ARGS__, 216); break; \
   case 217: res = _fname (__VA_ARGS__, 217); break; \
   case 218: res = _fname (__VA_ARGS__, 218); break; \
   case 219: res = _fname (__VA_ARGS__, 219); break; \
   case 220: res = _fname (__VA_ARGS__, 220); break; \
   case 221: res = _fname (__VA_ARGS__, 221); break; \
   case 222: res = _fname (__VA_ARGS__, 222); break; \
   case 223: res = _fname (__VA_ARGS__, 223); break; \
   case 224: res = _fname (__VA_ARGS__, 224); break; \
   case 225: res = _fname (__VA_ARGS__, 225); break; \
   case 226: res = _fname (__VA_ARGS__, 226); break; \
   case 227: res = _fname (__VA_ARGS__, 227); break; \
   case 228: res = _fname (__VA_ARGS__, 228); break; \
   case 229: res = _fname (__VA_ARGS__, 229); break; \
   case 230: res = _fname (__VA_ARGS__, 230); break; \
   case 231: res = _fname (__VA_ARGS__, 231); break; \
   case 232: res = _fname (__VA_ARGS__, 232); break; \
   case 233: res = _fname (__VA_ARGS__, 233); break; \
   case 234: res = _fname (__VA_ARGS__, 234); break; \
   case 235: res = _fname (__VA_ARGS__, 235); break; \
   case 236: res = _fname (__VA_ARGS__, 236); break; \
   case 237: res = _fname (__VA_ARGS__, 237); break; \
   case 238: res = _fname (__VA_ARGS__, 238); break; \
   case 239: res = _fname (__VA_ARGS__, 239); break; \
   case 240: res = _fname (__VA_ARGS__, 240); break; \
   case 241: res = _fname (__VA_ARGS__, 241); break; \
   case 242: res = _fname (__VA_ARGS__, 242); break; \
   case 243: res = _fname (__VA_ARGS__, 243); break; \
   case 244: res = _fname (__VA_ARGS__, 244); break; \
   case 245: res = _fname (__VA_ARGS__, 245); break; \
   case 246: res = _fname (__VA_ARGS__, 246); break; \
   case 247: res = _fname (__VA_ARGS__, 247); break; \
   case 248: res = _fname (__VA_ARGS__, 248); break; \
   case 249: res = _fname (__VA_ARGS__, 249); break; \
   case 250: res = _fname (__VA_ARGS__, 250); break; \
   case 251: res = _fname (__VA_ARGS__, 251); break; \
   case 252: res = _fname (__VA_ARGS__, 252); break; \
   case 253: res = _fname (__VA_ARGS__, 253); break; \
   case 254: res = _fname (__VA_ARGS__, 254); break; \
   case 255: res = _fname (__VA_ARGS__, 255); break; \
   default: break; \
} 


#define MANTISSA_SWITCHCASE( _fname, ...) \
switch (sc) { \
   case 0: \
      switch (interv) { \
         case 0: res = _fname (__VA_ARGS__, 0, 0);break; \
         case 1: res = _fname (__VA_ARGS__, 1, 0);break; \
         case 2: res = _fname (__VA_ARGS__, 2, 0);break; \
         case 3: res = _fname (__VA_ARGS__, 3, 0);break; \
      } \
      break; \
   case 1: \
      switch (interv) { \
         case 0: res = _fname (__VA_ARGS__, 0, 1);break; \
         case 1: res = _fname (__VA_ARGS__, 1, 1);break; \
         case 2: res = _fname (__VA_ARGS__, 2, 1);break; \
         case 3: res = _fname (__VA_ARGS__, 3, 1);break; \
      } \
      break; \
   case 2: \
      switch (interv) { \
         case 0: res = _fname (__VA_ARGS__, 0, 2);break; \
         case 1: res = _fname (__VA_ARGS__, 1, 2);break; \
         case 2: res = _fname (__VA_ARGS__, 2, 2);break; \
         case 3: res = _fname (__VA_ARGS__, 3, 2);break; \
      } \
      break; \
   default: break; \
}\

/*---------------------------------------------------------------*/
/*--- end                             host_generic_imm8_def.c ---*/
/*---------------------------------------------------------------*/
