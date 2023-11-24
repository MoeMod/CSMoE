// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

; Copyright 2010-2013 RAD Game Toosl

public asm_x64_cmpxchg64
public asm_x64_cmpxchg128

.CODE

;-------------------------------------------------------------------------------------------

;int cdecl
;asm_x64_cmpxchg64
;( void*,	// rcx
;  void*,	// rdx
;  const uint64 ); // r8

align 8
asm_x64_cmpxchg64 PROC 
;  // val* = rcx
;  // old* = rdx
;  // new = r8

  mov rax, [rdx]
  lock cmpxchg [rcx], r8
  sete cl
  mov [rdx], rax
  movzx rax,cl
  ret
asm_x64_cmpxchg64 ENDP

;-------------------------------------------------------------------------------------------

;int __cdecl asm_x64_cmpxchg128( __m128* pVal, __m128* pOld, const __m128 * pNew);

align 8
asm_x64_cmpxchg128 PROC 
;  // val* = rcx
;  // old* = rdx
;  // new = r8

  push rbx
  push rdi
  push rsi

  mov rdi, rcx ; rdi = val
  mov rsi, rdx ; rsi = old

 ; rdx:rax = old
 ; rcx:rbx = new

  mov rdx, [rsi+8]
  mov rax, [rsi]
  mov rcx, [r8 +8]
  mov rbx, [r8 ]

  lock cmpxchg16b [rdi]
  jne asm_x64_cmpxchg128_fail
  mov rax, 1

  pop rsi
  pop rdi
  pop rbx
  ret

align 8
asm_x64_cmpxchg128_fail:
  mov [rsi+8] , rdx
  mov [rsi]   , rax
  mov rax, 0

  pop rsi
  pop rdi
  pop rbx
  ret
align 8
asm_x64_cmpxchg128 ENDP


END




