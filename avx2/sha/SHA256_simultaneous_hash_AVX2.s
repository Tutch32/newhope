.set SIZE,32



.macro broadcast DST, OFF

.endm
   
.macro round A,B,C,D,E,F,G,H,i

   vmovdqa  (((\i+0)%16)*32)(%rsp), %ymm14
   vpsrld   $14, \E, %ymm12
   vpslld   $18, \E, %ymm13
   vpxor    %ymm12, %ymm13, %ymm8

   vpsrld   $9, \A, %ymm12
   vpslld   $23, \A, %ymm13
   vpxor    %ymm12, %ymm13, %ymm9

   vpxor    \E, %ymm8, %ymm8
   vpxor    \G, \F, %ymm10
   
   vpsrld   $5, %ymm8, %ymm12
   vpslld   $27, %ymm8, %ymm13
   vpxor    %ymm12, %ymm13, %ymm8
   vpbroadcastd   (%rbx), %ymm12

   vpaddd   %ymm14, \H, %ymm11
   vpxor    \A, %ymm9, %ymm9
   
   vpaddd   %ymm12, %ymm11, %ymm11
   vpand    \E, %ymm10, %ymm10
   
   vpsrld   $11, %ymm9, %ymm12
   vpslld   $21, %ymm9, %ymm13
   vpaddd   (((\i+9)%16)*32)(%rsp), %ymm14, %ymm14
   vpxor    %ymm12, %ymm13, %ymm9
   
   vpxor    \E, %ymm8, %ymm8
   vpxor    \G, %ymm10, %ymm10
   
   vpxor    \C, \B, \H
   vpxor    \A, %ymm9, %ymm9
   
   vpaddd   %ymm10, %ymm11, %ymm11
   
   vpsrld   $6, %ymm8, %ymm12
   vpslld   $26, %ymm8, %ymm13
   vpxor    %ymm12, %ymm13, %ymm8
   
   vpand    \A, \H, \H
   vpand    \C, \B, %ymm10
   
   vpsrld   $2, %ymm9, %ymm12
   vpslld   $30, %ymm9, %ymm13
   vpxor    %ymm12, %ymm13, %ymm9

   vpaddd   %ymm8, %ymm11, %ymm11
   
   vmovdqa  (((\i+1)%16)*32)(%rsp), %ymm8
   vmovdqa  (((\i+14)%16)*32)(%rsp), %ymm15
   
   vpaddd   %ymm10, \H, \H
   
   vpsrld   $11, %ymm8, %ymm12
   vpslld   $21, %ymm8, %ymm13
   
   vpaddd   %ymm11, \H, \H  
   vpaddd   %ymm11, \D, \D
   
   vpxor    %ymm12, %ymm13, %ymm11
   vpsrld   $2, %ymm15, %ymm12
   vpslld   $30, %ymm15, %ymm13
   
   vpxor    %ymm12, %ymm13, %ymm10 
    
   vpaddd   %ymm9, \H, \H
   lea      4(%rbx), %rbx
   

   # Calculate sigma0
 
   
   vpxor    %ymm8, %ymm11, %ymm11
   vpxor    %ymm15, %ymm10, %ymm10
   vpsrld   $3, %ymm8, %ymm8
   vpsrld   $10, %ymm15, %ymm15

   vpsrld   $7, %ymm11, %ymm12
   vpslld   $25, %ymm11, %ymm13
   vpxor    %ymm12, %ymm13, %ymm11

   vpsrld   $17, %ymm10, %ymm12
   vpslld   $15, %ymm10, %ymm13
   vpxor    %ymm12, %ymm13, %ymm10   
   
   vpxor    %ymm11, %ymm8, %ymm8

   vpaddd   %ymm8, %ymm14, %ymm14
   vpxor    %ymm10, %ymm15, %ymm15
   vpaddd   %ymm15, %ymm14, %ymm14

   vmovdqa  %ymm14, (((\i+0)%16)*32)(%rsp)
.endm

.macro round_last A,B,C,D,E,F,G,H,i

   vpsrld   $14, \E, %ymm12
   vpslld   $18, \E, %ymm13
   vpxor    %ymm12, %ymm13, %ymm8

   vpsrld   $9, \A, %ymm12
   vpslld   $23, \A, %ymm13
   vpxor    %ymm12, %ymm13, %ymm9

   vpxor    \E, %ymm8, %ymm8
   vpxor    \G, \F, %ymm10
   
   vpsrld   $5, %ymm8, %ymm12
   vpslld   $27, %ymm8, %ymm13
   vpxor    %ymm12, %ymm13, %ymm8
   vpbroadcastd   (%rbx), %ymm12

   vpaddd   (\i*32)(%rsp), \H, %ymm11
   vpxor    \A, %ymm9, %ymm9
   
   vpaddd   %ymm12, %ymm11, %ymm11
   lea      4(%rbx), %rbx
   vpand    \E, %ymm10, %ymm10
   
   vpsrld   $11, %ymm9, %ymm12
   vpslld   $21, %ymm9, %ymm13
   vpxor    %ymm12, %ymm13, %ymm9
   
   vpxor    \E, %ymm8, %ymm8
   vpxor    \G, %ymm10, %ymm10
   
   vpxor    \C, \B, \H
   vpxor    \A, %ymm9, %ymm9
   
   vpaddd   %ymm10, %ymm11, %ymm11
   
   vpsrld   $6, %ymm8, %ymm12
   vpslld   $26, %ymm8, %ymm13
   vpxor    %ymm12, %ymm13, %ymm8
   
   vpand    \A, \H, \H
   vpand    \C, \B, %ymm10
   
   vpsrld   $2, %ymm9, %ymm12
   vpslld   $30, %ymm9, %ymm13
   vpxor    %ymm12, %ymm13, %ymm9

   vpaddd   %ymm8, %ymm11, %ymm11
   vpaddd   %ymm10, \H, \H
   vpaddd   %ymm11, \D, \D
   vpaddd   %ymm11, \H, \H
   vpaddd   %ymm9, \H, \H
.endm
#########################################################
.align	16
.global SHA256_simultaneous_update
SHA256_simultaneous_update:

#rdi - hashs
#rsi - buff pointers
#rdx - min length


   push  %rbx
   push  %rbp
   push  %r12
   push  %r13
   push  %r14
   push  %r15
   mov   %rsp, %rbp     # copy %rsp
   
	sub	$(SIZE*16),%rsp
	and	$-64,%rsp		# align stack frame
   
   mov   8*0(%rsi), %r8
   mov   8*1(%rsi), %r9
   mov   8*2(%rsi), %r10
   mov   8*3(%rsi), %r11
   mov   8*4(%rsi), %r12
   mov   8*5(%rsi), %r13
   mov   8*6(%rsi), %r14
   mov   8*7(%rsi), %r15
   
# first step - load the message, taking a word of each message, into appropriate 
# location inside an ymm register

   
      
   jmp   _L_outer_loop
   
.align 16
_L_outer_loop:
   vmovdqu  (%r8), %ymm0
   vmovdqu  (%r9), %ymm1
   vmovdqu  (%r10), %ymm2
   vmovdqu  (%r11), %ymm3
   vmovdqu  (%r12), %ymm4
   vmovdqu  (%r13), %ymm5
   vmovdqu  (%r14), %ymm6
   vmovdqu  (%r15), %ymm7
   
   vshufps  $0x44, %ymm1, %ymm0, %ymm8
   vshufps  $0xee, %ymm1, %ymm0, %ymm0
   vshufps  $0x44, %ymm3, %ymm2, %ymm9
   vshufps  $0xee, %ymm3, %ymm2, %ymm2
   vshufps  $0xdd, %ymm9, %ymm8, %ymm3
   vshufps  $0x88, %ymm2, %ymm0, %ymm1
   vshufps  $0xdd, %ymm2, %ymm0, %ymm0
   vshufps  $0x88, %ymm9, %ymm8, %ymm8
   
   vshufps  $0x44, %ymm5, %ymm4, %ymm2
   vshufps  $0xee, %ymm5, %ymm4, %ymm4
   vshufps  $0x44, %ymm7, %ymm6, %ymm9
   vshufps  $0xee, %ymm7, %ymm6, %ymm6
   vshufps  $0xdd, %ymm9, %ymm2, %ymm7
   vshufps  $0x88, %ymm6, %ymm4, %ymm5
   vshufps  $0xdd, %ymm6, %ymm4, %ymm4
   vshufps  $0x88, %ymm9, %ymm2, %ymm9
   
   vperm2f128  $0x13, %ymm1, %ymm5, %ymm6
   vperm2f128  $0x02, %ymm1, %ymm5, %ymm2
   vperm2f128  $0x13, %ymm3, %ymm7, %ymm5
   vperm2f128  $0x02, %ymm3, %ymm7, %ymm1
   vperm2f128  $0x13, %ymm0, %ymm4, %ymm7
   vperm2f128  $0x02, %ymm0, %ymm4, %ymm3
   vperm2f128  $0x13, %ymm8, %ymm9, %ymm4
   vperm2f128  $0x02, %ymm8, %ymm9, %ymm0
   
   vpshufb (bswap_mask256), %ymm0, %ymm0
   vpshufb (bswap_mask256), %ymm1, %ymm1
   vpshufb (bswap_mask256), %ymm2, %ymm2
   vpshufb (bswap_mask256), %ymm3, %ymm3
   vpshufb (bswap_mask256), %ymm4, %ymm4
   vpshufb (bswap_mask256), %ymm5, %ymm5
   vpshufb (bswap_mask256), %ymm6, %ymm6
   vpshufb (bswap_mask256), %ymm7, %ymm7
   
   vmovdqa  %ymm0, SIZE*0(%rsp)
   vmovdqa  %ymm1, SIZE*1(%rsp)
   vmovdqa  %ymm2, SIZE*2(%rsp)
   vmovdqa  %ymm3, SIZE*3(%rsp)
   vmovdqa  %ymm4, SIZE*4(%rsp)
   vmovdqa  %ymm5, SIZE*5(%rsp)
   vmovdqa  %ymm6, SIZE*6(%rsp)
   vmovdqa  %ymm7, SIZE*7(%rsp)

   vmovdqu  32(%r8), %ymm0
   vmovdqu  32(%r9), %ymm1
   vmovdqu  32(%r10), %ymm2
   vmovdqu  32(%r11), %ymm3
   vmovdqu  32(%r12), %ymm4
   vmovdqu  32(%r13), %ymm5
   vmovdqu  32(%r14), %ymm6
   vmovdqu  32(%r15), %ymm7
   
   vshufps  $0x44, %ymm1, %ymm0, %ymm8
   vshufps  $0xee, %ymm1, %ymm0, %ymm0
   vshufps  $0x44, %ymm3, %ymm2, %ymm9
   vshufps  $0xee, %ymm3, %ymm2, %ymm2
   vshufps  $0xdd, %ymm9, %ymm8, %ymm3
   vshufps  $0x88, %ymm2, %ymm0, %ymm1
   vshufps  $0xdd, %ymm2, %ymm0, %ymm0
   vshufps  $0x88, %ymm9, %ymm8, %ymm8
   
   vshufps  $0x44, %ymm5, %ymm4, %ymm2
   vshufps  $0xee, %ymm5, %ymm4, %ymm4
   vshufps  $0x44, %ymm7, %ymm6, %ymm9
   vshufps  $0xee, %ymm7, %ymm6, %ymm6
   vshufps  $0xdd, %ymm9, %ymm2, %ymm7
   vshufps  $0x88, %ymm6, %ymm4, %ymm5
   vshufps  $0xdd, %ymm6, %ymm4, %ymm4
   vshufps  $0x88, %ymm9, %ymm2, %ymm9
   
   vperm2f128  $0x13, %ymm1, %ymm5, %ymm6
   vperm2f128  $0x02, %ymm1, %ymm5, %ymm2
   vperm2f128  $0x13, %ymm3, %ymm7, %ymm5
   vperm2f128  $0x02, %ymm3, %ymm7, %ymm1
   vperm2f128  $0x13, %ymm0, %ymm4, %ymm7
   vperm2f128  $0x02, %ymm0, %ymm4, %ymm3
   vperm2f128  $0x13, %ymm8, %ymm9, %ymm4
   vperm2f128  $0x02, %ymm8, %ymm9, %ymm0
   
   vpshufb (bswap_mask256), %ymm0, %ymm0
   vpshufb (bswap_mask256), %ymm1, %ymm1
   vpshufb (bswap_mask256), %ymm2, %ymm2
   vpshufb (bswap_mask256), %ymm3, %ymm3
   vpshufb (bswap_mask256), %ymm4, %ymm4
   vpshufb (bswap_mask256), %ymm5, %ymm5
   vpshufb (bswap_mask256), %ymm6, %ymm6
   vpshufb (bswap_mask256), %ymm7, %ymm7
   
   
   vmovdqa  %ymm0, SIZE*8(%rsp)
   vmovdqa  %ymm1, SIZE*9(%rsp)
   vmovdqa  %ymm2, SIZE*10(%rsp)
   vmovdqa  %ymm3, SIZE*11(%rsp)
   vmovdqa  %ymm4, SIZE*12(%rsp)
   vmovdqa  %ymm5, SIZE*13(%rsp)
   vmovdqa  %ymm6, SIZE*14(%rsp)
   vmovdqa  %ymm7, SIZE*15(%rsp)
   
   #load the values of H
   vmovdqa  SIZE*0(%rdi), %ymm0
   vmovdqa  SIZE*1(%rdi), %ymm1
   vmovdqa  SIZE*2(%rdi), %ymm2
   vmovdqa  SIZE*3(%rdi), %ymm3
   vmovdqa  SIZE*4(%rdi), %ymm4
   vmovdqa  SIZE*5(%rdi), %ymm5
   vmovdqa  SIZE*6(%rdi), %ymm6
   vmovdqa  SIZE*7(%rdi), %ymm7
   
      cmp   $64, %rdx
      jb    _L_finish
      
      sub   $64, %rdx
      
      #promote the buffer pointers
      lea   64(%r8), %r8
      lea   64(%r9), %r9
      lea   64(%r10), %r10
      lea   64(%r11), %r11
      lea   64(%r12), %r12
      lea   64(%r13), %r13
      lea   64(%r14), %r14
      lea   64(%r15), %r15
         
      #load address of the K table
      lea	K_table_256(%rip), %rbx
      
      mov   $3, %rcx
      jmp   _L_inner_loop
      
.align 16
_L_inner_loop:
         
         round %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, 0
         round %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, 1
         round %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, 2
         round %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, 3
         round %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, 4
         round %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, 5
         round %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, 6
         round %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, 7
         
         round %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, 8
         round %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, 9
         round %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, 10
         round %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, 11
         round %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, 12
         round %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, 13
         round %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, 14
         round %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, 15
         
         dec   %rcx
         jne   _L_inner_loop
         
      round_last %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, 0
      round_last %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, 1
      round_last %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, 2
      round_last %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, 3
      round_last %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, 4
      round_last %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, 5
      round_last %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, 6
      round_last %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, 7
      
      round_last %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, 8
      round_last %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, 9
      round_last %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, 10
      round_last %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, 11
      round_last %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, %ymm3, 12
      round_last %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, %ymm2, 13
      round_last %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, %ymm1, 14
      round_last %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7, %ymm0, 15
      
      #update hash values
      
      vpaddd  SIZE*0(%rdi), %ymm0, %ymm0
      vpaddd  SIZE*1(%rdi), %ymm1, %ymm1
      vpaddd  SIZE*2(%rdi), %ymm2, %ymm2
      vpaddd  SIZE*3(%rdi), %ymm3, %ymm3
      vpaddd  SIZE*4(%rdi), %ymm4, %ymm4
      vpaddd  SIZE*5(%rdi), %ymm5, %ymm5
      vpaddd  SIZE*6(%rdi), %ymm6, %ymm6
      vpaddd  SIZE*7(%rdi), %ymm7, %ymm7
      
      vmovdqa  %ymm0, SIZE*0(%rdi)
      vmovdqa  %ymm1, SIZE*1(%rdi)
      vmovdqa  %ymm2, SIZE*2(%rdi)
      vmovdqa  %ymm3, SIZE*3(%rdi)
      vmovdqa  %ymm4, SIZE*4(%rdi)
      vmovdqa  %ymm5, SIZE*5(%rdi)
      vmovdqa  %ymm6, SIZE*6(%rdi)
      vmovdqa  %ymm7, SIZE*7(%rdi)
      
      
      jmp   _L_outer_loop
      
_L_finish:

   mov   %r8, 8*0(%rsi)
   mov   %r9, 8*1(%rsi)
   mov   %r10,8*2(%rsi)
   mov   %r11,8*3(%rsi)
   mov   %r12,8*4(%rsi)
   mov   %r13,8*5(%rsi)
   mov   %r14,8*6(%rsi)
   mov   %r15,8*7(%rsi)

   
   mov   %rbp, %rsp
   pop   %r15
   pop   %r14
   pop   %r13
   pop   %r12
   pop   %rbp
   pop   %rbx
   ret
   
.align 64
bswap_mask256:
.byte 3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12,3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12
K_table_256:
.long	0x428a2f98
.long	0x71374491
.long	0xb5c0fbcf
.long	0xe9b5dba5
.long	0x3956c25b
.long	0x59f111f1
.long	0x923f82a4
.long	0xab1c5ed5
.long	0xd807aa98
.long	0x12835b01
.long	0x243185be
.long	0x550c7dc3
.long	0x72be5d74
.long	0x80deb1fe
.long	0x9bdc06a7
.long	0xc19bf174
.long	0xe49b69c1
.long	0xefbe4786
.long	0x0fc19dc6
.long	0x240ca1cc
.long	0x2de92c6f
.long	0x4a7484aa
.long	0x5cb0a9dc
.long	0x76f988da
.long	0x983e5152
.long	0xa831c66d
.long	0xb00327c8
.long	0xbf597fc7
.long	0xc6e00bf3
.long	0xd5a79147
.long	0x06ca6351
.long	0x14292967
.long	0x27b70a85
.long	0x2e1b2138
.long	0x4d2c6dfc
.long	0x53380d13
.long	0x650a7354
.long	0x766a0abb
.long	0x81c2c92e
.long	0x92722c85
.long	0xa2bfe8a1
.long	0xa81a664b
.long	0xc24b8b70
.long	0xc76c51a3
.long	0xd192e819
.long	0xd6990624
.long	0xf40e3585
.long	0x106aa070
.long	0x19a4c116
.long	0x1e376c08
.long	0x2748774c
.long	0x34b0bcb5
.long	0x391c0cb3
.long	0x4ed8aa4a
.long	0x5b9cca4f
.long	0x682e6ff3
.long	0x748f82ee
.long	0x78a5636f
.long	0x84c87814
.long	0x8cc70208
.long	0x90befffa
.long	0xa4506ceb
.long	0xbef9a3f7
.long	0xc67178f2

