format ELF64
use64

; These are functions GCC doesn't vectorize too good.

public sas_matrix_dot_vector
public sas_matrix_dot_vector_3x3
public glTranslatef
public glTranslated
public glScalef
public glScaled

extrn sas_update_mvp

extrn sas_current_matrix


section '.rodata' align 16

xmm_f000:    dd 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF ; filter for the last single
xmm_0fff:    dd 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 ; filter for the first three singles


section '.text' executable align 16


sas_matrix_dot_vector:
movaps  xmm0,[rsi]      ; vector

pshufd  xmm1,xmm0,0x00  ; vector[0]
mulps   xmm1,[rdi +  0] ; matrix[0]
pshufd  xmm2,xmm0,0x55  ; vector[1]
mulps   xmm2,[rdi + 16] ; matrix[1]
addps   xmm1,xmm2
pshufd  xmm2,xmm0,0xAA  ; vector[2]
mulps   xmm2,[rdi + 32] ; matrix[2]
addps   xmm1,xmm2
pshufd  xmm2,xmm0,0xFF  ; vector[3]
mulps   xmm2,[rdi + 48] ; matrix[3]
addps   xmm1,xmm2

movaps  [rsi],xmm1
ret


align 16

sas_matrix_dot_vector_3x3:
movaps  xmm0,[rsi]      ; vector

pshufd  xmm1,xmm0,0x00  ; vector[0]
mulps   xmm1,[rdi +  0] ; matrix[0]
pshufd  xmm2,xmm0,0x55  ; vector[1]
movups  xmm3,[rdi + 12] ; matrix[1]
mulps   xmm2,xmm3
addps   xmm1,xmm2
pshufd  xmm2,xmm0,0xAA  ; vector[2]
movups  xmm3,[rdi + 24] ; matrix[2]
mulps   xmm2,xmm3
addps   xmm1,xmm2

; Just save the first three singles
andps   xmm0,[xmm_f000]
andps   xmm1,[xmm_0fff]
orps    xmm0,xmm1

movaps  [rsi],xmm0
ret


align 16

glTranslatef:
pshufd  xmm0,xmm0,0x00  ; x
pshufd  xmm1,xmm1,0x00  ; y
pshufd  xmm2,xmm2,0x00  ; z

mov     rax,[sas_current_matrix]

mulps   xmm0,[rax +  0]
mulps   xmm1,[rax + 16]
mulps   xmm2,[rax + 32]

addps   xmm0,xmm1
addps   xmm0,xmm2
addps   xmm0,[rax + 48]
movaps  [rax + 48],xmm0

jmp     sas_update_mvp


align 16

glTranslated:
cvtsd2ss xmm0,xmm0
cvtsd2ss xmm1,xmm1
cvtsd2ss xmm2,xmm2
jmp     glTranslatef


align 16

glScalef:
pshufd  xmm0,xmm0,0x00  ; x
pshufd  xmm1,xmm1,0x00  ; y
pshufd  xmm2,xmm2,0x00  ; z

mov     rax,[sas_current_matrix]

mulps   xmm0,[rax +  0]
mulps   xmm1,[rax + 16]
mulps   xmm2,[rax + 32]

movaps  [rax +  0],xmm0
movaps  [rax + 16],xmm1
movaps  [rax + 32],xmm2

jmp     sas_update_mvp


align 16

glScaled:
cvtsd2ss xmm0,xmm0
cvtsd2ss xmm1,xmm1
cvtsd2ss xmm2,xmm2
jmp     glScalef
