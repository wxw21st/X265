;*****************************************************************************
;* Copyright (C) 2013 x265 project
;*
;* Authors: Praveen Kumar Tiwari <praveen@multicorewareinc.com>
;*          Murugan Vairavel <murugan@multicorewareinc.com>
;*
;* This program is free software; you can redistribute it and/or modify
;* it under the terms of the GNU General Public License as published by
;* the Free Software Foundation; either version 2 of the License, or
;* (at your option) any later version.
;*
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU General Public License for more details.
;*
;* You should have received a copy of the GNU General Public License
;* along with this program; if not, write to the Free Software
;* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
;*
;* This program is also available under a commercial proprietary license.
;* For more information, contact us at licensing@multicorewareinc.com.
;*****************************************************************************/

%include "x86inc.asm"
%include "x86util.asm"

SECTION_RODATA 32

tab_Vm:    db 0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0

SECTION .text

;-----------------------------------------------------------------------------
; void blockcopy_pp_2x4(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_2x4, 4, 7, 0, dest, deststride, src, srcstride

mov     r4w,     [r2]
mov     r5w,     [r2 + r3]
mov     r6w,     [r2 + 2 * r3]
lea      r3,     [r3 + r3 * 2]
mov      r3w,    [r2 + r3]

mov     [r0],            r4w
mov     [r0 + r1],       r5w
mov     [r0 + 2 * r1],   r6w
lea      r1,              [r1 + 2 * r1]
mov     [r0 + r1],       r3w

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_2x8(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_2x8, 4, 7, 0, dest, deststride, src, srcstride

mov     r4w,     [r2]
mov     r5w,     [r2 + r3]
mov     r6w,     [r2 + 2 * r3]

mov     [r0],            r4w
mov     [r0 + r1],       r5w
mov     [r0 + 2 * r1],   r6w

lea     r0,             [r0 + 2 * r1]
lea     r2,             [r2 + 2 * r3]

mov     r4w,             [r2 + r3]
mov     r5w,             [r2 + 2 * r3]

mov     [r0 + r1],       r4w
mov     [r0 + 2 * r1],   r5w

lea     r0,              [r0 + 2 * r1]
lea     r2,              [r2 + 2 * r3]

mov     r4w,             [r2 + r3]
mov     r5w,             [r2 + 2 * r3]

mov     [r0 + r1],       r4w
mov     [r0 + 2 * r1],   r5w

lea     r0,              [r0 + 2 * r1]
lea     r2,              [r2 + 2 * r3]

mov     r4w,             [r2 + r3]
mov     [r0 + r1],       r4w
RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_4x2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_4x2, 4, 6, 2, dest, deststride, src, srcstride

mov     r4d,     [r2]
mov     r5d,     [r2 + r3]

mov     [r0],            r4d
mov     [r0 + r1],       r5d

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_4x4(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_4x4, 4, 4, 4, dest, deststride, src, srcstride

movd     m0,     [r2]
movd     m1,     [r2 + r3]
movd     m2,     [r2 + 2 * r3]
lea      r3,     [r3 + r3 * 2]
movd     m3,     [r2 + r3]

movd     [r0],            m0
movd     [r0 + r1],       m1
movd     [r0 + 2 * r1],   m2
lea      r1,              [r1 + 2 * r1]
movd     [r0 + r1],       m3

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_4x8(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_4x8, 4, 6, 8, dest, deststride, src, srcstride

movd     m0,     [r2]
movd     m1,     [r2 + r3]
movd     m2,     [r2 + 2 * r3]
lea      r4,     [r2 + 2 * r3]
movd     m3,     [r4 + r3]

movd     m4,     [r4 + 2 * r3]
lea      r4,     [r4 + 2 * r3]
movd     m5,     [r4 + r3]
movd     m6,     [r4 + 2 * r3]
lea      r4,     [r4 + 2 * r3]
movd     m7,     [r4 + r3]

movd     [r0],                m0
movd     [r0 + r1],           m1
movd     [r0 + 2 * r1],       m2
lea      r5,                  [r0 + 2 * r1]
movd     [r5 + r1],           m3

movd     [r5 + 2 * r1],        m4
lea      r5,                   [r5 + 2 * r1]
movd     [r5 + r1],            m5
movd     [r5 + 2 * r1],        m6
lea      r5,                   [r5 + 2 * r1]
movd     [r5 + r1],            m7

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W4_H8 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 7, 8, dest, deststride, src, srcstride


mov         r4d,       %2

.loop
     movd     m0,     [r2]
     movd     m1,     [r2 + r3]
     movd     m2,     [r2 + 2 * r3]
     lea      r5,     [r2 + 2 * r3]
     movd     m3,     [r5 + r3]

     movd     m4,     [r5 + 2 * r3]
     lea      r5,     [r5 + 2 * r3]
     movd     m5,     [r5 + r3]
     movd     m6,     [r5 + 2 * r3]
     lea      r5,     [r5 + 2 * r3]
     movd     m7,     [r5 + r3]

     movd     [r0],                m0
     movd     [r0 + r1],           m1
     movd     [r0 + 2 * r1],       m2
     lea      r6,                  [r0 + 2 * r1]
     movd     [r6 + r1],           m3

     movd     [r6 + 2 * r1],       m4
     lea      r6,                  [r6 + 2 * r1]
     movd     [r6 + r1],           m5
     movd     [r6 + 2 * r1],       m6
     lea      r6,                  [r6 + 2 * r1]
     movd     [r6 + r1],           m7

    lea         r0,           [r0 + 8 * r1]
    lea         r2,           [r2 + 8 * r3]

    sub         r4d,           8
    jnz        .loop

RET
%endmacro

BLOCKCOPY_PP_W4_H8 4, 16

;-----------------------------------------------------------------------------
; void blockcopy_pp_6x8(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_6x8, 4, 7, 8, dest, deststride, src, srcstride

movd     m0,     [r2]
movd     m1,     [r2 + r3]
movd     m2,     [r2 + 2 * r3]
lea      r5,     [r2 + 2 * r3]
movd     m3,     [r5 + r3]

movd     m4,     [r5 + 2 * r3]
lea      r5,     [r5 + 2 * r3]
movd     m5,     [r5 + r3]
movd     m6,     [r5 + 2 * r3]
lea      r5,     [r5 + 2 * r3]
movd     m7,     [r5 + r3]

movd     [r0],                m0
movd     [r0 + r1],           m1
movd     [r0 + 2 * r1],       m2
lea      r6,                  [r0 + 2 * r1]
movd     [r6 + r1],           m3

movd     [r6 + 2 * r1],        m4
lea      r6,                   [r6 + 2 * r1]
movd     [r6 + r1],            m5
movd     [r6 + 2 * r1],        m6
lea      r6,                   [r6 + 2 * r1]
movd     [r6 + r1],            m7

mov     r4w,     [r2 + 4]
mov     r5w,     [r2 + r3 + 4]
mov     r6w,     [r2 + 2 * r3 + 4]

mov     [r0 + 4],            r4w
mov     [r0 + r1 + 4],       r5w
mov     [r0 + 2 * r1 + 4],   r6w

lea     r0,              [r0 + 2 * r1]
lea     r2,              [r2 + 2 * r3]

mov     r4w,             [r2 + r3 + 4]
mov     r5w,             [r2 + 2 * r3 + 4]

mov     [r0 + r1 + 4],       r4w
mov     [r0 + 2 * r1 + 4],   r5w

lea     r0,              [r0 + 2 * r1]
lea     r2,              [r2 + 2 * r3]

mov     r4w,             [r2 + r3 + 4]
mov     r5w,             [r2 + 2 * r3 + 4]

mov     [r0 + r1 + 4],       r4w
mov     [r0 + 2 * r1 + 4],   r5w

lea     r0,              [r0 + 2 * r1]
lea     r2,              [r2 + 2 * r3]

mov     r4w,             [r2 + r3 + 4]
mov     [r0 + r1 + 4],       r4w
RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_8x2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_8x2, 4, 4, 2, dest, deststride, src, srcstride

movh     m0,        [r2]
movh     m1,        [r2 + r3]

movh     [r0],       m0
movh     [r0 + r1],  m1

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_8x4(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_8x4, 4, 4, 4, dest, deststride, src, srcstride

movh     m0,     [r2]
movh     m1,     [r2 + r3]
movh     m2,     [r2 + 2 * r3]
lea      r3,     [r3 + r3 * 2]
movh     m3,     [r2 + r3]

movh     [r0],            m0
movh     [r0 + r1],       m1
movh     [r0 + 2 * r1],   m2
lea      r1,              [r1 + 2 * r1]
movh     [r0 + r1],       m3

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_8x6(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_8x6, 4, 7, 6, dest, deststride, src, srcstride

movh     m0,     [r2]
movh     m1,     [r2 + r3]
movh     m2,     [r2 + 2 * r3]
lea      r5,     [r2 + 2 * r3]
movh     m3,     [r5 + r3]
movh     m4,     [r5 + 2 * r3]
lea      r5,     [r5 + 2 * r3]
movh     m5,     [r5 + r3]

movh     [r0],            m0
movh     [r0 + r1],       m1
movh     [r0 + 2 * r1],   m2
lea      r6,              [r0 + 2 * r1]
movh     [r6 + r1],       m3
movh     [r6 + 2 * r1],   m4
lea      r6,              [r6 + 2 * r1]
movh     [r6 + r1],       m5

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_8x8(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_8x8, 4, 7, 8, dest, deststride, src, srcstride

movh     m0,     [r2]
movh     m1,     [r2 + r3]
movh     m2,     [r2 + 2 * r3]
lea      r5,     [r2 + 2 * r3]
movh     m3,     [r5 + r3]

movh     m4,     [r5 + 2 * r3]
lea      r5,     [r5 + 2 * r3]
movh     m5,     [r5 + r3]
movh     m6,     [r5 + 2 * r3]
lea      r5,     [r5 + 2 * r3]
movh     m7,     [r5 + r3]

movh     [r0],                m0
movh     [r0 + r1],           m1
movh     [r0 + 2 * r1],       m2
lea      r6,                  [r0 + 2 * r1]
movh     [r6 + r1],           m3

movh     [r6 + 2 * r1],        m4
lea      r6,                   [r6 + 2 * r1]
movh     [r6 + r1],            m5
movh     [r6 + 2 * r1],        m6
lea      r6,                   [r6 + 2 * r1]
movh     [r6 + r1],            m7

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W8_H8 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 7, 8, dest, deststride, src, srcstride


mov         r4d,       %2

.loop
     movh     m0,     [r2]
     movh     m1,     [r2 + r3]
     movh     m2,     [r2 + 2 * r3]
     lea      r5,     [r2 + 2 * r3]
     movh     m3,     [r5 + r3]

     movh     m4,     [r5 + 2 * r3]
     lea      r5,     [r5 + 2 * r3]
     movh     m5,     [r5 + r3]
     movh     m6,     [r5 + 2 * r3]
     lea      r5,     [r5 + 2 * r3]
     movh     m7,     [r5 + r3]

     movh     [r0],                m0
     movh     [r0 + r1],           m1
     movh     [r0 + 2 * r1],       m2
     lea      r6,                  [r0 + 2 * r1]
     movh     [r6 + r1],           m3

     movh     [r6 + 2 * r1],        m4
     lea      r6,                   [r6 + 2 * r1]
     movh     [r6 + r1],            m5
     movh     [r6 + 2 * r1],        m6
     lea      r6,                   [r6 + 2 * r1]
     movh     [r6 + r1],            m7

     lea         r0,           [r0 + 8 * r1]
     lea         r2,           [r2 + 8 * r3]

     sub         r4d,           8
     jnz        .loop

RET
%endmacro

BLOCKCOPY_PP_W8_H8 8, 16
BLOCKCOPY_PP_W8_H8 8, 32

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W12_H4 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 7, 8, dest, deststride, src, srcstride

mov         r4d,       %2

.loop
      movh     m0,     [r2]
      movd     m1,     [r2 + 8]

      movh     m2,     [r2 + r3]
      movd     m3,     [r2 + r3 + 8]

      movh     m4,     [r2 + 2 * r3]
      movd     m5,     [r2 + 2 * r3 + 8]

      lea      r5,     [r2 + 2 * r3]

      movh     m6,     [r5 + r3]
      movd     m7,     [r5 + r3 + 8]

      movh     [r0],                 m0
      movd     [r0 + 8],             m1

      movh     [r0 + r1],            m2
      movd     [r0 + r1 + 8],        m3

      movh     [r0 + 2 * r1],        m4
      movd     [r0 + 2 * r1 + 8],    m5

      lea      r6,                   [r0 + 2 * r1]

      movh     [r6 + r1],            m6
      movd     [r6 + r1 + 8],        m7

      lea      r0,                   [r0 + 4 * r1]
      lea      r2,                   [r2 + 4 * r3]

      sub      r4d,                   4
      jnz      .loop

RET
%endmacro

BLOCKCOPY_PP_W12_H4 12, 16

;-----------------------------------------------------------------------------
; void blockcopy_pp_16x4(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_16x4, 4, 4, 4, dest, deststride, src, srcstride

movu     m0,     [r2]
movu     m1,     [r2 + r3]
movu     m2,     [r2 + 2 * r3]
lea      r3,     [r3 + r3 * 2]
movu     m3,     [r2 + r3]

movu     [r0],            m0
movu     [r0 + r1],       m1
movu     [r0 + 2 * r1],   m2
lea      r1,              [r1 + 2 * r1]
movu     [r0 + r1],       m3

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_16x8(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_16x8, 4, 7, 8, dest, deststride, src, srcstride

movu     m0,     [r2]
movu     m1,     [r2 + r3]
movu     m2,     [r2 + 2 * r3]
lea      r5,     [r2 + 2 * r3]
movu     m3,     [r5 + r3]

movu     m4,     [r5 + 2 * r3]
lea      r5,     [r5 + 2 * r3]
movu     m5,     [r5 + r3]
movu     m6,     [r5 + 2 * r3]
lea      r5,     [r5 + 2 * r3]
movu     m7,     [r5 + r3]

movu     [r0],                m0
movu     [r0 + r1],           m1
movu     [r0 + 2 * r1],       m2
lea      r6,                  [r0 + 2 * r1]
movu     [r6 + r1],           m3

movu     [r6 + 2 * r1],        m4
lea      r6,                   [r6 + 2 * r1]
movu     [r6 + r1],            m5
movu     [r6 + 2 * r1],        m6
lea      r6,                   [r6 + 2 * r1]
movu     [r6 + r1],            m7

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_16x12(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_pp_16x12, 4, 7, 8, dest, deststride, src, srcstride

movu     m0,            [r2]
movu     m1,            [r2 + r3]
movu     m2,            [r2 + 2 * r3]
lea      r5,            [r2 + 2 * r3]
movu     m3,            [r5 + r3]

movu     m4,            [r5 + 2 * r3]
lea      r5,            [r5 + 2 * r3]
movu     m5,            [r5 + r3]
movu     m6,            [r5 + 2 * r3]
lea      r5,            [r5 + 2 * r3]
movu     m7,            [r5 + r3]

movu     [r0],            m0
movu     [r0 + r1],       m1
movu     [r0 + 2 * r1],   m2
lea      r6,              [r0 + 2 * r1]
movu     [r6 + r1],       m3

movu     [r6 + 2 * r1],   m4
lea      r6,              [r6 + 2 * r1]
movu     [r6 + r1],       m5
movu     [r6 + 2 * r1],   m6
lea      r6,              [r6 + 2 * r1]
movu     [r6 + r1],       m7

lea      r0,           [r0 + 8 * r1]
lea      r2,           [r2 + 8 * r3]

movu     m0,              [r2]
movu     m1,              [r2 + r3]
movu     m2,              [r2 + 2 * r3]
lea      r3,              [r3 + r3 * 2]
movu     m3,              [r2 + r3]

movu     [r0],            m0
movu     [r0 + r1],       m1
movu     [r0 + 2 * r1],   m2
lea      r1,              [r1 + 2 * r1]
movu     [r0 + r1],       m3

RET

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W16_H8 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 7, 8, dest, deststride, src, srcstride


mov      r4d,       %2

.loop
      movu     m0,     [r2]
      movu     m1,     [r2 + r3]
      movu     m2,     [r2 + 2 * r3]
      lea      r5,     [r2 + 2 * r3]
      movu     m3,     [r5 + r3]

      movu     m4,     [r5 + 2 * r3]
      lea      r5,     [r5 + 2 * r3]
      movu     m5,     [r5 + r3]
      movu     m6,     [r5 + 2 * r3]
      lea      r5,     [r5 + 2 * r3]
      movu     m7,     [r5 + r3]

      movu     [r0],            m0
      movu     [r0 + r1],       m1
      movu     [r0 + 2 * r1],   m2
      lea      r6,              [r0 + 2 * r1]
      movu     [r6 + r1],       m3

      movu     [r6 + 2 * r1],   m4
      lea      r6,              [r6 + 2 * r1]
      movu     [r6 + r1],       m5
      movu     [r6 + 2 * r1],   m6
      lea      r6,              [r6 + 2 * r1]
      movu     [r6 + r1],       m7

      lea      r0,           [r0 + 8 * r1]
      lea      r2,           [r2 + 8 * r3]

      sub      r4d,          8
      jnz      .loop

RET
%endmacro

BLOCKCOPY_PP_W16_H8 16, 16
BLOCKCOPY_PP_W16_H8 16, 32
BLOCKCOPY_PP_W16_H8 16, 64

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W24_H32 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 7, 8, dest, deststride, src, srcstride

mov         r4d,       %2

.loop
      movu     m0,     [r2]
      movh     m1,     [r2 + 16]

      movu     m2,     [r2 + r3]
      movh     m3,     [r2 + r3 + 16]

      movu     m4,     [r2 + 2 * r3]
      movh     m5,     [r2 + 2 * r3 + 16]

      lea      r5,     [r2 + 2 * r3]

      movu     m6,     [r5 + r3]
      movh     m7,     [r5 + r3 + 16]

      movu     [r0],                 m0
      movh     [r0 + 16],            m1

      movu     [r0 + r1],            m2
      movh     [r0 + r1 + 16],       m3

      movu     [r0 + 2 * r1],        m4
      movh     [r0 + 2 * r1 + 16],   m5

      lea      r6,                   [r0 + 2 * r1]

      movu     [r6 + r1],            m6
      movh     [r6 + r1 + 16],       m7

      lea      r0,                   [r0 + 4 * r1]
      lea      r2,                   [r2 + 4 * r3]

      sub      r4d,                  4
      jnz      .loop

RET
%endmacro

BLOCKCOPY_PP_W24_H32 24, 32

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W32_H4 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 7, 8, dest, deststride, src, srcstride

mov         r4d,       %2

.loop
      movu     m0,     [r2]
      movu     m1,     [r2 + 16]

      movu     m2,     [r2 + r3]
      movu     m3,     [r2 + r3 + 16]

      movu     m4,     [r2 + 2 * r3]
      movu     m5,     [r2 + 2 * r3 + 16]

      lea      r5,     [r2 + 2 * r3]

      movu     m6,     [r5 + r3]
      movu     m7,     [r5 + r3 + 16]

      movu     [r0],                 m0
      movu     [r0 + 16],            m1

      movu     [r0 + r1],            m2
      movu     [r0 + r1 + 16],       m3

      movu     [r0 + 2 * r1],        m4
      movu     [r0 + 2 * r1 + 16],   m5

      lea      r6,                   [r0 + 2 * r1]

      movu     [r6 + r1],            m6
      movu     [r6 + r1 + 16],       m7

      lea      r0,                   [r0 + 4 * r1]
      lea      r2,                   [r2 + 4 * r3]

      sub      r4d,                  4
      jnz      .loop

RET
%endmacro

BLOCKCOPY_PP_W32_H4 32, 8
BLOCKCOPY_PP_W32_H4 32, 16
BLOCKCOPY_PP_W32_H4 32, 24
BLOCKCOPY_PP_W32_H4 32, 32
BLOCKCOPY_PP_W32_H4 32, 64

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W48_H2 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 5, 8, dest, deststride, src, srcstride

mov         r4d,       %2

.loop
     movu     m0,     [r2]
     movu     m1,     [r2 + 16]
     movu     m2,     [r2 + 32]

     movu     m3,     [r2 + r3]
     movu     m4,     [r2 + r3 + 16]
     movu     m5,     [r2 + r3 + 32]

     movu     [r0],                 m0
     movu     [r0 + 16],            m1
     movu     [r0 + 32],            m2

     movu     [r0 + r1],            m3
     movu     [r0 + r1 + 16],       m4
     movu     [r0 + r1 + 32],       m5

     lea      r0,                   [r0 + 2 * r1]
     lea      r2,                   [r2 + 2 * r3]

     sub      r4d,                  2
     jnz      .loop

RET
%endmacro

BLOCKCOPY_PP_W48_H2 48, 64

;-----------------------------------------------------------------------------
; void blockcopy_pp_%1x%2(pixel *dest, intptr_t deststride, pixel *src, intptr_t srcstride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PP_W64_H2 2
INIT_XMM sse2
cglobal blockcopy_pp_%1x%2, 4, 5, 8, dest, deststride, src, srcstride

mov         r4d,       %2

.loop
     movu     m0,     [r2]
     movu     m1,     [r2 + 16]
     movu     m2,     [r2 + 32]
     movu     m3,     [r2 + 48]

     movu     m4,     [r2 + r3]
     movu     m5,     [r2 + r3 + 16]
     movu     m6,     [r2 + r3 + 32]
     movu     m7,     [r2 + r3 + 48]

     movu     [r0],                 m0
     movu     [r0 + 16],            m1
     movu     [r0 + 32],            m2
     movu     [r0 + 48],            m3

     movu     [r0 + r1],            m4
     movu     [r0 + r1 + 16],       m5
     movu     [r0 + r1 + 32],       m6
     movu     [r0 + r1 + 48],       m7

     lea      r0,                   [r0 + 2 * r1]
     lea      r2,                   [r2 + 2 * r3]

     sub      r4d,                  2
     jnz      .loop

RET
%endmacro

BLOCKCOPY_PP_W64_H2 64, 16
BLOCKCOPY_PP_W64_H2 64, 32
BLOCKCOPY_PP_W64_H2 64, 48
BLOCKCOPY_PP_W64_H2 64, 64

;-----------------------------------------------------------------------------
; void blockcopy_sp_2x4(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_sp_2x4, 4, 5, 4, dest, destStride, src, srcStride

add        r3,     r3

movd       m0,     [r2]
movd       m1,     [r2 + r3]
movd       m2,     [r2 + 2 * r3]
lea        r2,     [r2 + 2 * r3]
movd       m3,     [r2 + r3]

packuswb   m0,            m1
packuswb   m2,            m3

pextrw     r4,            m0,          0
mov        [r0],          r4w

pextrw     r4,            m0,          4
mov        [r0 + r1],     r4w

pextrw     r4,            m2,          0
mov        [r0 + 2 * r1], r4w

lea        r0,            [r0 + 2 * r1]
pextrw     r4,            m2,          4
mov        [r0 + r1],     r4w

RET


;-----------------------------------------------------------------------------
; void blockcopy_sp_2x8(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_sp_2x8, 4, 5, 8, dest, destStride, src, srcStride

add        r3,      r3

movd       m0,      [r2]
movd       m1,      [r2 + r3]
movd       m2,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movd       m3,      [r2 + r3]
movd       m4,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movd       m5,      [r2 + r3]
movd       m6,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movd       m7,      [r2 + r3]

packuswb   m0,            m1
packuswb   m2,            m3
packuswb   m4,            m5
packuswb   m6,            m7

pextrw     r4,            m0,          0
mov        [r0],          r4w

pextrw     r4,            m0,          4
mov        [r0 + r1],     r4w

pextrw     r4,            m2,          0
mov        [r0 + 2 * r1], r4w

lea        r0,            [r0 + 2 * r1]
pextrw     r4,            m2,          4
mov        [r0 + r1],     r4w

pextrw     r4,            m4,          0
mov        [r0 + 2 * r1], r4w

lea        r0,            [r0 + 2 * r1]
pextrw     r4,            m4,          4
mov        [r0 + r1],     r4w

pextrw     r4,            m6,          0
mov        [r0 + 2 * r1], r4w

lea        r0,            [r0 + 2 * r1]
pextrw     r4,            m6,          4
mov        [r0 + r1],     r4w

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_4x2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_sp_4x2, 4, 4, 2, dest, destStride, src, srcStride

add        r3,        r3

movh       m0,        [r2]
movh       m1,        [r2 + r3]

packuswb   m0,        m1

movd       [r0],      m0
pshufd     m0,        m0,        2
movd       [r0 + r1], m0

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_4x4(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_sp_4x4, 4, 4, 4, dest, destStride, src, srcStride

add        r3,     r3

movh       m0,     [r2]
movh       m1,     [r2 + r3]
movh       m2,     [r2 + 2 * r3]
lea        r2,     [r2 + 2 * r3]
movh       m3,     [r2 + r3]

packuswb   m0,            m1
packuswb   m2,            m3

movd       [r0],          m0
pshufd     m0,            m0,         2
movd       [r0 + r1],     m0
movd       [r0 + 2 * r1], m2
lea        r0,            [r0 + 2 * r1]
pshufd     m2,            m2,         2
movd       [r0 + r1],     m2

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_4x8(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_sp_4x8, 4, 4, 8, dest, destStride, src, srcStride

add        r3,      r3

movh       m0,      [r2]
movh       m1,      [r2 + r3]
movh       m2,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movh       m3,      [r2 + r3]
movh       m4,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movh       m5,      [r2 + r3]
movh       m6,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movh       m7,      [r2 + r3]

packuswb   m0,      m1
packuswb   m2,      m3
packuswb   m4,      m5
packuswb   m6,      m7

movd       [r0],          m0
pshufd     m0,            m0,         2
movd       [r0 + r1],     m0
movd       [r0 + 2 * r1], m2
lea        r0,            [r0 + 2 * r1]
pshufd     m2,            m2,         2
movd       [r0 + r1],     m2
movd       [r0 + 2 * r1], m4
lea        r0,            [r0 + 2 * r1]
pshufd     m4,            m4,         2
movd       [r0 + r1],     m4
movd       [r0 + 2 * r1], m6
lea        r0,            [r0 + 2 * r1]
pshufd     m6,            m6,         2
movd       [r0 + r1],     m6

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W4_H8 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 8, dest, destStride, src, srcStride

mov         r4d,    %2/8

add         r3,     r3

.loop
     movh       m0,      [r2]
     movh       m1,      [r2 + r3]
     movh       m2,      [r2 + 2 * r3]
     lea        r2,      [r2 + 2 * r3]
     movh       m3,      [r2 + r3]
     movh       m4,      [r2 + 2 * r3]
     lea        r2,      [r2 + 2 * r3]
     movh       m5,      [r2 + r3]
     movh       m6,      [r2 + 2 * r3]
     lea        r2,      [r2 + 2 * r3]
     movh       m7,      [r2 + r3]

     packuswb   m0,      m1
     packuswb   m2,      m3
     packuswb   m4,      m5
     packuswb   m6,      m7

     movd       [r0],          m0
     pshufd     m0,            m0,         2
     movd       [r0 + r1],     m0
     movd       [r0 + 2 * r1], m2
     lea        r0,            [r0 + 2 * r1]
     pshufd     m2,            m2,         2
     movd       [r0 + r1],     m2
     movd       [r0 + 2 * r1], m4
     lea        r0,            [r0 + 2 * r1]
     pshufd     m4,            m4,         2
     movd       [r0 + r1],     m4
     movd       [r0 + 2 * r1], m6
     lea        r0,            [r0 + 2 * r1]
     pshufd     m6,            m6,         2
     movd       [r0 + r1],     m6

     lea        r0,            [r0 + 2 * r1]
     lea        r2,            [r2 + 2 * r3]

     dec        r4d
     jnz        .loop

RET
%endmacro

BLOCKCOPY_SP_W4_H8 4, 16

;-----------------------------------------------------------------------------
; void blockcopy_sp_6x8(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W6_H4 2
INIT_XMM sse4
cglobal blockcopy_sp_6x8, 4, 6, 2, dest, destStride, src, srcStride

mov            r4d,           %2/2

add            r3,            r3

.loop
     movu       m0,           [r2]
     movu       m1,           [r2 + r3]


     packuswb   m0,           m1

     movd      [r0],          m0
     pextrw    r5,            m0,    2
     mov       [r0 + 4],      r5w

     pextrw    r5,            m0,    6
     pshufd     m0,           m0,    2
     movd      [r0 + r1],     m0
     mov       [r0 + r1 + 4], r5w

     lea        r0,           [r0 + 2 * r1]
     lea        r2,           [r2 + 2 * r3]

     dec        r4d
     jnz        .loop

RET
%endmacro

BLOCKCOPY_SP_W6_H4 6, 8

;-----------------------------------------------------------------------------
; void blockcopy_sp_8x2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_sp_8x2, 4, 4, 2, dest, destStride, src, srcStride

add        r3,         r3

movu       m0,         [r2]
movu       m1,         [r2 + r3]

packuswb   m0,         m1

movlps     [r0],       m0
movhps     [r0 + r1],  m0

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_8x4(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_sp_8x4, 4, 4, 4, dest, destStride, src, srcStride

add        r3,     r3

movu       m0,     [r2]
movu       m1,     [r2 + r3]
movu       m2,     [r2 + 2 * r3]
lea        r2,     [r2 + 2 * r3]
movu       m3,     [r2 + r3]

packuswb   m0,            m1
packuswb   m2,            m3

movlps     [r0],          m0
movhps     [r0 + r1],     m0
movlps     [r0 + 2 * r1], m2
lea        r0,            [r0 + 2 * r1]
movhps     [r0 + r1],     m2

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_8x6(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_sp_8x6, 4, 4, 6, dest, destStride, src, srcStride

add        r3,      r3

movu       m0,      [r2]
movu       m1,      [r2 + r3]
movu       m2,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movu       m3,      [r2 + r3]
movu       m4,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movu       m5,      [r2 + r3]

packuswb   m0,            m1
packuswb   m2,            m3
packuswb   m4,            m5

movlps     [r0],          m0
movhps     [r0 + r1],     m0
movlps     [r0 + 2 * r1], m2
lea        r0,            [r0 + 2 * r1]
movhps     [r0 + r1],     m2
movlps     [r0 + 2 * r1], m4
lea        r0,            [r0 + 2 * r1]
movhps     [r0 + r1],     m4

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_8x8(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockcopy_sp_8x8, 4, 4, 8, dest, destStride, src, srcStride

add        r3,      r3

movu       m0,      [r2]
movu       m1,      [r2 + r3]
movu       m2,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movu       m3,      [r2 + r3]
movu       m4,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movu       m5,      [r2 + r3]
movu       m6,      [r2 + 2 * r3]
lea        r2,      [r2 + 2 * r3]
movu       m7,      [r2 + r3]

packuswb   m0,      m1
packuswb   m2,      m3
packuswb   m4,      m5
packuswb   m6,      m7

movlps     [r0],          m0
movhps     [r0 + r1],     m0
movlps     [r0 + 2 * r1], m2
lea        r0,            [r0 + 2 * r1]
movhps     [r0 + r1],     m2
movlps     [r0 + 2 * r1], m4
lea        r0,            [r0 + 2 * r1]
movhps     [r0 + r1],     m4
movlps     [r0 + 2 * r1], m6
lea        r0,            [r0 + 2 * r1]
movhps     [r0 + r1],     m6

RET

;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W8_H8 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 8, dest, destStride, src, srcStride

mov         r4d,    %2/8

add         r3,     r3

.loop
     movu       m0,      [r2]
     movu       m1,      [r2 + r3]
     movu       m2,      [r2 + 2 * r3]
     lea        r2,      [r2 + 2 * r3]
     movu       m3,      [r2 + r3]
     movu       m4,      [r2 + 2 * r3]
     lea        r2,      [r2 + 2 * r3]
     movu       m5,      [r2 + r3]
     movu       m6,      [r2 + 2 * r3]
     lea        r2,      [r2 + 2 * r3]
     movu       m7,      [r2 + r3]

     packuswb   m0,      m1
     packuswb   m2,      m3
     packuswb   m4,      m5
     packuswb   m6,      m7

     movlps     [r0],          m0
     movhps     [r0 + r1],     m0
     movlps     [r0 + 2 * r1], m2
     lea        r0,            [r0 + 2 * r1]
     movhps     [r0 + r1],     m2
     movlps     [r0 + 2 * r1], m4
     lea        r0,            [r0 + 2 * r1]
     movhps     [r0 + r1],     m4
     movlps     [r0 + 2 * r1], m6
     lea        r0,            [r0 + 2 * r1]
     movhps     [r0 + r1],     m6

    lea         r0,            [r0 + 2 * r1]
    lea         r2,            [r2 + 2 * r3]

    dec         r4d
    jnz         .loop

RET
%endmacro

BLOCKCOPY_SP_W8_H8 8, 16


;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W12_H4 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 8, dest, destStride, src, srcStride

mov             r4d,     %2/4

add             r3,      r3

.loop
     movu       m0,      [r2]
     movu       m1,      [r2 + 16]
     movu       m2,      [r2 + r3]
     movu       m3,      [r2 + r3 + 16]
     movu       m4,      [r2 + 2 * r3]
     movu       m5,      [r2 + 2 * r3 + 16]
     lea        r2,      [r2 + 2 * r3]
     movu       m6,      [r2 + r3]
     movu       m7,      [r2 + r3 + 16]

     packuswb   m0,      m1
     packuswb   m2,      m3
     packuswb   m4,      m5
     packuswb   m6,      m7

     movh       [r0],              m0
     pshufd     m0,                m0,    2
     movd       [r0 + 8],          m0

     movh       [r0 + r1],         m2
     pshufd     m2,                m2,    2
     movd       [r0 + r1 + 8],     m2

     movh       [r0 + 2 * r1],     m4
     pshufd     m4,                m4,    2
     movd       [r0 + 2 * r1 + 8], m4

     lea        r0,                [r0 + 2 * r1]
     movh       [r0 + r1],         m6
     pshufd     m6,                m6,    2
     movd       [r0 + r1 + 8],     m6

     lea        r0,                [r0 + 2 * r1]
     lea        r2,                [r2 + 2 * r3]

     dec        r4d
     jnz        .loop

RET
%endmacro

BLOCKCOPY_SP_W12_H4 12, 16

;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W16_H4 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 8, dest, destStride, src, srcStride

mov             r4d,     %2/4

add             r3,      r3

.loop
     movu       m0,      [r2]
     movu       m1,      [r2 + 16]
     movu       m2,      [r2 + r3]
     movu       m3,      [r2 + r3 + 16]
     movu       m4,      [r2 + 2 * r3]
     movu       m5,      [r2 + 2 * r3 + 16]
     lea        r2,      [r2 + 2 * r3]
     movu       m6,      [r2 + r3]
     movu       m7,      [r2 + r3 + 16]

     packuswb   m0,      m1
     packuswb   m2,      m3
     packuswb   m4,      m5
     packuswb   m6,      m7

     movu       [r0],              m0
     movu       [r0 + r1],         m2
     movu       [r0 + 2 * r1],     m4
     lea        r0,                [r0 + 2 * r1]
     movu       [r0 + r1],         m6

     lea        r0,                [r0 + 2 * r1]
     lea        r2,                [r2 + 2 * r3]

     dec        r4d
     jnz        .loop

RET
%endmacro

BLOCKCOPY_SP_W16_H4 16,  4
BLOCKCOPY_SP_W16_H4 16,  8
BLOCKCOPY_SP_W16_H4 16, 12
BLOCKCOPY_SP_W16_H4 16, 16
BLOCKCOPY_SP_W16_H4 16, 32
BLOCKCOPY_SP_W16_H4 16, 64

;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W24_H2 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 6, dest, destStride, src, srcStride

mov             r4d,     %2/2

add             r3,      r3

.loop
     movu       m0,      [r2]
     movu       m1,      [r2 + 16]
     movu       m2,      [r2 + 32]
     movu       m3,      [r2 + r3]
     movu       m4,      [r2 + r3 + 16]
     movu       m5,      [r2 + r3 + 32]

     packuswb   m0,      m1
     packuswb   m2,      m3
     packuswb   m4,      m5

     movu       [r0],            m0
     movlps     [r0 + 16],       m2
     movhps     [r0 + r1],       m2
     movu       [r0 + r1 + 8],   m4

     lea        r0,              [r0 + 2 * r1]
     lea        r2,              [r2 + 2 * r3]

     dec        r4d
     jnz        .loop

RET
%endmacro

BLOCKCOPY_SP_W24_H2 24, 32

;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W32_H2 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 8, dest, destStride, src, srcStride

mov             r4d,     %2/2

add             r3,      r3

.loop
     movu       m0,      [r2]
     movu       m1,      [r2 + 16]
     movu       m2,      [r2 + 32]
     movu       m3,      [r2 + 48]
     movu       m4,      [r2 + r3]
     movu       m5,      [r2 + r3 + 16]
     movu       m6,      [r2 + r3 + 32]
     movu       m7,      [r2 + r3 + 48]

     packuswb   m0,      m1
     packuswb   m2,      m3
     packuswb   m4,      m5
     packuswb   m6,      m7

     movu       [r0],            m0
     movu       [r0 + 16],       m2
     movu       [r0 + r1],       m4
     movu       [r0 + r1 + 16],  m6

     lea        r0,              [r0 + 2 * r1]
     lea        r2,              [r2 + 2 * r3]

     dec        r4d
     jnz        .loop

RET
%endmacro

BLOCKCOPY_SP_W32_H2 32,  8
BLOCKCOPY_SP_W32_H2 32, 16
BLOCKCOPY_SP_W32_H2 32, 24
BLOCKCOPY_SP_W32_H2 32, 32
BLOCKCOPY_SP_W32_H2 32, 64

;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W48_H2 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 6, dest, destStride, src, srcStride

mov             r4d,     %2

add             r3,      r3

.loop
     movu       m0,        [r2]
     movu       m1,        [r2 + 16]
     movu       m2,        [r2 + 32]
     movu       m3,        [r2 + 48]
     movu       m4,        [r2 + 64]
     movu       m5,        [r2 + 80]

     packuswb   m0,        m1
     packuswb   m2,        m3
     packuswb   m4,        m5

     movu       [r0],      m0
     movu       [r0 + 16], m2
     movu       [r0 + 32], m4

     lea        r0,        [r0 + r1]
     lea        r2,        [r2 + r3]

     dec        r4d
     jnz        .loop

RET
%endmacro

BLOCKCOPY_SP_W48_H2 48, 64

;-----------------------------------------------------------------------------
; void blockcopy_sp_%1x%2(pixel *dest, intptr_t destStride, int16_t *src, intptr_t srcStride)
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_SP_W64_H1 2
INIT_XMM sse2
cglobal blockcopy_sp_%1x%2, 4, 5, 8, dest, destStride, src, srcStride

mov             r4d,       %2

add             r3,         r3

.loop
      movu      m0,        [r2]
      movu      m1,        [r2 + 16]
      movu      m2,        [r2 + 32]
      movu      m3,        [r2 + 48]
      movu      m4,        [r2 + 64]
      movu      m5,        [r2 + 80]
      movu      m6,        [r2 + 96]
      movu      m7,        [r2 + 112]

     packuswb   m0,        m1
     packuswb   m2,        m3
     packuswb   m4,        m5
     packuswb   m6,        m7

      movu      [r0],      m0
      movu      [r0 + 16], m2
      movu      [r0 + 32], m4
      movu      [r0 + 48], m6

      lea       r0,        [r0 + r1]
      lea       r2,        [r2 + r3]

      dec       r4d
      jnz       .loop

RET
%endmacro

BLOCKCOPY_SP_W64_H1 64, 16
BLOCKCOPY_SP_W64_H1 64, 32
BLOCKCOPY_SP_W64_H1 64, 48
BLOCKCOPY_SP_W64_H1 64, 64

;-----------------------------------------------------------------------------
; void blockfill_s_4x4(int16_t *dest, intptr_t destride, int16_t val)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockfill_s_4x4, 3, 3, 1, dest, destStride, val

add        r1,            r1

movd       m0,            r2d
pshuflw    m0,            m0,         0

movh       [r0],          m0
movh       [r0 + r1],     m0
movh       [r0 + 2 * r1], m0
lea        r0,            [r0 + 2 * r1]
movh       [r0 + r1],     m0

RET

;-----------------------------------------------------------------------------
; void blockfill_s_8x8(int16_t *dest, intptr_t destride, int16_t val)
;-----------------------------------------------------------------------------
INIT_XMM sse2
cglobal blockfill_s_8x8, 3, 3, 1, dest, destStride, val

add        r1,            r1

movd       m0,            r2d
pshuflw    m0,            m0,         0
pshufd     m0,            m0,         0

movu       [r0],          m0
movu       [r0 + r1],     m0
movu       [r0 + 2 * r1], m0

lea        r0,            [r0 + 2 * r1]
movu       [r0 + r1],     m0
movu       [r0 + 2 * r1], m0

lea        r0,            [r0 + 2 * r1]
movu       [r0 + r1],     m0
movu       [r0 + 2 * r1], m0

lea        r0,            [r0 + 2 * r1]
movu       [r0 + r1],     m0

RET

;-----------------------------------------------------------------------------
; void blockfill_s_%1x%2(int16_t *dest, intptr_t destride, int16_t val)
;-----------------------------------------------------------------------------
%macro BLOCKFILL_S_W16_H8 2
INIT_XMM sse2
cglobal blockfill_s_%1x%2, 3, 5, 1, dest, destStride, val

mov        r3d,           %2/8

add        r1,            r1

movd       m0,            r2d
pshuflw    m0,            m0,       0
pshufd     m0,            m0,       0

.loop
     movu       [r0],               m0
     movu       [r0 + 16],          m0

     movu       [r0 + r1],          m0
     movu       [r0 + r1 + 16],     m0

     movu       [r0 + 2 * r1],      m0
     movu       [r0 + 2 * r1 + 16], m0

     lea        r4,                 [r0 + 2 * r1]
     movu       [r4 + r1],          m0
     movu       [r4 + r1 + 16],     m0

     movu       [r0 + 4 * r1],      m0
     movu       [r0 + 4 * r1 + 16], m0

     lea        r4,                 [r0 + 4 * r1]
     movu       [r4 + r1],          m0
     movu       [r4 + r1 + 16],     m0

     movu       [r4 + 2 * r1],      m0
     movu       [r4 + 2 * r1 + 16], m0

     lea        r4,                 [r4 + 2 * r1]
     movu       [r4 + r1],          m0
     movu       [r4 + r1 + 16],     m0

     lea        r0,                 [r0 + 8 * r1]

     dec        r3d
     jnz        .loop

RET
%endmacro

BLOCKFILL_S_W16_H8 16, 16

;-----------------------------------------------------------------------------
; void blockfill_s_%1x%2(int16_t *dest, intptr_t destride, int16_t val)
;-----------------------------------------------------------------------------
%macro BLOCKFILL_S_W32_H4 2
INIT_XMM sse2
cglobal blockfill_s_%1x%2, 3, 5, 1, dest, destStride, val

mov        r3d,           %2/4

add        r1,            r1

movd       m0,            r2d
pshuflw    m0,            m0,       0
pshufd     m0,            m0,       0

.loop
     movu       [r0],               m0
     movu       [r0 + 16],          m0
     movu       [r0 + 32],          m0
     movu       [r0 + 48],          m0

     movu       [r0 + r1],          m0
     movu       [r0 + r1 + 16],     m0
     movu       [r0 + r1 + 32],     m0
     movu       [r0 + r1 + 48],     m0

     movu       [r0 + 2 * r1],      m0
     movu       [r0 + 2 * r1 + 16], m0
     movu       [r0 + 2 * r1 + 32], m0
     movu       [r0 + 2 * r1 + 48], m0

     lea        r4,                 [r0 + 2 * r1]

     movu       [r4 + r1],          m0
     movu       [r4 + r1 + 16],     m0
     movu       [r4 + r1 + 32],     m0
     movu       [r4 + r1 + 48],     m0

     lea        r0,                 [r0 + 4 * r1]

     dec        r3d
     jnz        .loop

RET
%endmacro

BLOCKFILL_S_W32_H4 32, 32



;-----------------------------------------------------------------------------
; void blockcopy_ps_2x4(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_2x4, 4, 4, 1, dest, destStride, src, srcStride

add        r1,            r1

movd       m0,            [r2]
pmovzxbw   m0,            m0
movd       [r0],          m0

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movd       [r0 + r1],     m0

movd       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movd       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movd       [r0 + r1],     m0

RET


;-----------------------------------------------------------------------------
; void blockcopy_ps_2x8(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_2x8, 4, 4, 1, dest, destStride, src, srcStride

add        r1,            r1

movd       m0,            [r2]
pmovzxbw   m0,            m0
movd       [r0],          m0

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movd       [r0 + r1],     m0

movd       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movd       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movd       [r0 + r1],     m0

movd       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movd       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movd       [r0 + r1],     m0

movd       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movd       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movd       [r0 + r1],     m0

RET

;-----------------------------------------------------------------------------
; void blockcopy_ps_4x2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_4x2, 4, 4, 1, dest, destStride, src, srcStride

add        r1,         r1

movd       m0,         [r2]
pmovzxbw   m0,         m0
movh       [r0],       m0

movd       m0,         [r2 + r3]
pmovzxbw   m0,         m0
movh       [r0 + r1],  m0

RET


;-----------------------------------------------------------------------------
; void blockcopy_ps_4x4(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_4x4, 4, 4, 1, dest, destStride, src, srcStride

add        r1,            r1

movd       m0,            [r2]
pmovzxbw   m0,            m0
movh       [r0],          m0

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movh       [r0 + r1],     m0

movd       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movh       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movd       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movh       [r0 + r1],     m0

RET


;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W4_H4 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 1, dest, destStride, src, srcStride

add     r1,      r1
mov    r4d,      %2/4

.loop
      movd       m0,            [r2]
      pmovzxbw   m0,            m0
      movh       [r0],          m0

      movd       m0,            [r2 + r3]
      pmovzxbw   m0,            m0
      movh       [r0 + r1],     m0

      movd       m0,            [r2 + 2 * r3]
      pmovzxbw   m0,            m0
      movh       [r0 + 2 * r1], m0

      lea        r2,            [r2 + 2 * r3]
      lea        r0,            [r0 + 2 * r1]

      movd       m0,            [r2 + r3]
      pmovzxbw   m0,            m0
      movh       [r0 + r1],     m0

      lea        r0,            [r0 + 2 * r1]
      lea        r2,            [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W4_H4 4, 8
BLOCKCOPY_PS_W4_H4 4, 16

;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W6_H4 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 1, dest, destStride, src, srcStride

add     r1,      r1
mov    r4d,      %2/4

.loop
      movh       m0,                [r2]
      pmovzxbw   m0,                m0
      movh       [r0],              m0
      pextrd     [r0 + 8],          m0,            2

      movh       m0,                [r2 + r3]
      pmovzxbw   m0,                m0
      movh       [r0 + r1],         m0
      pextrd     [r0 + r1 + 8],     m0,            2

      movh       m0,                [r2 + 2 * r3]
      pmovzxbw   m0,                m0
      movh       [r0 + 2 * r1],     m0
      pextrd     [r0 + 2 * r1 + 8], m0,            2

      lea        r2,                [r2 + 2 * r3]
      lea        r0,                [r0 + 2 * r1]

      movh       m0,                [r2 + r3]
      pmovzxbw   m0,                m0
      movh       [r0 + r1],         m0
      pextrd     [r0 + r1 + 8],     m0,            2

      lea        r0,                [r0 + 2 * r1]
      lea        r2,                [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W6_H4 6, 8

;-----------------------------------------------------------------------------
; void blockcopy_ps_8x2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_8x2, 4, 4, 1, dest, destStride, src, srcStride

add        r1,         r1

movh       m0,         [r2]
pmovzxbw   m0,         m0
movu       [r0],       m0

movh       m0,         [r2 + r3]
pmovzxbw   m0,         m0
movu       [r0 + r1],  m0

RET

;-----------------------------------------------------------------------------
; void blockcopy_ps_8x4(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_8x4, 4, 4, 1, dest, destStride, src, srcStride

add        r1,            r1

movh       m0,            [r2]
pmovzxbw   m0,            m0
movu       [r0],          m0

movh       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movu       [r0 + r1],     m0

movh       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movu       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movh       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movu       [r0 + r1],     m0

RET

;-----------------------------------------------------------------------------
; void blockcopy_ps_8x6(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_8x6, 4, 4, 1, dest, destStride, src, srcStride

add        r1,            r1

movh       m0,            [r2]
pmovzxbw   m0,            m0
movu       [r0],          m0

movh       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movu       [r0 + r1],     m0

movh       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movu       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movh       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movu       [r0 + r1],     m0

movh       m0,            [r2 + 2 * r3]
pmovzxbw   m0,            m0
movu       [r0 + 2 * r1], m0

lea        r2,            [r2 + 2 * r3]
lea        r0,            [r0 + 2 * r1]

movh       m0,            [r2 + r3]
pmovzxbw   m0,            m0
movu       [r0 + r1],     m0

RET

;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W8_H4 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 1, dest, destStride, src, srcStride

add     r1,      r1
mov    r4d,      %2/4

.loop
      movh       m0,            [r2]
      pmovzxbw   m0,            m0
      movu       [r0],          m0

      movh       m0,            [r2 + r3]
      pmovzxbw   m0,            m0
      movu       [r0 + r1],     m0

      movh       m0,            [r2 + 2 * r3]
      pmovzxbw   m0,            m0
      movu       [r0 + 2 * r1], m0

      lea        r2,            [r2 + 2 * r3]
      lea        r0,            [r0 + 2 * r1]

      movh       m0,            [r2 + r3]
      pmovzxbw   m0,            m0
      movu       [r0 + r1],     m0

      lea        r0,            [r0 + 2 * r1]
      lea        r2,            [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W8_H4  8,  8
BLOCKCOPY_PS_W8_H4  8, 16
BLOCKCOPY_PS_W8_H4  8, 32


;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W12_H2 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 3, dest, destStride, src, srcStride

add        r1,      r1
mov        r4d,     %2/2
pxor       m0,      m0

.loop
      movu       m1,             [r2]
      pmovzxbw   m2,             m1
      movu       [r0],           m2
      punpckhbw  m1,             m0
      movh       [r0 + 16],      m1

      movu       m1,             [r2 + r3]
      pmovzxbw   m2,             m1
      movu       [r0 + r1],      m2
      punpckhbw  m1,             m0
      movh       [r0 + r1 + 16], m1

      lea        r0,             [r0 + 2 * r1]
      lea        r2,             [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W12_H2 12, 16

;-----------------------------------------------------------------------------
; void blockcopy_ps_16x4(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
INIT_XMM sse4
cglobal blockcopy_ps_16x4, 4, 4, 3, dest, destStride, src, srcStride

add        r1,      r1
pxor       m0,      m0

movu       m1,                 [r2]
pmovzxbw   m2,                 m1
movu       [r0],               m2
punpckhbw  m1,                 m0
movu       [r0 + 16],          m1

movu       m1,                 [r2 + r3]
pmovzxbw   m2,                 m1
movu       [r0 + r1],          m2
punpckhbw  m1,                 m0
movu       [r0 + r1 + 16],     m1

movu       m1,                 [r2 + 2 * r3]
pmovzxbw   m2,                 m1
movu       [r0 + 2 * r1],      m2
punpckhbw  m1,                 m0
movu       [r0 + 2 * r1 + 16], m1

lea        r0,                 [r0 + 2 * r1]
lea        r2,                 [r2 + 2 * r3]

movu       m1,                 [r2 + r3]
pmovzxbw   m2,                 m1
movu       [r0 + r1],          m2
punpckhbw  m1,                 m0
movu       [r0 + r1 + 16],     m1

RET

;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W16_H4 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 3, dest, destStride, src, srcStride

add        r1,      r1
mov        r4d,     %2/4
pxor       m0,      m0

.loop
      movu       m1,                 [r2]
      pmovzxbw   m2,                 m1
      movu       [r0],               m2
      punpckhbw  m1,                 m0
      movu       [r0 + 16],          m1

      movu       m1,                 [r2 + r3]
      pmovzxbw   m2,                 m1
      movu       [r0 + r1],          m2
      punpckhbw  m1,                 m0
      movu       [r0 + r1 + 16],     m1

      movu       m1,                 [r2 + 2 * r3]
      pmovzxbw   m2,                 m1
      movu       [r0 + 2 * r1],      m2
      punpckhbw  m1,                 m0
      movu       [r0 + 2 * r1 + 16], m1

      lea        r0,                 [r0 + 2 * r1]
      lea        r2,                 [r2 + 2 * r3]

      movu       m1,                 [r2 + r3]
      pmovzxbw   m2,                 m1
      movu       [r0 + r1],          m2
      punpckhbw  m1,                 m0
      movu       [r0 + r1 + 16],     m1

      lea        r0,                 [r0 + 2 * r1]
      lea        r2,                 [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W16_H4 16,  8
BLOCKCOPY_PS_W16_H4 16, 12
BLOCKCOPY_PS_W16_H4 16, 16
BLOCKCOPY_PS_W16_H4 16, 32
BLOCKCOPY_PS_W16_H4 16, 64

;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W24_H2 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 3, dest, destStride, src, srcStride

add        r1,      r1
mov        r4d,     %2/2
pxor       m0,      m0

.loop
      movu       m1,             [r2]
      pmovzxbw   m2,             m1
      movu       [r0],           m2
      punpckhbw  m1,             m0
      movu       [r0 + 16],      m1

      movh       m1,             [r2 + 16]
      pmovzxbw   m1,             m1
      movu       [r0 + 32],      m1

      movu       m1,             [r2 + r3]
      pmovzxbw   m2,             m1
      movu       [r0 + r1],      m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 16], m1

      movh       m1,             [r2 + r3 + 16]
      pmovzxbw   m1,             m1
      movu       [r0 + r1 + 32], m1

      lea        r0,             [r0 + 2 * r1]
      lea        r2,             [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W24_H2 24, 32

;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W32_H2 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 3, dest, destStride, src, srcStride

add        r1,      r1
mov        r4d,     %2/2
pxor       m0,      m0

.loop
      movu       m1,             [r2]
      pmovzxbw   m2,             m1
      movu       [r0],           m2
      punpckhbw  m1,             m0
      movu       [r0 + 16],      m1

      movu       m1,             [r2 + 16]
      pmovzxbw   m2,             m1
      movu       [r0 + 32],      m2
      punpckhbw  m1,             m0
      movu       [r0 + 48],      m1

      movu       m1,             [r2 + r3]
      pmovzxbw   m2,             m1
      movu       [r0 + r1],      m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 16], m1

      movu       m1,             [r2 + r3 + 16]
      pmovzxbw   m2,             m1
      movu       [r0 + r1 + 32], m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 48], m1

      lea        r0,             [r0 + 2 * r1]
      lea        r2,             [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W32_H2 32,  8
BLOCKCOPY_PS_W32_H2 32, 16
BLOCKCOPY_PS_W32_H2 32, 24
BLOCKCOPY_PS_W32_H2 32, 32
BLOCKCOPY_PS_W32_H2 32, 64

;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W48_H2 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 3, dest, destStride, src, srcStride

add        r1,      r1
mov        r4d,     %2/2
pxor       m0,      m0

.loop
      movu       m1,             [r2]
      pmovzxbw   m2,             m1
      movu       [r0],           m2
      punpckhbw  m1,             m0
      movu       [r0 + 16],      m1

      movu       m1,             [r2 + 16]
      pmovzxbw   m2,             m1
      movu       [r0 + 32],      m2
      punpckhbw  m1,             m0
      movu       [r0 + 48],      m1

      movu       m1,             [r2 + 32]
      pmovzxbw   m2,             m1
      movu       [r0 + 64],      m2
      punpckhbw  m1,             m0
      movu       [r0 + 80],      m1

      movu       m1,             [r2 + r3]
      pmovzxbw   m2,             m1
      movu       [r0 + r1],      m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 16], m1

      movu       m1,             [r2 + r3 + 16]
      pmovzxbw   m2,             m1
      movu       [r0 + r1 + 32], m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 48], m1

      movu       m1,             [r2 + r3 + 32]
      pmovzxbw   m2,             m1
      movu       [r0 + r1 + 64], m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 80], m1

      lea        r0,             [r0 + 2 * r1]
      lea        r2,             [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W48_H2 48, 64

;-----------------------------------------------------------------------------
; void blockcopy_ps_%1x%2(int16_t *dest, intptr_t destStride, pixel *src, intptr_t srcStride);
;-----------------------------------------------------------------------------
%macro BLOCKCOPY_PS_W64_H2 2
INIT_XMM sse4
cglobal blockcopy_ps_%1x%2, 4, 5, 3, dest, destStride, src, srcStride

add        r1,      r1
mov        r4d,     %2/2
pxor       m0,      m0

.loop
      movu       m1,             [r2]
      pmovzxbw   m2,             m1
      movu       [r0],           m2
      punpckhbw  m1,             m0
      movu       [r0 + 16],      m1

      movu       m1,             [r2 + 16]
      pmovzxbw   m2,             m1
      movu       [r0 + 32],      m2
      punpckhbw  m1,             m0
      movu       [r0 + 48],      m1

      movu       m1,             [r2 + 32]
      pmovzxbw   m2,             m1
      movu       [r0 + 64],      m2
      punpckhbw  m1,             m0
      movu       [r0 + 80],      m1

      movu       m1,             [r2 + 48]
      pmovzxbw   m2,             m1
      movu       [r0 + 96],      m2
      punpckhbw  m1,             m0
      movu       [r0 + 112],     m1

      movu       m1,             [r2 + r3]
      pmovzxbw   m2,             m1
      movu       [r0 + r1],      m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 16], m1

      movu       m1,             [r2 + r3 + 16]
      pmovzxbw   m2,             m1
      movu       [r0 + r1 + 32], m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 48], m1

      movu       m1,             [r2 + r3 + 32]
      pmovzxbw   m2,             m1
      movu       [r0 + r1 + 64], m2
      punpckhbw  m1,             m0
      movu       [r0 + r1 + 80], m1

      movu       m1,              [r2 + r3 + 48]
      pmovzxbw   m2,              m1
      movu       [r0 + r1 + 96],  m2
      punpckhbw  m1,              m0
      movu       [r0 + r1 + 112], m1

      lea        r0,              [r0 + 2 * r1]
      lea        r2,              [r2 + 2 * r3]

      dec        r4d
      jnz        .loop

RET
%endmacro

BLOCKCOPY_PS_W64_H2 64, 16
BLOCKCOPY_PS_W64_H2 64, 32
BLOCKCOPY_PS_W64_H2 64, 48
BLOCKCOPY_PS_W64_H2 64, 64
