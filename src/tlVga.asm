  .386
  .model flat 

  ALIGNMENT             equ       2
  SCREEN_WIDTH          equ       320 
  SCREEN_HEIGHT         equ       200

; DrawScanline local variables
  dsDrawBuffer          equ       -4
  dsDepthBuffer         equ       -8
  dsWidth               equ       -12
  dsZI                  equ       -16 
  dsZIStep              equ       -20
  dsSourceX             equ       -24
  dsSourceXStep         equ       -28
  dsSourceY             equ       -32 
  dsSourceYStep         equ       -36
  dsLOCAL_WIDTH         equ        36

  .const
  HALF      dd 3F000000h
  SHORT_MAX dd 477fff00h

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

; mirror of struct _edge in tlGfx.c
  EdgeStruct struc 
    esDestX     dd ?
    esWidth     dd ?
    esHeight    dd ?
    esStep      dd ?
    esDirection dd ?
    esErr       dd ?
    esAdjUp     dd ?
    esAdjDown   dd ?
    esUVx       dd ?
    esUVy       dd ?
    esUVStepX   dd ?
    esUVStepY   dd ?
    esZI        dd ?
    esZIStep    dd ?
  EdgeStruct ends

; void DrawScanline(unsigned char* drawBuffer, unsigned short* depthBuffer,
;                   _edge* leftEdge, _edge* rightEdge, int y);
; inputs:
;   eax    -> drawBuffer 
;   edx    -> depthBuffer 
;   ebx    -> leftEdge 
;   ecx    -> rightEdge
;   ebp+8  -> y
;   ebp+12 -> texPtr
;   ebp+16 -> texWidth
;   ebp+20 -> texHeight
;
; there is a slower c version of this sub in tlGfx.c
;
public DrawScanline_
DrawScanline_ proc near 
  push ebp 
  mov  ebp, esp

  sub  esp, dsLOCAL_WIDTH

  push esi 
  push edi 

  ; don't need the draw/depth buffers for now
  mov  dword ptr [ebp + dsDrawBuffer], eax
  mov  dword ptr [ebp + dsDepthBuffer], edx

  ; if rightEdge.destX <= 0 don't bother drawing scanline 
  mov  edi, ecx ; set edi to right edge pointer
  mov  esi, [edi].esDestX
  cmp  esi, 0 
  jle  DRAW_SCANLINE_COMPLETE

  ; if leftEdge.destX >= SCREEN_WIDTH don't bother drawing scanline
  ; ebx already set to left edge pointer
  mov  edx, [ebx].esDestX
  cmp  edx, SCREEN_WIDTH
  jge  DRAW_SCANLINE_COMPLETE

  ; if leftEdge.destX >= rightEdge.destX don't bother drawing scanline
  sub  esi, edx ; rightEdge.destX - leftEdge.destX
  jle  DRAW_SCANLINE_COMPLETE 
  
  mov  dword ptr [ebp + dsWidth], esi

  ; set up the step values (zi & uv)

  ; ZI
  fld   dword ptr [edi].esZI ; load right edge zi to fpu
  fsub  dword ptr [ebx].esZI ; subtract left edge zi
  fidiv dword ptr [ebp + dsWidth] ; divide by width
  fst   dword ptr [ebp + dsZIStep] ; this gets the step

  fmul  dword ptr [HALF] ; divide the step by two then 
  fadd  dword ptr [ebx].esZI ; add zi

  fstp  dword ptr [ebp + dsZI] ; add a half to zi 

  ; UV 

  ; x
  fld   dword ptr [edi].esUVx
  fsub  dword ptr [ebx].esUVx 
  fidiv dword ptr [ebp + dsWidth]
  fst   dword ptr [ebp + dsSourceXStep]

  fmul  dword ptr [HALF]
  fadd  dword ptr [ebx].esUVx 

  fstp  dword ptr [ebp + dsSourceX]

  ; y 
  fld   dword ptr [edi].esUVy 
  fsub  dword ptr [ebx].esUVy 
  fidiv dword ptr [ebp + dsWidth]
  fst   dword ptr [ebp + dsSourceYStep]

  fmul  dword ptr [HALF]
  fadd  dword ptr [ebx].esUVy

  fstp  dword ptr [ebp + dsSourceY]

  ; clip left edge
  mov  esi, [ebx].esDestX 
  cmp  esi, 0
  jge  DRAW_SCANLINE_LEFTEDGE_CLIPPED ; if leftEdge.destX >= 0 dont clip

  fild dword ptr [ebx].esDestX ; load startX to fpu
  fchs  ; flip the sign (-startX)

  fst st(1)
  fmul dword ptr [ebp + dsZIStep]
  fadd dword ptr [ebp + dsZI]
  fstp dword ptr [ebp + dsZI] ; (zi += (ziStep * (-startX)))

  fst st(1)
  fmul dword ptr [ebp + dsSourceXStep]
  fadd dword ptr [ebp + dsSourceX]
  fstp dword ptr [ebp + dsSourceX]

  fmul dword ptr [ebp + dsSourceYStep]
  fadd dword ptr [ebp + dsSourceY]
  fstp dword ptr [ebp + dsSourceY]

  xor  esi, esi
DRAW_SCANLINE_LEFTEDGE_CLIPPED:

  ; clip right edge
  mov  ecx, [edi].esDestX
  cmp  ecx, SCREEN_WIDTH
  jle  DRAW_SCANLINE_RIGHTEDGE_CLIPPED ; if rightEdge.destX <= SCREEN_WIDTH dont clip
  
  mov  ecx, SCREEN_WIDTH
DRAW_SCANLINE_RIGHTEDGE_CLIPPED:

  sub  ecx, esi

  mov  ax, SCREEN_WIDTH
  mul  word ptr [ebp + 8]
  add  si, ax

  mov  edi, esi

  add  esi, dword ptr [ebp + dsDrawBuffer]

  shl  edi, 1 
  add  edi, dword ptr [ebp + dsDepthBuffer]

DRAW_SCANLINE_INNER_LOOP:
  fld  dword ptr [ebp + dsZI]
  fmul dword ptr [SHORT_MAX]
  fistp word ptr [ebp + dsWidth]
  mov ax, word ptr [ebp + dsWidth]
  cmp ax, word ptr [edi]
  jle DRAW_SCANLINE_DEPTH_FAILED

  mov  word ptr [edi], ax

  fld  dword ptr [ebp + dsSourceY]
  fimul dword ptr [ebp + 20]
  fsub  dword ptr [ebp + dsSourceY]
  fadd  dword ptr [HALF]

  fistp dword ptr [ebp + dsWidth]
  mov eax, dword ptr [ebp + dsWidth]

  mul dword ptr [ebp + 16]

  fld  dword ptr [ebp + dsSourceX]
  fimul dword ptr [ebp + 16]
  fsub dword ptr [ebp + dsSourceX]
  fadd dword ptr [HALF]
  fistp dword ptr [ebp + dsWidth]
  add eax, dword ptr [ebp + dsWidth]

  mov ebx, dword ptr [ebp + 12]
  add ebx, eax

  mov  al, byte ptr [ebx]
  mov  byte ptr [esi], al

DRAW_SCANLINE_DEPTH_FAILED:

  fld  dword ptr [ebp + dsZI]
  fadd dword ptr [ebp + dsZIStep]
  fstp dword ptr [ebp + dsZI]

  fld  dword ptr [ebp + dsSourceX]
  fadd dword ptr [ebp + dsSourceXStep]
  fstp dword ptr [ebp + dsSourceX]

  fld  dword ptr [ebp + dsSourceY]
  fadd dword ptr [ebp + dsSourceYStep]
  fstp dword ptr [ebp + dsSourceY]

  inc  esi
  add  edi, 2
  dec  ecx 
  jnz  DRAW_SCANLINE_INNER_LOOP

DRAW_SCANLINE_COMPLETE:

  pop  edi 
  pop  esi

  mov  esp, ebp

  pop  ebp 
  ret
DrawScanline_ endp

  end
