/*****************************************************************************
 * pixel.cpp: Pixel operator functions
 *****************************************************************************
 * Copyright (C) 2012-2020 x265 project
 *
 * Authors: Min Chen <chenm003@163.com> Xiangwen Wang <wxw21st@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at wxw21st@163.com.
 *****************************************************************************/

#include "x265.h"

// ***************************************************************************
// * SAD Functions
// ***************************************************************************
#if (USE_ASM >= ASM_SSE2)
UInt32 xSad32x32( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;
    CUInt8 *P = pSrc;
    CUInt8 *Q = pRef;
    __m128i sumA = _mm_setzero_si128();
    __m128i sumB = _mm_setzero_si128();

    for( y=0; y<32; y++ ) {
        sumA = _mm_add_epi32(sumA, _mm_sad_epu8(_mm_load_si128((__m128i*)P + 0), _mm_loadu_si128((__m128i*)Q + 0)));
        sumB = _mm_add_epi32(sumB, _mm_sad_epu8(_mm_load_si128((__m128i*)P + 1), _mm_loadu_si128((__m128i*)Q + 1)));
        P += nStrideSrc;
        Q += nStrideRef;
    }
    sumA = _mm_add_epi32(sumA, sumB);
    sumA = _mm_add_epi32(_mm_srli_si128(sumA, 8), sumA);
    uiSad = _mm_cvtsi128_si32(sumA);
    return uiSad;
}
#else // ASM_NONE
UInt32 xSad32x32( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<32; y++ ) {
        for( x=0; x<32; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}
#endif

#if (USE_ASM >= ASM_SSE2)
UInt32 xSad16x16( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;
    CUInt8 *P = pSrc;
    CUInt8 *Q = pRef;
    __m128i sumA = _mm_setzero_si128();

    for( y=0; y<16; y++ ) {
        sumA = _mm_add_epi32(sumA, _mm_sad_epu8(_mm_load_si128((__m128i*)P + 0), _mm_loadu_si128((__m128i*)Q + 0)));
        P += nStrideSrc;
        Q += nStrideRef;
    }
    sumA = _mm_add_epi32(_mm_srli_si128(sumA, 8), sumA);
    uiSad = _mm_cvtsi128_si32(sumA);
    return uiSad;
}
#else // ASM_NONE
UInt32 xSad16x16( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<16; y++ ) {
        for( x=0; x<16; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}
#endif

#if (USE_ASM >= ASM_SSE2)
UInt32 xSad8x8( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;
    CUInt8 *P = pSrc;
    CUInt8 *Q = pRef;
    __m128i sumA = _mm_setzero_si128();

    for( y=0; y<8; y++ ) {
        sumA = _mm_add_epi32(sumA, _mm_sad_epu8(_mm_loadl_epi64((__m128i*)P), _mm_loadl_epi64((__m128i*)Q)));
        P += nStrideSrc;
        Q += nStrideRef;
    }
    uiSad = _mm_cvtsi128_si32(sumA);
    return uiSad;
}
#else // ASM_NONE
UInt32 xSad8x8( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<8; y++ ) {
        for( x=0; x<8; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}
#endif

#if (USE_ASM >= ASM_SSE2)
UInt32 xSad4x4( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;
    CUInt8 *P = pSrc;
    CUInt8 *Q = pRef;
    __m128i sumA = _mm_setzero_si128();

    for( y=0; y<4; y++ ) {
        sumA = _mm_add_epi32(sumA, _mm_sad_epu8(_mm_cvtsi32_si128(*(UInt32*)P), _mm_cvtsi32_si128(*(UInt32*)Q)));
        P += nStrideSrc;
        Q += nStrideRef;
    }
    uiSad = _mm_cvtsi128_si32(sumA);
    return uiSad;
}
#else // ASM_NONE
UInt32 xSad4x4( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt x, y;
    UInt32 uiSad = 0;

    for( y=0; y<4; y++ ) {
        for( x=0; x<4; x++ ) {
            uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
        }
    }
    return uiSad;
}
#endif

xSad *xSadN[MAX_CU_DEPTH] = {
    xSad4x4,    /*  4 x 4 */
    xSad8x8,    /*  8 x 8 */
    xSad16x16,   /* 16 x 16 */
    xSad32x32,   /* 32 x 32 */
};


// ***************************************************************************
// * Interface Functions
// ***************************************************************************
#if (USE_ASM >= ASM_SSE4)
UInt32 xQuant4( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    UInt32 uiAcSum = 0;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt uiQ = xg_quantScales[nQpMod6];
    Int iQBits = 19 + nQpDiv6;

    UInt32 iRnd1 = (eSType == SLICE_I ? 171 : 85)<<7;
    __m128i QRN = _mm_set1_epi32((iRnd1<<16) | uiQ);
    __m128i QN2 = _mm_set1_epi16(1<<(3+nQpDiv6));
    __m128i acSum = _mm_setzero_si128();

    for( y=0; y < iHeight; y++ ) {
        __m128i T00 = _mm_loadl_epi64((__m128i*)&pSrc[y*nStride]);
        __m128i T01 = _mm_unpacklo_epi16(T00, T00);
        __m128i T10 = _mm_abs_epi16(T00);
        __m128i T20 = _mm_unpacklo_epi16(T10, QN2);
        __m128i T30 = _mm_madd_epi16(T20, QRN);
        __m128i T40 = _mm_srai_epi32(T30, iQBits);
        __m128i T50 = _mm_sign_epi32(T40, T01);
        __m128i T60 = _mm_packs_epi32(T50, T50);
        acSum = _mm_add_epi32(acSum, T40);
        _mm_storel_epi64((__m128i*)&pDst[y*nStride], T60);
    }
    //acSum = _mm_add_epi32(acSum, _mm_unpackhi_epi64(acSum, acSum));
    //acSum = _mm_add_epi32(acSum, _mm_srli_si128(acSum, 4));
    acSum = _mm_hadd_epi32(acSum, acSum);
    acSum = _mm_hadd_epi32(acSum, acSum);
    uiAcSum = _mm_cvtsi128_si32(acSum);

    return uiAcSum;
}
UInt32 xQuant8( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    UInt32 uiAcSum = 0;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt uiQ = xg_quantScales[nQpMod6];
    Int iQBits = 18 + nQpDiv6;

    UInt32 iRnd1 = (eSType == SLICE_I ? 171 : 85)<<6;
    __m128i QRN = _mm_set1_epi32((iRnd1<<16) | uiQ);
    __m128i QN2 = _mm_set1_epi16(1<<(3+nQpDiv6));
    __m128i acSumA = _mm_setzero_si128();
    __m128i acSumB = _mm_setzero_si128();

    for( y=0; y < iHeight; y++ ) {
        __m128i T00  = _mm_loadu_si128((__m128i*)&pSrc[y*nStride]);
        __m128i T01A = _mm_unpacklo_epi16(T00, T00);
        __m128i T01B = _mm_unpackhi_epi16(T00, T00);
        __m128i T10  = _mm_abs_epi16(T00);
        __m128i T20A = _mm_unpacklo_epi16(T10, QN2);
        __m128i T20B = _mm_unpackhi_epi16(T10, QN2);
        __m128i T30A = _mm_madd_epi16(T20A, QRN);
        __m128i T30B = _mm_madd_epi16(T20B, QRN);
        __m128i T40A = _mm_srai_epi32(T30A, iQBits);
        __m128i T40B = _mm_srai_epi32(T30B, iQBits);
        __m128i T50A = _mm_sign_epi32(T40A, T01A);
        __m128i T50B = _mm_sign_epi32(T40B, T01B);
        __m128i T60  = _mm_packs_epi32(T50A, T50B);
        acSumA = _mm_add_epi32(acSumA, T40A);
        acSumB = _mm_add_epi32(acSumB, T40B);
        _mm_storeu_si128((__m128i*)&pDst[y*nStride], T60);
    }
    acSumA = _mm_add_epi32(acSumA, acSumB);
    acSumA = _mm_hadd_epi32(acSumA, acSumA);
    acSumA = _mm_hadd_epi32(acSumA, acSumA);
    uiAcSum = _mm_cvtsi128_si32(acSumA);

    return uiAcSum;
}
UInt32 xQuant16( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    UInt32 uiAcSum = 0;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt uiQ = xg_quantScales[nQpMod6];
    Int iQBits = 17 + nQpDiv6;

    UInt32 iRnd1 = (eSType == SLICE_I ? 171 : 85)<<5;
    __m128i QRN = _mm_set1_epi32((iRnd1<<16) | uiQ);
    __m128i QN2 = _mm_set1_epi16(1<<(3+nQpDiv6));
    __m128i acSumA = _mm_setzero_si128();
    __m128i acSumB = _mm_setzero_si128();

    for( y=0; y<iHeight; y++ ) {
        __m128i T00A = _mm_loadu_si128((__m128i*)&pSrc[y*nStride  ]);
        __m128i T00B = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+8]);
        __m128i T01A = _mm_unpacklo_epi16(T00A, T00A);
        __m128i T01B = _mm_unpackhi_epi16(T00A, T00A);
        __m128i T01C = _mm_unpacklo_epi16(T00B, T00B);
        __m128i T01D = _mm_unpackhi_epi16(T00B, T00B);
        __m128i T10A = _mm_abs_epi16(T00A);
        __m128i T10B = _mm_abs_epi16(T00B);
        __m128i T20A = _mm_unpacklo_epi16(T10A, QN2);
        __m128i T20B = _mm_unpackhi_epi16(T10A, QN2);
        __m128i T20C = _mm_unpacklo_epi16(T10B, QN2);
        __m128i T20D = _mm_unpackhi_epi16(T10B, QN2);
        __m128i T30A = _mm_madd_epi16(T20A, QRN);
        __m128i T30B = _mm_madd_epi16(T20B, QRN);
        __m128i T30C = _mm_madd_epi16(T20C, QRN);
        __m128i T30D = _mm_madd_epi16(T20D, QRN);
        __m128i T40A = _mm_srai_epi32(T30A, iQBits);
        __m128i T40B = _mm_srai_epi32(T30B, iQBits);
        __m128i T40C = _mm_srai_epi32(T30C, iQBits);
        __m128i T40D = _mm_srai_epi32(T30D, iQBits);
        __m128i T50A = _mm_sign_epi32(T40A, T01A);
        __m128i T50B = _mm_sign_epi32(T40B, T01B);
        __m128i T50C = _mm_sign_epi32(T40C, T01C);
        __m128i T50D = _mm_sign_epi32(T40D, T01D);
        __m128i T60A = _mm_packs_epi32(T50A, T50B);
        __m128i T60B = _mm_packs_epi32(T50C, T50D);
        _mm_storeu_si128((__m128i*)&pDst[y*nStride  ], T60A);
        _mm_storeu_si128((__m128i*)&pDst[y*nStride+8], T60B);
        acSumA = _mm_add_epi32(acSumA, _mm_add_epi32(T40A, T40B));
        acSumB = _mm_add_epi32(acSumB, _mm_add_epi32(T40C, T40D));
    }
    acSumA = _mm_add_epi32(acSumA, acSumB);
    acSumA = _mm_hadd_epi32(acSumA, acSumA);
    acSumA = _mm_hadd_epi32(acSumA, acSumA);
    uiAcSum = _mm_cvtsi128_si32(acSumA);

    return uiAcSum;
}
UInt32 xQuant32( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    UInt32 uiAcSum = 0;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt uiQ = xg_quantScales[nQpMod6];
    Int iQBits = 16 + nQpDiv6;

    UInt32 iRnd1 = (eSType == SLICE_I ? 171 : 85)<<4;
    __m128i QRN = _mm_set1_epi32((iRnd1<<16) | uiQ);
    __m128i QN2 = _mm_set1_epi16(1<<(3+nQpDiv6));
    __m128i acSumA = _mm_setzero_si128();
    __m128i acSumB = _mm_setzero_si128();

    for( y=0; y<iHeight; y++ ) {
        #pragma unroll(2)
        for( x=0; x<2; x++ ) {
            __m128i T00A = _mm_loadu_si128((__m128i*)&pSrc[y*nStride  +x*16]);
            __m128i T00B = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+8+x*16]);
            __m128i T01A = _mm_unpacklo_epi16(T00A, T00A);
            __m128i T01B = _mm_unpackhi_epi16(T00A, T00A);
            __m128i T01C = _mm_unpacklo_epi16(T00B, T00B);
            __m128i T01D = _mm_unpackhi_epi16(T00B, T00B);
            __m128i T10A = _mm_abs_epi16(T00A);
            __m128i T10B = _mm_abs_epi16(T00B);
            __m128i T20A = _mm_unpacklo_epi16(T10A, QN2);
            __m128i T20B = _mm_unpackhi_epi16(T10A, QN2);
            __m128i T20C = _mm_unpacklo_epi16(T10B, QN2);
            __m128i T20D = _mm_unpackhi_epi16(T10B, QN2);
            __m128i T30A = _mm_madd_epi16(T20A, QRN);
            __m128i T30B = _mm_madd_epi16(T20B, QRN);
            __m128i T30C = _mm_madd_epi16(T20C, QRN);
            __m128i T30D = _mm_madd_epi16(T20D, QRN);
            __m128i T40A = _mm_srai_epi32(T30A, iQBits);
            __m128i T40B = _mm_srai_epi32(T30B, iQBits);
            __m128i T40C = _mm_srai_epi32(T30C, iQBits);
            __m128i T40D = _mm_srai_epi32(T30D, iQBits);
            __m128i T50A = _mm_sign_epi32(T40A, T01A);
            __m128i T50B = _mm_sign_epi32(T40B, T01B);
            __m128i T50C = _mm_sign_epi32(T40C, T01C);
            __m128i T50D = _mm_sign_epi32(T40D, T01D);
            __m128i T60A = _mm_packs_epi32(T50A, T50B);
            __m128i T60B = _mm_packs_epi32(T50C, T50D);
            _mm_storeu_si128((__m128i*)&pDst[y*nStride  +x*16], T60A);
            _mm_storeu_si128((__m128i*)&pDst[y*nStride+8+x*16], T60B);
            acSumA = _mm_add_epi32(acSumA, _mm_add_epi32(T40A, T40B));
            acSumB = _mm_add_epi32(acSumB, _mm_add_epi32(T40C, T40D));
        }
    }
    acSumA = _mm_add_epi32(acSumA, acSumB);
    acSumA = _mm_hadd_epi32(acSumA, acSumA);
    acSumA = _mm_hadd_epi32(acSumA, acSumA);
    uiAcSum += _mm_cvtsi128_si32(acSumA);
    return uiAcSum;
}
typedef UInt32 (*Quant_ptr)( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType );
Quant_ptr xQuantN[4] = {
    xQuant4,
    xQuant8,
    xQuant16,
    xQuant32,
};
UInt32 xQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, xSliceType eSType )
{
    return xQuantN[xLog2(iWidth-1)-2](pDst, pSrc, nStride, nQP, iHeight, eSType);
}

#else // ASM_NONE
UInt32 xQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, xSliceType eSType )
{
    int x, y;
    UInt32 uiAcSum = 0;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt uiLog2TrSize = xLog2( iWidth - 1);
    UInt uiQ = xg_quantScales[nQpMod6];
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - uiLog2TrSize;  // Represents scaling through forward transform
    Int iQBits = QUANT_SHIFT + nQpDiv6 + iTransformShift;

    Int32 iRnd = (eSType == SLICE_I ? 171 : 85) << (iQBits-9);

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < iWidth; x++ ) {
            Int iLevel;
            Int  iSign;
            UInt nBlockPos = y * nStride + x;
            iLevel  = pSrc[nBlockPos];
            iSign   = (iLevel < 0 ? -1: 1);

            iLevel = ( abs(iLevel) * uiQ + iRnd ) >> iQBits;
            uiAcSum += iLevel;
            iLevel *= iSign;
            pDst[nBlockPos] = iLevel;
            pDst[nBlockPos] = Clip3(-32768, 32767, pDst[nBlockPos]);
        }
    }
    return uiAcSum;
}
#endif

#if (USE_ASM >= ASM_SSE4)
void xDeQuant4( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - 2;  // Represents scaling through forward transform
    Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
    Int32 iRnd = 1 << (nShift-1);
    Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < 4; x++ ) {
            UInt nBlockPos = y * nStride + x;
            Int32 iCoeffQ = ( pSrc[nBlockPos] * iScale + iRnd ) >> nShift;
            pDst[nBlockPos] = Clip3(-32768, 32767, iCoeffQ);
        }
    }
}
void xDeQuant8( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - 3;  // Represents scaling through forward transform
    Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
    Int32 iRnd = 1 << (nShift-1);
    Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < 8; x++ ) {
            UInt nBlockPos = y * nStride + x;
            Int32 iCoeffQ = ( pSrc[nBlockPos] * iScale + iRnd ) >> nShift;
            pDst[nBlockPos] = Clip3(-32768, 32767, iCoeffQ);
        }
    }
}
void xDeQuant16( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - 4;  // Represents scaling through forward transform
    Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
    Int32 iRnd = 1 << (nShift-1);
    Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < 16; x++ ) {
            UInt nBlockPos = y * nStride + x;
            Int32 iCoeffQ = ( pSrc[nBlockPos] * iScale + iRnd ) >> nShift;
            pDst[nBlockPos] = Clip3(-32768, 32767, iCoeffQ);
        }
    }
}
void xDeQuant32( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int x, y;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - 5;  // Represents scaling through forward transform
    Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
    Int32 iRnd = 1 << (nShift-1);
    Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < 32; x++ ) {
            UInt nBlockPos = y * nStride + x;
            Int32 iCoeffQ = ( pSrc[nBlockPos] * iScale + iRnd ) >> nShift;
            pDst[nBlockPos] = Clip3(-32768, 32767, iCoeffQ);
        }
    }
}
typedef void (*DeQuant_ptr)( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType );
DeQuant_ptr xDeQuantN[4] = {
    xDeQuant4,
    xDeQuant8,
    xDeQuant16,
    xDeQuant32,
};
void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, xSliceType eSType )
{
    xDeQuantN[xLog2(iWidth-1)-2](pDst, pSrc, nStride, nQP, iHeight, eSType);
}
#else // ASM_NONE
void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, xSliceType eSType )
{
    int x, y;
    CUInt nQpDiv6 = nQP / 6;
    CUInt nQpMod6 = nQP % 6;
    UInt uiLog2TrSize = xLog2( iWidth - 1 );
    UInt iTransformShift = MAX_TR_DYNAMIC_RANGE - 8 - uiLog2TrSize;  // Represents scaling through forward transform
    Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - iTransformShift;
    Int32 iRnd = 1 << (nShift-1);
    Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

    for( y=0; y < iHeight; y++ ) {
        for( x=0; x < iWidth; x++ ) {
            UInt nBlockPos = y * nStride + x;
            Int32 iCoeffQ = ( pSrc[nBlockPos] * iScale + iRnd ) >> nShift;
            pDst[nBlockPos] = Clip3(-32768, 32767, iCoeffQ);
        }
    }
}
#endif

// ***************************************************************************
// * DCT/IDCT Functions
// ***************************************************************************
#if (USE_ASM >= ASM_SSE4)
void xSubDCT4(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    // Const
    __m128i zero        = _mm_setzero_si128();
    __m128i c_1         = _mm_set1_epi32(1);
    __m128i c_128       = _mm_set1_epi32(128);
    __m128i c16_64_64   = _mm_set1_epi32(0x00400040);
    __m128i c16_n64_64  = _mm_set1_epi32(0xFFC00040);
    __m128i c16_36_83   = _mm_set1_epi32(0x00240053);
    __m128i c16_n83_36  = _mm_set1_epi32(0xFFAD0024);
    __m128i c32_64_64   = _mm_set_epi32( 64, 64, 64, 64);
    __m128i c32_36_83   = _mm_set_epi32( 36, 83, 36, 83);
    __m128i c32_n64_64  = _mm_set_epi32(-64, 64,-64, 64);
    __m128i c32_n83_36  = _mm_set_epi32(-83, 36,-83, 36);

    // Sub
    __m128i T00A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[0*nStride]);
    __m128i T01A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[1*nStride]);
    __m128i T02A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[2*nStride]);
    __m128i T03A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[3*nStride]);
    __m128i T00B = _mm_cvtsi32_si128(*(UInt32*)&pRef[0*nStrideRef]);
    __m128i T01B = _mm_cvtsi32_si128(*(UInt32*)&pRef[1*nStrideRef]);
    __m128i T02B = _mm_cvtsi32_si128(*(UInt32*)&pRef[2*nStrideRef]);
    __m128i T03B = _mm_cvtsi32_si128(*(UInt32*)&pRef[3*nStrideRef]);
    __m128i T10A = _mm_unpacklo_epi8(T00A, zero);
    __m128i T11A = _mm_unpacklo_epi8(T01A, zero);
    __m128i T12A = _mm_unpacklo_epi8(T02A, zero);
    __m128i T13A = _mm_unpacklo_epi8(T03A, zero);
    __m128i T10B = _mm_unpacklo_epi8(T00B, zero);
    __m128i T11B = _mm_unpacklo_epi8(T01B, zero);
    __m128i T12B = _mm_unpacklo_epi8(T02B, zero);
    __m128i T13B = _mm_unpacklo_epi8(T03B, zero);
    __m128i T20  = _mm_sub_epi16(T10A, T10B);       // [03 02 01 00]
    __m128i T21  = _mm_sub_epi16(T11A, T11B);       // [13 12 11 10]
    __m128i T22  = _mm_sub_epi16(T12A, T12B);       // [23 22 21 20]
    __m128i T23  = _mm_sub_epi16(T13A, T13B);       // [33 32 31 30]

    // DCT1
    __m128i T30  = _mm_unpacklo_epi32(T20, T21);        // [13 12 03 02 11 10 01 00]
    __m128i T31  = _mm_unpacklo_epi32(T22, T23);        // [33 32 23 22 31 30 21 20]
    __m128i T32  = _mm_shufflehi_epi16(T30, 0xB1);      // [12 13 02 03 11 10 01 00]
    __m128i T33  = _mm_shufflehi_epi16(T31, 0xB1);      // [32 33 22 23 31 30 21 20]
    __m128i T40  = _mm_unpacklo_epi64(T32, T33);        // [31 30 21 20 11 10 01 00]
    __m128i T41  = _mm_unpackhi_epi64(T32, T33);        // [32 33 22 23 12 13 02 03]
    __m128i T50  = _mm_add_epi16(T40, T41);             // [1+2 0+3]
    __m128i T51  = _mm_sub_epi16(T40, T41);             // [1-2 0-3]
    __m128i T60  = _mm_madd_epi16(c16_64_64,  T50);     // [ 64*s12 + 64*s03] = [03 02 01 00]
    __m128i T61  = _mm_madd_epi16(c16_36_83,  T51);     // [ 36*d12 + 83*d03] = [13 12 11 10]
    __m128i T62  = _mm_madd_epi16(c16_n64_64, T50);     // [-64*s12 + 64*s03] = [23 22 21 20]
    __m128i T63  = _mm_madd_epi16(c16_n83_36, T51);     // [-83*d12 + 36*d03] = [33 32 31 30]
    __m128i T70  = _mm_srai_epi32(_mm_add_epi32(c_1, T60), 1);  // [03 02 01 00]
    __m128i T71  = _mm_srai_epi32(_mm_add_epi32(c_1, T61), 1);  // [13 12 11 10]
    __m128i T72  = _mm_srai_epi32(_mm_add_epi32(c_1, T62), 1);  // [23 22 21 20]
    __m128i T73  = _mm_srai_epi32(_mm_add_epi32(c_1, T63), 1);  // [33 32 31 30]

    // DCT2
    __m128i T80  = _mm_unpacklo_epi64(T70, T71);    // [11 10 01 00]
    __m128i T81_ = _mm_unpackhi_epi64(T70, T71);    //                 [13 12 03 02]
    __m128i T81  = _mm_shuffle_epi32(T81_, 0xB1);   // [12 13 02 03]
    __m128i T82  = _mm_unpacklo_epi64(T72, T73);    // [31 30 21 20]
    __m128i T83_ = _mm_unpackhi_epi64(T72, T73);    //                 [33 32 23 22]
    __m128i T83  = _mm_shuffle_epi32(T83_, 0xB1);   // [32 33 22 23]
    __m128i T90A = _mm_add_epi32(T80, T81);         // [1+2 0+3] = [s12 s03]
    __m128i T90B = _mm_add_epi32(T82, T83);
    __m128i T91A = _mm_sub_epi32(T80, T81);         // [1-2 0-3] = [d12 d03]
    __m128i T91B = _mm_sub_epi32(T82, T83);
    __m128i TA0A = _mm_slli_epi32(T90A, 6);         // [ 64*s12 64*s03]
    __m128i TA0B = _mm_slli_epi32(T90B, 6);
    __m128i TA1A = _mm_mullo_epi32(c32_36_83,  T91A); // [ 36*d12 83*d03]
    __m128i TA1B = _mm_mullo_epi32(c32_36_83,  T91B);
    __m128i TA2A = _mm_mullo_epi32(c32_n64_64, T90A); // [-64*s12 64*s03]
    __m128i TA2B = _mm_mullo_epi32(c32_n64_64, T90B);
    __m128i TA3A = _mm_mullo_epi32(c32_n83_36, T91A); // [-83*d12 36*d03]
    __m128i TA3B = _mm_mullo_epi32(c32_n83_36, T91B);
    __m128i TB0  = _mm_hadd_epi32(TA0A, TA0B);      // [03 02 01 00]
    __m128i TB1  = _mm_hadd_epi32(TA1A, TA1B);      // [13 12 11 10]
    __m128i TB2  = _mm_hadd_epi32(TA2A, TA2B);      // [23 22 21 20]
    __m128i TB3  = _mm_hadd_epi32(TA3A, TA3B);      // [33 32 31 30]
    __m128i TC0  = _mm_srai_epi32(_mm_add_epi32(TB0, c_128), 8);
    __m128i TC1  = _mm_srai_epi32(_mm_add_epi32(TB1, c_128), 8);
    __m128i TC2  = _mm_srai_epi32(_mm_add_epi32(TB2, c_128), 8);
    __m128i TC3  = _mm_srai_epi32(_mm_add_epi32(TB3, c_128), 8);
    __m128i TD0  = _mm_packs_epi32(TC0, TC1);       // [13 12 11 10 03 02 01 00]
    __m128i TD1  = _mm_packs_epi32(TC2, TC3);       // [33 32 31 30 23 22 21 20]

    _mm_storel_epi64((__m128i*)&pDst[0*nStride], TD0);
    _mm_storeh_pi   ((__m64*  )&pDst[1*nStride], _mm_castsi128_ps(TD0));
    _mm_storel_epi64((__m128i*)&pDst[2*nStride], TD1);
    _mm_storeh_pi   ((__m64*  )&pDst[3*nStride], _mm_castsi128_ps(TD1));
}
#else // ASM_NONE
void xSubDCT4(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int nShift;
    int rnd;
    int i, j;

    // Sub
    for( i=0; i<4; i++ ) {
        for( j=0; j<4; j++ ) {
            piTmp0[i * 4 + j] = pSrc[i * nStride + j] - pRef[i * nStrideRef + j];
        }
    }

    // DCT1
    nShift = 2-1;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        /* Even and Odd */
        Int32 s03 = piTmp0[i*4+0] + piTmp0[i*4+3];
        Int32 d03 = piTmp0[i*4+0] - piTmp0[i*4+3];
        Int32 s12 = piTmp0[i*4+1] + piTmp0[i*4+2];
        Int32 d12 = piTmp0[i*4+1] - piTmp0[i*4+2];

        piTmp1[0*4+i] = ( 64*s03 + 64*s12 + rnd ) >> nShift;
        piTmp1[2*4+i] = ( 64*s03 - 64*s12 + rnd ) >> nShift;
        piTmp1[1*4+i] = ( 83*d03 + 36*d12 + rnd ) >> nShift;
        piTmp1[3*4+i] = ( 36*d03 - 83*d12 + rnd ) >> nShift;
    }

    // DCT2
    nShift = 2+6;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        /* Even and Odd */
        Int32 s03 = piTmp1[i*4+0] + piTmp1[i*4+3];
        Int32 d03 = piTmp1[i*4+0] - piTmp1[i*4+3];
        Int32 s12 = piTmp1[i*4+1] + piTmp1[i*4+2];
        Int32 d12 = piTmp1[i*4+1] - piTmp1[i*4+2];

        pDst[0*nStride+i] = ( 64*s03 + 64*s12 + rnd ) >> nShift;
        pDst[2*nStride+i] = ( 64*s03 - 64*s12 + rnd ) >> nShift;
        pDst[1*nStride+i] = ( 83*d03 + 36*d12 + rnd ) >> nShift;
        pDst[3*nStride+i] = ( 36*d03 - 83*d12 + rnd ) >> nShift;
    }
}
#endif

void xSubDST4(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int nShift;
    int rnd;
    int i, j;

    // Sub
    for( i=0; i<4; i++ ) {
        for( j=0; j<4; j++ ) {
            piTmp0[i * 4 + j] = pSrc[i * nStride + j] - pRef[i * nStrideRef + j];
        }
    }

    // DCT1
    nShift = 2-1;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        // Intermediate Variables
        Int32 c0 = piTmp0[i*4+0] + piTmp0[i*4+3];
        Int32 c1 = piTmp0[i*4+1] + piTmp0[i*4+3];
        Int32 c2 = piTmp0[i*4+0] - piTmp0[i*4+1];
        Int32 c3 = 74* piTmp0[i*4+2];
        Int32 c4 = (piTmp0[i*4+0]+ piTmp0[i*4+1] - piTmp0[i*4+3]);

        piTmp1[0*4+i] =  ( 29 * c0 + 55 * c1 + c3 + rnd ) >> nShift;
        piTmp1[1*4+i] =  ( 74 * c4                + rnd ) >> nShift;
        piTmp1[2*4+i] =  ( 29 * c2 + 55 * c0 - c3 + rnd ) >> nShift;
        piTmp1[3*4+i] =  ( 55 * c2 - 29 * c1 + c3 + rnd ) >> nShift;
    }

    // DCT2
    nShift = 2+6;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        // Intermediate Variables
        Int32 c0 = piTmp1[i*4+0] + piTmp1[i*4+3];
        Int32 c1 = piTmp1[i*4+1] + piTmp1[i*4+3];
        Int32 c2 = piTmp1[i*4+0] - piTmp1[i*4+1];
        Int32 c3 = 74* piTmp1[i*4+2];
        Int32 c4 = (piTmp1[i*4+0]+ piTmp1[i*4+1] - piTmp1[i*4+3]);

        pDst[0*nStride+i] =  ( 29 * c0 + 55 * c1 + c3 + rnd ) >> nShift;
        pDst[1*nStride+i] =  ( 74 * c4                + rnd ) >> nShift;
        pDst[2*nStride+i] =  ( 29 * c2 + 55 * c0 - c3 + rnd ) >> nShift;
        pDst[3*nStride+i] =  ( 55 * c2 - 29 * c1 + c3 + rnd ) >> nShift;
    }
}

void xSubDCT8(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int nShift;
    int rnd;
    int i, j;

    // Sub
    for( i=0; i<8; i++ ) {
        for( j=0; j<8; j++ ) {
            piTmp0[i * 8 + j] = pSrc[i * nStride + j] - pRef[i * nStrideRef + j];
        }
    }

    // DCT1
    nShift = 3-1;
    rnd    = 1<<(nShift-1);
    for( i=0; i<8; i++ ) {
        /* Even and Odd */
        Int32 s07 = piTmp0[i*8+0] + piTmp0[i*8+7];
        Int32 d07 = piTmp0[i*8+0] - piTmp0[i*8+7];
        Int32 s16 = piTmp0[i*8+1] + piTmp0[i*8+6];
        Int32 d16 = piTmp0[i*8+1] - piTmp0[i*8+6];
        Int32 s25 = piTmp0[i*8+2] + piTmp0[i*8+5];
        Int32 d25 = piTmp0[i*8+2] - piTmp0[i*8+5];
        Int32 s34 = piTmp0[i*8+3] + piTmp0[i*8+4];
        Int32 d34 = piTmp0[i*8+3] - piTmp0[i*8+4];

        /* EE and EO */
        Int32 EE0 = s07 + s34;
        Int32 EO0 = s07 - s34;
        Int32 EE1 = s16 + s25;
        Int32 EO1 = s16 - s25;

        piTmp1[0*8+i] = (64*EE0 + 64*EE1 + rnd) >> nShift;
        piTmp1[4*8+i] = (64*EE0 - 64*EE1 + rnd) >> nShift;
        piTmp1[2*8+i] = (83*EO0 + 36*EO1 + rnd) >> nShift;
        piTmp1[6*8+i] = (36*EO0 - 83*EO1 + rnd) >> nShift;

        piTmp1[1*8+i] = (89*d07 + 75*d16 + 50*d25 + 18*d34 + rnd) >> nShift;
        piTmp1[3*8+i] = (75*d07 - 18*d16 - 89*d25 - 50*d34 + rnd) >> nShift;
        piTmp1[5*8+i] = (50*d07 - 89*d16 + 18*d25 + 75*d34 + rnd) >> nShift;
        piTmp1[7*8+i] = (18*d07 - 50*d16 + 75*d25 - 89*d34 + rnd) >> nShift;
    }

    // DCT2
    nShift = 3+6;
    rnd    = 1<<(nShift-1);
    for( i=0; i<8; i++ ) {
        /* Even and Odd */
        Int32 s07 = piTmp1[i*8+0] + piTmp1[i*8+7];
        Int32 d07 = piTmp1[i*8+0] - piTmp1[i*8+7];
        Int32 s16 = piTmp1[i*8+1] + piTmp1[i*8+6];
        Int32 d16 = piTmp1[i*8+1] - piTmp1[i*8+6];
        Int32 s25 = piTmp1[i*8+2] + piTmp1[i*8+5];
        Int32 d25 = piTmp1[i*8+2] - piTmp1[i*8+5];
        Int32 s34 = piTmp1[i*8+3] + piTmp1[i*8+4];
        Int32 d34 = piTmp1[i*8+3] - piTmp1[i*8+4];

        /* EE and EO */
        Int32 EE0 = s07 + s34;
        Int32 EO0 = s07 - s34;
        Int32 EE1 = s16 + s25;
        Int32 EO1 = s16 - s25;

        pDst[0*nStride+i] = (64*EE0 + 64*EE1 + rnd) >> nShift;
        pDst[4*nStride+i] = (64*EE0 - 64*EE1 + rnd) >> nShift;
        pDst[2*nStride+i] = (83*EO0 + 36*EO1 + rnd) >> nShift;
        pDst[6*nStride+i] = (36*EO0 - 83*EO1 + rnd) >> nShift;

        pDst[1*nStride+i] = (89*d07 + 75*d16 + 50*d25 + 18*d34 + rnd) >> nShift;
        pDst[3*nStride+i] = (75*d07 - 18*d16 - 89*d25 - 50*d34 + rnd) >> nShift;
        pDst[5*nStride+i] = (50*d07 - 89*d16 + 18*d25 + 75*d34 + rnd) >> nShift;
        pDst[7*nStride+i] = (18*d07 - 50*d16 + 75*d25 - 89*d34 + rnd) >> nShift;
    }
}

void xSubDCT16(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int nShift;
    int rnd;
    int i, j;

    // Sub
    for( i=0; i<16; i++ ) {
        for( j=0; j<16; j++ ) {
            piTmp0[i * 16 + j] = pSrc[i * nStride + j] - pRef[i * nStrideRef + j];
        }
    }

    // DCT1
    nShift = 4-1;
    rnd    = 1<<(nShift-1);
    for( i=0; i<16; i++ ) {
        /* Even and Odd */
        Int32 E0 = piTmp0[i*16+0] + piTmp0[i*16+15];
        Int32 O0 = piTmp0[i*16+0] - piTmp0[i*16+15];
        Int32 E1 = piTmp0[i*16+1] + piTmp0[i*16+14];
        Int32 O1 = piTmp0[i*16+1] - piTmp0[i*16+14];
        Int32 E2 = piTmp0[i*16+2] + piTmp0[i*16+13];
        Int32 O2 = piTmp0[i*16+2] - piTmp0[i*16+13];
        Int32 E3 = piTmp0[i*16+3] + piTmp0[i*16+12];
        Int32 O3 = piTmp0[i*16+3] - piTmp0[i*16+12];
        Int32 E4 = piTmp0[i*16+4] + piTmp0[i*16+11];
        Int32 O4 = piTmp0[i*16+4] - piTmp0[i*16+11];
        Int32 E5 = piTmp0[i*16+5] + piTmp0[i*16+10];
        Int32 O5 = piTmp0[i*16+5] - piTmp0[i*16+10];
        Int32 E6 = piTmp0[i*16+6] + piTmp0[i*16+ 9];
        Int32 O6 = piTmp0[i*16+6] - piTmp0[i*16+ 9];
        Int32 E7 = piTmp0[i*16+7] + piTmp0[i*16+ 8];
        Int32 O7 = piTmp0[i*16+7] - piTmp0[i*16+ 8];

        /* EE and EO */
        Int32 EE0 = E0 + E7;
        Int32 EO0 = E0 - E7;
        Int32 EE1 = E1 + E6;
        Int32 EO1 = E1 - E6;
        Int32 EE2 = E2 + E5;
        Int32 EO2 = E2 - E5;
        Int32 EE3 = E3 + E4;
        Int32 EO3 = E3 - E4;

        /* EEE and EEO */
        Int32 EEE0 = EE0 + EE3;
        Int32 EEO0 = EE0 - EE3;
        Int32 EEE1 = EE1 + EE2;
        Int32 EEO1 = EE1 - EE2;

        piTmp1[ 0*16+i] = (64*EEE0 + 64*EEE1 + rnd) >> nShift;
        piTmp1[ 8*16+i] = (64*EEE0 - 64*EEE1 + rnd) >> nShift;
        piTmp1[ 4*16+i] = (83*EEO0 + 36*EEO1 + rnd) >> nShift;
        piTmp1[12*16+i] = (36*EEO0 - 83*EEO1 + rnd) >> nShift;

        piTmp1[ 2*16+i] = (89*EO0 + 75*EO1 + 50*EO2 + 18*EO3 + rnd) >> nShift;
        piTmp1[ 6*16+i] = (75*EO0 - 18*EO1 - 89*EO2 - 50*EO3 + rnd) >> nShift;
        piTmp1[10*16+i] = (50*EO0 - 89*EO1 + 18*EO2 + 75*EO3 + rnd) >> nShift;
        piTmp1[14*16+i] = (18*EO0 - 50*EO1 + 75*EO2 - 89*EO3 + rnd) >> nShift;

        piTmp1[ 1*16+i] = (90*O0 + 87*O1 + 80*O2 + 70*O3 + 57*O4 + 43*O5 + 25*O6 +  9*O7 + rnd) >> nShift;
        piTmp1[ 3*16+i] = (87*O0 + 57*O1 +  9*O2 - 43*O3 - 80*O4 - 90*O5 - 70*O6 - 25*O7 + rnd) >> nShift;
        piTmp1[ 5*16+i] = (80*O0 +  9*O1 - 70*O2 - 87*O3 - 25*O4 + 57*O5 + 90*O6 + 43*O7 + rnd) >> nShift;
        piTmp1[ 7*16+i] = (70*O0 - 43*O1 - 87*O2 +  9*O3 + 90*O4 + 25*O5 - 80*O6 - 57*O7 + rnd) >> nShift;
        piTmp1[ 9*16+i] = (57*O0 - 80*O1 - 25*O2 + 90*O3 -  9*O4 - 87*O5 + 43*O6 + 70*O7 + rnd) >> nShift;
        piTmp1[11*16+i] = (43*O0 - 90*O1 + 57*O2 + 25*O3 - 87*O4 + 70*O5 +  9*O6 - 80*O7 + rnd) >> nShift;
        piTmp1[13*16+i] = (25*O0 - 70*O1 + 90*O2 - 80*O3 + 43*O4 +  9*O5 - 57*O6 + 87*O7 + rnd) >> nShift;
        piTmp1[15*16+i] = ( 9*O0 - 25*O1 + 43*O2 - 57*O3 + 70*O4 - 80*O5 + 87*O6 - 90*O7 + rnd) >> nShift;
    }

    // DCT2
    nShift = 4+6;
    rnd    = 1<<(nShift-1);
    for( i=0; i<16; i++ ) {
        /* Even and Odd */
        Int32 E0 = piTmp1[i*16+0] + piTmp1[i*16+15];
        Int32 O0 = piTmp1[i*16+0] - piTmp1[i*16+15];
        Int32 E1 = piTmp1[i*16+1] + piTmp1[i*16+14];
        Int32 O1 = piTmp1[i*16+1] - piTmp1[i*16+14];
        Int32 E2 = piTmp1[i*16+2] + piTmp1[i*16+13];
        Int32 O2 = piTmp1[i*16+2] - piTmp1[i*16+13];
        Int32 E3 = piTmp1[i*16+3] + piTmp1[i*16+12];
        Int32 O3 = piTmp1[i*16+3] - piTmp1[i*16+12];
        Int32 E4 = piTmp1[i*16+4] + piTmp1[i*16+11];
        Int32 O4 = piTmp1[i*16+4] - piTmp1[i*16+11];
        Int32 E5 = piTmp1[i*16+5] + piTmp1[i*16+10];
        Int32 O5 = piTmp1[i*16+5] - piTmp1[i*16+10];
        Int32 E6 = piTmp1[i*16+6] + piTmp1[i*16+ 9];
        Int32 O6 = piTmp1[i*16+6] - piTmp1[i*16+ 9];
        Int32 E7 = piTmp1[i*16+7] + piTmp1[i*16+ 8];
        Int32 O7 = piTmp1[i*16+7] - piTmp1[i*16+ 8];

        /* EE and EO */
        Int32 EE0 = E0 + E7;
        Int32 EO0 = E0 - E7;
        Int32 EE1 = E1 + E6;
        Int32 EO1 = E1 - E6;
        Int32 EE2 = E2 + E5;
        Int32 EO2 = E2 - E5;
        Int32 EE3 = E3 + E4;
        Int32 EO3 = E3 - E4;

        /* EEE and EEO */
        Int32 EEE0 = EE0 + EE3;
        Int32 EEO0 = EE0 - EE3;
        Int32 EEE1 = EE1 + EE2;
        Int32 EEO1 = EE1 - EE2;

        pDst[ 0*nStride+i] = (64*EEE0 + 64*EEE1 + rnd) >> nShift;
        pDst[ 8*nStride+i] = (64*EEE0 - 64*EEE1 + rnd) >> nShift;
        pDst[ 4*nStride+i] = (83*EEO0 + 36*EEO1 + rnd) >> nShift;
        pDst[12*nStride+i] = (36*EEO0 - 83*EEO1 + rnd) >> nShift;

        pDst[ 2*nStride+i] = (89*EO0 + 75*EO1 + 50*EO2 + 18*EO3 + rnd) >> nShift;
        pDst[ 6*nStride+i] = (75*EO0 - 18*EO1 - 89*EO2 - 50*EO3 + rnd) >> nShift;
        pDst[10*nStride+i] = (50*EO0 - 89*EO1 + 18*EO2 + 75*EO3 + rnd) >> nShift;
        pDst[14*nStride+i] = (18*EO0 - 50*EO1 + 75*EO2 - 89*EO3 + rnd) >> nShift;

        pDst[ 1*nStride+i] = (90*O0 + 87*O1 + 80*O2 + 70*O3 + 57*O4 + 43*O5 + 25*O6 +  9*O7 + rnd) >> nShift;
        pDst[ 3*nStride+i] = (87*O0 + 57*O1 +  9*O2 - 43*O3 - 80*O4 - 90*O5 - 70*O6 - 25*O7 + rnd) >> nShift;
        pDst[ 5*nStride+i] = (80*O0 +  9*O1 - 70*O2 - 87*O3 - 25*O4 + 57*O5 + 90*O6 + 43*O7 + rnd) >> nShift;
        pDst[ 7*nStride+i] = (70*O0 - 43*O1 - 87*O2 +  9*O3 + 90*O4 + 25*O5 - 80*O6 - 57*O7 + rnd) >> nShift;
        pDst[ 9*nStride+i] = (57*O0 - 80*O1 - 25*O2 + 90*O3 -  9*O4 - 87*O5 + 43*O6 + 70*O7 + rnd) >> nShift;
        pDst[11*nStride+i] = (43*O0 - 90*O1 + 57*O2 + 25*O3 - 87*O4 + 70*O5 +  9*O6 - 80*O7 + rnd) >> nShift;
        pDst[13*nStride+i] = (25*O0 - 70*O1 + 90*O2 - 80*O3 + 43*O4 +  9*O5 - 57*O6 + 87*O7 + rnd) >> nShift;
        pDst[15*nStride+i] = ( 9*O0 - 25*O1 + 43*O2 - 57*O3 + 70*O4 - 80*O5 + 87*O6 - 90*O7 + rnd) >> nShift;
    }
}

void xSubDCT32(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int nShift;
    int rnd;
    int i, j;
    Int32 E[16],O[16];
    Int32 EE[8],EO[8];
    Int32 EEE[4],EEO[4];
    Int32 EEEE[2],EEEO[2];

    // Sub
    for( i=0; i<32; i++ ) {
        for( j=0; j<32; j++ ) {
            piTmp0[i * 32 + j] = pSrc[i * nStride + j] - pRef[i * nStrideRef + j];
        }
    }

    // DCT1
    nShift = 5-1;
    rnd    = 1<<(nShift-1);
    for( i=0; i<32; i++ ) {
        /* E and O */
        E[ 0] = piTmp0[i*32+ 0] + piTmp0[i*32+31];
        O[ 0] = piTmp0[i*32+ 0] - piTmp0[i*32+31];
        E[ 1] = piTmp0[i*32+ 1] + piTmp0[i*32+30];
        O[ 1] = piTmp0[i*32+ 1] - piTmp0[i*32+30];
        E[ 2] = piTmp0[i*32+ 2] + piTmp0[i*32+29];
        O[ 2] = piTmp0[i*32+ 2] - piTmp0[i*32+29];
        E[ 3] = piTmp0[i*32+ 3] + piTmp0[i*32+28];
        O[ 3] = piTmp0[i*32+ 3] - piTmp0[i*32+28];
        E[ 4] = piTmp0[i*32+ 4] + piTmp0[i*32+27];
        O[ 4] = piTmp0[i*32+ 4] - piTmp0[i*32+27];
        E[ 5] = piTmp0[i*32+ 5] + piTmp0[i*32+26];
        O[ 5] = piTmp0[i*32+ 5] - piTmp0[i*32+26];
        E[ 6] = piTmp0[i*32+ 6] + piTmp0[i*32+25];
        O[ 6] = piTmp0[i*32+ 6] - piTmp0[i*32+25];
        E[ 7] = piTmp0[i*32+ 7] + piTmp0[i*32+24];
        O[ 7] = piTmp0[i*32+ 7] - piTmp0[i*32+24];
        E[ 8] = piTmp0[i*32+ 8] + piTmp0[i*32+23];
        O[ 8] = piTmp0[i*32+ 8] - piTmp0[i*32+23];
        E[ 9] = piTmp0[i*32+ 9] + piTmp0[i*32+22];
        O[ 9] = piTmp0[i*32+ 9] - piTmp0[i*32+22];
        E[10] = piTmp0[i*32+10] + piTmp0[i*32+21];
        O[10] = piTmp0[i*32+10] - piTmp0[i*32+21];
        E[11] = piTmp0[i*32+11] + piTmp0[i*32+20];
        O[11] = piTmp0[i*32+11] - piTmp0[i*32+20];
        E[12] = piTmp0[i*32+12] + piTmp0[i*32+19];
        O[12] = piTmp0[i*32+12] - piTmp0[i*32+19];
        E[13] = piTmp0[i*32+13] + piTmp0[i*32+18];
        O[13] = piTmp0[i*32+13] - piTmp0[i*32+18];
        E[14] = piTmp0[i*32+14] + piTmp0[i*32+17];
        O[14] = piTmp0[i*32+14] - piTmp0[i*32+17];
        E[15] = piTmp0[i*32+15] + piTmp0[i*32+16];
        O[15] = piTmp0[i*32+15] - piTmp0[i*32+16];

        /* EE and EO */
        EE[0] = E[0] + E[15];
        EO[0] = E[0] - E[15];
        EE[1] = E[1] + E[14];
        EO[1] = E[1] - E[14];
        EE[2] = E[2] + E[13];
        EO[2] = E[2] - E[13];
        EE[3] = E[3] + E[12];
        EO[3] = E[3] - E[12];
        EE[4] = E[4] + E[11];
        EO[4] = E[4] - E[11];
        EE[5] = E[5] + E[10];
        EO[5] = E[5] - E[10];
        EE[6] = E[6] + E[ 9];
        EO[6] = E[6] - E[ 9];
        EE[7] = E[7] + E[ 8];
        EO[7] = E[7] - E[ 8];

        /* EEE and EEO */
        EEE[0] = EE[0] + EE[7];
        EEO[0] = EE[0] - EE[7];
        EEE[1] = EE[1] + EE[6];
        EEO[1] = EE[1] - EE[6];
        EEE[2] = EE[2] + EE[5];
        EEO[2] = EE[2] - EE[5];
        EEE[3] = EE[3] + EE[4];
        EEO[3] = EE[3] - EE[4];

        /* EEEE and EEEO */
        EEEE[0] = EEE[0] + EEE[3];
        EEEO[0] = EEE[0] - EEE[3];
        EEEE[1] = EEE[1] + EEE[2];
        EEEO[1] = EEE[1] - EEE[2];

        // 0, 8, 16, 24
        piTmp1[ 0*32+i] = (64*EEEE[0] + 64*EEEE[1] + rnd) >> nShift;
        piTmp1[16*32+i] = (64*EEEE[0] - 64*EEEE[1] + rnd) >> nShift;
        piTmp1[ 8*32+i] = (83*EEEO[0] + 36*EEEO[1] + rnd) >> nShift;
        piTmp1[24*32+i] = (36*EEEO[0] - 83*EEEO[1] + rnd) >> nShift;

        // 4, 12, 20, 28
        piTmp1[ 4*32+i] = (89*EEO[0] + 75*EEO[1] + 50*EEO[2] + 18*EEO[3] + rnd) >> nShift;
        piTmp1[12*32+i] = (75*EEO[0] - 18*EEO[1] - 89*EEO[2] - 50*EEO[3] + rnd) >> nShift;
        piTmp1[20*32+i] = (50*EEO[0] - 89*EEO[1] + 18*EEO[2] + 75*EEO[3] + rnd) >> nShift;
        piTmp1[28*32+i] = (18*EEO[0] - 50*EEO[1] + 75*EEO[2] - 89*EEO[3] + rnd) >> nShift;

        // 2, 6, 10, 14, 18, 22, 26, 30
        piTmp1[ 2*32+i] = (90*EO[0] + 87*EO[1] + 80*EO[2] + 70*EO[3] + 57*EO[4] + 43*EO[5] + 25*EO[6] +  9*EO[7] + rnd) >> nShift;
        piTmp1[ 6*32+i] = (87*EO[0] + 57*EO[1] +  9*EO[2] - 43*EO[3] - 80*EO[4] - 90*EO[5] - 70*EO[6] - 25*EO[7] + rnd) >> nShift;
        piTmp1[10*32+i] = (80*EO[0] +  9*EO[1] - 70*EO[2] - 87*EO[3] - 25*EO[4] + 57*EO[5] + 90*EO[6] + 43*EO[7] + rnd) >> nShift;
        piTmp1[14*32+i] = (70*EO[0] - 43*EO[1] - 87*EO[2] +  9*EO[3] + 90*EO[4] + 25*EO[5] - 80*EO[6] - 57*EO[7] + rnd) >> nShift;
        piTmp1[18*32+i] = (57*EO[0] - 80*EO[1] - 25*EO[2] + 90*EO[3] -  9*EO[4] - 87*EO[5] + 43*EO[6] + 70*EO[7] + rnd) >> nShift;
        piTmp1[22*32+i] = (43*EO[0] - 90*EO[1] + 57*EO[2] + 25*EO[3] - 87*EO[4] + 70*EO[5] +  9*EO[6] - 80*EO[7] + rnd) >> nShift;
        piTmp1[26*32+i] = (25*EO[0] - 70*EO[1] + 90*EO[2] - 80*EO[3] + 43*EO[4] +  9*EO[5] - 57*EO[6] + 87*EO[7] + rnd) >> nShift;
        piTmp1[30*32+i] = ( 9*EO[0] - 25*EO[1] + 43*EO[2] - 57*EO[3] + 70*EO[4] - 80*EO[5] + 87*EO[6] - 90*EO[7] + rnd) >> nShift;

        // 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31
        piTmp1[ 1*32+i] = (90*O[ 0] + 90*O[ 1] + 88*O[ 2] + 85*O[ 3] + 82*O[ 4] + 78*O[ 5] + 73*O[ 6] + 67*O[ 7] +
                           61*O[ 8] + 54*O[ 9] + 46*O[10] + 38*O[11] + 31*O[12] + 22*O[13] + 13*O[14] +  4*O[15] + rnd) >> nShift;
        piTmp1[ 3*32+i] = (90*O[ 0] + 82*O[ 1] + 67*O[ 2] + 46*O[ 3] + 22*O[ 4] -  4*O[ 5] - 31*O[ 6] - 54*O[ 7] -
                           73*O[ 8] - 85*O[ 9] - 90*O[10] - 88*O[11] - 78*O[12] - 61*O[13] - 38*O[14] - 13*O[15] + rnd) >> nShift;
        piTmp1[ 5*32+i] = (88*O[ 0] + 67*O[ 1] + 31*O[ 2] - 13*O[ 3] - 54*O[ 4] - 82*O[ 5] - 90*O[ 6] - 78*O[ 7] -
                           46*O[ 8] -  4*O[ 9] + 38*O[10] + 73*O[11] + 90*O[12] + 85*O[13] + 61*O[14] + 22*O[15] + rnd) >> nShift;
        piTmp1[ 7*32+i] = (85*O[ 0] + 46*O[ 1] - 13*O[ 2] - 67*O[ 3] - 90*O[ 4] - 73*O[ 5] - 22*O[ 6] + 38*O[ 7] +
                           82*O[ 8] + 88*O[ 9] + 54*O[10] -  4*O[11] - 61*O[12] - 90*O[13] - 78*O[14] - 31*O[15] + rnd) >> nShift;
        piTmp1[ 9*32+i] = (82*O[ 0] + 22*O[ 1] - 54*O[ 2] - 90*O[ 3] - 61*O[ 4] + 13*O[ 5] + 78*O[ 6] + 85*O[ 7] +
                           31*O[ 8] - 46*O[ 9] - 90*O[10] - 67*O[11] +  4*O[12] + 73*O[13] + 88*O[14] + 38*O[15] + rnd) >> nShift;
        piTmp1[11*32+i] = (78*O[ 0] -  4*O[ 1] - 82*O[ 2] - 73*O[ 3] + 13*O[ 4] + 85*O[ 5] + 67*O[ 6] - 22*O[ 7] -
                           88*O[ 8] - 61*O[ 9] + 31*O[10] + 90*O[11] + 54*O[12] - 38*O[13] - 90*O[14] - 46*O[15] + rnd) >> nShift;
        piTmp1[13*32+i] = (73*O[ 0] - 31*O[ 1] - 90*O[ 2] - 22*O[ 3] + 78*O[ 4] + 67*O[ 5] - 38*O[ 6] - 90*O[ 7] -
                           13*O[ 8] + 82*O[ 9] + 61*O[10] - 46*O[11] - 88*O[12] -  4*O[13] + 85*O[14] + 54*O[15] + rnd) >> nShift;
        piTmp1[15*32+i] = (67*O[ 0] - 54*O[ 1] - 78*O[ 2] + 38*O[ 3] + 85*O[ 4] - 22*O[ 5] - 90*O[ 6] +  4*O[ 7] +
                           90*O[ 8] + 13*O[ 9] - 88*O[10] - 31*O[11] + 82*O[12] + 46*O[13] - 73*O[14] - 61*O[15] + rnd) >> nShift;
        piTmp1[17*32+i] = (61*O[ 0] - 73*O[ 1] - 46*O[ 2] + 82*O[ 3] + 31*O[ 4] - 88*O[ 5] - 13*O[ 6] + 90*O[ 7] -
                            4*O[ 8] - 90*O[ 9] + 22*O[10] + 85*O[11] - 38*O[12] - 78*O[13] + 54*O[14] + 67*O[15] + rnd) >> nShift;
        piTmp1[19*32+i] = (54*O[ 0] - 85*O[ 1] -  4*O[ 2] + 88*O[ 3] - 46*O[ 4] - 61*O[ 5] + 82*O[ 6] + 13*O[ 7] -
                           90*O[ 8] + 38*O[ 9] + 67*O[10] - 78*O[11] - 22*O[12] + 90*O[13] - 31*O[14] - 73*O[15] + rnd) >> nShift;
        piTmp1[21*32+i] = (46*O[ 0] - 90*O[ 1] + 38*O[ 2] + 54*O[ 3] - 90*O[ 4] + 31*O[ 5] + 61*O[ 6] - 88*O[ 7] +
                           22*O[ 8] + 67*O[ 9] - 85*O[10] + 13*O[11] + 73*O[12] - 82*O[13] +  4*O[14] + 78*O[15] + rnd) >> nShift;
        piTmp1[23*32+i] = (38*O[ 0] - 88*O[ 1] + 73*O[ 2] -  4*O[ 3] - 67*O[ 4] + 90*O[ 5] - 46*O[ 6] - 31*O[ 7] +
                           85*O[ 8] - 78*O[ 9] + 13*O[10] + 61*O[11] - 90*O[12] + 54*O[13] + 22*O[14] - 82*O[15] + rnd) >> nShift;
        piTmp1[25*32+i] = (31*O[ 0] - 78*O[ 1] + 90*O[ 2] - 61*O[ 3] +  4*O[ 4] + 54*O[ 5] - 88*O[ 6] + 82*O[ 7] -
                           38*O[ 8] - 22*O[ 9] + 73*O[10] - 90*O[11] + 67*O[12] - 13*O[13] - 46*O[14] + 85*O[15] + rnd) >> nShift;
        piTmp1[27*32+i] = (22*O[ 0] - 61*O[ 1] + 85*O[ 2] - 90*O[ 3] + 73*O[ 4] - 38*O[ 5] -  4*O[ 6] + 46*O[ 7] -
                           78*O[ 8] + 90*O[ 9] - 82*O[10] + 54*O[11] - 13*O[12] - 31*O[13] + 67*O[14] - 88*O[15] + rnd) >> nShift;
        piTmp1[29*32+i] = (13*O[ 0] - 38*O[ 1] + 61*O[ 2] - 78*O[ 3] + 88*O[ 4] - 90*O[ 5] + 85*O[ 6] - 73*O[ 7] +
                           54*O[ 8] - 31*O[ 9] +  4*O[10] + 22*O[11] - 46*O[12] + 67*O[13] - 82*O[14] + 90*O[15] + rnd) >> nShift;
        piTmp1[31*32+i] = ( 4*O[ 0] - 13*O[ 1] + 22*O[ 2] - 31*O[ 3] + 38*O[ 4] - 46*O[ 5] + 54*O[ 6] - 61*O[ 7] +
                           67*O[ 8] - 73*O[ 9] + 78*O[10] - 82*O[11] + 85*O[12] - 88*O[13] + 90*O[14] - 90*O[15] + rnd) >> nShift;
    }

    // DCT2
    nShift = 5+6;
    rnd    = 1<<(nShift-1);
    for( i=0; i<32; i++ ) {
        /* E and O */
        E[ 0] = piTmp1[i*32+ 0] + piTmp1[i*32+31];
        O[ 0] = piTmp1[i*32+ 0] - piTmp1[i*32+31];
        E[ 1] = piTmp1[i*32+ 1] + piTmp1[i*32+30];
        O[ 1] = piTmp1[i*32+ 1] - piTmp1[i*32+30];
        E[ 2] = piTmp1[i*32+ 2] + piTmp1[i*32+29];
        O[ 2] = piTmp1[i*32+ 2] - piTmp1[i*32+29];
        E[ 3] = piTmp1[i*32+ 3] + piTmp1[i*32+28];
        O[ 3] = piTmp1[i*32+ 3] - piTmp1[i*32+28];
        E[ 4] = piTmp1[i*32+ 4] + piTmp1[i*32+27];
        O[ 4] = piTmp1[i*32+ 4] - piTmp1[i*32+27];
        E[ 5] = piTmp1[i*32+ 5] + piTmp1[i*32+26];
        O[ 5] = piTmp1[i*32+ 5] - piTmp1[i*32+26];
        E[ 6] = piTmp1[i*32+ 6] + piTmp1[i*32+25];
        O[ 6] = piTmp1[i*32+ 6] - piTmp1[i*32+25];
        E[ 7] = piTmp1[i*32+ 7] + piTmp1[i*32+24];
        O[ 7] = piTmp1[i*32+ 7] - piTmp1[i*32+24];
        E[ 8] = piTmp1[i*32+ 8] + piTmp1[i*32+23];
        O[ 8] = piTmp1[i*32+ 8] - piTmp1[i*32+23];
        E[ 9] = piTmp1[i*32+ 9] + piTmp1[i*32+22];
        O[ 9] = piTmp1[i*32+ 9] - piTmp1[i*32+22];
        E[10] = piTmp1[i*32+10] + piTmp1[i*32+21];
        O[10] = piTmp1[i*32+10] - piTmp1[i*32+21];
        E[11] = piTmp1[i*32+11] + piTmp1[i*32+20];
        O[11] = piTmp1[i*32+11] - piTmp1[i*32+20];
        E[12] = piTmp1[i*32+12] + piTmp1[i*32+19];
        O[12] = piTmp1[i*32+12] - piTmp1[i*32+19];
        E[13] = piTmp1[i*32+13] + piTmp1[i*32+18];
        O[13] = piTmp1[i*32+13] - piTmp1[i*32+18];
        E[14] = piTmp1[i*32+14] + piTmp1[i*32+17];
        O[14] = piTmp1[i*32+14] - piTmp1[i*32+17];
        E[15] = piTmp1[i*32+15] + piTmp1[i*32+16];
        O[15] = piTmp1[i*32+15] - piTmp1[i*32+16];

        /* EE and EO */
        EE[0] = E[0] + E[15];
        EO[0] = E[0] - E[15];
        EE[1] = E[1] + E[14];
        EO[1] = E[1] - E[14];
        EE[2] = E[2] + E[13];
        EO[2] = E[2] - E[13];
        EE[3] = E[3] + E[12];
        EO[3] = E[3] - E[12];
        EE[4] = E[4] + E[11];
        EO[4] = E[4] - E[11];
        EE[5] = E[5] + E[10];
        EO[5] = E[5] - E[10];
        EE[6] = E[6] + E[ 9];
        EO[6] = E[6] - E[ 9];
        EE[7] = E[7] + E[ 8];
        EO[7] = E[7] - E[ 8];

        /* EEE and EEO */
        EEE[0] = EE[0] + EE[7];
        EEO[0] = EE[0] - EE[7];
        EEE[1] = EE[1] + EE[6];
        EEO[1] = EE[1] - EE[6];
        EEE[2] = EE[2] + EE[5];
        EEO[2] = EE[2] - EE[5];
        EEE[3] = EE[3] + EE[4];
        EEO[3] = EE[3] - EE[4];

        /* EEEE and EEEO */
        EEEE[0] = EEE[0] + EEE[3];
        EEEO[0] = EEE[0] - EEE[3];
        EEEE[1] = EEE[1] + EEE[2];
        EEEO[1] = EEE[1] - EEE[2];

        // 0, 8, 16, 24
        pDst[ 0*nStride+i] = (64*EEEE[0] + 64*EEEE[1] + rnd) >> nShift;
        pDst[16*nStride+i] = (64*EEEE[0] - 64*EEEE[1] + rnd) >> nShift;
        pDst[ 8*nStride+i] = (83*EEEO[0] + 36*EEEO[1] + rnd) >> nShift;
        pDst[24*nStride+i] = (36*EEEO[0] - 83*EEEO[1] + rnd) >> nShift;

        // 4, 12, 20, 28
        pDst[ 4*nStride+i] = (89*EEO[0] + 75*EEO[1] + 50*EEO[2] + 18*EEO[3] + rnd) >> nShift;
        pDst[12*nStride+i] = (75*EEO[0] - 18*EEO[1] - 89*EEO[2] - 50*EEO[3] + rnd) >> nShift;
        pDst[20*nStride+i] = (50*EEO[0] - 89*EEO[1] + 18*EEO[2] + 75*EEO[3] + rnd) >> nShift;
        pDst[28*nStride+i] = (18*EEO[0] - 50*EEO[1] + 75*EEO[2] - 89*EEO[3] + rnd) >> nShift;

        // 2, 6, 10, 14, 18, 22, 26, 30
        pDst[ 2*nStride+i] = (90*EO[0] + 87*EO[1] + 80*EO[2] + 70*EO[3] + 57*EO[4] + 43*EO[5] + 25*EO[6] +  9*EO[7] + rnd) >> nShift;
        pDst[ 6*nStride+i] = (87*EO[0] + 57*EO[1] +  9*EO[2] - 43*EO[3] - 80*EO[4] - 90*EO[5] - 70*EO[6] - 25*EO[7] + rnd) >> nShift;
        pDst[10*nStride+i] = (80*EO[0] +  9*EO[1] - 70*EO[2] - 87*EO[3] - 25*EO[4] + 57*EO[5] + 90*EO[6] + 43*EO[7] + rnd) >> nShift;
        pDst[14*nStride+i] = (70*EO[0] - 43*EO[1] - 87*EO[2] +  9*EO[3] + 90*EO[4] + 25*EO[5] - 80*EO[6] - 57*EO[7] + rnd) >> nShift;
        pDst[18*nStride+i] = (57*EO[0] - 80*EO[1] - 25*EO[2] + 90*EO[3] -  9*EO[4] - 87*EO[5] + 43*EO[6] + 70*EO[7] + rnd) >> nShift;
        pDst[22*nStride+i] = (43*EO[0] - 90*EO[1] + 57*EO[2] + 25*EO[3] - 87*EO[4] + 70*EO[5] +  9*EO[6] - 80*EO[7] + rnd) >> nShift;
        pDst[26*nStride+i] = (25*EO[0] - 70*EO[1] + 90*EO[2] - 80*EO[3] + 43*EO[4] +  9*EO[5] - 57*EO[6] + 87*EO[7] + rnd) >> nShift;
        pDst[30*nStride+i] = ( 9*EO[0] - 25*EO[1] + 43*EO[2] - 57*EO[3] + 70*EO[4] - 80*EO[5] + 87*EO[6] - 90*EO[7] + rnd) >> nShift;

        // 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31
        pDst[ 1*nStride+i] = (90*O[ 0] + 90*O[ 1] + 88*O[ 2] + 85*O[ 3] + 82*O[ 4] + 78*O[ 5] + 73*O[ 6] + 67*O[ 7] +
                              61*O[ 8] + 54*O[ 9] + 46*O[10] + 38*O[11] + 31*O[12] + 22*O[13] + 13*O[14] +  4*O[15] + rnd) >> nShift;
        pDst[ 3*nStride+i] = (90*O[ 0] + 82*O[ 1] + 67*O[ 2] + 46*O[ 3] + 22*O[ 4] -  4*O[ 5] - 31*O[ 6] - 54*O[ 7] -
                              73*O[ 8] - 85*O[ 9] - 90*O[10] - 88*O[11] - 78*O[12] - 61*O[13] - 38*O[14] - 13*O[15] + rnd) >> nShift;
        pDst[ 5*nStride+i] = (88*O[ 0] + 67*O[ 1] + 31*O[ 2] - 13*O[ 3] - 54*O[ 4] - 82*O[ 5] - 90*O[ 6] - 78*O[ 7] -
                              46*O[ 8] -  4*O[ 9] + 38*O[10] + 73*O[11] + 90*O[12] + 85*O[13] + 61*O[14] + 22*O[15] + rnd) >> nShift;
        pDst[ 7*nStride+i] = (85*O[ 0] + 46*O[ 1] - 13*O[ 2] - 67*O[ 3] - 90*O[ 4] - 73*O[ 5] - 22*O[ 6] + 38*O[ 7] +
                              82*O[ 8] + 88*O[ 9] + 54*O[10] -  4*O[11] - 61*O[12] - 90*O[13] - 78*O[14] - 31*O[15] + rnd) >> nShift;
        pDst[ 9*nStride+i] = (82*O[ 0] + 22*O[ 1] - 54*O[ 2] - 90*O[ 3] - 61*O[ 4] + 13*O[ 5] + 78*O[ 6] + 85*O[ 7] +
                              31*O[ 8] - 46*O[ 9] - 90*O[10] - 67*O[11] +  4*O[12] + 73*O[13] + 88*O[14] + 38*O[15] + rnd) >> nShift;
        pDst[11*nStride+i] = (78*O[ 0] -  4*O[ 1] - 82*O[ 2] - 73*O[ 3] + 13*O[ 4] + 85*O[ 5] + 67*O[ 6] - 22*O[ 7] -
                              88*O[ 8] - 61*O[ 9] + 31*O[10] + 90*O[11] + 54*O[12] - 38*O[13] - 90*O[14] - 46*O[15] + rnd) >> nShift;
        pDst[13*nStride+i] = (73*O[ 0] - 31*O[ 1] - 90*O[ 2] - 22*O[ 3] + 78*O[ 4] + 67*O[ 5] - 38*O[ 6] - 90*O[ 7] -
                              13*O[ 8] + 82*O[ 9] + 61*O[10] - 46*O[11] - 88*O[12] -  4*O[13] + 85*O[14] + 54*O[15] + rnd) >> nShift;
        pDst[15*nStride+i] = (67*O[ 0] - 54*O[ 1] - 78*O[ 2] + 38*O[ 3] + 85*O[ 4] - 22*O[ 5] - 90*O[ 6] +  4*O[ 7] +
                              90*O[ 8] + 13*O[ 9] - 88*O[10] - 31*O[11] + 82*O[12] + 46*O[13] - 73*O[14] - 61*O[15] + rnd) >> nShift;
        pDst[17*nStride+i] = (61*O[ 0] - 73*O[ 1] - 46*O[ 2] + 82*O[ 3] + 31*O[ 4] - 88*O[ 5] - 13*O[ 6] + 90*O[ 7] -
                               4*O[ 8] - 90*O[ 9] + 22*O[10] + 85*O[11] - 38*O[12] - 78*O[13] + 54*O[14] + 67*O[15] + rnd) >> nShift;
        pDst[19*nStride+i] = (54*O[ 0] - 85*O[ 1] -  4*O[ 2] + 88*O[ 3] - 46*O[ 4] - 61*O[ 5] + 82*O[ 6] + 13*O[ 7] -
                              90*O[ 8] + 38*O[ 9] + 67*O[10] - 78*O[11] - 22*O[12] + 90*O[13] - 31*O[14] - 73*O[15] + rnd) >> nShift;
        pDst[21*nStride+i] = (46*O[ 0] - 90*O[ 1] + 38*O[ 2] + 54*O[ 3] - 90*O[ 4] + 31*O[ 5] + 61*O[ 6] - 88*O[ 7] +
                              22*O[ 8] + 67*O[ 9] - 85*O[10] + 13*O[11] + 73*O[12] - 82*O[13] +  4*O[14] + 78*O[15] + rnd) >> nShift;
        pDst[23*nStride+i] = (38*O[ 0] - 88*O[ 1] + 73*O[ 2] -  4*O[ 3] - 67*O[ 4] + 90*O[ 5] - 46*O[ 6] - 31*O[ 7] +
                              85*O[ 8] - 78*O[ 9] + 13*O[10] + 61*O[11] - 90*O[12] + 54*O[13] + 22*O[14] - 82*O[15] + rnd) >> nShift;
        pDst[25*nStride+i] = (31*O[ 0] - 78*O[ 1] + 90*O[ 2] - 61*O[ 3] +  4*O[ 4] + 54*O[ 5] - 88*O[ 6] + 82*O[ 7] -
                              38*O[ 8] - 22*O[ 9] + 73*O[10] - 90*O[11] + 67*O[12] - 13*O[13] - 46*O[14] + 85*O[15] + rnd) >> nShift;
        pDst[27*nStride+i] = (22*O[ 0] - 61*O[ 1] + 85*O[ 2] - 90*O[ 3] + 73*O[ 4] - 38*O[ 5] -  4*O[ 6] + 46*O[ 7] -
                              78*O[ 8] + 90*O[ 9] - 82*O[10] + 54*O[11] - 13*O[12] - 31*O[13] + 67*O[14] - 88*O[15] + rnd) >> nShift;
        pDst[29*nStride+i] = (13*O[ 0] - 38*O[ 1] + 61*O[ 2] - 78*O[ 3] + 88*O[ 4] - 90*O[ 5] + 85*O[ 6] - 73*O[ 7] +
                              54*O[ 8] - 31*O[ 9] +  4*O[10] + 22*O[11] - 46*O[12] + 67*O[13] - 82*O[14] + 90*O[15] + rnd) >> nShift;
        pDst[31*nStride+i] = ( 4*O[ 0] - 13*O[ 1] + 22*O[ 2] - 31*O[ 3] + 38*O[ 4] - 46*O[ 5] + 54*O[ 6] - 61*O[ 7] +
                              67*O[ 8] - 73*O[ 9] + 78*O[10] - 82*O[11] + 85*O[12] - 88*O[13] + 90*O[14] - 90*O[15] + rnd) >> nShift;
    }
}

typedef void xSubDCT_t(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
);

xSubDCT_t *xSubDctN[MAX_CU_DEPTH+1] = {
    xSubDST4,
    xSubDCT4,
    xSubDCT8,
    xSubDCT16,
    xSubDCT32,
};

void xSubDct(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1,
    Int iWidth, UInt nMode
)
{
    CInt nLog2Width  = xLog2( iWidth - 1 );
    CInt bUseDst     = (iWidth == 4) && (nMode != MODE_INVALID);

    xSubDctN[nLog2Width - 1 - bUseDst]( pDst, pSrc, nStride, pRef, nStrideRef, piTmp0, piTmp1 );
}

#if (USE_ASM >= ASM_SSE4)
void xIDstAdd4(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;

    __m128i zero        = _mm_setzero_si128();
    __m128i c32_64      = _mm_set1_epi32(64);
    __m128i c32_2048    = _mm_set1_epi32(2048);
    __m128i c16_m0      = _mm_set_epi16( 55, 84, 74, 29, 55, 84, 74, 29);
    __m128i c16_m1      = _mm_set_epi16(-84,-29, 74, 55,-84,-29, 74, 55);
    __m128i c16_m2      = _mm_set_epi16( 74,-74,  0, 74, 74,-74,  0, 74);
    __m128i c16_m3      = _mm_set_epi16(-29, 55,-74, 84,-29, 55,-74, 84);
    __m128i c16_trans   = _mm_set_epi8(15, 14, 11, 10, 7, 6, 3, 2, 13, 12, 9, 8, 5, 4, 1, 0);

    // DST1
    __m128i T000 = _mm_loadl_epi64((__m128i*)&pSrc[0*nStride]);     // [03 02 01 00]
    __m128i T001 = _mm_loadl_epi64((__m128i*)&pSrc[1*nStride]);     // [13 12 11 10]
    __m128i T002 = _mm_loadl_epi64((__m128i*)&pSrc[2*nStride]);     // [23 22 21 20]
    __m128i T003 = _mm_loadl_epi64((__m128i*)&pSrc[3*nStride]);     // [33 32 31 30]
#if 1
    __m128i T010 = _mm_unpacklo_epi16(T000, T001);                  // [13 03 12 02 11 01 10 00]
    __m128i T011 = _mm_unpacklo_epi16(T002, T003);                  // [33 23 32 22 31 21 30 20]
    __m128i T020 = _mm_unpacklo_epi32(T010, T011);                  // [31 21 11 01 30 20 10 00]
    __m128i T021 = _mm_unpackhi_epi32(T010, T011);                  // [33 23 13 03 32 22 12 02]
    __m128i T030 = _mm_madd_epi16(c16_m0, T020);
    __m128i T031 = _mm_madd_epi16(c16_m1, T020);
    __m128i T032 = _mm_madd_epi16(c16_m2, T020);
    __m128i T033 = _mm_madd_epi16(c16_m3, T020);
    __m128i T034 = _mm_madd_epi16(c16_m0, T021);
    __m128i T035 = _mm_madd_epi16(c16_m1, T021);
    __m128i T036 = _mm_madd_epi16(c16_m2, T021);
    __m128i T037 = _mm_madd_epi16(c16_m3, T021);
    __m128i T040 = _mm_hadd_epi32(T030, T031);
    __m128i T041 = _mm_hadd_epi32(T032, T033);
    __m128i T042 = _mm_hadd_epi32(T034, T035);
    __m128i T043 = _mm_hadd_epi32(T036, T037);
    __m128i T050 = _mm_srai_epi32(_mm_add_epi32(T040, c32_64), 7);      // [11 01 10 00]
    __m128i T051 = _mm_srai_epi32(_mm_add_epi32(T041, c32_64), 7);      // [13 03 12 02]
    __m128i T052 = _mm_srai_epi32(_mm_add_epi32(T042, c32_64), 7);      // [31 21 30 20]
    __m128i T053 = _mm_srai_epi32(_mm_add_epi32(T043, c32_64), 7);      // [33 23 32 22]
    __m128i T060 = _mm_packs_epi32(T050, T051);                         // [13 03 12 02 11 01 10 00]
    __m128i T061 = _mm_packs_epi32(T052, T053);                         // [33 23 32 22 31 21 30 20]
    __m128i T070 = _mm_unpacklo_epi32(T060, T061);                      // [31 21 11 01 30 20 10 00]
    __m128i T071 = _mm_unpackhi_epi32(T060, T061);                      // [33 23 13 03 32 22 12 02]
#else
    __m128i T010 = _mm_add_epi16(T000, T002);                       // [C03 C02 C01 C00]
    __m128i T011 = _mm_add_epi16(T002, T003);                       // [C13 C12 C11 C10]
    __m128i T012 = _mm_sub_epi16(T000, T003);                       // [C23 C22 C21 C20]
    __m128i T013 = T001;                                            // [C33 C32 C31 C30]
    __m128i T014 = _mm_add_epi16(_mm_sub_epi16(T000, T002), T003);  // [C43 C42 C41 C40]
    __m128i T020 = _mm_unpacklo_epi16(T010, T011);                  // [C13 C03 C12 C02 C11 C01 C10 C00]
    __m128i T021 = _mm_unpacklo_epi16(T012, T013);                  // [C33 C23 C32 C22 C31 C21 C30 C20]
    __m128i T030 = _mm_unpacklo_epi32(T020, T021);                  // [C31 C21 C11 C01 C30 C20 C10 C00]
    __m128i T031 = _mm_unpackhi_epi32(T020, T021);                  // [C33 C23 C13 C03 C32 C22 C12 C02]
    __m128i c16_74_0_55_29      = _mm_set_epi16( 74,  0, 55, 29, 74,  0, 55, 29);
    __m128i c16_74_55_n29_0     = _mm_set_epi16( 74, 55,-29,  0, 74, 55,-29,  0);
    __m128i c16_n74_29_0_55     = _mm_set_epi16(-74, 29,  0, 55,-74, 29,  0, 55);
    __m128i c32_74              = _mm_set1_epi32(74);
    __m128i T040 = _mm_madd_epi16(T030, c16_74_0_55_29);
    __m128i T041 = _mm_madd_epi16(T031, c16_74_0_55_29);
    __m128i T042 = _mm_madd_epi16(T030, c16_74_55_n29_0);
    __m128i T043 = _mm_madd_epi16(T031, c16_74_55_n29_0);
    __m128i T044 = _mm_mullo_epi32(c32_74, _mm_cvtepi16_epi32(T014));
    __m128i T046 = _mm_madd_epi16(T030, c16_n74_29_0_55);
    __m128i T047 = _mm_madd_epi16(T031, c16_n74_29_0_55);
    __m128i T050 = _mm_hadd_epi32(T040, T041);                      // [30 20 10 00]
    __m128i T051 = _mm_hadd_epi32(T042, T043);                      // [31 21 11 01]
    __m128i T052 = T044;                                            // [32 22 12 02]
    __m128i T053 = _mm_hadd_epi32(T046, T047);                      // [33 23 13 03]
    __m128i T060 = _mm_srai_epi32(_mm_add_epi32(T050, c32_64), 7);
    __m128i T061 = _mm_srai_epi32(_mm_add_epi32(T051, c32_64), 7);
    __m128i T062 = _mm_srai_epi32(_mm_add_epi32(T052, c32_64), 7);
    __m128i T063 = _mm_srai_epi32(_mm_add_epi32(T053, c32_64), 7);
    __m128i T070 = _mm_packs_epi32(T060, T061);                     // [31 21 11 01 30 20 10 00]
    __m128i T071 = _mm_packs_epi32(T062, T063);                     // [33 23 13 03 32 22 12 02]
#endif

    // DST2
    __m128i T080 = _mm_madd_epi16(c16_m0, T070);
    __m128i T081 = _mm_madd_epi16(c16_m1, T070);
    __m128i T082 = _mm_madd_epi16(c16_m2, T070);
    __m128i T083 = _mm_madd_epi16(c16_m3, T070);
    __m128i T084 = _mm_madd_epi16(c16_m0, T071);
    __m128i T085 = _mm_madd_epi16(c16_m1, T071);
    __m128i T086 = _mm_madd_epi16(c16_m2, T071);
    __m128i T087 = _mm_madd_epi16(c16_m3, T071);
    __m128i T090 = _mm_hadd_epi32(T080, T081);
    __m128i T091 = _mm_hadd_epi32(T082, T083);
    __m128i T092 = _mm_hadd_epi32(T084, T085);
    __m128i T093 = _mm_hadd_epi32(T086, T087);
    __m128i T100 = _mm_srai_epi32(_mm_add_epi32(T090, c32_2048), 12);   // [11 01 10 00]
    __m128i T101 = _mm_srai_epi32(_mm_add_epi32(T091, c32_2048), 12);   // [13 03 12 02]
    __m128i T102 = _mm_srai_epi32(_mm_add_epi32(T092, c32_2048), 12);   // [31 21 30 20]
    __m128i T103 = _mm_srai_epi32(_mm_add_epi32(T093, c32_2048), 12);   // [33 23 32 22]
    __m128i T110 = _mm_packs_epi32(T100, T101);                         // [13 03 12 02 11 01 10 00]
    __m128i T111 = _mm_packs_epi32(T102, T103);                         // [33 23 32 22 31 21 30 20]
    __m128i T120 = _mm_shuffle_epi8(T110, c16_trans);                   // [13 12 11 10 03 02 01 00]
    __m128i T121 = _mm_shuffle_epi8(T111, c16_trans);                   // [33 32 31 30 23 22 21 20]

    // Add
    __m128i T130 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[0*nStride]);
    __m128i T131 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[1*nStride]);
    __m128i T132 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[2*nStride]);
    __m128i T133 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[3*nStride]);
    __m128i T140 = _mm_unpacklo_epi32(T130, T131);                   // [13 12 11 10 03 02 01 00]
    __m128i T141 = _mm_unpacklo_epi32(T132, T133);                   // [33 32 31 30 23 22 21 20]
    __m128i T150 = _mm_cvtepu8_epi16(T140);
    __m128i T151 = _mm_cvtepu8_epi16(T141);
    __m128i T160 = _mm_add_epi16(T120, T150);
    __m128i T161 = _mm_add_epi16(T121, T151);
    __m128i T170 = _mm_packus_epi16(T160, T161);

    *(unsigned int *)&pDst[0*nStrideDst] = _mm_cvtsi128_si32(T170);
    *(unsigned int *)&pDst[1*nStrideDst] = _mm_cvtsi128_si32(_mm_srli_si128(T170, 4));
    *(unsigned int *)&pDst[2*nStrideDst] = _mm_cvtsi128_si32(_mm_srli_si128(T170, 8));
    *(unsigned int *)&pDst[3*nStrideDst] = _mm_cvtsi128_si32(_mm_srli_si128(T170,12));
}
#else // ASM_NONE
void xIDstAdd4(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;

    // DCT1
    nShift = SHIFT_INV_1ST;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        // Intermediate Variables
        Int32 c0 = pSrc[0*nStride+i] + pSrc[2*nStride+i];
        Int32 c1 = pSrc[2*nStride+i] + pSrc[3*nStride+i];
        Int32 c2 = pSrc[0*nStride+i] - pSrc[3*nStride+i];
        Int32 c3 = 74* pSrc[1*nStride+i];
        Int32 c4 = pSrc[0*nStride+i] - pSrc[2*nStride+i] + pSrc[3*nStride+i];

        piTmp0[i*4+0] = Clip3( -32768, 32767, ( 29 * c0 + 55 * c1 + c3 + rnd ) >> nShift);
        piTmp0[i*4+1] = Clip3( -32768, 32767, ( 55 * c2 - 29 * c1 + c3 + rnd ) >> nShift);
        piTmp0[i*4+2] = Clip3( -32768, 32767, ( 74 * c4                + rnd ) >> nShift);
        piTmp0[i*4+3] = Clip3( -32768, 32767, ( 55 * c0 + 29 * c2 - c3 + rnd ) >> nShift);
    }

    // DCT2
    nShift = SHIFT_INV_2ND;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        // Intermediate Variables
        Int32 c0 = piTmp0[0*4+i] + piTmp0[2*4+i];
        Int32 c1 = piTmp0[2*4+i] + piTmp0[3*4+i];
        Int32 c2 = piTmp0[0*4+i] - piTmp0[3*4+i];
        Int32 c3 = 74* piTmp0[1*4+i];
        Int32 c4 = piTmp0[0*4+i] - piTmp0[2*4+i] + piTmp0[3*4+i];

        piTmp1[i*4+0] = Clip3( -32768, 32767, ( 29 * c0 + 55 * c1 + c3 + rnd ) >> nShift);
        piTmp1[i*4+1] = Clip3( -32768, 32767, ( 55 * c2 - 29 * c1 + c3 + rnd ) >> nShift);
        piTmp1[i*4+2] = Clip3( -32768, 32767, ( 74 * c4                + rnd ) >> nShift);
        piTmp1[i*4+3] = Clip3( -32768, 32767, ( 55 * c0 + 29 * c2 - c3 + rnd ) >> nShift);
    }

    // Add
    for( i=0; i<4; i++ ) {
        for( j=0; j<4; j++ ) {
            pDst[i * nStrideDst + j] = Clip( piTmp1[i * 4 + j] + pRef[i * nStride + j] );
        }
    }
}
#endif

#if (USE_ASM >= ASM_SSE4)
void xIDctAdd4(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;

    __m128i zero                = _mm_setzero_si128();
    __m128i c16_36_83           = _mm_set1_epi32(0x00240053);
    __m128i c16_n83_36          = _mm_set1_epi32(0xFFAD0024);
    __m128i c16_64_64           = _mm_set1_epi32(0x00400040);
    __m128i c16_n64_64          = _mm_set1_epi32(0xFFC00040);
    __m128i c32_64              = _mm_set1_epi32(64);
    __m128i c32_2048            = _mm_set1_epi32(2048);

    // Load
    __m128i T000 = _mm_loadl_epi64((__m128i*)&pSrc[0*nStride]);     // [03 02 01 00]
    __m128i T001 = _mm_loadl_epi64((__m128i*)&pSrc[1*nStride]);     // [13 12 11 10]
    __m128i T002 = _mm_loadl_epi64((__m128i*)&pSrc[2*nStride]);     // [23 22 21 20]
    __m128i T003 = _mm_loadl_epi64((__m128i*)&pSrc[3*nStride]);     // [33 32 31 30]

    // DCT1
    __m128i T010 = _mm_unpacklo_epi16(T000, T002);                  // [23 03 22 02 21 01 20 00]
    __m128i T011 = _mm_unpacklo_epi16(T001, T003);                  // [33 13 32 12 31 11 30 10]
    __m128i T020 = _mm_madd_epi16(T011, c16_36_83);                 // [ 36*(3) + 83*(1)] = O0
    __m128i T021 = _mm_madd_epi16(T011, c16_n83_36);                // [-83*(3) + 36*(1)] = O1
    __m128i T022 = _mm_madd_epi16(T010, c16_64_64);                 // [ 64*(2) + 64*(0)] = E0
    __m128i T023 = _mm_madd_epi16(T010, c16_n64_64);                // [-64*(2) + 64*(0)] = E1
    __m128i T030 = _mm_add_epi32(T022, T020);                       // [E0 + O0]
    __m128i T031 = _mm_add_epi32(T023, T021);                       // [E1 + O1]
    __m128i T032 = _mm_sub_epi32(T023, T021);                       // [E1 - O1]
    __m128i T033 = _mm_sub_epi32(T022, T020);                       // [E0 - O0]
    __m128i T040 = _mm_srai_epi32(_mm_add_epi32(c32_64, T030), 7);  // [(E0 + O0 + rnd) >> nShift] = [30 20 10 00]
    __m128i T041 = _mm_srai_epi32(_mm_add_epi32(c32_64, T031), 7);  // [(E1 + O1 + rnd) >> nShift] = [31 21 11 01]
    __m128i T042 = _mm_srai_epi32(_mm_add_epi32(c32_64, T032), 7);  // [(E1 - O1 + rnd) >> nShift] = [32 22 12 02]
    __m128i T043 = _mm_srai_epi32(_mm_add_epi32(c32_64, T033), 7);  // [(E0 - O0 + rnd) >> nShift] = [33 23 13 03]

    // DCT2
    __m128i T050 = _mm_packs_epi32(T040, T041);                     // [31 21 11 01 30 20 10 00]
    __m128i T051 = _mm_packs_epi32(T042, T043);                     // [33 23 13 03 32 22 12 02]
    //__m128i T050 = _mm_set_epi32(0x00310021, 0x00110001, 0x00300020, 0x00100000);
    //__m128i T051 = _mm_set_epi32(0x00330023, 0x00130003, 0x00320022, 0x00120002);
    __m128i c16_trans = _mm_set_epi8(15, 14, 11, 10, 7, 6, 3, 2, 13, 12, 9, 8, 5, 4, 1, 0);
    __m128i T060 = _mm_shuffle_epi8(T050, c16_trans);               // [31 11 30 10 21 01 20 00]
    __m128i T061 = _mm_shuffle_epi8(T051, c16_trans);               // [33 13 32 12 23 03 22 02]
    __m128i T070 = _mm_unpacklo_epi64(T060, T061);                  // [23 03 22 02 21 01 20 00]
    __m128i T071 = _mm_unpackhi_epi64(T060, T061);                  // [33 13 32 12 31 11 30 10]
    __m128i T080 = _mm_madd_epi16(T071, c16_36_83);                 // [ 36*(3) + 83*(1)] = O0
    __m128i T081 = _mm_madd_epi16(T071, c16_n83_36);                // [-83*(3) + 36*(1)] = O1
    __m128i T082 = _mm_madd_epi16(T070, c16_64_64);                 // [ 64*(2) + 64*(0)] = E0
    __m128i T083 = _mm_madd_epi16(T070, c16_n64_64);                // [-64*(2) + 64*(0)] = E1
    __m128i T090 = _mm_add_epi32(T082, T080);                       // [E0 + O0]
    __m128i T091 = _mm_add_epi32(T083, T081);                       // [E1 + O1]
    __m128i T092 = _mm_sub_epi32(T083, T081);                       // [E1 - O1]
    __m128i T093 = _mm_sub_epi32(T082, T080);                       // [E0 - O0]
    __m128i T100 = _mm_srai_epi32(_mm_add_epi32(c32_2048, T090), 12);   // [(E0 + O0 + rnd) >> nShift] = [30 20 10 00]
    __m128i T101 = _mm_srai_epi32(_mm_add_epi32(c32_2048, T091), 12);   // [(E1 + O1 + rnd) >> nShift] = [31 21 11 01]
    __m128i T102 = _mm_srai_epi32(_mm_add_epi32(c32_2048, T092), 12);   // [(E1 - O1 + rnd) >> nShift] = [32 22 12 02]
    __m128i T103 = _mm_srai_epi32(_mm_add_epi32(c32_2048, T093), 12);   // [(E0 - O0 + rnd) >> nShift] = [33 23 13 03]
    __m128i T110 = _mm_packs_epi32(T100, T102);                     // [32 22 12 02 30 20 10 00]
    __m128i T111 = _mm_packs_epi32(T101, T103);                     // [33 23 13 03 31 21 11 01]
    __m128i T120 = _mm_unpacklo_epi16(T110, T111);                  // [31 30 21 20 11 10 01 00]
    __m128i T121 = _mm_unpackhi_epi16(T110, T111);                  // [33 32 23 22 13 12 03 02]
    __m128i T130 = _mm_unpacklo_epi32(T120, T121);                  // [13 12 11 10 03 02 01 00]
    __m128i T131 = _mm_unpackhi_epi32(T120, T121);                  // [33 32 31 30 23 22 21 20]
    __m128i T140 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[0*nStride]);
    __m128i T141 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[1*nStride]);
    __m128i T142 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[2*nStride]);
    __m128i T143 = _mm_cvtsi32_si128(*(unsigned int *)&pRef[3*nStride]);
    __m128i T150 = _mm_unpacklo_epi32(T140, T141);                   // [13 12 11 10 03 02 01 00]
    __m128i T151 = _mm_unpacklo_epi32(T142, T143);                   // [33 32 31 30 23 22 21 20]
    __m128i T160 = _mm_cvtepu8_epi16(T150);
    __m128i T161 = _mm_cvtepu8_epi16(T151);
    __m128i T170 = _mm_add_epi16(T130, T160);
    __m128i T171 = _mm_add_epi16(T131, T161);
    __m128i T180 = _mm_packus_epi16(T170, T171);

    *(unsigned int *)&pDst[0*nStrideDst] = _mm_cvtsi128_si32(T180);
    *(unsigned int *)&pDst[1*nStrideDst] = _mm_cvtsi128_si32(_mm_srli_si128(T180, 4));
    *(unsigned int *)&pDst[2*nStrideDst] = _mm_cvtsi128_si32(_mm_srli_si128(T180, 8));
    *(unsigned int *)&pDst[3*nStrideDst] = _mm_cvtsi128_si32(_mm_srli_si128(T180,12));
}
#else // ASM_NONE
void xIDctAdd4(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;

    // DCT1
    nShift = SHIFT_INV_1ST;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 = 83*pSrc[1*nStride+i] + 36*pSrc[3*nStride+i];
        Int32 O1 = 36*pSrc[1*nStride+i] - 83*pSrc[3*nStride+i];
        Int32 E0 = 64*pSrc[0*nStride+i] + 64*pSrc[2*nStride+i];
        Int32 E1 = 64*pSrc[0*nStride+i] - 64*pSrc[2*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        piTmp0[i*4+0] = Clip3( -32768, 32767, (E0 + O0 + rnd) >> nShift);
        piTmp0[i*4+1] = Clip3( -32768, 32767, (E1 + O1 + rnd) >> nShift);
        piTmp0[i*4+2] = Clip3( -32768, 32767, (E1 - O1 + rnd) >> nShift);
        piTmp0[i*4+3] = Clip3( -32768, 32767, (E0 - O0 + rnd) >> nShift);
    }

    // DCT2
    nShift = SHIFT_INV_2ND;
    rnd    = 1<<(nShift-1);
    for( i=0; i<4; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 = 83*piTmp0[1*4+i] + 36*piTmp0[3*4+i];
        Int32 O1 = 36*piTmp0[1*4+i] - 83*piTmp0[3*4+i];
        Int32 E0 = 64*piTmp0[0*4+i] + 64*piTmp0[2*4+i];
        Int32 E1 = 64*piTmp0[0*4+i] - 64*piTmp0[2*4+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        piTmp1[i*4+0] = Clip3( -32768, 32767, (E0 + O0 + rnd) >> nShift);
        piTmp1[i*4+1] = Clip3( -32768, 32767, (E1 + O1 + rnd) >> nShift);
        piTmp1[i*4+2] = Clip3( -32768, 32767, (E1 - O1 + rnd) >> nShift);
        piTmp1[i*4+3] = Clip3( -32768, 32767, (E0 - O0 + rnd) >> nShift);
    }

    // Add
    for( i=0; i<4; i++ ) {
        for( j=0; j<4; j++ ) {
            pDst[i * nStrideDst + j] = Clip( piTmp1[i * 4 + j] + pRef[i * nStride + j] );
        }
    }
}
#endif

#if (USE_ASM >= ASM_SSE4)
void xIDctAdd8(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;

    __m128i zero        = _mm_setzero_si128();
    __m128i c32_64      = _mm_set1_epi32(64);
    __m128i c32_2048    = _mm_set1_epi32(2048);
    __m128i c16_36_83   = _mm_set1_epi32(0x00240053);
    __m128i c16_n83_36  = _mm_set1_epi32(0xFFAD0024);
    __m128i c16_n64_64  = _mm_set1_epi32(0xFFC00040);
    __m128i c16_64_64   = _mm_set1_epi32(0x00400040);
    __m128i c16_75_89   = _mm_set1_epi32(0x004B0059);
    __m128i c16_18_50   = _mm_set1_epi32(0x00120032);
    __m128i c16_n18_75  = _mm_set1_epi32(0xFFEE004B);
    __m128i c16_n50_n89 = _mm_set1_epi32(0xFFCEFFA7);
    __m128i c16_n89_50  = _mm_set1_epi32(0xFFA70032);
    __m128i c16_75_18   = _mm_set1_epi32(0x004B0012);
    __m128i c16_n50_18  = _mm_set1_epi32(0xFFCE0012);
    __m128i c16_n89_75  = _mm_set1_epi32(0xFFA7004B);

    // DCT1
    __m128i T000  = _mm_load_si128((__m128i*)&pSrc[0*nStride]); // [07 06 05 04 03 02 01 00]
    __m128i T001  = _mm_load_si128((__m128i*)&pSrc[1*nStride]); // [17 16 15 14 13 12 11 10]
    __m128i T002  = _mm_load_si128((__m128i*)&pSrc[2*nStride]); // [27 26 25 24 23 22 21 20]
    __m128i T003  = _mm_load_si128((__m128i*)&pSrc[3*nStride]); // [37 36 35 34 33 32 31 30]
    __m128i T004  = _mm_load_si128((__m128i*)&pSrc[4*nStride]); // [47 46 45 44 43 42 41 40]
    __m128i T005  = _mm_load_si128((__m128i*)&pSrc[5*nStride]); // [57 56 55 54 53 52 51 50]
    __m128i T006  = _mm_load_si128((__m128i*)&pSrc[6*nStride]); // [67 66 65 64 63 62 61 60]
    __m128i T007  = _mm_load_si128((__m128i*)&pSrc[7*nStride]); // [77 76 75 74 73 72 71 70]
    __m128i T010A = _mm_unpacklo_epi16(T000, T004);             // [43 03 42 02 41 01 40 00]
    __m128i T010B = _mm_unpackhi_epi16(T000, T004);             // [47 07 46 06 45 05 44 04]
    __m128i T011A = _mm_unpacklo_epi16(T002, T006);             // [63 23 62 22 61 21 60 20]
    __m128i T011B = _mm_unpackhi_epi16(T002, T006);             // [67 27 66 26 65 25 64 24]
    __m128i T012A = _mm_unpacklo_epi16(T001, T003);             // [33 13 32 12 31 11 31 10]
    __m128i T012B = _mm_unpackhi_epi16(T001, T003);             // [37 17 36 16 35 15 34 14]
    __m128i T013A = _mm_unpacklo_epi16(T005, T007);             // [73 53 72 52 71 51 71 50]
    __m128i T013B = _mm_unpackhi_epi16(T005, T007);             // [77 57 76 56 75 55 74 54]
    __m128i T020A = _mm_madd_epi16(T010A, c16_64_64 );          // EE0
    __m128i T020B = _mm_madd_epi16(T010B, c16_64_64 );
    __m128i T021A = _mm_madd_epi16(T010A, c16_n64_64);          // EE1
    __m128i T021B = _mm_madd_epi16(T010B, c16_n64_64);
    __m128i T022A = _mm_madd_epi16(T011A, c16_36_83 );          // EO0
    __m128i T022B = _mm_madd_epi16(T011B, c16_36_83 );
    __m128i T023A = _mm_madd_epi16(T011A, c16_n83_36 );         // EO1
    __m128i T023B = _mm_madd_epi16(T011B, c16_n83_36 );
    __m128i T024A = _mm_add_epi32(_mm_madd_epi16(T012A, c16_75_89  ), _mm_madd_epi16(T013A, c16_18_50   )); // O0
    __m128i T024B = _mm_add_epi32(_mm_madd_epi16(T012B, c16_75_89  ), _mm_madd_epi16(T013B, c16_18_50   ));
    __m128i T025A = _mm_add_epi32(_mm_madd_epi16(T012A, c16_n18_75 ), _mm_madd_epi16(T013A, c16_n50_n89 )); // O1
    __m128i T025B = _mm_add_epi32(_mm_madd_epi16(T012B, c16_n18_75 ), _mm_madd_epi16(T013B, c16_n50_n89 ));
    __m128i T026A = _mm_add_epi32(_mm_madd_epi16(T012A, c16_n89_50 ), _mm_madd_epi16(T013A, c16_75_18   )); // O2
    __m128i T026B = _mm_add_epi32(_mm_madd_epi16(T012B, c16_n89_50 ), _mm_madd_epi16(T013B, c16_75_18   ));
    __m128i T027A = _mm_add_epi32(_mm_madd_epi16(T012A, c16_n50_18 ), _mm_madd_epi16(T013A, c16_n89_75  )); // O3
    __m128i T027B = _mm_add_epi32(_mm_madd_epi16(T012B, c16_n50_18 ), _mm_madd_epi16(T013B, c16_n89_75  ));
    __m128i T030A = _mm_add_epi32(T020A, T022A);                // E0 = EE0 + EO0
    __m128i T030B = _mm_add_epi32(T020B, T022B);
    __m128i T031A = _mm_add_epi32(T021A, T023A);                // E1 = EE1 + EO1
    __m128i T031B = _mm_add_epi32(T021B, T023B);
    __m128i T032A = _mm_sub_epi32(T021A, T023A);                // E2 = EE1 - EO1
    __m128i T032B = _mm_sub_epi32(T021B, T023B);
    __m128i T033A = _mm_sub_epi32(T020A, T022A);                // E3 = EE0 - EO0
    __m128i T033B = _mm_sub_epi32(T020B, T022B);
    __m128i T040A = _mm_add_epi32(T030A, c32_64);               // E0 + rnd
    __m128i T040B = _mm_add_epi32(T030B, c32_64);
    __m128i T041A = _mm_add_epi32(T031A, c32_64);               // E1 + rnd
    __m128i T041B = _mm_add_epi32(T031B, c32_64);
    __m128i T042A = _mm_add_epi32(T032A, c32_64);               // E2 + rnd
    __m128i T042B = _mm_add_epi32(T032B, c32_64);
    __m128i T043A = _mm_add_epi32(T033A, c32_64);               // E3 + rnd
    __m128i T043B = _mm_add_epi32(T033B, c32_64);
    __m128i T050A = _mm_add_epi32(T040A, T024A);                // E0 + O0 + rnd
    __m128i T050B = _mm_add_epi32(T040B, T024B);
    __m128i T051A = _mm_add_epi32(T041A, T025A);                // E1 + O1 + rnd
    __m128i T051B = _mm_add_epi32(T041B, T025B);
    __m128i T052A = _mm_add_epi32(T042A, T026A);                // E2 + O2 + rnd
    __m128i T052B = _mm_add_epi32(T042B, T026B);
    __m128i T053A = _mm_add_epi32(T043A, T027A);                // E3 + O3 + rnd
    __m128i T053B = _mm_add_epi32(T043B, T027B);
    __m128i T054A = _mm_sub_epi32(T043A, T027A);                // E3 - O3 + rnd
    __m128i T054B = _mm_sub_epi32(T043B, T027B);
    __m128i T055A = _mm_sub_epi32(T042A, T026A);                // E2 - O2 + rnd
    __m128i T055B = _mm_sub_epi32(T042B, T026B);
    __m128i T056A = _mm_sub_epi32(T041A, T025A);                // E1 - O1 + rnd
    __m128i T056B = _mm_sub_epi32(T041B, T025B);
    __m128i T057A = _mm_sub_epi32(T040A, T024A);                // E0 - O0 + rnd
    __m128i T057B = _mm_sub_epi32(T040B, T024B);
    __m128i T060A = _mm_srai_epi32(T050A, 7);                   // [30 20 10 00]
    __m128i T060B = _mm_srai_epi32(T050B, 7);                   // [70 60 50 40]
    __m128i T061A = _mm_srai_epi32(T051A, 7);                   // [31 21 11 01]
    __m128i T061B = _mm_srai_epi32(T051B, 7);                   // [71 61 51 41]
    __m128i T062A = _mm_srai_epi32(T052A, 7);                   // [32 22 12 02]
    __m128i T062B = _mm_srai_epi32(T052B, 7);                   // [72 62 52 42]
    __m128i T063A = _mm_srai_epi32(T053A, 7);                   // [33 23 13 03]
    __m128i T063B = _mm_srai_epi32(T053B, 7);                   // [73 63 53 43]
    __m128i T064A = _mm_srai_epi32(T054A, 7);                   // [33 24 14 04]
    __m128i T064B = _mm_srai_epi32(T054B, 7);                   // [74 64 54 44]
    __m128i T065A = _mm_srai_epi32(T055A, 7);                   // [35 25 15 05]
    __m128i T065B = _mm_srai_epi32(T055B, 7);                   // [75 65 55 45]
    __m128i T066A = _mm_srai_epi32(T056A, 7);                   // [36 26 16 06]
    __m128i T066B = _mm_srai_epi32(T056B, 7);                   // [76 66 56 46]
    __m128i T067A = _mm_srai_epi32(T057A, 7);                   // [37 27 17 07]
    __m128i T067B = _mm_srai_epi32(T057B, 7);                   // [77 67 57 47]
    __m128i T070  = _mm_packs_epi32(T060A, T060B);              // [70 60 50 40 30 20 10 00]
    __m128i T071  = _mm_packs_epi32(T061A, T061B);              // [71 61 51 41 31 21 11 01]
    __m128i T072  = _mm_packs_epi32(T062A, T062B);              // [72 62 52 42 32 22 12 02]
    __m128i T073  = _mm_packs_epi32(T063A, T063B);              // [73 63 53 43 33 23 13 03]
    __m128i T074  = _mm_packs_epi32(T064A, T064B);              // [74 64 54 44 34 24 14 04]
    __m128i T075  = _mm_packs_epi32(T065A, T065B);              // [75 65 55 45 35 25 15 05]
    __m128i T076  = _mm_packs_epi32(T066A, T066B);              // [76 66 56 46 36 26 16 06]
    __m128i T077  = _mm_packs_epi32(T067A, T067B);              // [77 67 57 47 37 27 17 07]

    // DCT2
    __m128i c8_tran0            = _mm_set_epi8(15, 14, 11, 10, 7, 6, 3, 2, 13, 12, 5, 4, 9, 8, 1, 0);
    __m128i c8_tran1            = _mm_set_epi8(15, 14, 11, 10, 7, 6, 3, 2, 13, 12, 9, 8, 5, 4, 1, 0);
    __m128i c8_tran2            = _mm_set_epi8(11, 10, 15, 14, 3, 2, 7, 6, 9, 8, 13, 12, 1, 0, 5, 4);
    __m128i c16_36_83_64_64     = _mm_set_epi16( 36, 83, 64, 64, 36, 83, 64, 64);
    __m128i c16_n83_36_n64_64   = _mm_set_epi16(-83, 36,-64, 64,-83, 36,-64, 64);
    __m128i c16_18_50_75_89     = _mm_set_epi16( 18, 50, 75, 89, 18, 50, 75, 89);
    __m128i c16_n50_n89_n18_75  = _mm_set_epi16(-50,-89,-18, 75,-50,-89,-18, 75);
    __m128i c16_75_18_n89_50    = _mm_set_epi16( 75, 18,-89, 50, 75, 18,-89, 50);
    __m128i c16_n89_75_n50_18   = _mm_set_epi16(-89, 75,-50, 18,-89, 75,-50, 18);
    __m128i T080  = _mm_shuffle_epi8(T070, c8_tran0);           // [07 05 03 01 06 02 04 00]
    __m128i T081  = _mm_shuffle_epi8(T071, c8_tran0);           // [17 15 13 11 16 12 14 10]
    __m128i T082  = _mm_shuffle_epi8(T072, c8_tran0);           // [27 25 23 21 26 22 24 20]
    __m128i T083  = _mm_shuffle_epi8(T073, c8_tran0);           // [37 35 33 31 36 32 34 30]
    __m128i T084  = _mm_shuffle_epi8(T074, c8_tran0);           // [47 45 43 41 46 42 44 40]
    __m128i T085  = _mm_shuffle_epi8(T075, c8_tran0);           // [57 55 53 51 56 52 54 50]
    __m128i T086  = _mm_shuffle_epi8(T076, c8_tran0);           // [67 65 63 61 66 62 64 60]
    __m128i T087  = _mm_shuffle_epi8(T077, c8_tran0);           // [77 75 73 71 76 72 74 70]
    __m128i T090  = _mm_unpacklo_epi64(T080, T081);             // [14 10 16 12 06 02 04 00]
    __m128i T091  = _mm_unpacklo_epi64(T082, T083);             // [34 30 36 32 26 22 24 20]
    __m128i T092  = _mm_unpacklo_epi64(T084, T085);             // [54 50 56 52 46 42 44 40]
    __m128i T093  = _mm_unpacklo_epi64(T086, T087);             // [74 70 76 72 66 62 64 60]
    __m128i T094  = _mm_unpackhi_epi64(T080, T081);             // [17 15 13 11 07 05 03 01]
    __m128i T095  = _mm_unpackhi_epi64(T082, T083);             // [37 35 33 31 27 25 23 21]
    __m128i T096  = _mm_unpackhi_epi64(T084, T085);             // [57 55 53 51 47 45 43 41]
    __m128i T097  = _mm_unpackhi_epi64(T086, T087);             // [77 75 73 71 67 65 63 61]
    __m128i T100  = _mm_madd_epi16(T090, c16_36_83_64_64);      // [EO0_1 EE0_1 EO0_0 EE0_0]
    __m128i T101  = _mm_madd_epi16(T090, c16_n83_36_n64_64);    // [EO1_1 EE1_1 EO1_0 EE1_0]
    __m128i T102  = _mm_madd_epi16(T091, c16_36_83_64_64);      // [EO0_3 EE0_3 EO0_2 EE0_2]
    __m128i T103  = _mm_madd_epi16(T091, c16_n83_36_n64_64);    // [EO1_3 EE1_3 EO1_2 EE1_2]
    __m128i T104  = _mm_madd_epi16(T092, c16_36_83_64_64);      // [EO0_5 EE0_5 EO0_4 EE0_4]
    __m128i T105  = _mm_madd_epi16(T092, c16_n83_36_n64_64);    // [EO1_5 EE1_5 EO1_4 EE1_4]
    __m128i T106  = _mm_madd_epi16(T093, c16_36_83_64_64);      // [EO0_7 EE0_7 EO0_6 EE0_6]
    __m128i T107  = _mm_madd_epi16(T093, c16_n83_36_n64_64);    // [EO1_7 EE1_7 EO1_6 EE1_6]
    __m128i T110A = _mm_madd_epi16(T094, c16_18_50_75_89);      // [O0_1B O0_1A O0_0B O0_0A]
    __m128i T110B = _mm_madd_epi16(T094, c16_n50_n89_n18_75);   // [O1_1B O1_1A O1_0B O1_0A]
    __m128i T110C = _mm_madd_epi16(T094, c16_75_18_n89_50);     // [O2_1B O2_1A O2_0B O2_0A]
    __m128i T110D = _mm_madd_epi16(T094, c16_n89_75_n50_18);    // [O3_1B O3_1A O3_0B O3_0A]
    __m128i T111A = _mm_madd_epi16(T095, c16_18_50_75_89);      // [O0_3B O0_3A O0_2B O0_2A]
    __m128i T111B = _mm_madd_epi16(T095, c16_n50_n89_n18_75);   // [O1_3B O1_3A O1_2B O1_2A]
    __m128i T111C = _mm_madd_epi16(T095, c16_75_18_n89_50);     // [O2_3B O2_3A O2_2B O2_2A]
    __m128i T111D = _mm_madd_epi16(T095, c16_n89_75_n50_18);    // [O3_3B O3_3A O3_2B O3_2A]
    __m128i T112A = _mm_madd_epi16(T096, c16_18_50_75_89);      // [O0_5B O0_5A O0_4B O0_4A]
    __m128i T112B = _mm_madd_epi16(T096, c16_n50_n89_n18_75);   // [O1_5B O1_5A O1_4B O1_4A]
    __m128i T112C = _mm_madd_epi16(T096, c16_75_18_n89_50);     // [O2_5B O2_5A O2_4B O2_4A]
    __m128i T112D = _mm_madd_epi16(T096, c16_n89_75_n50_18);    // [O3_5B O3_5A O3_4B O3_4A]
    __m128i T113A = _mm_madd_epi16(T097, c16_18_50_75_89);      // [O0_7B O0_7A O0_6B O0_6A]
    __m128i T113B = _mm_madd_epi16(T097, c16_n50_n89_n18_75);   // [O1_7B O1_7A O1_6B O1_6A]
    __m128i T113C = _mm_madd_epi16(T097, c16_75_18_n89_50);     // [O2_7B O2_7A O2_6B O2_6A]
    __m128i T113D = _mm_madd_epi16(T097, c16_n89_75_n50_18);    // [O3_7B O3_7A O3_6B O3_6A]
    __m128i T120A = _mm_hadd_epi32(T110A, T110B);               // [O1_1 O1_0 O0_1 O0_0]
    __m128i T120B = _mm_hadd_epi32(T111A, T111B);               // [O1_3 O1_2 O0_3 O0_2]
    __m128i T120C = _mm_hadd_epi32(T112A, T112B);               // [O1_5 O1_4 O0_5 O0_4]
    __m128i T120D = _mm_hadd_epi32(T113A, T113B);               // [O1_7 O1_6 O0_7 O0_6]
    __m128i T121A = _mm_hadd_epi32(T110C, T110D);               // [O3_1 O3_0 O2_1 O2_0]
    __m128i T121B = _mm_hadd_epi32(T111C, T111D);               // [O3_3 O3_2 O2_3 O2_2]
    __m128i T121C = _mm_hadd_epi32(T112C, T112D);               // [O3_5 O3_4 O2_5 O2_4]
    __m128i T121D = _mm_hadd_epi32(T113C, T113D);               // [O3_7 O3_6 O2_7 O2_6]
    __m128i T122A = _mm_hadd_epi32(T100, T101);                 // [E1_1 E1_0 E0_1 E0_0]
    __m128i T122B = _mm_hadd_epi32(T102, T103);                 // [E1_3 E1_2 E0_3 E0_2]
    __m128i T122C = _mm_hadd_epi32(T104, T105);                 // [E1_5 E1_4 E0_5 E0_5]
    __m128i T122D = _mm_hadd_epi32(T106, T107);                 // [E1_7 E1_6 E0_7 E0_7]
    __m128i T123A = _mm_hsub_epi32(T100, T101);                 // [E2_1 E2_0 E3_1 E3_0]
    __m128i T123B = _mm_hsub_epi32(T102, T103);                 // [E2_3 E2_2 E3_3 E3_2]
    __m128i T123C = _mm_hsub_epi32(T104, T105);                 // [E2_5 E2_4 E3_5 E3_4]
    __m128i T123D = _mm_hsub_epi32(T106, T107);                 // [E2_7 E2_6 E3_7 E3_6]
    __m128i T124A = _mm_shuffle_epi32(T123A, 0x4E);             // [E3_1 E3_0 E2_1 E2_0]
    __m128i T124B = _mm_shuffle_epi32(T123B, 0x4E);             // [E3_3 E3_2 E2_3 E2_2]
    __m128i T124C = _mm_shuffle_epi32(T123C, 0x4E);             // [E3_5 E3_4 E2_5 E2_4]
    __m128i T124D = _mm_shuffle_epi32(T123D, 0x4E);             // [E3_7 E3_6 E2_7 E2_6]
    __m128i T125A = _mm_add_epi32(T122A, c32_2048);             // Rnd 
    __m128i T125B = _mm_add_epi32(T122B, c32_2048);
    __m128i T125C = _mm_add_epi32(T122C, c32_2048);
    __m128i T125D = _mm_add_epi32(T122D, c32_2048);
    __m128i T126A = _mm_add_epi32(T124A, c32_2048);
    __m128i T126B = _mm_add_epi32(T124B, c32_2048);
    __m128i T126C = _mm_add_epi32(T124C, c32_2048);
    __m128i T126D = _mm_add_epi32(T124D, c32_2048);
    __m128i T130A = _mm_add_epi32(T125A, T120A);                // [11 01 10 00] = [E1_1+O1_1 E1_0+O1_0 E0_1+O0_1 E0_0+O0_0]
    __m128i T130B = _mm_add_epi32(T125B, T120B);                // [31 21 30 20] = [E1_3+O1_3 E1_2+O1_2 E0_3+O0_3 E0_2+O0_2]
    __m128i T130C = _mm_add_epi32(T125C, T120C);                // [51 41 50 40] = [E1_5+O1_5 E1_4+O1_4 E0_5+O0_5 E0_4+O0_4]
    __m128i T130D = _mm_add_epi32(T125D, T120D);                // [71 61 70 60] = [E1_7+O1_7 E1_6+O1_6 E0_7+O0_7 E0_6+O0_6]
    __m128i T131A = _mm_add_epi32(T126A, T121A);                // [13 03 12 02] = [E3_1+O3_1 E3_0+O3_0 E2_1+O2_1 E2_0+O2_0]
    __m128i T131B = _mm_add_epi32(T126B, T121B);                // [33 23 32 22] = [E3_3+O3_3 E3_2+O3_2 E2_3+O2_3 E2_2+O2_2]
    __m128i T131C = _mm_add_epi32(T126C, T121C);                // [53 43 52 42] = [E3_5+O3_5 E3_4+O3_4 E2_5+O2_5 E2_4+O2_4]
    __m128i T131D = _mm_add_epi32(T126D, T121D);                // [73 63 72 62] = [E3_7+O3_7 E3_6+O3_6 E2_7+O2_7 E2_6+O2_6]
    __m128i T132A = _mm_sub_epi32(T126A, T121A);                // [14 04 15 05] = [E3_1-O3_1 E3_0-O3_0 E2_1-O2_1 E2_0-O2_0]
    __m128i T132B = _mm_sub_epi32(T126B, T121B);                // [34 24 35 25] = [E3_3-O3_3 E3_2-O3_2 E2_3-O2_3 E2_2-O2_2]
    __m128i T132C = _mm_sub_epi32(T126C, T121C);                // [54 44 55 45] = [E3_5-O3_5 E3_4-O3_4 E2_5-O2_5 E2_4-O2_4]
    __m128i T132D = _mm_sub_epi32(T126D, T121D);                // [74 64 75 65] = [E3_7-O3_7 E3_6-O3_6 E2_7-O2_7 E2_6-O2_6]
    __m128i T133A = _mm_sub_epi32(T125A, T120A);                // [16 06 17 07] = [E1_1-O1_1 E1_0-O1_0 E0_1-O0_1 E0_0-O0_0]
    __m128i T133B = _mm_sub_epi32(T125B, T120B);                // [36 26 37 27] = [E3_1-O3_1 E1_2-O1_2 E0_3-O0_3 E0_2-O0_2]
    __m128i T133C = _mm_sub_epi32(T125C, T120C);                // [56 46 57 47] = [E5_1-O5_1 E1_4-O1_4 E0_5-O0_5 E0_4-O0_4]
    __m128i T133D = _mm_sub_epi32(T125D, T120D);                // [76 66 77 67] = [E7_1-O7_1 E1_6-O1_6 E0_7-O0_7 E0_6-O0_6]
    __m128i T140A = _mm_srai_epi32(T130A, 12);                  // [11 01 10 00]
    __m128i T140B = _mm_srai_epi32(T130B, 12);                  // [31 21 30 20]
    __m128i T140C = _mm_srai_epi32(T130C, 12);                  // [51 41 50 40]
    __m128i T140D = _mm_srai_epi32(T130D, 12);                  // [71 61 70 60]
    __m128i T141A = _mm_srai_epi32(T131A, 12);                  // [13 03 12 02]
    __m128i T141B = _mm_srai_epi32(T131B, 12);                  // [33 23 32 22]
    __m128i T141C = _mm_srai_epi32(T131C, 12);                  // [53 43 52 42]
    __m128i T141D = _mm_srai_epi32(T131D, 12);                  // [63 63 72 62]
    __m128i T142A = _mm_srai_epi32(T132A, 12);                  // [14 04 15 05]
    __m128i T142B = _mm_srai_epi32(T132B, 12);                  // [34 24 35 25]
    __m128i T142C = _mm_srai_epi32(T132C, 12);                  // [54 44 55 45]
    __m128i T142D = _mm_srai_epi32(T132D, 12);                  // [74 64 75 65]
    __m128i T143A = _mm_srai_epi32(T133A, 12);                  // [16 06 17 07]
    __m128i T143B = _mm_srai_epi32(T133B, 12);                  // [36 26 37 27]
    __m128i T143C = _mm_srai_epi32(T133C, 12);                  // [56 46 57 47]
    __m128i T143D = _mm_srai_epi32(T133D, 12);                  // [76 66 77 67]
    __m128i T150  = _mm_packs_epi32(T140A, T141A);              // [13 03 12 02 11 01 10 00]
    __m128i T151  = _mm_packs_epi32(T140B, T141B);              // [33 23 32 22 31 21 30 20]
    __m128i T152  = _mm_packs_epi32(T140C, T141C);              // [53 43 52 42 51 41 50 40]
    __m128i T153  = _mm_packs_epi32(T140D, T141D);              // [73 63 72 62 71 61 70 60]
    __m128i T154  = _mm_packs_epi32(T142A, T143A);              // [16 06 17 07 14 04 15 05]
    __m128i T155  = _mm_packs_epi32(T142B, T143B);              // [36 26 37 27 34 24 35 25]
    __m128i T156  = _mm_packs_epi32(T142C, T143C);              // [56 46 57 47 54 44 55 45]
    __m128i T157  = _mm_packs_epi32(T142D, T143D);              // [76 66 77 67 74 64 75 65]
    __m128i T160  = _mm_shuffle_epi8(T150, c8_tran1);           // [13 12 11 10 03 02 01 00]
    __m128i T161  = _mm_shuffle_epi8(T151, c8_tran1);           // [33 32 31 30 23 22 21 20]
    __m128i T162  = _mm_shuffle_epi8(T152, c8_tran1);           // [53 52 51 50 43 42 41 40]
    __m128i T163  = _mm_shuffle_epi8(T153, c8_tran1);           // [73 72 71 70 63 62 61 60]
    __m128i T164  = _mm_shuffle_epi8(T154, c8_tran2);           // [17 16 15 14 07 06 05 04]
    __m128i T165  = _mm_shuffle_epi8(T155, c8_tran2);           // [37 36 35 34 27 26 25 24]
    __m128i T166  = _mm_shuffle_epi8(T156, c8_tran2);           // [57 56 55 54 47 46 45 44]
    __m128i T167  = _mm_shuffle_epi8(T157, c8_tran2);           // [77 76 75 74 67 66 65 64]
    __m128i T170  = _mm_unpacklo_epi64(T160, T164);             // [07 06 05 04 03 02 01 00]
    __m128i T171  = _mm_unpackhi_epi64(T160, T164);             // [17 16 15 14 13 12 11 10]
    __m128i T172  = _mm_unpacklo_epi64(T161, T165);             // [27 26 25 24 23 22 21 20]
    __m128i T173  = _mm_unpackhi_epi64(T161, T165);             // [37 36 35 34 33 32 31 30]
    __m128i T174  = _mm_unpacklo_epi64(T162, T166);             // [47 46 45 44 43 42 41 40]
    __m128i T175  = _mm_unpackhi_epi64(T162, T166);             // [57 56 55 54 53 52 51 50]
    __m128i T176  = _mm_unpacklo_epi64(T163, T167);             // [67 66 65 64 63 62 61 60]
    __m128i T177  = _mm_unpackhi_epi64(T163, T167);             // [77 76 75 74 73 72 71 70]

    // Add
    __m128i T180 = _mm_loadl_epi64((__m128i*)&pRef[0*nStride]);
    __m128i T181 = _mm_loadl_epi64((__m128i*)&pRef[1*nStride]);
    __m128i T182 = _mm_loadl_epi64((__m128i*)&pRef[2*nStride]);
    __m128i T183 = _mm_loadl_epi64((__m128i*)&pRef[3*nStride]);
    __m128i T184 = _mm_loadl_epi64((__m128i*)&pRef[4*nStride]);
    __m128i T185 = _mm_loadl_epi64((__m128i*)&pRef[5*nStride]);
    __m128i T186 = _mm_loadl_epi64((__m128i*)&pRef[6*nStride]);
    __m128i T187 = _mm_loadl_epi64((__m128i*)&pRef[7*nStride]);
    __m128i T190 = _mm_cvtepu8_epi16(T180);
    __m128i T191 = _mm_cvtepu8_epi16(T181);
    __m128i T192 = _mm_cvtepu8_epi16(T182);
    __m128i T193 = _mm_cvtepu8_epi16(T183);
    __m128i T194 = _mm_cvtepu8_epi16(T184);
    __m128i T195 = _mm_cvtepu8_epi16(T185);
    __m128i T196 = _mm_cvtepu8_epi16(T186);
    __m128i T197 = _mm_cvtepu8_epi16(T187);
    __m128i T200 = _mm_add_epi16(T170, T190);
    __m128i T201 = _mm_add_epi16(T171, T191);
    __m128i T202 = _mm_add_epi16(T172, T192);
    __m128i T203 = _mm_add_epi16(T173, T193);
    __m128i T204 = _mm_add_epi16(T174, T194);
    __m128i T205 = _mm_add_epi16(T175, T195);
    __m128i T206 = _mm_add_epi16(T176, T196);
    __m128i T207 = _mm_add_epi16(T177, T197);
    __m128i T210 = _mm_packus_epi16(T200, T201);
    __m128i T211 = _mm_packus_epi16(T202, T203);
    __m128i T212 = _mm_packus_epi16(T204, T205);
    __m128i T213 = _mm_packus_epi16(T206, T207);
    _mm_storel_epi64((__m128i*)&pDst[0*nStrideDst], T210);
    //_mm_storel_epi64((__m128i*)&pDst[1*nStrideDst], _mm_unpackhi_epi64(T210, T210));
    _mm_storeh_pi   ((__m64*  )&pDst[1*nStrideDst], _mm_castsi128_ps(T210));
    _mm_storel_epi64((__m128i*)&pDst[2*nStrideDst], T211);
    _mm_storeh_pi   ((__m64*  )&pDst[3*nStrideDst], _mm_castsi128_ps(T211));
    _mm_storel_epi64((__m128i*)&pDst[4*nStrideDst], T212);
    _mm_storeh_pi   ((__m64*  )&pDst[5*nStrideDst], _mm_castsi128_ps(T212));
    _mm_storel_epi64((__m128i*)&pDst[6*nStrideDst], T213);
    _mm_storeh_pi   ((__m64*  )&pDst[7*nStrideDst], _mm_castsi128_ps(T213));
}
#else // ASM_NONE
void xIDctAdd8(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;

    // DCT1
    nShift = SHIFT_INV_1ST;
    rnd    = 1<<(nShift-1);
    for( i=0; i<8; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 = 89*pSrc[1*nStride+i] + 75*pSrc[3*nStride+i] + 50*pSrc[5*nStride+i] + 18*pSrc[7*nStride+i];
        Int32 O1 = 75*pSrc[1*nStride+i] - 18*pSrc[3*nStride+i] - 89*pSrc[5*nStride+i] - 50*pSrc[7*nStride+i];
        Int32 O2 = 50*pSrc[1*nStride+i] - 89*pSrc[3*nStride+i] + 18*pSrc[5*nStride+i] + 75*pSrc[7*nStride+i];
        Int32 O3 = 18*pSrc[1*nStride+i] - 50*pSrc[3*nStride+i] + 75*pSrc[5*nStride+i] - 89*pSrc[7*nStride+i];

        Int32 EO0 = 83*pSrc[2*nStride+i] + 36*pSrc[6*nStride+i];
        Int32 EO1 = 36*pSrc[2*nStride+i] - 83*pSrc[6*nStride+i];
        Int32 EE0 = 64*pSrc[0*nStride+i] + 64*pSrc[4*nStride+i];
        Int32 EE1 = 64*pSrc[0*nStride+i] - 64*pSrc[4*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        Int32 E0 = EE0 + EO0;
        Int32 E3 = EE0 - EO0;
        Int32 E1 = EE1 + EO1;
        Int32 E2 = EE1 - EO1;

        piTmp0[i*8+0] = Clip3( -32768, 32767, (E0 + O0 + rnd) >> nShift);
        piTmp0[i*8+1] = Clip3( -32768, 32767, (E1 + O1 + rnd) >> nShift);
        piTmp0[i*8+2] = Clip3( -32768, 32767, (E2 + O2 + rnd) >> nShift);
        piTmp0[i*8+3] = Clip3( -32768, 32767, (E3 + O3 + rnd) >> nShift);
        piTmp0[i*8+4] = Clip3( -32768, 32767, (E3 - O3 + rnd) >> nShift);
        piTmp0[i*8+5] = Clip3( -32768, 32767, (E2 - O2 + rnd) >> nShift);
        piTmp0[i*8+6] = Clip3( -32768, 32767, (E1 - O1 + rnd) >> nShift);
        piTmp0[i*8+7] = Clip3( -32768, 32767, (E0 - O0 + rnd) >> nShift);
    }

    // DCT2
    nShift = SHIFT_INV_2ND;
    rnd    = 1<<(nShift-1);
    for( i=0; i<8; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 = 89*piTmp0[1*8+i] + 75*piTmp0[3*8+i] + 50*piTmp0[5*8+i] + 18*piTmp0[7*8+i];
        Int32 O1 = 75*piTmp0[1*8+i] - 18*piTmp0[3*8+i] - 89*piTmp0[5*8+i] - 50*piTmp0[7*8+i];
        Int32 O2 = 50*piTmp0[1*8+i] - 89*piTmp0[3*8+i] + 18*piTmp0[5*8+i] + 75*piTmp0[7*8+i];
        Int32 O3 = 18*piTmp0[1*8+i] - 50*piTmp0[3*8+i] + 75*piTmp0[5*8+i] - 89*piTmp0[7*8+i];

        Int32 EO0 = 83*piTmp0[2*8+i] + 36*piTmp0[6*8+i];
        Int32 EO1 = 36*piTmp0[2*8+i] - 83*piTmp0[6*8+i];
        Int32 EE0 = 64*piTmp0[0*8+i] + 64*piTmp0[4*8+i];
        Int32 EE1 = 64*piTmp0[0*8+i] - 64*piTmp0[4*8+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        Int32 E0 = EE0 + EO0;
        Int32 E3 = EE0 - EO0;
        Int32 E1 = EE1 + EO1;
        Int32 E2 = EE1 - EO1;

        piTmp1[i*8+0] = Clip3( -32768, 32767, (E0 + O0 + rnd) >> nShift);
        piTmp1[i*8+1] = Clip3( -32768, 32767, (E1 + O1 + rnd) >> nShift);
        piTmp1[i*8+2] = Clip3( -32768, 32767, (E2 + O2 + rnd) >> nShift);
        piTmp1[i*8+3] = Clip3( -32768, 32767, (E3 + O3 + rnd) >> nShift);
        piTmp1[i*8+4] = Clip3( -32768, 32767, (E3 - O3 + rnd) >> nShift);
        piTmp1[i*8+5] = Clip3( -32768, 32767, (E2 - O2 + rnd) >> nShift);
        piTmp1[i*8+6] = Clip3( -32768, 32767, (E1 - O1 + rnd) >> nShift);
        piTmp1[i*8+7] = Clip3( -32768, 32767, (E0 - O0 + rnd) >> nShift);
    }

    // Add
    for( i=0; i<8; i++ ) {
        for( j=0; j<8; j++ ) {
            pDst[i * nStrideDst + j] = Clip( piTmp1[i * 8 + j] + pRef[i * nStride + j] );
        }
    }
}
#endif

void xIDctAdd16(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;

    // DCT1
    nShift = SHIFT_INV_1ST;
    rnd    = 1<<(nShift-1);
    for( i=0; i<16; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 =   90*pSrc[ 1*nStride+i] + 87*pSrc[ 3*nStride+i] + 80*pSrc[ 5*nStride+i] + 70*pSrc[ 7*nStride+i]
                   + 57*pSrc[ 9*nStride+i] + 43*pSrc[11*nStride+i] + 25*pSrc[13*nStride+i] +  9*pSrc[15*nStride+i];
        Int32 O1 =   87*pSrc[ 1*nStride+i] + 57*pSrc[ 3*nStride+i] +  9*pSrc[ 5*nStride+i] - 43*pSrc[ 7*nStride+i]
                   - 80*pSrc[ 9*nStride+i] - 90*pSrc[11*nStride+i] - 70*pSrc[13*nStride+i] - 25*pSrc[15*nStride+i];
        Int32 O2 =   80*pSrc[ 1*nStride+i] +  9*pSrc[ 3*nStride+i] - 70*pSrc[ 5*nStride+i] - 87*pSrc[ 7*nStride+i]
                   - 25*pSrc[ 9*nStride+i] + 57*pSrc[11*nStride+i] + 90*pSrc[13*nStride+i] + 43*pSrc[15*nStride+i];
        Int32 O3 =   70*pSrc[ 1*nStride+i] - 43*pSrc[ 3*nStride+i] - 87*pSrc[ 5*nStride+i] +  9*pSrc[ 7*nStride+i]
                   + 90*pSrc[ 9*nStride+i] + 25*pSrc[11*nStride+i] - 80*pSrc[13*nStride+i] - 57*pSrc[15*nStride+i];
        Int32 O4 =   57*pSrc[ 1*nStride+i] - 80*pSrc[ 3*nStride+i] - 25*pSrc[ 5*nStride+i] + 90*pSrc[ 7*nStride+i]
                   -  9*pSrc[ 9*nStride+i] - 87*pSrc[11*nStride+i] + 43*pSrc[13*nStride+i] + 70*pSrc[15*nStride+i];
        Int32 O5 =   43*pSrc[ 1*nStride+i] - 90*pSrc[ 3*nStride+i] + 57*pSrc[ 5*nStride+i] + 25*pSrc[ 7*nStride+i]
                   - 87*pSrc[ 9*nStride+i] + 70*pSrc[11*nStride+i] +  9*pSrc[13*nStride+i] - 80*pSrc[15*nStride+i];
        Int32 O6 =   25*pSrc[ 1*nStride+i] - 70*pSrc[ 3*nStride+i] + 90*pSrc[ 5*nStride+i] - 80*pSrc[ 7*nStride+i]
                   + 43*pSrc[ 9*nStride+i] +  9*pSrc[11*nStride+i] - 57*pSrc[13*nStride+i] + 87*pSrc[15*nStride+i];
        Int32 O7 =    9*pSrc[ 1*nStride+i] - 25*pSrc[ 3*nStride+i] + 43*pSrc[ 5*nStride+i] - 57*pSrc[ 7*nStride+i]
                   + 70*pSrc[ 9*nStride+i] - 80*pSrc[11*nStride+i] + 87*pSrc[13*nStride+i] - 90*pSrc[15*nStride+i];

        Int32 EO0 = 89*pSrc[ 2*nStride+i] + 75*pSrc[ 6*nStride+i] + 50*pSrc[10*nStride+i] + 18*pSrc[14*nStride+i];
        Int32 EO1 = 75*pSrc[ 2*nStride+i] - 18*pSrc[ 6*nStride+i] - 89*pSrc[10*nStride+i] - 50*pSrc[14*nStride+i];
        Int32 EO2 = 50*pSrc[ 2*nStride+i] - 89*pSrc[ 6*nStride+i] + 18*pSrc[10*nStride+i] + 75*pSrc[14*nStride+i];
        Int32 EO3 = 18*pSrc[ 2*nStride+i] - 50*pSrc[ 6*nStride+i] + 75*pSrc[10*nStride+i] - 89*pSrc[14*nStride+i];

        Int32 EEO0 = 83*pSrc[4*nStride+i] + 36*pSrc[12*nStride+i];
        Int32 EEO1 = 36*pSrc[4*nStride+i] - 83*pSrc[12*nStride+i];
        Int32 EEE0 = 64*pSrc[0*nStride+i] + 64*pSrc[ 8*nStride+i];
        Int32 EEE1 = 64*pSrc[0*nStride+i] - 64*pSrc[ 8*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        Int32 EE0 = EEE0 + EEO0;
        Int32 EE3 = EEE0 - EEO0;
        Int32 EE1 = EEE1 + EEO1;
        Int32 EE2 = EEE1 - EEO1;

        Int32 E0 = EE0 + EO0;
        Int32 E7 = EE0 - EO0;
        Int32 E1 = EE1 + EO1;
        Int32 E6 = EE1 - EO1;
        Int32 E2 = EE2 + EO2;
        Int32 E5 = EE2 - EO2;
        Int32 E3 = EE3 + EO3;
        Int32 E4 = EE3 - EO3;

        piTmp0[i*16+ 0] = Clip3( -32768, 32767, (E0 + O0 + rnd) >> nShift);
        piTmp0[i*16+15] = Clip3( -32768, 32767, (E0 - O0 + rnd) >> nShift);
        piTmp0[i*16+ 1] = Clip3( -32768, 32767, (E1 + O1 + rnd) >> nShift);
        piTmp0[i*16+14] = Clip3( -32768, 32767, (E1 - O1 + rnd) >> nShift);
        piTmp0[i*16+ 2] = Clip3( -32768, 32767, (E2 + O2 + rnd) >> nShift);
        piTmp0[i*16+13] = Clip3( -32768, 32767, (E2 - O2 + rnd) >> nShift);
        piTmp0[i*16+ 3] = Clip3( -32768, 32767, (E3 + O3 + rnd) >> nShift);
        piTmp0[i*16+12] = Clip3( -32768, 32767, (E3 - O3 + rnd) >> nShift);
        piTmp0[i*16+ 4] = Clip3( -32768, 32767, (E4 + O4 + rnd) >> nShift);
        piTmp0[i*16+11] = Clip3( -32768, 32767, (E4 - O4 + rnd) >> nShift);
        piTmp0[i*16+ 5] = Clip3( -32768, 32767, (E5 + O5 + rnd) >> nShift);
        piTmp0[i*16+10] = Clip3( -32768, 32767, (E5 - O5 + rnd) >> nShift);
        piTmp0[i*16+ 6] = Clip3( -32768, 32767, (E6 + O6 + rnd) >> nShift);
        piTmp0[i*16+ 9] = Clip3( -32768, 32767, (E6 - O6 + rnd) >> nShift);
        piTmp0[i*16+ 7] = Clip3( -32768, 32767, (E7 + O7 + rnd) >> nShift);
        piTmp0[i*16+ 8] = Clip3( -32768, 32767, (E7 - O7 + rnd) >> nShift);
    }

    // DCT2
    nShift = SHIFT_INV_2ND;
    rnd    = 1<<(nShift-1);
    for( i=0; i<16; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        Int32 O0 =   90*piTmp0[ 1*16+i] + 87*piTmp0[ 3*16+i] + 80*piTmp0[ 5*16+i] + 70*piTmp0[ 7*16+i]
                   + 57*piTmp0[ 9*16+i] + 43*piTmp0[11*16+i] + 25*piTmp0[13*16+i] +  9*piTmp0[15*16+i];
        Int32 O1 =   87*piTmp0[ 1*16+i] + 57*piTmp0[ 3*16+i] +  9*piTmp0[ 5*16+i] - 43*piTmp0[ 7*16+i]
                   - 80*piTmp0[ 9*16+i] - 90*piTmp0[11*16+i] - 70*piTmp0[13*16+i] - 25*piTmp0[15*16+i];
        Int32 O2 =   80*piTmp0[ 1*16+i] +  9*piTmp0[ 3*16+i] - 70*piTmp0[ 5*16+i] - 87*piTmp0[ 7*16+i]
                   - 25*piTmp0[ 9*16+i] + 57*piTmp0[11*16+i] + 90*piTmp0[13*16+i] + 43*piTmp0[15*16+i];
        Int32 O3 =   70*piTmp0[ 1*16+i] - 43*piTmp0[ 3*16+i] - 87*piTmp0[ 5*16+i] +  9*piTmp0[ 7*16+i]
                   + 90*piTmp0[ 9*16+i] + 25*piTmp0[11*16+i] - 80*piTmp0[13*16+i] - 57*piTmp0[15*16+i];
        Int32 O4 =   57*piTmp0[ 1*16+i] - 80*piTmp0[ 3*16+i] - 25*piTmp0[ 5*16+i] + 90*piTmp0[ 7*16+i]
                   -  9*piTmp0[ 9*16+i] - 87*piTmp0[11*16+i] + 43*piTmp0[13*16+i] + 70*piTmp0[15*16+i];
        Int32 O5 =   43*piTmp0[ 1*16+i] - 90*piTmp0[ 3*16+i] + 57*piTmp0[ 5*16+i] + 25*piTmp0[ 7*16+i]
                   - 87*piTmp0[ 9*16+i] + 70*piTmp0[11*16+i] +  9*piTmp0[13*16+i] - 80*piTmp0[15*16+i];
        Int32 O6 =   25*piTmp0[ 1*16+i] - 70*piTmp0[ 3*16+i] + 90*piTmp0[ 5*16+i] - 80*piTmp0[ 7*16+i]
                   + 43*piTmp0[ 9*16+i] +  9*piTmp0[11*16+i] - 57*piTmp0[13*16+i] + 87*piTmp0[15*16+i];
        Int32 O7 =    9*piTmp0[ 1*16+i] - 25*piTmp0[ 3*16+i] + 43*piTmp0[ 5*16+i] - 57*piTmp0[ 7*16+i]
                   + 70*piTmp0[ 9*16+i] - 80*piTmp0[11*16+i] + 87*piTmp0[13*16+i] - 90*piTmp0[15*16+i];

        Int32 EO0 = 89*piTmp0[ 2*16+i] + 75*piTmp0[ 6*16+i] + 50*piTmp0[10*16+i] + 18*piTmp0[14*16+i];
        Int32 EO1 = 75*piTmp0[ 2*16+i] - 18*piTmp0[ 6*16+i] - 89*piTmp0[10*16+i] - 50*piTmp0[14*16+i];
        Int32 EO2 = 50*piTmp0[ 2*16+i] - 89*piTmp0[ 6*16+i] + 18*piTmp0[10*16+i] + 75*piTmp0[14*16+i];
        Int32 EO3 = 18*piTmp0[ 2*16+i] - 50*piTmp0[ 6*16+i] + 75*piTmp0[10*16+i] - 89*piTmp0[14*16+i];

        Int32 EEO0 = 83*piTmp0[4*16+i] + 36*piTmp0[12*16+i];
        Int32 EEO1 = 36*piTmp0[4*16+i] - 83*piTmp0[12*16+i];
        Int32 EEE0 = 64*piTmp0[0*16+i] + 64*piTmp0[ 8*16+i];
        Int32 EEE1 = 64*piTmp0[0*16+i] - 64*piTmp0[ 8*16+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        Int32 EE0 = EEE0 + EEO0;
        Int32 EE3 = EEE0 - EEO0;
        Int32 EE1 = EEE1 + EEO1;
        Int32 EE2 = EEE1 - EEO1;

        Int32 E0 = EE0 + EO0;
        Int32 E7 = EE0 - EO0;
        Int32 E1 = EE1 + EO1;
        Int32 E6 = EE1 - EO1;
        Int32 E2 = EE2 + EO2;
        Int32 E5 = EE2 - EO2;
        Int32 E3 = EE3 + EO3;
        Int32 E4 = EE3 - EO3;

        piTmp1[i*16+ 0] = Clip3( -32768, 32767, (E0 + O0 + rnd) >> nShift);
        piTmp1[i*16+15] = Clip3( -32768, 32767, (E0 - O0 + rnd) >> nShift);
        piTmp1[i*16+ 1] = Clip3( -32768, 32767, (E1 + O1 + rnd) >> nShift);
        piTmp1[i*16+14] = Clip3( -32768, 32767, (E1 - O1 + rnd) >> nShift);
        piTmp1[i*16+ 2] = Clip3( -32768, 32767, (E2 + O2 + rnd) >> nShift);
        piTmp1[i*16+13] = Clip3( -32768, 32767, (E2 - O2 + rnd) >> nShift);
        piTmp1[i*16+ 3] = Clip3( -32768, 32767, (E3 + O3 + rnd) >> nShift);
        piTmp1[i*16+12] = Clip3( -32768, 32767, (E3 - O3 + rnd) >> nShift);
        piTmp1[i*16+ 4] = Clip3( -32768, 32767, (E4 + O4 + rnd) >> nShift);
        piTmp1[i*16+11] = Clip3( -32768, 32767, (E4 - O4 + rnd) >> nShift);
        piTmp1[i*16+ 5] = Clip3( -32768, 32767, (E5 + O5 + rnd) >> nShift);
        piTmp1[i*16+10] = Clip3( -32768, 32767, (E5 - O5 + rnd) >> nShift);
        piTmp1[i*16+ 6] = Clip3( -32768, 32767, (E6 + O6 + rnd) >> nShift);
        piTmp1[i*16+ 9] = Clip3( -32768, 32767, (E6 - O6 + rnd) >> nShift);
        piTmp1[i*16+ 7] = Clip3( -32768, 32767, (E7 + O7 + rnd) >> nShift);
        piTmp1[i*16+ 8] = Clip3( -32768, 32767, (E7 - O7 + rnd) >> nShift);
    }

    // Add
    for( i=0; i<16; i++ ) {
        for( j=0; j<16; j++ ) {
            pDst[i * nStrideDst + j] = Clip( piTmp1[i * 16 + j] + pRef[i * nStride + j] );
        }
    }
}

void xIDctAdd32(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    int i, j;
    int nShift;
    int rnd;
    Int32 E[16],O[16];
    Int32 EE[8],EO[8];
    Int32 EEE[4],EEO[4];
    Int32 EEEE[2],EEEO[2];

    // DCT1
    nShift = SHIFT_INV_1ST;
    rnd    = 1<<(nShift-1);
    for( i=0; i<32; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        O[ 0] =   90*pSrc[ 1*nStride+i] + 90*pSrc[ 3*nStride+i] + 88*pSrc[ 5*nStride+i] + 85*pSrc[ 7*nStride+i]
                + 82*pSrc[ 9*nStride+i] + 78*pSrc[11*nStride+i] + 73*pSrc[13*nStride+i] + 67*pSrc[15*nStride+i]
                + 61*pSrc[17*nStride+i] + 54*pSrc[19*nStride+i] + 46*pSrc[21*nStride+i] + 38*pSrc[23*nStride+i]
                + 31*pSrc[25*nStride+i] + 22*pSrc[27*nStride+i] + 13*pSrc[29*nStride+i] +  4*pSrc[31*nStride+i];
        O[ 1] =   90*pSrc[ 1*nStride+i] + 82*pSrc[ 3*nStride+i] + 67*pSrc[ 5*nStride+i] + 46*pSrc[ 7*nStride+i]
                + 22*pSrc[ 9*nStride+i] -  4*pSrc[11*nStride+i] - 31*pSrc[13*nStride+i] - 54*pSrc[15*nStride+i]
                - 73*pSrc[17*nStride+i] - 85*pSrc[19*nStride+i] - 90*pSrc[21*nStride+i] - 88*pSrc[23*nStride+i]
                - 78*pSrc[25*nStride+i] - 61*pSrc[27*nStride+i] - 38*pSrc[29*nStride+i] - 13*pSrc[31*nStride+i];
        O[ 2] =   88*pSrc[ 1*nStride+i] + 67*pSrc[ 3*nStride+i] + 31*pSrc[ 5*nStride+i] - 13*pSrc[ 7*nStride+i]
                - 54*pSrc[ 9*nStride+i] - 82*pSrc[11*nStride+i] - 90*pSrc[13*nStride+i] - 78*pSrc[15*nStride+i]
                - 46*pSrc[17*nStride+i] -  4*pSrc[19*nStride+i] + 38*pSrc[21*nStride+i] + 73*pSrc[23*nStride+i]
                + 90*pSrc[25*nStride+i] + 85*pSrc[27*nStride+i] + 61*pSrc[29*nStride+i] + 22*pSrc[31*nStride+i];
        O[ 3] =   85*pSrc[ 1*nStride+i] + 46*pSrc[ 3*nStride+i] - 13*pSrc[ 5*nStride+i] - 67*pSrc[ 7*nStride+i]
                - 90*pSrc[ 9*nStride+i] - 73*pSrc[11*nStride+i] - 22*pSrc[13*nStride+i] + 38*pSrc[15*nStride+i]
                + 82*pSrc[17*nStride+i] + 88*pSrc[19*nStride+i] + 54*pSrc[21*nStride+i] -  4*pSrc[23*nStride+i]
                - 61*pSrc[25*nStride+i] - 90*pSrc[27*nStride+i] - 78*pSrc[29*nStride+i] - 31*pSrc[31*nStride+i];
        O[ 4] =   82*pSrc[ 1*nStride+i] + 22*pSrc[ 3*nStride+i] - 54*pSrc[ 5*nStride+i] - 90*pSrc[ 7*nStride+i]
                - 61*pSrc[ 9*nStride+i] + 13*pSrc[11*nStride+i] + 78*pSrc[13*nStride+i] + 85*pSrc[15*nStride+i]
                + 31*pSrc[17*nStride+i] - 46*pSrc[19*nStride+i] - 90*pSrc[21*nStride+i] - 67*pSrc[23*nStride+i]
                +  4*pSrc[25*nStride+i] + 73*pSrc[27*nStride+i] + 88*pSrc[29*nStride+i] + 38*pSrc[31*nStride+i];
        O[ 5] =   78*pSrc[ 1*nStride+i] -  4*pSrc[ 3*nStride+i] - 82*pSrc[ 5*nStride+i] - 73*pSrc[ 7*nStride+i]
                + 13*pSrc[ 9*nStride+i] + 85*pSrc[11*nStride+i] + 67*pSrc[13*nStride+i] - 22*pSrc[15*nStride+i]
                - 88*pSrc[17*nStride+i] - 61*pSrc[19*nStride+i] + 31*pSrc[21*nStride+i] + 90*pSrc[23*nStride+i]
                + 54*pSrc[25*nStride+i] - 38*pSrc[27*nStride+i] - 90*pSrc[29*nStride+i] - 46*pSrc[31*nStride+i];
        O[ 6] =   73*pSrc[ 1*nStride+i] - 31*pSrc[ 3*nStride+i] - 90*pSrc[ 5*nStride+i] - 22*pSrc[ 7*nStride+i]
                + 78*pSrc[ 9*nStride+i] + 67*pSrc[11*nStride+i] - 38*pSrc[13*nStride+i] - 90*pSrc[15*nStride+i]
                - 13*pSrc[17*nStride+i] + 82*pSrc[19*nStride+i] + 61*pSrc[21*nStride+i] - 46*pSrc[23*nStride+i]
                - 88*pSrc[25*nStride+i] -  4*pSrc[27*nStride+i] + 85*pSrc[29*nStride+i] + 54*pSrc[31*nStride+i];
        O[ 7] =   67*pSrc[ 1*nStride+i] - 54*pSrc[ 3*nStride+i] - 78*pSrc[ 5*nStride+i] + 38*pSrc[ 7*nStride+i]
                + 85*pSrc[ 9*nStride+i] - 22*pSrc[11*nStride+i] - 90*pSrc[13*nStride+i] +  4*pSrc[15*nStride+i]
                + 90*pSrc[17*nStride+i] + 13*pSrc[19*nStride+i] - 88*pSrc[21*nStride+i] - 31*pSrc[23*nStride+i]
                + 82*pSrc[25*nStride+i] + 46*pSrc[27*nStride+i] - 73*pSrc[29*nStride+i] - 61*pSrc[31*nStride+i];
        O[ 8] =   61*pSrc[ 1*nStride+i] - 73*pSrc[ 3*nStride+i] - 46*pSrc[ 5*nStride+i] + 82*pSrc[ 7*nStride+i]
                + 31*pSrc[ 9*nStride+i] - 88*pSrc[11*nStride+i] - 13*pSrc[13*nStride+i] + 90*pSrc[15*nStride+i]
                -  4*pSrc[17*nStride+i] - 90*pSrc[19*nStride+i] + 22*pSrc[21*nStride+i] + 85*pSrc[23*nStride+i]
                - 38*pSrc[25*nStride+i] - 78*pSrc[27*nStride+i] + 54*pSrc[29*nStride+i] + 67*pSrc[31*nStride+i];
        O[ 9] =   54*pSrc[ 1*nStride+i] - 85*pSrc[ 3*nStride+i] -  4*pSrc[ 5*nStride+i] + 88*pSrc[ 7*nStride+i]
                - 46*pSrc[ 9*nStride+i] - 61*pSrc[11*nStride+i] + 82*pSrc[13*nStride+i] + 13*pSrc[15*nStride+i]
                - 90*pSrc[17*nStride+i] + 38*pSrc[19*nStride+i] + 67*pSrc[21*nStride+i] - 78*pSrc[23*nStride+i]
                - 22*pSrc[25*nStride+i] + 90*pSrc[27*nStride+i] - 31*pSrc[29*nStride+i] - 73*pSrc[31*nStride+i];
        O[10] =   46*pSrc[ 1*nStride+i] - 90*pSrc[ 3*nStride+i] + 38*pSrc[ 5*nStride+i] + 54*pSrc[ 7*nStride+i]
                - 90*pSrc[ 9*nStride+i] + 31*pSrc[11*nStride+i] + 61*pSrc[13*nStride+i] - 88*pSrc[15*nStride+i]
                + 22*pSrc[17*nStride+i] + 67*pSrc[19*nStride+i] - 85*pSrc[21*nStride+i] + 13*pSrc[23*nStride+i]
                + 73*pSrc[25*nStride+i] - 82*pSrc[27*nStride+i] +  4*pSrc[29*nStride+i] + 78*pSrc[31*nStride+i];
        O[11] =   38*pSrc[ 1*nStride+i] - 88*pSrc[ 3*nStride+i] + 73*pSrc[ 5*nStride+i] -  4*pSrc[ 7*nStride+i]
                - 67*pSrc[ 9*nStride+i] + 90*pSrc[11*nStride+i] - 46*pSrc[13*nStride+i] - 31*pSrc[15*nStride+i]
                + 85*pSrc[17*nStride+i] - 78*pSrc[19*nStride+i] + 13*pSrc[21*nStride+i] + 61*pSrc[23*nStride+i]
                - 90*pSrc[25*nStride+i] + 54*pSrc[27*nStride+i] + 22*pSrc[29*nStride+i] - 82*pSrc[31*nStride+i];
        O[12] =   31*pSrc[ 1*nStride+i] - 78*pSrc[ 3*nStride+i] + 90*pSrc[ 5*nStride+i] - 61*pSrc[ 7*nStride+i]
                +  4*pSrc[ 9*nStride+i] + 54*pSrc[11*nStride+i] - 88*pSrc[13*nStride+i] + 82*pSrc[15*nStride+i]
                - 38*pSrc[17*nStride+i] - 22*pSrc[19*nStride+i] + 73*pSrc[21*nStride+i] - 90*pSrc[23*nStride+i]
                + 67*pSrc[25*nStride+i] - 13*pSrc[27*nStride+i] - 46*pSrc[29*nStride+i] + 85*pSrc[31*nStride+i];
        O[13] =   22*pSrc[ 1*nStride+i] - 61*pSrc[ 3*nStride+i] + 85*pSrc[ 5*nStride+i] - 90*pSrc[ 7*nStride+i]
                + 73*pSrc[ 9*nStride+i] - 38*pSrc[11*nStride+i] -  4*pSrc[13*nStride+i] + 46*pSrc[15*nStride+i]
                - 78*pSrc[17*nStride+i] + 90*pSrc[19*nStride+i] - 82*pSrc[21*nStride+i] + 54*pSrc[23*nStride+i]
                - 13*pSrc[25*nStride+i] - 31*pSrc[27*nStride+i] + 67*pSrc[29*nStride+i] - 88*pSrc[31*nStride+i];
        O[14] =   13*pSrc[ 1*nStride+i] - 38*pSrc[ 3*nStride+i] + 61*pSrc[ 5*nStride+i] - 78*pSrc[ 7*nStride+i]
                + 88*pSrc[ 9*nStride+i] - 90*pSrc[11*nStride+i] + 85*pSrc[13*nStride+i] - 73*pSrc[15*nStride+i]
                + 54*pSrc[17*nStride+i] - 31*pSrc[19*nStride+i] +  4*pSrc[21*nStride+i] + 22*pSrc[23*nStride+i]
                - 46*pSrc[25*nStride+i] + 67*pSrc[27*nStride+i] - 82*pSrc[29*nStride+i] + 90*pSrc[31*nStride+i];
        O[15] =    4*pSrc[ 1*nStride+i] - 13*pSrc[ 3*nStride+i] + 22*pSrc[ 5*nStride+i] - 31*pSrc[ 7*nStride+i]
                + 38*pSrc[ 9*nStride+i] - 46*pSrc[11*nStride+i] + 54*pSrc[13*nStride+i] - 61*pSrc[15*nStride+i]
                + 67*pSrc[17*nStride+i] - 73*pSrc[19*nStride+i] + 78*pSrc[21*nStride+i] - 82*pSrc[23*nStride+i]
                + 85*pSrc[25*nStride+i] - 88*pSrc[27*nStride+i] + 90*pSrc[29*nStride+i] - 90*pSrc[31*nStride+i];

        EO[0] =   90*pSrc[ 2*nStride+i] + 87*pSrc[ 6*nStride+i] + 80*pSrc[10*nStride+i] + 70*pSrc[14*nStride+i]
                + 57*pSrc[18*nStride+i] + 43*pSrc[22*nStride+i] + 25*pSrc[26*nStride+i] +  9*pSrc[30*nStride+i];
        EO[1] =   87*pSrc[ 2*nStride+i] + 57*pSrc[ 6*nStride+i] +  9*pSrc[10*nStride+i] - 43*pSrc[14*nStride+i]
                - 80*pSrc[18*nStride+i] - 90*pSrc[22*nStride+i] - 70*pSrc[26*nStride+i] - 25*pSrc[30*nStride+i];
        EO[2] =   80*pSrc[ 2*nStride+i] +  9*pSrc[ 6*nStride+i] - 70*pSrc[10*nStride+i] - 87*pSrc[14*nStride+i]
                - 25*pSrc[18*nStride+i] + 57*pSrc[22*nStride+i] + 90*pSrc[26*nStride+i] + 43*pSrc[30*nStride+i];
        EO[3] =   70*pSrc[ 2*nStride+i] - 43*pSrc[ 6*nStride+i] - 87*pSrc[10*nStride+i] +  9*pSrc[14*nStride+i]
                + 90*pSrc[18*nStride+i] + 25*pSrc[22*nStride+i] - 80*pSrc[26*nStride+i] - 57*pSrc[30*nStride+i];
        EO[4] =   57*pSrc[ 2*nStride+i] - 80*pSrc[ 6*nStride+i] - 25*pSrc[10*nStride+i] + 90*pSrc[14*nStride+i]
                -  9*pSrc[18*nStride+i] - 87*pSrc[22*nStride+i] + 43*pSrc[26*nStride+i] + 70*pSrc[30*nStride+i];
        EO[5] =   43*pSrc[ 2*nStride+i] - 90*pSrc[ 6*nStride+i] + 57*pSrc[10*nStride+i] + 25*pSrc[14*nStride+i]
                - 87*pSrc[18*nStride+i] + 70*pSrc[22*nStride+i] +  9*pSrc[26*nStride+i] - 80*pSrc[30*nStride+i];
        EO[6] =   25*pSrc[ 2*nStride+i] - 70*pSrc[ 6*nStride+i] + 90*pSrc[10*nStride+i] - 80*pSrc[14*nStride+i]
                + 43*pSrc[18*nStride+i] +  9*pSrc[22*nStride+i] - 57*pSrc[26*nStride+i] + 87*pSrc[30*nStride+i];
        EO[7] =    9*pSrc[ 2*nStride+i] - 25*pSrc[ 6*nStride+i] + 43*pSrc[10*nStride+i] - 57*pSrc[14*nStride+i]
                + 70*pSrc[18*nStride+i] - 80*pSrc[22*nStride+i] + 87*pSrc[26*nStride+i] - 90*pSrc[30*nStride+i];

        EEO[0] =   89*pSrc[ 4*nStride+i] + 75*pSrc[12*nStride+i] + 50*pSrc[20*nStride+i] + 18*pSrc[28*nStride+i];
        EEO[1] =   75*pSrc[ 4*nStride+i] - 18*pSrc[12*nStride+i] - 89*pSrc[20*nStride+i] - 50*pSrc[28*nStride+i];
        EEO[2] =   50*pSrc[ 4*nStride+i] - 89*pSrc[12*nStride+i] + 18*pSrc[20*nStride+i] + 75*pSrc[28*nStride+i];
        EEO[3] =   18*pSrc[ 4*nStride+i] - 50*pSrc[12*nStride+i] + 75*pSrc[20*nStride+i] - 89*pSrc[28*nStride+i];

        EEEO[0] = 83*pSrc[8*nStride+i] + 36*pSrc[24*nStride+i];
        EEEO[1] = 36*pSrc[8*nStride+i] - 83*pSrc[24*nStride+i];
        EEEE[0] = 64*pSrc[0*nStride+i] + 64*pSrc[16*nStride+i];
        EEEE[1] = 64*pSrc[0*nStride+i] - 64*pSrc[16*nStride+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        EEE[0] = EEEE[0] + EEEO[0];
        EEE[3] = EEEE[0] - EEEO[0];
        EEE[1] = EEEE[1] + EEEO[1];
        EEE[2] = EEEE[1] - EEEO[1];

        EE[0] = EEE[0] + EEO[0];
        EE[7] = EEE[0] - EEO[0];
        EE[1] = EEE[1] + EEO[1];
        EE[6] = EEE[1] - EEO[1];
        EE[5] = EEE[2] - EEO[2];
        EE[2] = EEE[2] + EEO[2];
        EE[3] = EEE[3] + EEO[3];
        EE[4] = EEE[3] - EEO[3];

        E[ 0] = EE[0] + EO[0];
        E[ 8] = EE[7] - EO[7];
        E[ 1] = EE[1] + EO[1];
        E[ 9] = EE[6] - EO[6];
        E[ 2] = EE[2] + EO[2];
        E[10] = EE[5] - EO[5];
        E[ 3] = EE[3] + EO[3];
        E[11] = EE[4] - EO[4];
        E[ 4] = EE[4] + EO[4];
        E[12] = EE[3] - EO[3];
        E[ 5] = EE[5] + EO[5];
        E[13] = EE[2] - EO[2];
        E[ 6] = EE[6] + EO[6];
        E[14] = EE[1] - EO[1];
        E[ 7] = EE[7] + EO[7];
        E[15] = EE[0] - EO[0];

        piTmp0[i*32+ 0] = Clip3( -32768, 32767, (E[ 0] + O[ 0] + rnd) >> nShift);
        piTmp0[i*32+16] = Clip3( -32768, 32767, (E[15] - O[15] + rnd) >> nShift);
        piTmp0[i*32+ 1] = Clip3( -32768, 32767, (E[ 1] + O[ 1] + rnd) >> nShift);
        piTmp0[i*32+17] = Clip3( -32768, 32767, (E[14] - O[14] + rnd) >> nShift);
        piTmp0[i*32+ 2] = Clip3( -32768, 32767, (E[ 2] + O[ 2] + rnd) >> nShift);
        piTmp0[i*32+18] = Clip3( -32768, 32767, (E[13] - O[13] + rnd) >> nShift);
        piTmp0[i*32+ 3] = Clip3( -32768, 32767, (E[ 3] + O[ 3] + rnd) >> nShift);
        piTmp0[i*32+19] = Clip3( -32768, 32767, (E[12] - O[12] + rnd) >> nShift);
        piTmp0[i*32+ 4] = Clip3( -32768, 32767, (E[ 4] + O[ 4] + rnd) >> nShift);
        piTmp0[i*32+20] = Clip3( -32768, 32767, (E[11] - O[11] + rnd) >> nShift);
        piTmp0[i*32+ 5] = Clip3( -32768, 32767, (E[ 5] + O[ 5] + rnd) >> nShift);
        piTmp0[i*32+21] = Clip3( -32768, 32767, (E[10] - O[10] + rnd) >> nShift);
        piTmp0[i*32+ 6] = Clip3( -32768, 32767, (E[ 6] + O[ 6] + rnd) >> nShift);
        piTmp0[i*32+22] = Clip3( -32768, 32767, (E[ 9] - O[ 9] + rnd) >> nShift);
        piTmp0[i*32+ 7] = Clip3( -32768, 32767, (E[ 7] + O[ 7] + rnd) >> nShift);
        piTmp0[i*32+23] = Clip3( -32768, 32767, (E[ 8] - O[ 8] + rnd) >> nShift);
        piTmp0[i*32+ 8] = Clip3( -32768, 32767, (E[ 8] + O[ 8] + rnd) >> nShift);
        piTmp0[i*32+24] = Clip3( -32768, 32767, (E[ 7] - O[ 7] + rnd) >> nShift);
        piTmp0[i*32+ 9] = Clip3( -32768, 32767, (E[ 9] + O[ 9] + rnd) >> nShift);
        piTmp0[i*32+25] = Clip3( -32768, 32767, (E[ 6] - O[ 6] + rnd) >> nShift);
        piTmp0[i*32+10] = Clip3( -32768, 32767, (E[10] + O[10] + rnd) >> nShift);
        piTmp0[i*32+26] = Clip3( -32768, 32767, (E[ 5] - O[ 5] + rnd) >> nShift);
        piTmp0[i*32+11] = Clip3( -32768, 32767, (E[11] + O[11] + rnd) >> nShift);
        piTmp0[i*32+27] = Clip3( -32768, 32767, (E[ 4] - O[ 4] + rnd) >> nShift);
        piTmp0[i*32+12] = Clip3( -32768, 32767, (E[12] + O[12] + rnd) >> nShift);
        piTmp0[i*32+28] = Clip3( -32768, 32767, (E[ 3] - O[ 3] + rnd) >> nShift);
        piTmp0[i*32+13] = Clip3( -32768, 32767, (E[13] + O[13] + rnd) >> nShift);
        piTmp0[i*32+29] = Clip3( -32768, 32767, (E[ 2] - O[ 2] + rnd) >> nShift);
        piTmp0[i*32+14] = Clip3( -32768, 32767, (E[14] + O[14] + rnd) >> nShift);
        piTmp0[i*32+30] = Clip3( -32768, 32767, (E[ 1] - O[ 1] + rnd) >> nShift);
        piTmp0[i*32+15] = Clip3( -32768, 32767, (E[15] + O[15] + rnd) >> nShift);
        piTmp0[i*32+31] = Clip3( -32768, 32767, (E[ 0] - O[ 0] + rnd) >> nShift);
    }

    // DCT2
    nShift = SHIFT_INV_2ND;
    rnd    = 1<<(nShift-1);
    for( i=0; i<32; i++ ) {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        O[ 0] =   90*piTmp0[ 1*32+i] + 90*piTmp0[ 3*32+i] + 88*piTmp0[ 5*32+i] + 85*piTmp0[ 7*32+i]
                + 82*piTmp0[ 9*32+i] + 78*piTmp0[11*32+i] + 73*piTmp0[13*32+i] + 67*piTmp0[15*32+i]
                + 61*piTmp0[17*32+i] + 54*piTmp0[19*32+i] + 46*piTmp0[21*32+i] + 38*piTmp0[23*32+i]
                + 31*piTmp0[25*32+i] + 22*piTmp0[27*32+i] + 13*piTmp0[29*32+i] +  4*piTmp0[31*32+i];
        O[ 1] =   90*piTmp0[ 1*32+i] + 82*piTmp0[ 3*32+i] + 67*piTmp0[ 5*32+i] + 46*piTmp0[ 7*32+i]
                + 22*piTmp0[ 9*32+i] -  4*piTmp0[11*32+i] - 31*piTmp0[13*32+i] - 54*piTmp0[15*32+i]
                - 73*piTmp0[17*32+i] - 85*piTmp0[19*32+i] - 90*piTmp0[21*32+i] - 88*piTmp0[23*32+i]
                - 78*piTmp0[25*32+i] - 61*piTmp0[27*32+i] - 38*piTmp0[29*32+i] - 13*piTmp0[31*32+i];
        O[ 2] =   88*piTmp0[ 1*32+i] + 67*piTmp0[ 3*32+i] + 31*piTmp0[ 5*32+i] - 13*piTmp0[ 7*32+i]
                - 54*piTmp0[ 9*32+i] - 82*piTmp0[11*32+i] - 90*piTmp0[13*32+i] - 78*piTmp0[15*32+i]
                - 46*piTmp0[17*32+i] -  4*piTmp0[19*32+i] + 38*piTmp0[21*32+i] + 73*piTmp0[23*32+i]
                + 90*piTmp0[25*32+i] + 85*piTmp0[27*32+i] + 61*piTmp0[29*32+i] + 22*piTmp0[31*32+i];
        O[ 3] =   85*piTmp0[ 1*32+i] + 46*piTmp0[ 3*32+i] - 13*piTmp0[ 5*32+i] - 67*piTmp0[ 7*32+i]
                - 90*piTmp0[ 9*32+i] - 73*piTmp0[11*32+i] - 22*piTmp0[13*32+i] + 38*piTmp0[15*32+i]
                + 82*piTmp0[17*32+i] + 88*piTmp0[19*32+i] + 54*piTmp0[21*32+i] -  4*piTmp0[23*32+i]
                - 61*piTmp0[25*32+i] - 90*piTmp0[27*32+i] - 78*piTmp0[29*32+i] - 31*piTmp0[31*32+i];
        O[ 4] =   82*piTmp0[ 1*32+i] + 22*piTmp0[ 3*32+i] - 54*piTmp0[ 5*32+i] - 90*piTmp0[ 7*32+i]
                - 61*piTmp0[ 9*32+i] + 13*piTmp0[11*32+i] + 78*piTmp0[13*32+i] + 85*piTmp0[15*32+i]
                + 31*piTmp0[17*32+i] - 46*piTmp0[19*32+i] - 90*piTmp0[21*32+i] - 67*piTmp0[23*32+i]
                +  4*piTmp0[25*32+i] + 73*piTmp0[27*32+i] + 88*piTmp0[29*32+i] + 38*piTmp0[31*32+i];
        O[ 5] =   78*piTmp0[ 1*32+i] -  4*piTmp0[ 3*32+i] - 82*piTmp0[ 5*32+i] - 73*piTmp0[ 7*32+i]
                + 13*piTmp0[ 9*32+i] + 85*piTmp0[11*32+i] + 67*piTmp0[13*32+i] - 22*piTmp0[15*32+i]
                - 88*piTmp0[17*32+i] - 61*piTmp0[19*32+i] + 31*piTmp0[21*32+i] + 90*piTmp0[23*32+i]
                + 54*piTmp0[25*32+i] - 38*piTmp0[27*32+i] - 90*piTmp0[29*32+i] - 46*piTmp0[31*32+i];
        O[ 6] =   73*piTmp0[ 1*32+i] - 31*piTmp0[ 3*32+i] - 90*piTmp0[ 5*32+i] - 22*piTmp0[ 7*32+i]
                + 78*piTmp0[ 9*32+i] + 67*piTmp0[11*32+i] - 38*piTmp0[13*32+i] - 90*piTmp0[15*32+i]
                - 13*piTmp0[17*32+i] + 82*piTmp0[19*32+i] + 61*piTmp0[21*32+i] - 46*piTmp0[23*32+i]
                - 88*piTmp0[25*32+i] -  4*piTmp0[27*32+i] + 85*piTmp0[29*32+i] + 54*piTmp0[31*32+i];
        O[ 7] =   67*piTmp0[ 1*32+i] - 54*piTmp0[ 3*32+i] - 78*piTmp0[ 5*32+i] + 38*piTmp0[ 7*32+i]
                + 85*piTmp0[ 9*32+i] - 22*piTmp0[11*32+i] - 90*piTmp0[13*32+i] +  4*piTmp0[15*32+i]
                + 90*piTmp0[17*32+i] + 13*piTmp0[19*32+i] - 88*piTmp0[21*32+i] - 31*piTmp0[23*32+i]
                + 82*piTmp0[25*32+i] + 46*piTmp0[27*32+i] - 73*piTmp0[29*32+i] - 61*piTmp0[31*32+i];
        O[ 8] =   61*piTmp0[ 1*32+i] - 73*piTmp0[ 3*32+i] - 46*piTmp0[ 5*32+i] + 82*piTmp0[ 7*32+i]
                + 31*piTmp0[ 9*32+i] - 88*piTmp0[11*32+i] - 13*piTmp0[13*32+i] + 90*piTmp0[15*32+i]
                -  4*piTmp0[17*32+i] - 90*piTmp0[19*32+i] + 22*piTmp0[21*32+i] + 85*piTmp0[23*32+i]
                - 38*piTmp0[25*32+i] - 78*piTmp0[27*32+i] + 54*piTmp0[29*32+i] + 67*piTmp0[31*32+i];
        O[ 9] =   54*piTmp0[ 1*32+i] - 85*piTmp0[ 3*32+i] -  4*piTmp0[ 5*32+i] + 88*piTmp0[ 7*32+i]
                - 46*piTmp0[ 9*32+i] - 61*piTmp0[11*32+i] + 82*piTmp0[13*32+i] + 13*piTmp0[15*32+i]
                - 90*piTmp0[17*32+i] + 38*piTmp0[19*32+i] + 67*piTmp0[21*32+i] - 78*piTmp0[23*32+i]
                - 22*piTmp0[25*32+i] + 90*piTmp0[27*32+i] - 31*piTmp0[29*32+i] - 73*piTmp0[31*32+i];
        O[10] =   46*piTmp0[ 1*32+i] - 90*piTmp0[ 3*32+i] + 38*piTmp0[ 5*32+i] + 54*piTmp0[ 7*32+i]
                - 90*piTmp0[ 9*32+i] + 31*piTmp0[11*32+i] + 61*piTmp0[13*32+i] - 88*piTmp0[15*32+i]
                + 22*piTmp0[17*32+i] + 67*piTmp0[19*32+i] - 85*piTmp0[21*32+i] + 13*piTmp0[23*32+i]
                + 73*piTmp0[25*32+i] - 82*piTmp0[27*32+i] +  4*piTmp0[29*32+i] + 78*piTmp0[31*32+i];
        O[11] =   38*piTmp0[ 1*32+i] - 88*piTmp0[ 3*32+i] + 73*piTmp0[ 5*32+i] -  4*piTmp0[ 7*32+i]
                - 67*piTmp0[ 9*32+i] + 90*piTmp0[11*32+i] - 46*piTmp0[13*32+i] - 31*piTmp0[15*32+i]
                + 85*piTmp0[17*32+i] - 78*piTmp0[19*32+i] + 13*piTmp0[21*32+i] + 61*piTmp0[23*32+i]
                - 90*piTmp0[25*32+i] + 54*piTmp0[27*32+i] + 22*piTmp0[29*32+i] - 82*piTmp0[31*32+i];
        O[12] =   31*piTmp0[ 1*32+i] - 78*piTmp0[ 3*32+i] + 90*piTmp0[ 5*32+i] - 61*piTmp0[ 7*32+i]
                +  4*piTmp0[ 9*32+i] + 54*piTmp0[11*32+i] - 88*piTmp0[13*32+i] + 82*piTmp0[15*32+i]
                - 38*piTmp0[17*32+i] - 22*piTmp0[19*32+i] + 73*piTmp0[21*32+i] - 90*piTmp0[23*32+i]
                + 67*piTmp0[25*32+i] - 13*piTmp0[27*32+i] - 46*piTmp0[29*32+i] + 85*piTmp0[31*32+i];
        O[13] =   22*piTmp0[ 1*32+i] - 61*piTmp0[ 3*32+i] + 85*piTmp0[ 5*32+i] - 90*piTmp0[ 7*32+i]
                + 73*piTmp0[ 9*32+i] - 38*piTmp0[11*32+i] -  4*piTmp0[13*32+i] + 46*piTmp0[15*32+i]
                - 78*piTmp0[17*32+i] + 90*piTmp0[19*32+i] - 82*piTmp0[21*32+i] + 54*piTmp0[23*32+i]
                - 13*piTmp0[25*32+i] - 31*piTmp0[27*32+i] + 67*piTmp0[29*32+i] - 88*piTmp0[31*32+i];
        O[14] =   13*piTmp0[ 1*32+i] - 38*piTmp0[ 3*32+i] + 61*piTmp0[ 5*32+i] - 78*piTmp0[ 7*32+i]
                + 88*piTmp0[ 9*32+i] - 90*piTmp0[11*32+i] + 85*piTmp0[13*32+i] - 73*piTmp0[15*32+i]
                + 54*piTmp0[17*32+i] - 31*piTmp0[19*32+i] +  4*piTmp0[21*32+i] + 22*piTmp0[23*32+i]
                - 46*piTmp0[25*32+i] + 67*piTmp0[27*32+i] - 82*piTmp0[29*32+i] + 90*piTmp0[31*32+i];
        O[15] =    4*piTmp0[ 1*32+i] - 13*piTmp0[ 3*32+i] + 22*piTmp0[ 5*32+i] - 31*piTmp0[ 7*32+i]
                + 38*piTmp0[ 9*32+i] - 46*piTmp0[11*32+i] + 54*piTmp0[13*32+i] - 61*piTmp0[15*32+i]
                + 67*piTmp0[17*32+i] - 73*piTmp0[19*32+i] + 78*piTmp0[21*32+i] - 82*piTmp0[23*32+i]
                + 85*piTmp0[25*32+i] - 88*piTmp0[27*32+i] + 90*piTmp0[29*32+i] - 90*piTmp0[31*32+i];

        EO[0] =   90*piTmp0[ 2*32+i] + 87*piTmp0[ 6*32+i] + 80*piTmp0[10*32+i] + 70*piTmp0[14*32+i]
                + 57*piTmp0[18*32+i] + 43*piTmp0[22*32+i] + 25*piTmp0[26*32+i] +  9*piTmp0[30*32+i];
        EO[1] =   87*piTmp0[ 2*32+i] + 57*piTmp0[ 6*32+i] +  9*piTmp0[10*32+i] - 43*piTmp0[14*32+i]
                - 80*piTmp0[18*32+i] - 90*piTmp0[22*32+i] - 70*piTmp0[26*32+i] - 25*piTmp0[30*32+i];
        EO[2] =   80*piTmp0[ 2*32+i] +  9*piTmp0[ 6*32+i] - 70*piTmp0[10*32+i] - 87*piTmp0[14*32+i]
                - 25*piTmp0[18*32+i] + 57*piTmp0[22*32+i] + 90*piTmp0[26*32+i] + 43*piTmp0[30*32+i];
        EO[3] =   70*piTmp0[ 2*32+i] - 43*piTmp0[ 6*32+i] - 87*piTmp0[10*32+i] +  9*piTmp0[14*32+i]
                + 90*piTmp0[18*32+i] + 25*piTmp0[22*32+i] - 80*piTmp0[26*32+i] - 57*piTmp0[30*32+i];
        EO[4] =   57*piTmp0[ 2*32+i] - 80*piTmp0[ 6*32+i] - 25*piTmp0[10*32+i] + 90*piTmp0[14*32+i]
                -  9*piTmp0[18*32+i] - 87*piTmp0[22*32+i] + 43*piTmp0[26*32+i] + 70*piTmp0[30*32+i];
        EO[5] =   43*piTmp0[ 2*32+i] - 90*piTmp0[ 6*32+i] + 57*piTmp0[10*32+i] + 25*piTmp0[14*32+i]
                - 87*piTmp0[18*32+i] + 70*piTmp0[22*32+i] +  9*piTmp0[26*32+i] - 80*piTmp0[30*32+i];
        EO[6] =   25*piTmp0[ 2*32+i] - 70*piTmp0[ 6*32+i] + 90*piTmp0[10*32+i] - 80*piTmp0[14*32+i]
                + 43*piTmp0[18*32+i] +  9*piTmp0[22*32+i] - 57*piTmp0[26*32+i] + 87*piTmp0[30*32+i];
        EO[7] =    9*piTmp0[ 2*32+i] - 25*piTmp0[ 6*32+i] + 43*piTmp0[10*32+i] - 57*piTmp0[14*32+i]
                + 70*piTmp0[18*32+i] - 80*piTmp0[22*32+i] + 87*piTmp0[26*32+i] - 90*piTmp0[30*32+i];

        EEO[0] =   89*piTmp0[ 4*32+i] + 75*piTmp0[12*32+i] + 50*piTmp0[20*32+i] + 18*piTmp0[28*32+i];
        EEO[1] =   75*piTmp0[ 4*32+i] - 18*piTmp0[12*32+i] - 89*piTmp0[20*32+i] - 50*piTmp0[28*32+i];
        EEO[2] =   50*piTmp0[ 4*32+i] - 89*piTmp0[12*32+i] + 18*piTmp0[20*32+i] + 75*piTmp0[28*32+i];
        EEO[3] =   18*piTmp0[ 4*32+i] - 50*piTmp0[12*32+i] + 75*piTmp0[20*32+i] - 89*piTmp0[28*32+i];

        EEEO[0] = 83*piTmp0[8*32+i] + 36*piTmp0[24*32+i];
        EEEO[1] = 36*piTmp0[8*32+i] - 83*piTmp0[24*32+i];
        EEEE[0] = 64*piTmp0[0*32+i] + 64*piTmp0[16*32+i];
        EEEE[1] = 64*piTmp0[0*32+i] - 64*piTmp0[16*32+i];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
        EEE[0] = EEEE[0] + EEEO[0];
        EEE[3] = EEEE[0] - EEEO[0];
        EEE[1] = EEEE[1] + EEEO[1];
        EEE[2] = EEEE[1] - EEEO[1];

        EE[0] = EEE[0] + EEO[0];
        EE[7] = EEE[0] - EEO[0];
        EE[1] = EEE[1] + EEO[1];
        EE[6] = EEE[1] - EEO[1];
        EE[5] = EEE[2] - EEO[2];
        EE[2] = EEE[2] + EEO[2];
        EE[3] = EEE[3] + EEO[3];
        EE[4] = EEE[3] - EEO[3];

        E[ 0] = EE[0] + EO[0];
        E[ 8] = EE[7] - EO[7];
        E[ 1] = EE[1] + EO[1];
        E[ 9] = EE[6] - EO[6];
        E[ 2] = EE[2] + EO[2];
        E[10] = EE[5] - EO[5];
        E[ 3] = EE[3] + EO[3];
        E[11] = EE[4] - EO[4];
        E[ 4] = EE[4] + EO[4];
        E[12] = EE[3] - EO[3];
        E[ 5] = EE[5] + EO[5];
        E[13] = EE[2] - EO[2];
        E[ 6] = EE[6] + EO[6];
        E[14] = EE[1] - EO[1];
        E[ 7] = EE[7] + EO[7];
        E[15] = EE[0] - EO[0];

        piTmp1[i*32+ 0] = Clip3( -32768, 32767, (E[ 0] + O[ 0] + rnd) >> nShift);
        piTmp1[i*32+16] = Clip3( -32768, 32767, (E[15] - O[15] + rnd) >> nShift);
        piTmp1[i*32+ 1] = Clip3( -32768, 32767, (E[ 1] + O[ 1] + rnd) >> nShift);
        piTmp1[i*32+17] = Clip3( -32768, 32767, (E[14] - O[14] + rnd) >> nShift);
        piTmp1[i*32+ 2] = Clip3( -32768, 32767, (E[ 2] + O[ 2] + rnd) >> nShift);
        piTmp1[i*32+18] = Clip3( -32768, 32767, (E[13] - O[13] + rnd) >> nShift);
        piTmp1[i*32+ 3] = Clip3( -32768, 32767, (E[ 3] + O[ 3] + rnd) >> nShift);
        piTmp1[i*32+19] = Clip3( -32768, 32767, (E[12] - O[12] + rnd) >> nShift);
        piTmp1[i*32+ 4] = Clip3( -32768, 32767, (E[ 4] + O[ 4] + rnd) >> nShift);
        piTmp1[i*32+20] = Clip3( -32768, 32767, (E[11] - O[11] + rnd) >> nShift);
        piTmp1[i*32+ 5] = Clip3( -32768, 32767, (E[ 5] + O[ 5] + rnd) >> nShift);
        piTmp1[i*32+21] = Clip3( -32768, 32767, (E[10] - O[10] + rnd) >> nShift);
        piTmp1[i*32+ 6] = Clip3( -32768, 32767, (E[ 6] + O[ 6] + rnd) >> nShift);
        piTmp1[i*32+22] = Clip3( -32768, 32767, (E[ 9] - O[ 9] + rnd) >> nShift);
        piTmp1[i*32+ 7] = Clip3( -32768, 32767, (E[ 7] + O[ 7] + rnd) >> nShift);
        piTmp1[i*32+23] = Clip3( -32768, 32767, (E[ 8] - O[ 8] + rnd) >> nShift);
        piTmp1[i*32+ 8] = Clip3( -32768, 32767, (E[ 8] + O[ 8] + rnd) >> nShift);
        piTmp1[i*32+24] = Clip3( -32768, 32767, (E[ 7] - O[ 7] + rnd) >> nShift);
        piTmp1[i*32+ 9] = Clip3( -32768, 32767, (E[ 9] + O[ 9] + rnd) >> nShift);
        piTmp1[i*32+25] = Clip3( -32768, 32767, (E[ 6] - O[ 6] + rnd) >> nShift);
        piTmp1[i*32+10] = Clip3( -32768, 32767, (E[10] + O[10] + rnd) >> nShift);
        piTmp1[i*32+26] = Clip3( -32768, 32767, (E[ 5] - O[ 5] + rnd) >> nShift);
        piTmp1[i*32+11] = Clip3( -32768, 32767, (E[11] + O[11] + rnd) >> nShift);
        piTmp1[i*32+27] = Clip3( -32768, 32767, (E[ 4] - O[ 4] + rnd) >> nShift);
        piTmp1[i*32+12] = Clip3( -32768, 32767, (E[12] + O[12] + rnd) >> nShift);
        piTmp1[i*32+28] = Clip3( -32768, 32767, (E[ 3] - O[ 3] + rnd) >> nShift);
        piTmp1[i*32+13] = Clip3( -32768, 32767, (E[13] + O[13] + rnd) >> nShift);
        piTmp1[i*32+29] = Clip3( -32768, 32767, (E[ 2] - O[ 2] + rnd) >> nShift);
        piTmp1[i*32+14] = Clip3( -32768, 32767, (E[14] + O[14] + rnd) >> nShift);
        piTmp1[i*32+30] = Clip3( -32768, 32767, (E[ 1] - O[ 1] + rnd) >> nShift);
        piTmp1[i*32+15] = Clip3( -32768, 32767, (E[15] + O[15] + rnd) >> nShift);
        piTmp1[i*32+31] = Clip3( -32768, 32767, (E[ 0] - O[ 0] + rnd) >> nShift);
    }

    // Add
    for( i=0; i<32; i++ ) {
        for( j=0; j<32; j++ ) {
            pDst[i * nStrideDst + j] = Clip( piTmp1[i * 32 + j] + pRef[i * nStride + j] );
        }
    }
}

typedef void xIDCTADD_t(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
);

xIDCTADD_t *xIDctAddN[MAX_CU_DEPTH+1] = {
    xIDstAdd4,
    xIDctAdd4,
    xIDctAdd8,
    xIDctAdd16,
    xIDctAdd32,
};

void xIDctAdd(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1,
    Int iWidth, UInt nMode
)
{
    CInt nLog2Width  = xLog2( iWidth - 1 );
    CInt bUseDst     = (iWidth == 4) && (nMode != MODE_INVALID);

    xIDctAddN[nLog2Width - 1 - bUseDst](pDst, nStrideDst, pSrc, nStride, pRef, piTmp0, piTmp1);
}
