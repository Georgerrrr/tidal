  .386 
  .model flat 

  ALIGNMENT               equ 2 

  .code 

  locals 

  align ALIGNMENT

; fixed_t FixedMul(fixed_t a, fixed_t b);
; inputs:
;   eax -> a 
;   edx -> b 
; return eax
public FixedMul_
FixedMul_ proc near 
  push ebp 
  mov  ebp, esp 

  ; multiply as integers then shift down 16
  ; put result (edx:eax) into eax

  imul edx ; edx:eax = eax * edx
  add  eax, 8000h ; add 0.5 for rounding
  adc  edx, 0
  shr  eax, 16    ; truncate fraction

  shl  edx, 16    ; top half of edx = integer part of output
  add  eax, edx 

  pop  ebp 
  ret
FixedMul_ endp

; fixed_t FixedDiv(fixed_t a, fixed_t b);
; inputs:
;   eax -> a 
;   edx -> b 
; return eax 
public FixedDiv_
FixedDiv_ proc near 
  push ebp 
  mov  ebp, esp 

  ; shift a up by 16 (putting carry in edx)
  ; then divide as normal

  mov  ebx, edx

  ; use cx as a flag to see if we had to make A positive
  ; so we can flip it back later
  xor  cx,  cx  ; cx = 0 
  and  eax, eax
  jns  FD_NO_SIGN_FLIP_A
  inc  cx 
  neg  eax

FD_NO_SIGN_FLIP_A:

  rol  eax, 16  ; put top half in bottom half, and vice versa

  xor  edx, edx
  mov  dx,  ax
  xor  ax,  ax

  ; if A and B are negative, then we can reset the flag 
  ; as the output will be positive anyway 
  and  ebx, ebx 
  jns  FD_NO_SIGN_FLIP_B
  dec  cx 
  neg  ebx 

FD_NO_SIGN_FLIP_B:

  div  ebx      ; output to eax

  ; round up in case where divisor is even
  shr  ebx, 1
  adc  ebx, 0
  dec  ebx
  cmp  ebx, edx
  adc  eax, 0

  and  cx, cx
  jz   FD_NO_SIGN_FLIP_OUT
  neg  eax

FD_NO_SIGN_FLIP_OUT:

  pop  ebp 
  ret 
FixedDiv_ endp

; void VectorTransform(xform_t matrix, fvec4_t* iVec, fvec4_t* oVec);
; inputs:
;   eax -> matrix ptr 
;   edx -> input vector ptr 
;   ebx -> output vector ptr 
public VectorTransform_
VectorTransform_ proc near 
  push ebp 
  mov  ebp, esp 

  push esi 
  push edi 

  mov  edi, ebx ; edi = destination ptr
  mov  esi, edx ; esi = source ptr
  mov  ebx, eax ; ebx = matrix ptr

XFORM_MACRO MACRO
  mov  eax, dword ptr [ebx] ; eax = *(matrix ptr)
  mov  edx, dword ptr [esi] ; edx = *(source ptr)
  call FixedMul_
  mov  dword ptr [edi], eax
  add  ebx, 4     ; matrix ptr ++ 

  mov  eax, dword ptr [ebx] 
  mov  edx, dword ptr [esi+4]
  call FixedMul_
  add  dword ptr [edi], eax
  add  ebx, 4     ; matrix ptr ++ 

  mov  eax, dword ptr [ebx] 
  mov  edx, dword ptr [esi+8]
  call FixedMul_
  add  dword ptr [edi], eax
  add  ebx, 4     ; matrix ptr ++ 

  mov  eax, [ebx] 
  add  dword ptr [edi], eax
  ENDM

  XFORM_MACRO
  add  ebx, 4     
  add  edi, 4

  XFORM_MACRO
  add  ebx, 4     
  add  edi, 4

  XFORM_MACRO
  add  ebx, 4     
  add  edi, 4

  XFORM_MACRO

  pop  edi 
  pop  esi 

  pop  ebp 
  ret 
VectorTransform_ endp

end
