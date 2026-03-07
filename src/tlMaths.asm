  .386 
  .model flat 

  ALIGNMENT               equ 2 

  .code 

  locals 

  align ALIGNMENT

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

  mov  esi, eax ; esi = matrix ptr
  mov  edi, edx ; edi = source ptr
  ; ebx = output ptr 

  ; X
  fld  dword ptr [edi + 0]
  fmul dword ptr [esi + 0]
  fld  dword ptr [edi + 0]
  fmul dword ptr [esi + 16]
  fld  dword ptr [edi + 0]
  fmul dword ptr [esi + 32]

  ; Y
  fld  dword ptr [edi + 4]
  fmul dword ptr [esi + 4]
  fld  dword ptr [edi + 4]
  fmul dword ptr [esi + 16 + 4]
  fld  dword ptr [edi + 4]
  fmul dword ptr [esi + 32 + 4]
  fxch st(2)
  faddp st(5), st(0)
  faddp st(3), st(0)
  faddp st(1), st(0) 

  ; Z
  fld  dword ptr [edi + 8]
  fmul dword ptr [esi + 8]
  fld  dword ptr [edi + 8]
  fmul dword ptr [esi + 16 + 8]
  fld  dword ptr [edi + 8]
  fmul dword ptr [esi + 32 + 8]
  fxch st(2)
  faddp st(5), st(0)
  faddp st(3), st(0)
  faddp st(1), st(0) 

  fxch st(2)
  fadd dword ptr [esi + 12]
  fxch st(1)
  fadd dword ptr [esi + 16 + 12]
  fxch st(2)
  fadd dword ptr [esi + 32 + 12]
  fxch st(1)

  fstp dword ptr [ebx]
  fstp dword ptr [ebx + 8]
  fstp dword ptr [ebx + 4]

  pop  edi 
  pop  esi 

  pop  ebp 
  ret 
VectorTransform_ endp

; void ConcatTransforms(xform_t t1, xform_t t2, xform_t out);
; inputs:
;   eax -> transform ptr 1 
;   edx -> transform ptr 2 
;   ebx -> output ptr 
public ConcatTransforms_
ConcatTransforms_ proc near 
  push ebp 
  mov  ebp, esp 

  push esi 
  push edi 

  mov  esi, eax 
  mov  edi, edx

  mov  ecx, 15

  ; esi = transform ptr 1
  ; edi = transform ptr 2
  ; ebx = output ptr 

CONCAT_TRANSFORM_LOOP:
  mov  eax, ecx ; ax = row...
  shr  eax, 2  ; ...cx / 4
  shl  eax, 4  ; mul 16 to get the address

  mov  edx, ecx ; dx = col...
  and  edx, 3  ; ...cx & 3

  fld  dword ptr [esi + eax]
  fmul dword ptr [edi + edx * 4]
  fld  dword ptr [esi + eax + 4]
  fmul dword ptr [edi + 16 + edx * 4]
  fld  dword ptr [esi + eax + 8]
  fmul dword ptr [edi + 32 + edx * 4]
  fld  dword ptr [esi + eax + 12]
  fmul dword ptr [edi + 48 + edx * 4]
  faddp 
  faddp 
  faddp 
  fstp  dword ptr [ebx + ecx * 4]

  dec  ecx 
  jns  CONCAT_TRANSFORM_LOOP

  pop  edi 
  pop  esi 
  pop  ebp 
  ret 
ConcatTransforms_ endp

end
