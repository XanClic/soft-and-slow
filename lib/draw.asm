format ELF64
use64

public sas_assembly_triangle_helper

extrn sas_get_and_check_index
extrn sas_transform_fragment

extrn sas_current_position
extrn sas_current_color
extrn sas_current_texcoord


section '.rodata' align 16

xmm_one:     dd 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000 ; 1.f
xmm_oh_five: dd 0x3F000000, 0x3F000000, 0x3F000000, 0x3F000000 ; .5f
xmm_f000:    dd 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF ; filter for the last single
xmm_0fff:    dd 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 ; filter for the first three singles

section '.bss' writeable align 16

xmm_tmp: dd ?, ?, ?, ?

section '.text' executable align 16

; XMM0 : d1
; XMM1 : d2
; XMM2 : d3
; XMM3 : unit1
; XMM4 : unit2
; XMM8 : v1
; XMM9 : vec1
; XMM10: vec2
; RDI  : c1
; RSI  : c2
; RDX  : c3
; RCX  : t1
; R8   : t2
; R9   : t3
sas_assembly_triangle_helper:
; so we can call sas_transform_fragment (also, make place for local variables)
sub     rsp,0xE8
mov     [rsp+0xE0],rbx

; some flags
; Bit 0: s_set_to_one
; Bit 1: t_set_to_one
xor     ebx,ebx

; save parameter arrays on stack, because the registers will be trashed later
; on
movaps  xmm5,[rdi]
movaps  xmm6,[rsi]
movaps  xmm7,[rdx]
movaps  xmm11,[rcx]
movaps  xmm12,[r8]
movaps  xmm13,[r9]

movaps  [rsp+0x80],xmm5
movaps  [rsp+0x90],xmm6
movaps  [rsp+0xA0],xmm7
movaps  [rsp+0xB0],xmm11
movaps  [rsp+0xC0],xmm12
movaps  [rsp+0xD0],xmm13

; Pack all depth values into one XMM register so there are two more available
shufps  xmm0,xmm1,11001100b  ; [  ? d2  ? d1 ]
pshufd  xmm11,xmm0,11111000b ; [  ?  ? d2 d1 ]
shufps  xmm11,xmm2,11000100b ; [  ? d3 d2 d1 ]

; Do the same for the units
shufps  xmm3,xmm4,11001100b ; [  ? u2  ? u1 ]
movaps  xmm12,xmm3

xorps   xmm13,xmm13     ; s = 0.f
movaps  xmm15,[xmm_one] ; 1.f

; TODO: Adjust if necessary (those are 6 bytes -- aligns the inner loop)
nop     word [rax+rax+1]


sas_assembly_triangle_s_loop:
xorps   xmm14,xmm14     ; t = 0.f

and     bl,not 0x02


sas_assembly_triangle_t_loop:
movaps  xmm0,xmm13      ; s
mulps   xmm0,xmm9       ; s * vec1
movaps  xmm1,xmm14      ; t
mulps   xmm1,xmm10      ; t * vec2
addps   xmm0,xmm1       ; s * vec1 + t * vec2
addps   xmm0,xmm8       ; v1 + s * vec1 + t * vec2

movaps  xmm1,xmm0
andps   xmm1,[xmm_f000] ; just save the w coordinate

; Bring xyz to [0; 1]
addps   xmm0,xmm15          ; v1 + s * veec1 + t * vec2 + 1.f
mulps   xmm0,[xmm_oh_five]  ; (v1 + s * vec1 + t * vec2 + 1.f) / 2.f

andps   xmm0,[xmm_0fff] ; discard the w coordinate

xorps   xmm2,xmm2       ; 0.f
cmpleps xmm2,xmm0       ; compare for negative (discard those); bits will be set
                        ; if positive (0 is less/equal than those)
movaps  [xmm_tmp],xmm2
mov     rax,[xmm_tmp]
not     rax
test    rax,rax
;jnz     sas_assembly_triangle_discard_fragment
mov     eax,[xmm_tmp+8]
test    eax,eax
;jz      sas_assembly_triangle_discard_fragment

movaps  xmm2,xmm15      ; 1.f
cmpleps xmm2,xmm0       ; compare for less than one (keep those); bits will be
                        ; cleared if less (1 is not less/equal than those)
movaps  [xmm_tmp],xmm2
mov     rax,[xmm_tmp]
test    rax,rax
;jnz     sas_assembly_triangle_discard_fragment
mov     eax,[xmm_tmp+8]
test    eax,eax
;jnz     sas_assembly_triangle_discard_fragment

orps    xmm0,xmm1

movaps  [sas_current_position],xmm0


call    sas_get_and_check_index
test    eax,eax
;jz      sas_assembly_triangle_discard_fragment


movaps  xmm0,xmm15      ; 1.f
subps   xmm0,xmm14      ; 1.f - t
subps   xmm0,xmm13      ; 1.f - s - t = w1

pshufd  xmm3,xmm11,0x00 ; d1
mulps   xmm0,xmm3       ; w1 *= d1

movaps  xmm1,xmm13      ; s = w2
pshufd  xmm3,xmm11,0x55 ; d2
mulps   xmm1,xmm3       ; w2 *= d2

movaps  xmm2,xmm14      ; t = w3
pshufd  xmm3,xmm11,0xAA ; d3
mulps   xmm2,xmm3       ; w3 *= d3


movaps  xmm3,xmm0       ; w1
addps   xmm3,xmm1       ; w1 + w2
addps   xmm3,xmm2       ; w1 + w2 + w3
; TODO: Can someone please check whether this rcp/mul-mul is faster than the
; plain simple div-div?
rcpps   xmm3,xmm3       ; 1.f / (w1 + w2 + w3) = dd


; 0:w1, 1:w2, 2:w3, 3:dd, 4-7:-, 8:v1, 9:vec1, 10:vec2, 11:dx, 12:ux, 13:s, 14:t, 15:1.f

movaps  xmm4,[rsp+0x80] ; c1
mulps   xmm4,xmm0       ; c1 * w1
mulps   xmm0,[rsp+0xB0] ; t1 * w1

movaps  xmm5,[rsp+0x90] ; c2
mulps   xmm5,xmm1       ; c2 * w2
mulps   xmm1,[rsp+0xC0] ; t2 * w2

movaps  xmm6,[rsp+0xA0] ; c3
mulps   xmm6,xmm2       ; c3 * w3
mulps   xmm2,[rsp+0xD0] ; t3 * w3

addps   xmm4,xmm5       ; c1 * w1 + c2 * w2
addps   xmm0,xmm1       ; t1 * w1 + t2 * w2
addps   xmm4,xmm6       ; c1 * w1 + c2 * w2 + c3 * w3
addps   xmm0,xmm2       ; t1 * w1 + t2 * w2 + t3 * w3

; (See TODO above)
mulps   xmm4,xmm3       ; (c1 * w1 + c2 * w2 + c3 * w3) / (w1 + w2 + w3)
mulps   xmm0,xmm3       ; (t1 * w1 + t2 * w2 + t3 * w3) / (w1 + w2 + w3)


movaps  [sas_current_color],xmm4
movaps  [sas_current_texcoord],xmm0 ; just taking 0 as the unit to use (TODO)


; 0-7:-, 8:v1, 9:vec1, 10:vec2, 11:dx, 12:ux, 13:s, 14:t, 15:1.f


movaps  [rsp+0x00],xmm8
movaps  [rsp+0x10],xmm9
movaps  [rsp+0x20],xmm10
movaps  [rsp+0x30],xmm11
movaps  [rsp+0x40],xmm12
movaps  [rsp+0x50],xmm13
movaps  [rsp+0x60],xmm14
movaps  [rsp+0x70],xmm15

call    sas_transform_fragment

movaps  xmm8,[rsp+0x00]
movaps  xmm9,[rsp+0x10]
movaps  xmm10,[rsp+0x20]
movaps  xmm11,[rsp+0x30]
movaps  xmm12,[rsp+0x40]
movaps  xmm13,[rsp+0x50]
movaps  xmm14,[rsp+0x60]
movaps  xmm15,[rsp+0x70]


sas_assembly_triangle_discard_fragment:

pshufd  xmm0,xmm12,0xAA ; unit2
addps   xmm14,xmm0      ; t += unit2

movss   xmm0,xmm14
addss   xmm0,xmm13      ; t + s

cmpless xmm0,xmm15      ; t + s <= 1.f
movd    eax,xmm0
test    eax,eax         ; 0 -> (t + s > 1.f)

jnz     sas_assembly_triangle_t_loop

test    bl,0x2
jnz     sas_assembly_triangle_next_s

movaps  xmm14,xmm15     ; t = 1.f
subps   xmm14,xmm13     ; t = 1.f - s

or      ebx,0x2
jmp     sas_assembly_triangle_t_loop

sas_assembly_triangle_next_s:


pshufd  xmm0,xmm12,0x00 ; unit1
addps   xmm13,xmm0      ; s += unit1

movss   xmm0,xmm13
cmpless xmm0,xmm15      ; s <= 1.f
movd    eax,xmm0
test    eax,eax         ; 0 -> (s > 1.f)

jnz     sas_assembly_triangle_s_loop

test    bl,0x1
jnz     sas_assembly_triangle_return

movaps  xmm13,xmm15     ; s = 1.f

or      ebx,0x1
jmp     sas_assembly_triangle_s_loop

sas_assembly_triangle_return:
mov     rbx,[rsp+0xE0]
add     rsp,0xE8
ret
