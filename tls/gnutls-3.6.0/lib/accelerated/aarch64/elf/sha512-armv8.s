# 1 "lib/accelerated/aarch64/elf/sha512-armv8.s.tmp.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "lib/accelerated/aarch64/elf/sha512-armv8.s.tmp.S"
# Copyright (c) 2011-2016, Andy Polyakov <appro@openssl.org>
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# * Redistributions of source code must retain copyright notices,
# this list of conditions and the following disclaimer.

# * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following
# disclaimer in the documentation and/or other materials
# provided with the distribution.

# * Neither the name of the Andy Polyakov nor the names of its
# copyright holder and contributors may be used to endorse or
# promote products derived from this software without specific
# prior written permission.

# ALTERNATIVELY, provided that this notice is retained in full, this
# product may be distributed under the terms of the GNU General Public
# License (GPL), in which case the provisions of the GPL apply INSTEAD OF
# those given above.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# *** This file is auto-generated ***

# 1 "lib/accelerated/aarch64/aarch64-common.h" 1
# 41 "lib/accelerated/aarch64/elf/sha512-armv8.s.tmp.S" 2

.text


.globl sha512_block_data_order
.type sha512_block_data_order,%function
.align 6
sha512_block_data_order:
 stp x29,x30,[sp,#-128]!
 add x29,sp,#0

 stp x19,x20,[sp,#16]
 stp x21,x22,[sp,#32]
 stp x23,x24,[sp,#48]
 stp x25,x26,[sp,#64]
 stp x27,x28,[sp,#80]
 sub sp,sp,#4*8

 ldp x20,x21,[x0]
 ldp x22,x23,[x0,#2*8]
 ldp x24,x25,[x0,#4*8]
 add x2,x1,x2,lsl#7
 ldp x26,x27,[x0,#6*8]
 adr x30,.LK512
 stp x0,x2,[x29,#96]

.Loop:
 ldp x3,x4,[x1],#2*8
 ldr x19,[x30],#8
 eor x28,x21,x22
 str x1,[x29,#112]

 rev x3,x3

 ror x16,x24,#14
 add x27,x27,x19
 eor x6,x24,x24,ror#23
 and x17,x25,x24
 bic x19,x26,x24
 add x27,x27,x3
 orr x17,x17,x19
 eor x19,x20,x21
 eor x16,x16,x6,ror#18
 ror x6,x20,#28
 add x27,x27,x17
 eor x17,x20,x20,ror#5
 add x27,x27,x16
 and x28,x28,x19
 add x23,x23,x27
 eor x28,x28,x21
 eor x17,x6,x17,ror#34
 add x27,x27,x28
 ldr x28,[x30],#8


 rev x4,x4

 ldp x5,x6,[x1],#2*8
 add x27,x27,x17
 ror x16,x23,#14
 add x26,x26,x28
 eor x7,x23,x23,ror#23
 and x17,x24,x23
 bic x28,x25,x23
 add x26,x26,x4
 orr x17,x17,x28
 eor x28,x27,x20
 eor x16,x16,x7,ror#18
 ror x7,x27,#28
 add x26,x26,x17
 eor x17,x27,x27,ror#5
 add x26,x26,x16
 and x19,x19,x28
 add x22,x22,x26
 eor x19,x19,x20
 eor x17,x7,x17,ror#34
 add x26,x26,x19
 ldr x19,[x30],#8


 rev x5,x5

 add x26,x26,x17
 ror x16,x22,#14
 add x25,x25,x19
 eor x8,x22,x22,ror#23
 and x17,x23,x22
 bic x19,x24,x22
 add x25,x25,x5
 orr x17,x17,x19
 eor x19,x26,x27
 eor x16,x16,x8,ror#18
 ror x8,x26,#28
 add x25,x25,x17
 eor x17,x26,x26,ror#5
 add x25,x25,x16
 and x28,x28,x19
 add x21,x21,x25
 eor x28,x28,x27
 eor x17,x8,x17,ror#34
 add x25,x25,x28
 ldr x28,[x30],#8


 rev x6,x6

 ldp x7,x8,[x1],#2*8
 add x25,x25,x17
 ror x16,x21,#14
 add x24,x24,x28
 eor x9,x21,x21,ror#23
 and x17,x22,x21
 bic x28,x23,x21
 add x24,x24,x6
 orr x17,x17,x28
 eor x28,x25,x26
 eor x16,x16,x9,ror#18
 ror x9,x25,#28
 add x24,x24,x17
 eor x17,x25,x25,ror#5
 add x24,x24,x16
 and x19,x19,x28
 add x20,x20,x24
 eor x19,x19,x26
 eor x17,x9,x17,ror#34
 add x24,x24,x19
 ldr x19,[x30],#8


 rev x7,x7

 add x24,x24,x17
 ror x16,x20,#14
 add x23,x23,x19
 eor x10,x20,x20,ror#23
 and x17,x21,x20
 bic x19,x22,x20
 add x23,x23,x7
 orr x17,x17,x19
 eor x19,x24,x25
 eor x16,x16,x10,ror#18
 ror x10,x24,#28
 add x23,x23,x17
 eor x17,x24,x24,ror#5
 add x23,x23,x16
 and x28,x28,x19
 add x27,x27,x23
 eor x28,x28,x25
 eor x17,x10,x17,ror#34
 add x23,x23,x28
 ldr x28,[x30],#8


 rev x8,x8

 ldp x9,x10,[x1],#2*8
 add x23,x23,x17
 ror x16,x27,#14
 add x22,x22,x28
 eor x11,x27,x27,ror#23
 and x17,x20,x27
 bic x28,x21,x27
 add x22,x22,x8
 orr x17,x17,x28
 eor x28,x23,x24
 eor x16,x16,x11,ror#18
 ror x11,x23,#28
 add x22,x22,x17
 eor x17,x23,x23,ror#5
 add x22,x22,x16
 and x19,x19,x28
 add x26,x26,x22
 eor x19,x19,x24
 eor x17,x11,x17,ror#34
 add x22,x22,x19
 ldr x19,[x30],#8


 rev x9,x9

 add x22,x22,x17
 ror x16,x26,#14
 add x21,x21,x19
 eor x12,x26,x26,ror#23
 and x17,x27,x26
 bic x19,x20,x26
 add x21,x21,x9
 orr x17,x17,x19
 eor x19,x22,x23
 eor x16,x16,x12,ror#18
 ror x12,x22,#28
 add x21,x21,x17
 eor x17,x22,x22,ror#5
 add x21,x21,x16
 and x28,x28,x19
 add x25,x25,x21
 eor x28,x28,x23
 eor x17,x12,x17,ror#34
 add x21,x21,x28
 ldr x28,[x30],#8


 rev x10,x10

 ldp x11,x12,[x1],#2*8
 add x21,x21,x17
 ror x16,x25,#14
 add x20,x20,x28
 eor x13,x25,x25,ror#23
 and x17,x26,x25
 bic x28,x27,x25
 add x20,x20,x10
 orr x17,x17,x28
 eor x28,x21,x22
 eor x16,x16,x13,ror#18
 ror x13,x21,#28
 add x20,x20,x17
 eor x17,x21,x21,ror#5
 add x20,x20,x16
 and x19,x19,x28
 add x24,x24,x20
 eor x19,x19,x22
 eor x17,x13,x17,ror#34
 add x20,x20,x19
 ldr x19,[x30],#8


 rev x11,x11

 add x20,x20,x17
 ror x16,x24,#14
 add x27,x27,x19
 eor x14,x24,x24,ror#23
 and x17,x25,x24
 bic x19,x26,x24
 add x27,x27,x11
 orr x17,x17,x19
 eor x19,x20,x21
 eor x16,x16,x14,ror#18
 ror x14,x20,#28
 add x27,x27,x17
 eor x17,x20,x20,ror#5
 add x27,x27,x16
 and x28,x28,x19
 add x23,x23,x27
 eor x28,x28,x21
 eor x17,x14,x17,ror#34
 add x27,x27,x28
 ldr x28,[x30],#8


 rev x12,x12

 ldp x13,x14,[x1],#2*8
 add x27,x27,x17
 ror x16,x23,#14
 add x26,x26,x28
 eor x15,x23,x23,ror#23
 and x17,x24,x23
 bic x28,x25,x23
 add x26,x26,x12
 orr x17,x17,x28
 eor x28,x27,x20
 eor x16,x16,x15,ror#18
 ror x15,x27,#28
 add x26,x26,x17
 eor x17,x27,x27,ror#5
 add x26,x26,x16
 and x19,x19,x28
 add x22,x22,x26
 eor x19,x19,x20
 eor x17,x15,x17,ror#34
 add x26,x26,x19
 ldr x19,[x30],#8


 rev x13,x13

 add x26,x26,x17
 ror x16,x22,#14
 add x25,x25,x19
 eor x0,x22,x22,ror#23
 and x17,x23,x22
 bic x19,x24,x22
 add x25,x25,x13
 orr x17,x17,x19
 eor x19,x26,x27
 eor x16,x16,x0,ror#18
 ror x0,x26,#28
 add x25,x25,x17
 eor x17,x26,x26,ror#5
 add x25,x25,x16
 and x28,x28,x19
 add x21,x21,x25
 eor x28,x28,x27
 eor x17,x0,x17,ror#34
 add x25,x25,x28
 ldr x28,[x30],#8


 rev x14,x14

 ldp x15,x0,[x1],#2*8
 add x25,x25,x17
 str x6,[sp,#24]
 ror x16,x21,#14
 add x24,x24,x28
 eor x6,x21,x21,ror#23
 and x17,x22,x21
 bic x28,x23,x21
 add x24,x24,x14
 orr x17,x17,x28
 eor x28,x25,x26
 eor x16,x16,x6,ror#18
 ror x6,x25,#28
 add x24,x24,x17
 eor x17,x25,x25,ror#5
 add x24,x24,x16
 and x19,x19,x28
 add x20,x20,x24
 eor x19,x19,x26
 eor x17,x6,x17,ror#34
 add x24,x24,x19
 ldr x19,[x30],#8


 rev x15,x15

 add x24,x24,x17
 str x7,[sp,#0]
 ror x16,x20,#14
 add x23,x23,x19
 eor x7,x20,x20,ror#23
 and x17,x21,x20
 bic x19,x22,x20
 add x23,x23,x15
 orr x17,x17,x19
 eor x19,x24,x25
 eor x16,x16,x7,ror#18
 ror x7,x24,#28
 add x23,x23,x17
 eor x17,x24,x24,ror#5
 add x23,x23,x16
 and x28,x28,x19
 add x27,x27,x23
 eor x28,x28,x25
 eor x17,x7,x17,ror#34
 add x23,x23,x28
 ldr x28,[x30],#8


 rev x0,x0

 ldp x1,x2,[x1]
 add x23,x23,x17
 str x8,[sp,#8]
 ror x16,x27,#14
 add x22,x22,x28
 eor x8,x27,x27,ror#23
 and x17,x20,x27
 bic x28,x21,x27
 add x22,x22,x0
 orr x17,x17,x28
 eor x28,x23,x24
 eor x16,x16,x8,ror#18
 ror x8,x23,#28
 add x22,x22,x17
 eor x17,x23,x23,ror#5
 add x22,x22,x16
 and x19,x19,x28
 add x26,x26,x22
 eor x19,x19,x24
 eor x17,x8,x17,ror#34
 add x22,x22,x19
 ldr x19,[x30],#8


 rev x1,x1

 ldr x6,[sp,#24]
 add x22,x22,x17
 str x9,[sp,#16]
 ror x16,x26,#14
 add x21,x21,x19
 eor x9,x26,x26,ror#23
 and x17,x27,x26
 bic x19,x20,x26
 add x21,x21,x1
 orr x17,x17,x19
 eor x19,x22,x23
 eor x16,x16,x9,ror#18
 ror x9,x22,#28
 add x21,x21,x17
 eor x17,x22,x22,ror#5
 add x21,x21,x16
 and x28,x28,x19
 add x25,x25,x21
 eor x28,x28,x23
 eor x17,x9,x17,ror#34
 add x21,x21,x28
 ldr x28,[x30],#8


 rev x2,x2

 ldr x7,[sp,#0]
 add x21,x21,x17
 str x10,[sp,#24]
 ror x16,x25,#14
 add x20,x20,x28
 ror x9,x4,#1
 and x17,x26,x25
 ror x8,x1,#19
 bic x28,x27,x25
 ror x10,x21,#28
 add x20,x20,x2
 eor x16,x16,x25,ror#18
 eor x9,x9,x4,ror#8
 orr x17,x17,x28
 eor x28,x21,x22
 eor x16,x16,x25,ror#41
 eor x10,x10,x21,ror#34
 add x20,x20,x17
 and x19,x19,x28
 eor x8,x8,x1,ror#61
 eor x9,x9,x4,lsr#7
 add x20,x20,x16
 eor x19,x19,x22
 eor x17,x10,x21,ror#39
 eor x8,x8,x1,lsr#6
 add x3,x3,x12
 add x24,x24,x20
 add x20,x20,x19
 ldr x19,[x30],#8
 add x3,x3,x9
 add x20,x20,x17
 add x3,x3,x8
.Loop_16_xx:
 ldr x8,[sp,#8]
 str x11,[sp,#0]
 ror x16,x24,#14
 add x27,x27,x19
 ror x10,x5,#1
 and x17,x25,x24
 ror x9,x2,#19
 bic x19,x26,x24
 ror x11,x20,#28
 add x27,x27,x3
 eor x16,x16,x24,ror#18
 eor x10,x10,x5,ror#8
 orr x17,x17,x19
 eor x19,x20,x21
 eor x16,x16,x24,ror#41
 eor x11,x11,x20,ror#34
 add x27,x27,x17
 and x28,x28,x19
 eor x9,x9,x2,ror#61
 eor x10,x10,x5,lsr#7
 add x27,x27,x16
 eor x28,x28,x21
 eor x17,x11,x20,ror#39
 eor x9,x9,x2,lsr#6
 add x4,x4,x13
 add x23,x23,x27
 add x27,x27,x28
 ldr x28,[x30],#8
 add x4,x4,x10
 add x27,x27,x17
 add x4,x4,x9
 ldr x9,[sp,#16]
 str x12,[sp,#8]
 ror x16,x23,#14
 add x26,x26,x28
 ror x11,x6,#1
 and x17,x24,x23
 ror x10,x3,#19
 bic x28,x25,x23
 ror x12,x27,#28
 add x26,x26,x4
 eor x16,x16,x23,ror#18
 eor x11,x11,x6,ror#8
 orr x17,x17,x28
 eor x28,x27,x20
 eor x16,x16,x23,ror#41
 eor x12,x12,x27,ror#34
 add x26,x26,x17
 and x19,x19,x28
 eor x10,x10,x3,ror#61
 eor x11,x11,x6,lsr#7
 add x26,x26,x16
 eor x19,x19,x20
 eor x17,x12,x27,ror#39
 eor x10,x10,x3,lsr#6
 add x5,x5,x14
 add x22,x22,x26
 add x26,x26,x19
 ldr x19,[x30],#8
 add x5,x5,x11
 add x26,x26,x17
 add x5,x5,x10
 ldr x10,[sp,#24]
 str x13,[sp,#16]
 ror x16,x22,#14
 add x25,x25,x19
 ror x12,x7,#1
 and x17,x23,x22
 ror x11,x4,#19
 bic x19,x24,x22
 ror x13,x26,#28
 add x25,x25,x5
 eor x16,x16,x22,ror#18
 eor x12,x12,x7,ror#8
 orr x17,x17,x19
 eor x19,x26,x27
 eor x16,x16,x22,ror#41
 eor x13,x13,x26,ror#34
 add x25,x25,x17
 and x28,x28,x19
 eor x11,x11,x4,ror#61
 eor x12,x12,x7,lsr#7
 add x25,x25,x16
 eor x28,x28,x27
 eor x17,x13,x26,ror#39
 eor x11,x11,x4,lsr#6
 add x6,x6,x15
 add x21,x21,x25
 add x25,x25,x28
 ldr x28,[x30],#8
 add x6,x6,x12
 add x25,x25,x17
 add x6,x6,x11
 ldr x11,[sp,#0]
 str x14,[sp,#24]
 ror x16,x21,#14
 add x24,x24,x28
 ror x13,x8,#1
 and x17,x22,x21
 ror x12,x5,#19
 bic x28,x23,x21
 ror x14,x25,#28
 add x24,x24,x6
 eor x16,x16,x21,ror#18
 eor x13,x13,x8,ror#8
 orr x17,x17,x28
 eor x28,x25,x26
 eor x16,x16,x21,ror#41
 eor x14,x14,x25,ror#34
 add x24,x24,x17
 and x19,x19,x28
 eor x12,x12,x5,ror#61
 eor x13,x13,x8,lsr#7
 add x24,x24,x16
 eor x19,x19,x26
 eor x17,x14,x25,ror#39
 eor x12,x12,x5,lsr#6
 add x7,x7,x0
 add x20,x20,x24
 add x24,x24,x19
 ldr x19,[x30],#8
 add x7,x7,x13
 add x24,x24,x17
 add x7,x7,x12
 ldr x12,[sp,#8]
 str x15,[sp,#0]
 ror x16,x20,#14
 add x23,x23,x19
 ror x14,x9,#1
 and x17,x21,x20
 ror x13,x6,#19
 bic x19,x22,x20
 ror x15,x24,#28
 add x23,x23,x7
 eor x16,x16,x20,ror#18
 eor x14,x14,x9,ror#8
 orr x17,x17,x19
 eor x19,x24,x25
 eor x16,x16,x20,ror#41
 eor x15,x15,x24,ror#34
 add x23,x23,x17
 and x28,x28,x19
 eor x13,x13,x6,ror#61
 eor x14,x14,x9,lsr#7
 add x23,x23,x16
 eor x28,x28,x25
 eor x17,x15,x24,ror#39
 eor x13,x13,x6,lsr#6
 add x8,x8,x1
 add x27,x27,x23
 add x23,x23,x28
 ldr x28,[x30],#8
 add x8,x8,x14
 add x23,x23,x17
 add x8,x8,x13
 ldr x13,[sp,#16]
 str x0,[sp,#8]
 ror x16,x27,#14
 add x22,x22,x28
 ror x15,x10,#1
 and x17,x20,x27
 ror x14,x7,#19
 bic x28,x21,x27
 ror x0,x23,#28
 add x22,x22,x8
 eor x16,x16,x27,ror#18
 eor x15,x15,x10,ror#8
 orr x17,x17,x28
 eor x28,x23,x24
 eor x16,x16,x27,ror#41
 eor x0,x0,x23,ror#34
 add x22,x22,x17
 and x19,x19,x28
 eor x14,x14,x7,ror#61
 eor x15,x15,x10,lsr#7
 add x22,x22,x16
 eor x19,x19,x24
 eor x17,x0,x23,ror#39
 eor x14,x14,x7,lsr#6
 add x9,x9,x2
 add x26,x26,x22
 add x22,x22,x19
 ldr x19,[x30],#8
 add x9,x9,x15
 add x22,x22,x17
 add x9,x9,x14
 ldr x14,[sp,#24]
 str x1,[sp,#16]
 ror x16,x26,#14
 add x21,x21,x19
 ror x0,x11,#1
 and x17,x27,x26
 ror x15,x8,#19
 bic x19,x20,x26
 ror x1,x22,#28
 add x21,x21,x9
 eor x16,x16,x26,ror#18
 eor x0,x0,x11,ror#8
 orr x17,x17,x19
 eor x19,x22,x23
 eor x16,x16,x26,ror#41
 eor x1,x1,x22,ror#34
 add x21,x21,x17
 and x28,x28,x19
 eor x15,x15,x8,ror#61
 eor x0,x0,x11,lsr#7
 add x21,x21,x16
 eor x28,x28,x23
 eor x17,x1,x22,ror#39
 eor x15,x15,x8,lsr#6
 add x10,x10,x3
 add x25,x25,x21
 add x21,x21,x28
 ldr x28,[x30],#8
 add x10,x10,x0
 add x21,x21,x17
 add x10,x10,x15
 ldr x15,[sp,#0]
 str x2,[sp,#24]
 ror x16,x25,#14
 add x20,x20,x28
 ror x1,x12,#1
 and x17,x26,x25
 ror x0,x9,#19
 bic x28,x27,x25
 ror x2,x21,#28
 add x20,x20,x10
 eor x16,x16,x25,ror#18
 eor x1,x1,x12,ror#8
 orr x17,x17,x28
 eor x28,x21,x22
 eor x16,x16,x25,ror#41
 eor x2,x2,x21,ror#34
 add x20,x20,x17
 and x19,x19,x28
 eor x0,x0,x9,ror#61
 eor x1,x1,x12,lsr#7
 add x20,x20,x16
 eor x19,x19,x22
 eor x17,x2,x21,ror#39
 eor x0,x0,x9,lsr#6
 add x11,x11,x4
 add x24,x24,x20
 add x20,x20,x19
 ldr x19,[x30],#8
 add x11,x11,x1
 add x20,x20,x17
 add x11,x11,x0
 ldr x0,[sp,#8]
 str x3,[sp,#0]
 ror x16,x24,#14
 add x27,x27,x19
 ror x2,x13,#1
 and x17,x25,x24
 ror x1,x10,#19
 bic x19,x26,x24
 ror x3,x20,#28
 add x27,x27,x11
 eor x16,x16,x24,ror#18
 eor x2,x2,x13,ror#8
 orr x17,x17,x19
 eor x19,x20,x21
 eor x16,x16,x24,ror#41
 eor x3,x3,x20,ror#34
 add x27,x27,x17
 and x28,x28,x19
 eor x1,x1,x10,ror#61
 eor x2,x2,x13,lsr#7
 add x27,x27,x16
 eor x28,x28,x21
 eor x17,x3,x20,ror#39
 eor x1,x1,x10,lsr#6
 add x12,x12,x5
 add x23,x23,x27
 add x27,x27,x28
 ldr x28,[x30],#8
 add x12,x12,x2
 add x27,x27,x17
 add x12,x12,x1
 ldr x1,[sp,#16]
 str x4,[sp,#8]
 ror x16,x23,#14
 add x26,x26,x28
 ror x3,x14,#1
 and x17,x24,x23
 ror x2,x11,#19
 bic x28,x25,x23
 ror x4,x27,#28
 add x26,x26,x12
 eor x16,x16,x23,ror#18
 eor x3,x3,x14,ror#8
 orr x17,x17,x28
 eor x28,x27,x20
 eor x16,x16,x23,ror#41
 eor x4,x4,x27,ror#34
 add x26,x26,x17
 and x19,x19,x28
 eor x2,x2,x11,ror#61
 eor x3,x3,x14,lsr#7
 add x26,x26,x16
 eor x19,x19,x20
 eor x17,x4,x27,ror#39
 eor x2,x2,x11,lsr#6
 add x13,x13,x6
 add x22,x22,x26
 add x26,x26,x19
 ldr x19,[x30],#8
 add x13,x13,x3
 add x26,x26,x17
 add x13,x13,x2
 ldr x2,[sp,#24]
 str x5,[sp,#16]
 ror x16,x22,#14
 add x25,x25,x19
 ror x4,x15,#1
 and x17,x23,x22
 ror x3,x12,#19
 bic x19,x24,x22
 ror x5,x26,#28
 add x25,x25,x13
 eor x16,x16,x22,ror#18
 eor x4,x4,x15,ror#8
 orr x17,x17,x19
 eor x19,x26,x27
 eor x16,x16,x22,ror#41
 eor x5,x5,x26,ror#34
 add x25,x25,x17
 and x28,x28,x19
 eor x3,x3,x12,ror#61
 eor x4,x4,x15,lsr#7
 add x25,x25,x16
 eor x28,x28,x27
 eor x17,x5,x26,ror#39
 eor x3,x3,x12,lsr#6
 add x14,x14,x7
 add x21,x21,x25
 add x25,x25,x28
 ldr x28,[x30],#8
 add x14,x14,x4
 add x25,x25,x17
 add x14,x14,x3
 ldr x3,[sp,#0]
 str x6,[sp,#24]
 ror x16,x21,#14
 add x24,x24,x28
 ror x5,x0,#1
 and x17,x22,x21
 ror x4,x13,#19
 bic x28,x23,x21
 ror x6,x25,#28
 add x24,x24,x14
 eor x16,x16,x21,ror#18
 eor x5,x5,x0,ror#8
 orr x17,x17,x28
 eor x28,x25,x26
 eor x16,x16,x21,ror#41
 eor x6,x6,x25,ror#34
 add x24,x24,x17
 and x19,x19,x28
 eor x4,x4,x13,ror#61
 eor x5,x5,x0,lsr#7
 add x24,x24,x16
 eor x19,x19,x26
 eor x17,x6,x25,ror#39
 eor x4,x4,x13,lsr#6
 add x15,x15,x8
 add x20,x20,x24
 add x24,x24,x19
 ldr x19,[x30],#8
 add x15,x15,x5
 add x24,x24,x17
 add x15,x15,x4
 ldr x4,[sp,#8]
 str x7,[sp,#0]
 ror x16,x20,#14
 add x23,x23,x19
 ror x6,x1,#1
 and x17,x21,x20
 ror x5,x14,#19
 bic x19,x22,x20
 ror x7,x24,#28
 add x23,x23,x15
 eor x16,x16,x20,ror#18
 eor x6,x6,x1,ror#8
 orr x17,x17,x19
 eor x19,x24,x25
 eor x16,x16,x20,ror#41
 eor x7,x7,x24,ror#34
 add x23,x23,x17
 and x28,x28,x19
 eor x5,x5,x14,ror#61
 eor x6,x6,x1,lsr#7
 add x23,x23,x16
 eor x28,x28,x25
 eor x17,x7,x24,ror#39
 eor x5,x5,x14,lsr#6
 add x0,x0,x9
 add x27,x27,x23
 add x23,x23,x28
 ldr x28,[x30],#8
 add x0,x0,x6
 add x23,x23,x17
 add x0,x0,x5
 ldr x5,[sp,#16]
 str x8,[sp,#8]
 ror x16,x27,#14
 add x22,x22,x28
 ror x7,x2,#1
 and x17,x20,x27
 ror x6,x15,#19
 bic x28,x21,x27
 ror x8,x23,#28
 add x22,x22,x0
 eor x16,x16,x27,ror#18
 eor x7,x7,x2,ror#8
 orr x17,x17,x28
 eor x28,x23,x24
 eor x16,x16,x27,ror#41
 eor x8,x8,x23,ror#34
 add x22,x22,x17
 and x19,x19,x28
 eor x6,x6,x15,ror#61
 eor x7,x7,x2,lsr#7
 add x22,x22,x16
 eor x19,x19,x24
 eor x17,x8,x23,ror#39
 eor x6,x6,x15,lsr#6
 add x1,x1,x10
 add x26,x26,x22
 add x22,x22,x19
 ldr x19,[x30],#8
 add x1,x1,x7
 add x22,x22,x17
 add x1,x1,x6
 ldr x6,[sp,#24]
 str x9,[sp,#16]
 ror x16,x26,#14
 add x21,x21,x19
 ror x8,x3,#1
 and x17,x27,x26
 ror x7,x0,#19
 bic x19,x20,x26
 ror x9,x22,#28
 add x21,x21,x1
 eor x16,x16,x26,ror#18
 eor x8,x8,x3,ror#8
 orr x17,x17,x19
 eor x19,x22,x23
 eor x16,x16,x26,ror#41
 eor x9,x9,x22,ror#34
 add x21,x21,x17
 and x28,x28,x19
 eor x7,x7,x0,ror#61
 eor x8,x8,x3,lsr#7
 add x21,x21,x16
 eor x28,x28,x23
 eor x17,x9,x22,ror#39
 eor x7,x7,x0,lsr#6
 add x2,x2,x11
 add x25,x25,x21
 add x21,x21,x28
 ldr x28,[x30],#8
 add x2,x2,x8
 add x21,x21,x17
 add x2,x2,x7
 ldr x7,[sp,#0]
 str x10,[sp,#24]
 ror x16,x25,#14
 add x20,x20,x28
 ror x9,x4,#1
 and x17,x26,x25
 ror x8,x1,#19
 bic x28,x27,x25
 ror x10,x21,#28
 add x20,x20,x2
 eor x16,x16,x25,ror#18
 eor x9,x9,x4,ror#8
 orr x17,x17,x28
 eor x28,x21,x22
 eor x16,x16,x25,ror#41
 eor x10,x10,x21,ror#34
 add x20,x20,x17
 and x19,x19,x28
 eor x8,x8,x1,ror#61
 eor x9,x9,x4,lsr#7
 add x20,x20,x16
 eor x19,x19,x22
 eor x17,x10,x21,ror#39
 eor x8,x8,x1,lsr#6
 add x3,x3,x12
 add x24,x24,x20
 add x20,x20,x19
 ldr x19,[x30],#8
 add x3,x3,x9
 add x20,x20,x17
 add x3,x3,x8
 cbnz x19,.Loop_16_xx

 ldp x0,x2,[x29,#96]
 ldr x1,[x29,#112]
 sub x30,x30,#648

 ldp x3,x4,[x0]
 ldp x5,x6,[x0,#2*8]
 add x1,x1,#14*8
 ldp x7,x8,[x0,#4*8]
 add x20,x20,x3
 ldp x9,x10,[x0,#6*8]
 add x21,x21,x4
 add x22,x22,x5
 add x23,x23,x6
 stp x20,x21,[x0]
 add x24,x24,x7
 add x25,x25,x8
 stp x22,x23,[x0,#2*8]
 add x26,x26,x9
 add x27,x27,x10
 cmp x1,x2
 stp x24,x25,[x0,#4*8]
 stp x26,x27,[x0,#6*8]
 b.ne .Loop

 ldp x19,x20,[x29,#16]
 add sp,sp,#4*8
 ldp x21,x22,[x29,#32]
 ldp x23,x24,[x29,#48]
 ldp x25,x26,[x29,#64]
 ldp x27,x28,[x29,#80]
 ldp x29,x30,[sp],#128
 ret
.size sha512_block_data_order,.-sha512_block_data_order

.align 6
.type .LK512,%object
.LK512:
.quad 0x428a2f98d728ae22,0x7137449123ef65cd
.quad 0xb5c0fbcfec4d3b2f,0xe9b5dba58189dbbc
.quad 0x3956c25bf348b538,0x59f111f1b605d019
.quad 0x923f82a4af194f9b,0xab1c5ed5da6d8118
.quad 0xd807aa98a3030242,0x12835b0145706fbe
.quad 0x243185be4ee4b28c,0x550c7dc3d5ffb4e2
.quad 0x72be5d74f27b896f,0x80deb1fe3b1696b1
.quad 0x9bdc06a725c71235,0xc19bf174cf692694
.quad 0xe49b69c19ef14ad2,0xefbe4786384f25e3
.quad 0x0fc19dc68b8cd5b5,0x240ca1cc77ac9c65
.quad 0x2de92c6f592b0275,0x4a7484aa6ea6e483
.quad 0x5cb0a9dcbd41fbd4,0x76f988da831153b5
.quad 0x983e5152ee66dfab,0xa831c66d2db43210
.quad 0xb00327c898fb213f,0xbf597fc7beef0ee4
.quad 0xc6e00bf33da88fc2,0xd5a79147930aa725
.quad 0x06ca6351e003826f,0x142929670a0e6e70
.quad 0x27b70a8546d22ffc,0x2e1b21385c26c926
.quad 0x4d2c6dfc5ac42aed,0x53380d139d95b3df
.quad 0x650a73548baf63de,0x766a0abb3c77b2a8
.quad 0x81c2c92e47edaee6,0x92722c851482353b
.quad 0xa2bfe8a14cf10364,0xa81a664bbc423001
.quad 0xc24b8b70d0f89791,0xc76c51a30654be30
.quad 0xd192e819d6ef5218,0xd69906245565a910
.quad 0xf40e35855771202a,0x106aa07032bbd1b8
.quad 0x19a4c116b8d2d0c8,0x1e376c085141ab53
.quad 0x2748774cdf8eeb99,0x34b0bcb5e19b48a8
.quad 0x391c0cb3c5c95a63,0x4ed8aa4ae3418acb
.quad 0x5b9cca4f7763e373,0x682e6ff3d6b2b8a3
.quad 0x748f82ee5defb2fc,0x78a5636f43172f60
.quad 0x84c87814a1f0ab72,0x8cc702081a6439ec
.quad 0x90befffa23631e28,0xa4506cebde82bde9
.quad 0xbef9a3f7b2c67915,0xc67178f2e372532b
.quad 0xca273eceea26619c,0xd186b8c721c0c207
.quad 0xeada7dd6cde0eb1e,0xf57d4f7fee6ed178
.quad 0x06f067aa72176fba,0x0a637dc5a2c898a6
.quad 0x113f9804bef90dae,0x1b710b35131c471b
.quad 0x28db77f523047d84,0x32caab7b40c72493
.quad 0x3c9ebe0a15c9bebc,0x431d67c49c100d4c
.quad 0x4cc5d4becb3e42b6,0x597f299cfc657e2a
.quad 0x5fcb6fab3ad6faec,0x6c44198c4a475817
.quad 0
.size .LK512,.-.LK512
.align 3
.L_gnutls_arm_cpuid_s:



.quad _gnutls_arm_cpuid_s-.

.byte 83,72,65,53,49,50,32,98,108,111,99,107,32,116,114,97,110,115,102,111,114,109,32,102,111,114,32,65,82,77,118,56,44,32,67,82,89,80,84,79,71,65,77,83,32,98,121,32,60,97,112,112,114,111,64,111,112,101,110,115,115,108,46,111,114,103,62,0
.align 2
.align 2
.comm _gnutls_arm_cpuid_s,4,4

.section .note.GNU-stack,"",%progbits
