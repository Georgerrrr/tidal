  .386
  .model flat 

  ALIGNMENT             equ       2

  .code 

  locals 

  align ALIGNMENT

public SetVideoMode_
SetVideoMode_ proc near 
  push bp
  mov  bp, sp 

  xor  ah, ah 
  int  10h

  pop  bp
  ret 
SetVideoMode_ endp

; void FillBuffer(void* buffer, int length);
; inputs:
;   eax -> buffer 
;   edx -> length

public FillBuffer_
FillBuffer_ proc near 
  push ebp 
  mov  ebp, esp 

  push edi

  mov  edi, eax
  mov  ecx, edx
  shr  ecx, 1
  
  xor  eax, eax ; eax = 0

  rep  stosw

  pop  edi

  pop  ebp
  ret 
FillBuffer_ endp

; void CopyBuffer(void* to, void* from, int length);
; inputs:
;   eax -> to 
;   edx -> from 
;   ebx -> length

public CopyBuffer_
CopyBuffer_ proc near 
  push ebp 
  mov  ebp, esp

  push esi
  push edi

  mov  ecx, ebx
  shr  ecx, 2   ; ecx = length / 4

  mov  esi, edx 
  mov  edi, eax

  rep  movsd

  pop  edi 
  pop  esi

  pop  ebp 
  ret
CopyBuffer_ endp

; void DrawDepth(unsigned char* to, unsigned short* from, int length);
; inputs:
;   eax -> to 
;   edx -> from 
;   ebx -> length

public DrawDepth_
DrawDepth_ proc near 
  push ebp 
  mov  ebp, esp

  push esi 
  push edi 

  mov  esi, edx 
  mov  edi, eax

  mov  ecx, ebx

  inc  esi

DRAW_DEPTH_LOOP:
  movsb
  inc  esi 
  dec  ecx 
  jnz  DRAW_DEPTH_LOOP

  pop  edi 
  pop  esi

  pop  ebp
  ret
DrawDepth_ endp

; void DrawScanline(int y, _edge* leftEdge, _edge* rightEdge);
; inputs:
;   eax -> y 
;   edx -> longEdge 
;   ebx -> shortEdge
public DrawScanline_
DrawScanline_ proc near 

DrawScanline_ endp

  end
