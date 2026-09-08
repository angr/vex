#!/bin/sh

gcc -O0 -g -ggdb filegen.c
./a.out

# Add license text and remove duplicates in the generated structures
# TODO: deal with the hardcoded line numbers

head -n 7  host_generic_AVX512.h > host_generic_simd512-nodup.h
cat license.txt >> host_generic_simd512-nodup.h 
tail -n +8 host_generic_AVX512.h | head -n -4 | perl -ne 'print unless $dup{$_}++;' >> host_generic_simd512-nodup.h
tail -n 4 host_generic_AVX512.h >> host_generic_simd512-nodup.h

head -n 7 guest_AVX512.h > guest_AVX512-nodup.h
cat license.txt >> guest_AVX512-nodup.h
tail -n +8  guest_AVX512.h | head -n +39 >> guest_AVX512-nodup.h
tail -n +47 guest_AVX512.h | head -n -4 | perl -ne 'print unless $dup{$_}++;' >> guest_AVX512-nodup.h
tail -n 4 guest_AVX512.h >> guest_AVX512-nodup.h

head -n 7 host_AVX512.h > host_AVX512-nodup.h
cat license.txt >> host_AVX512-nodup.h
tail -n +8  host_AVX512.h | head -n +16 >> host_AVX512-nodup.h
tail -n +24 host_AVX512.h | head -n -4 | perl -ne 'print unless $dup{$_}++;' >> host_AVX512-nodup.h
tail -n 4 host_AVX512.h >> host_AVX512-nodup.h

# Must also be sorted because the code relies on certain IRs being in order
# for example, Iop_V512to64_0, Iop_V512to64_1 and so on
head -n 7 libvex_ir_AVX512.h > libvex_ir_AVX512-nodup.h
cat license.txt >> libvex_ir_AVX512-nodup.h
tail -n +8  libvex_ir_AVX512.h | head -n +6  >> libvex_ir_AVX512-nodup.h
tail -n +14 libvex_ir_AVX512.h | head -n -5 | sort | uniq >> libvex_ir_AVX512-nodup.h
tail -n 5 libvex_ir_AVX512.h >> libvex_ir_AVX512-nodup.h


cp host_generic_simd512-nodup.h ./../VEX/priv/host_generic_AVX512.h
cp libvex_ir_AVX512-nodup.h ./../VEX/pub/libvex_ir_AVX512.h
cp guest_AVX512-nodup.h ./../VEX/priv/guest_AVX512.h
cp host_AVX512-nodup.h ./../VEX/priv/host_AVX512.h

rm host_generic_simd512-nodup.h host_generic_AVX512.h guest_AVX512-nodup.h guest_AVX512.h host_AVX512-nodup.h host_AVX512.h libvex_ir_AVX512-nodup.h libvex_ir_AVX512.h 
