/*****************************************************************************
 * pixel.cpp: Pixel operator functions
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Min Chen <chenm003@163.com> , Xiangwen Wang <wxw21st@163.com>
 *          Yanan Zhao
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
// * Declaration of TRUE ASM Functions
// ***************************************************************************
#if SATD_USE_TRUE_ASM

//SATD
extern "C" int x265_pixel_satd_4x4_sse4(pixel*, intptr_t, pixel*, intptr_t);
extern "C" int x265_pixel_satd_8x8_sse4(pixel*, intptr_t, pixel*, intptr_t);
extern "C" int x265_pixel_satd_16x16_sse4(pixel*, intptr_t, pixel*, intptr_t);
extern "C" int x265_pixel_satd_32x32_sse4(pixel*, intptr_t, pixel*, intptr_t);

//SAD
extern "C" int x265_pixel_sad_4x4_sse4(pixel*, intptr_t, pixel*, intptr_t);
extern "C" int x265_pixel_sad_8x8_sse4(pixel*, intptr_t, pixel*, intptr_t);
extern "C" int x265_pixel_sad_16x16_sse4(pixel*, intptr_t, pixel*, intptr_t);
extern "C" int x265_pixel_sad_32x32_sse4(pixel*, intptr_t, pixel*, intptr_t);
#endif


// ***************************************************************************
// * SAD Functions
// ***************************************************************************
int H4[4][4] = {{ 1,  1,  1,  1},
				{ 1, -1,  1, -1},
				{ 1,  1, -1, -1},
				{ 1, -1, -1,  1}} ;
int H8[8][8] = {{ 1,  1,  1,  1,  1,  1,  1,  1},
				{ 1, -1,  1, -1,  1, -1,  1, -1},
				{ 1,  1, -1, -1,  1,  1, -1, -1},
				{ 1, -1, -1,  1,  1, -1, -1,  1},
				{ 1,  1,  1,  1, -1, -1, -1, -1},
				{ 1, -1,  1, -1, -1,  1, -1,  1},
				{ 1,  1, -1, -1, -1, -1,  1,  1},
				{ 1, -1, -1,  1, -1,  1,  1, -1}} ;

UInt32 xCalcHADs4x4( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt32 uiSad = 0;

#if SATD_USE_ASM
	__m128i c_zero   = _mm_setzero_si128();
	__m128i T10a	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)pSrc				), c_zero);//16bit: xx xx xx xx 03 02 01 00
	__m128i T10b	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)pRef				), c_zero);
	__m128i T11a	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)(pSrc+nStrideSrc  )), c_zero);
	__m128i T11b	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)(pRef+nStrideRef  )), c_zero);
	__m128i T12a	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)(pSrc+2*nStrideSrc)), c_zero);
	__m128i T12b	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)(pRef+2*nStrideRef)), c_zero);
	__m128i T13a	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)(pSrc+3*nStrideSrc)), c_zero);
	__m128i T13b	 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(UInt32*)(pRef+3*nStrideRef)), c_zero);

	__m128i T30 = _mm_shufflelo_epi16(_mm_sub_epi16(T10a, T10b), 0x9C);//16bit:xx xx xx xx 02 01 03 00
	__m128i T31 = _mm_shufflelo_epi16(_mm_sub_epi16(T11a, T11b), 0x9C);
	__m128i T32 = _mm_shufflelo_epi16(_mm_sub_epi16(T12a, T12b), 0x9C);
	__m128i T33 = _mm_shufflelo_epi16(_mm_sub_epi16(T13a, T13b), 0x9C);

	__m128i T40 = _mm_unpacklo_epi64(T30, T31);//16bit: 12 11 13 10 02 01 03 00
	__m128i T41 = _mm_unpacklo_epi64(T32, T33);//16bit: 32 31 33 30 22 21 23 20
	__m128i T50 = _mm_hadd_epi16(T40, T41);//16bit: 31+32 30+33 21+22 20+23 11+12 10+13 01+02 00+03
	__m128i T51 = _mm_hsub_epi16(T40, T41);//16bit: 31-32 30-33 21-22 20-23 11-12 10-13 01-02 00-03

	__m128i T60 = _mm_hadd_epi16(T50, T50);//16bit: DstRow 03 02 01 00
	__m128i T63 = _mm_hsub_epi16(T50, T50);//16bit: DstRow 33 32 31 30
	__m128i T61 = _mm_hsub_epi16(T51, T51);//16bit: DstRow 13 12 11 10
	__m128i T62 = _mm_hadd_epi16(T51, T51);//16bit: DstRow 23 22 21 20

	//SATD2
	__m128i T80 = _mm_unpacklo_epi64(_mm_shufflelo_epi16(T60, 0x9C), _mm_shufflelo_epi16(T61, 0x9C));//16bit: 12 11 13 10 02 01 03 00
	__m128i T81 = _mm_unpacklo_epi64(_mm_shufflelo_epi16(T62, 0x9C), _mm_shufflelo_epi16(T63, 0x9C));//16bit: 32 31 33 30 22 21 23 20

	__m128i T90 = _mm_hadd_epi16(T80, T81);//16bit: 31+32 30+33 21+22 20+23 11+12 10+13 01+02 00+03
	__m128i T91 = _mm_hsub_epi16(T80, T81);//16bit: 31-32 30-33 21-22 20-23 11-12 10-13 01-02 00-03
	__m128i TA0 = _mm_hadd_epi16(T90, T90);//16bit: RealDstRow 03 02 01 00
	__m128i TA3 = _mm_hsub_epi16(T90, T90);//16bit: RealDstRow 33 32 31 30
	__m128i TA1 = _mm_hsub_epi16(T91, T91);//16bit: RealDstRow 13 12 11 10
	__m128i TA2 = _mm_hadd_epi16(T91, T91);//16bit: RealDstRow 23 22 21 20

	__m128i TB0 = _mm_abs_epi16(TA0);
	__m128i TB1 = _mm_abs_epi16(TA1);
	__m128i TB2 = _mm_abs_epi16(TA2);
	__m128i TB3 = _mm_abs_epi16(TA3);

	__m128i TC0 = _mm_add_epi16(_mm_add_epi16(TB0, TB1), _mm_add_epi16(TB2, TB3));
	__m128i TD0 = _mm_unpacklo_epi16(TC0, c_zero);//32bit
	__m128i TE0 = _mm_hadd_epi32(TD0, TD0);
	__m128i TF0 = _mm_hadd_epi32(TE0, TE0);
	uiSad		= _mm_cvtsi128_si32(TF0);
	uiSad		= ((uiSad+1)>>1) ;

	//__m128i aa = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
	//__m128i mask = _mm_set_epi8(8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	//__m128i bb = _mm_shuffle_epi8(aa, mask);
	//__m128i cc = _mm_shuffle_epi32(bb, 0x1B);
#else
	UInt x, y;
	uiSad = 0;
	int p4[4][4] ;
	int iRow, iColumn ;
	for (iRow = 0 ; iRow < 4 ; iRow ++)
		for (iColumn = 0 ; iColumn < 4 ; iColumn ++)
			p4[iRow][iColumn] = pSrc[iRow * nStrideSrc + iColumn] - pRef[iRow * nStrideRef + iColumn] ;

	int Temp4[4][4] ;
	int t ;

	for (iRow = 0 ; iRow < 4 ; iRow ++)	
		for (iColumn = 0 ; iColumn < 4 ; iColumn ++){
			Temp4[iRow][iColumn] = 0 ;
			for (t = 0 ; t < 4 ; t ++)
				Temp4[iRow][iColumn] += (H4[iRow][t] * p4[t][iColumn]) ;
		}

		int p4_Transformed[4][4] ;
		for (iRow = 0 ; iRow < 4 ; iRow ++)
			for (iColumn = 0 ; iColumn < 4 ; iColumn ++){
				p4_Transformed[iRow][iColumn] = 0 ;
				for (t = 0 ; t < 4 ; t ++)
					p4_Transformed[iRow][iColumn] += Temp4[iRow][t] * H4[t][iColumn] ;
			}

			int iAbsSum4 ;
			iAbsSum4 = 0 ;
			for (iRow = 0 ; iRow < 4 ; iRow ++){
				for (iColumn = 0 ; iColumn < 4 ; iColumn ++){
					iAbsSum4 += abs(p4_Transformed[iRow][iColumn]) ;
				}
			}
			uiSad = ((iAbsSum4+1)>>1) ;
#endif

			return uiSad;
}

UInt32 xCalcHADs8x8( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt32 uiSad = 0;

#if SATD_USE_ASM
	__m128i c_zero   = _mm_setzero_si128();
	__m128i c_mask   = _mm_set_epi8(15,14,13,12,11,10,9,8,   5,2,6,1,4,3,7,0);
	__m128i T00a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)pSrc				 ), c_mask);//8bit: xx xx ... xx xx 05 02 06 01 04 03 07 00
	__m128i T00b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)pRef				 ), c_mask);
	__m128i T01a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pSrc+nStrideSrc  )), c_mask);
	__m128i T01b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pRef+nStrideRef  )), c_mask);
	__m128i T02a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pSrc+2*nStrideSrc)), c_mask);
	__m128i T02b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pRef+2*nStrideRef)), c_mask);
	__m128i T03a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pSrc+3*nStrideSrc)), c_mask);
	__m128i T03b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pRef+3*nStrideRef)), c_mask);
	__m128i T04a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pSrc+4*nStrideSrc)), c_mask);
	__m128i T04b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pRef+4*nStrideRef)), c_mask);
	__m128i T05a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pSrc+5*nStrideSrc)), c_mask);
	__m128i T05b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pRef+5*nStrideRef)), c_mask);
	__m128i T06a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pSrc+6*nStrideSrc)), c_mask);
	__m128i T06b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pRef+6*nStrideRef)), c_mask);
	__m128i T07a	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pSrc+7*nStrideSrc)), c_mask);
	__m128i T07b	 = _mm_shuffle_epi8(_mm_loadl_epi64((__m128i*)(pRef+7*nStrideRef)), c_mask);

	__m128i T10a	 = _mm_unpacklo_epi8(T00a, c_zero);//16bit: 05 02 06 01 04 03 07 00
	__m128i T10b	 = _mm_unpacklo_epi8(T00b, c_zero);
	__m128i T11a	 = _mm_unpacklo_epi8(T01a, c_zero);
	__m128i T11b	 = _mm_unpacklo_epi8(T01b, c_zero);
	__m128i T12a	 = _mm_unpacklo_epi8(T02a, c_zero);
	__m128i T12b	 = _mm_unpacklo_epi8(T02b, c_zero);
	__m128i T13a	 = _mm_unpacklo_epi8(T03a, c_zero);
	__m128i T13b	 = _mm_unpacklo_epi8(T03b, c_zero);
	__m128i T14a	 = _mm_unpacklo_epi8(T04a, c_zero);
	__m128i T14b	 = _mm_unpacklo_epi8(T04b, c_zero);
	__m128i T15a	 = _mm_unpacklo_epi8(T05a, c_zero);
	__m128i T15b	 = _mm_unpacklo_epi8(T05b, c_zero);
	__m128i T16a	 = _mm_unpacklo_epi8(T06a, c_zero);
	__m128i T16b	 = _mm_unpacklo_epi8(T06b, c_zero);
	__m128i T17a	 = _mm_unpacklo_epi8(T07a, c_zero);
	__m128i T17b	 = _mm_unpacklo_epi8(T07b, c_zero);

	__m128i T20      = _mm_sub_epi16(T10a, T10b);//16bit Resi: 05 02 06 01 04 03 07 00
	__m128i T21      = _mm_sub_epi16(T11a, T11b);
	__m128i T22      = _mm_sub_epi16(T12a, T12b);
	__m128i T23      = _mm_sub_epi16(T13a, T13b);
	__m128i T24      = _mm_sub_epi16(T14a, T14b);
	__m128i T25      = _mm_sub_epi16(T15a, T15b);
	__m128i T26      = _mm_sub_epi16(T16a, T16b);
	__m128i T27      = _mm_sub_epi16(T17a, T17b);

	__m128i T30a = _mm_hadd_epi16(T20, T27);//16bit: 72+75 71+76 73+74 70+77 02+05 01+06 03+04 00+07
	__m128i T31a = _mm_hadd_epi16(T23, T24);//16bit: 42+45 41+46 43+44 40+47
	__m128i T32a = _mm_hadd_epi16(T21, T26);//16bit: 62+65 61+66 63+64 60+67
	__m128i T33a = _mm_hadd_epi16(T22, T25);//16bit: 52+55 51+56 53+54 50+57
	__m128i T30b = _mm_hsub_epi16(T20, T27);//16bit: 2-5 1-6 3-4 0-7 02-05 01-06 03-04 00-07
	__m128i T31b = _mm_hsub_epi16(T23, T24);//16bit: 2-5 1-6 3-4 0-7
	__m128i T32b = _mm_hsub_epi16(T21, T26);//16bit: 2-5 1-6 3-4 0-7
	__m128i T33b = _mm_hsub_epi16(T22, T25);//16bit: 2-5 1-6 3-4 0-7

	__m128i T40aa = _mm_hadd_epi16(T30a, T31a);//16bit: (1+6)+(2+5) (0+7)+(3+4) Row 4 3 7 0
	__m128i T41aa = _mm_hadd_epi16(T32a, T33a);//16bit: (1+6)+(2+5) (0+7)+(3+4) Row 5 2 6 1
	__m128i T40ab = _mm_hsub_epi16(T30a, T31a);//16bit: (1+6)-(2+5) (0+7)-(3+4) Row 4 3 7 0
	__m128i T41ab = _mm_hsub_epi16(T32a, T33a);//16bit: (1+6)-(2+5) (0+7)-(3+4) Row 5 2 6 1
	__m128i T40ba = _mm_hadd_epi16(T30b, T31b);//16bit: (1-6)+(2-5) (0-7)+(3-4)
	__m128i T41ba = _mm_hadd_epi16(T32b, T33b);//16bit: (1-6)+(2-5) (0-7)+(3-4)
	__m128i T40bb = _mm_hsub_epi16(T30b, T31b);//16bit: (1-6)-(2-5) (0-7)-(3-4)
	__m128i T41bb = _mm_hsub_epi16(T32b, T33b);//16bit: (1-6)-(2-5) (0-7)-(3-4)

	__m128i T50   = _mm_hadd_epi16(T40aa, T41aa);//16bit DstRow: 05 02 06 01 04 03 07 00
	__m128i T54   = _mm_hadd_epi16(T40ba, T41ba);
	__m128i T51   = _mm_hsub_epi16(T40bb, T41bb);
	__m128i T55   = _mm_hsub_epi16(T40ab, T41ab);
	__m128i T52   = _mm_hadd_epi16(T40bb, T41bb);
	__m128i T56   = _mm_hadd_epi16(T40ab, T41ab);
	__m128i T53   = _mm_hsub_epi16(T40aa, T41aa);
	__m128i T57   = _mm_hsub_epi16(T40ba, T41ba);

	//SATD2
	__m128i T60a = _mm_hadd_epi16(T50, T51);//16bit: 2+5 1+6 3+4 0+7 02+05 01+06 03+04 00+07
	__m128i T61a = _mm_hadd_epi16(T52, T53);//16bit: 2+5 1+6 3+4 0+7
	__m128i T62a = _mm_hadd_epi16(T54, T55);//16bit: 2+5 1+6 3+4 0+7
	__m128i T63a = _mm_hadd_epi16(T56, T57);//16bit: 2+5 1+6 3+4 0+7
	__m128i T60b = _mm_hsub_epi16(T50, T51);//16bit: 2-5 1-6 3-4 0-7 02-05 01-06 03-04 00-07
	__m128i T61b = _mm_hsub_epi16(T52, T53);//16bit: 2-5 1-6 3-4 0-7
	__m128i T62b = _mm_hsub_epi16(T54, T55);//16bit: 2-5 1-6 3-4 0-7
	__m128i T63b = _mm_hsub_epi16(T56, T57);//16bit: 2-5 1-6 3-4 0-7

	__m128i T70aa = _mm_hadd_epi16(T60a, T61a);//16bit: (1+6)+(2+5) (0+7)+(3+4) Row 3 2 1 0
	__m128i T71aa = _mm_hadd_epi16(T62a, T63a);//16bit: (1+6)+(2+5) (0+7)+(3+4) Row 7 6 5 4
	__m128i T70ab = _mm_hsub_epi16(T60a, T61a);//16bit: (1+6)-(2+5) (0+7)-(3+4) Row 3 2 1 0
	__m128i T71ab = _mm_hsub_epi16(T62a, T63a);//16bit: (1+6)-(2+5) (0+7)-(3+4) Row 7 6 5 4
	__m128i T70ba = _mm_hadd_epi16(T60b, T61b);//16bit: (1-6)+(2-5) (0-7)+(3-4)
	__m128i T71ba = _mm_hadd_epi16(T62b, T63b);//16bit: (1-6)+(2-5) (0-7)+(3-4)
	__m128i T70bb = _mm_hsub_epi16(T60b, T61b);//16bit: (1-6)-(2-5) (0-7)-(3-4)
	__m128i T71bb = _mm_hsub_epi16(T62b, T63b);//16bit: (1-6)-(2-5) (0-7)-(3-4)

	__m128i T80   = _mm_hadd_epi16(T70aa, T71aa);
	__m128i T84   = _mm_hadd_epi16(T70ba, T71ba);
	__m128i T81   = _mm_hsub_epi16(T70bb, T71bb);
	__m128i T85   = _mm_hsub_epi16(T70ab, T71ab);
	__m128i T82   = _mm_hadd_epi16(T70bb, T71bb);
	__m128i T86   = _mm_hadd_epi16(T70ab, T71ab);
	__m128i T83   = _mm_hsub_epi16(T70aa, T71aa);
	__m128i T87   = _mm_hsub_epi16(T70ba, T71ba);

	__m128i T90 = _mm_abs_epi16(T80);
	__m128i T91 = _mm_abs_epi16(T81);
	__m128i T92 = _mm_abs_epi16(T82);
	__m128i T93 = _mm_abs_epi16(T83);
	__m128i T94 = _mm_abs_epi16(T84);
	__m128i T95 = _mm_abs_epi16(T85);
	__m128i T96 = _mm_abs_epi16(T86);
	__m128i T97 = _mm_abs_epi16(T87);

	__m128i TA0 = _mm_hadd_epi16(T90, T91);//16bit
	__m128i TA1 = _mm_hadd_epi16(T92, T93);
	__m128i TA2 = _mm_hadd_epi16(T94, T95);
	__m128i TA3 = _mm_hadd_epi16(T96, T97);

	__m128i TB0 = _mm_unpackhi_epi16(TA0, c_zero);//32bit
	__m128i TB1 = _mm_unpackhi_epi16(TA1, c_zero);
	__m128i TB2 = _mm_unpackhi_epi16(TA2, c_zero);
	__m128i TB3 = _mm_unpackhi_epi16(TA3, c_zero);
	__m128i TB4 = _mm_unpacklo_epi16(TA0, c_zero);//32bit
	__m128i TB5 = _mm_unpacklo_epi16(TA1, c_zero);
	__m128i TB6 = _mm_unpacklo_epi16(TA2, c_zero);
	__m128i TB7 = _mm_unpacklo_epi16(TA3, c_zero);

	__m128i TC0 = _mm_add_epi32(_mm_add_epi32(TB0, TB1), _mm_add_epi32(TB2, TB3));
	__m128i TC1 = _mm_add_epi32(_mm_add_epi32(TB4, TB5), _mm_add_epi32(TB6, TB7));
	__m128i TD0 = _mm_add_epi32(TC0, TC1);//32bit
	__m128i TE0 = _mm_hadd_epi32(TD0, TD0);
	__m128i TF0 = _mm_hadd_epi32(TE0, TE0);
	uiSad		= _mm_cvtsi128_si32(TF0);
	uiSad		= ((uiSad+2)>>2) ;
#else
	int p8[8][8] ;
	int iRow, iColumn ;
	for (iRow = 0 ; iRow < 8 ; iRow ++)
		for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
			p8[iRow][iColumn] = pSrc[iRow * nStrideSrc + iColumn] - pRef[iRow * nStrideRef + iColumn] ;

	int Temp8[8][8] ;
	int t ;
	
	for (iRow = 0 ; iRow < 8 ; iRow ++){
		for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
			Temp8[iRow][iColumn] = 0 ;
			for (t = 0 ; t < 8 ; t ++)
				Temp8[iRow][iColumn] += (H8[iRow][t] * p8[t][iColumn]) ;
		}
	}
	int p8_Transformed[8][8] ;
	for (iRow = 0 ; iRow < 8 ; iRow ++){
		for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
			p8_Transformed[iRow][iColumn] = 0 ;
			for (t = 0 ; t < 8 ; t ++)
				p8_Transformed[iRow][iColumn] += Temp8[iRow][t] * H8[t][iColumn] ;
		}
	}
	int iAbsSum8 ;
	iAbsSum8 = 0 ;
	for (iRow = 0 ; iRow < 8 ; iRow ++)
		for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
			iAbsSum8 += abs(p8_Transformed[iRow][iColumn]) ;

	uiSad = ((iAbsSum8+2)>>2) ;
#endif

	return uiSad;
}

UInt32 xCalcHADs16x16( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt32 uiSad = 0;

#if SATD_USE_ASM
	uiSad += xCalcHADs8x8(pSrc				       , nStrideSrc, pRef				        , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+8			       , nStrideSrc, pRef+8				      , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+8*nStrideSrc  , nStrideSrc, pRef+8*nStrideRef  , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+8*nStrideSrc+8, nStrideSrc, pRef+8*nStrideRef+8, nStrideRef);

#else
	int p16[16][16] ;
	int iRow, iColumn ;
	for (iRow = 0 ; iRow < 16 ; iRow ++)
		for (iColumn = 0 ; iColumn < 16 ; iColumn ++)
			p16[iRow][iColumn] = pSrc[iRow * nStrideSrc + iColumn] - pRef[iRow * nStrideRef + iColumn] ;

	int i8x8Block_Row, i8x8Block_Column ;
	for (i8x8Block_Row = 0 ; i8x8Block_Row < 16 / 8 ; i8x8Block_Row ++)
		for (i8x8Block_Column = 0 ; i8x8Block_Column < 16 / 8 ; i8x8Block_Column ++){
			int p8[8][8] ;
			for (iRow = 0 ; iRow < 8 ; iRow ++)
				for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
					p8[iRow][iColumn] = p16[i8x8Block_Row * 8 + iRow][i8x8Block_Column * 8 + iColumn] ;

			int Temp8[8][8] ;
			int t ;
			for (iRow = 0 ; iRow < 8 ; iRow ++)
				for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
					Temp8[iRow][iColumn] = 0 ;
					for (t = 0 ; t < 8 ; t ++)
						Temp8[iRow][iColumn] += (H8[iRow][t] * p8[t][iColumn]) ;
				}

				int p8_Transformed[8][8] ;
				for (iRow = 0 ; iRow < 8 ; iRow ++)
					for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
						p8_Transformed[iRow][iColumn] = 0 ;
						for (t = 0 ; t < 8 ; t ++)
							p8_Transformed[iRow][iColumn] += Temp8[iRow][t] * H8[t][iColumn] ;
					}

					int iAbsSum8 = 0 ;
					for (iRow = 0 ; iRow < 8 ; iRow ++)
						for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
							iAbsSum8 += abs(p8_Transformed[iRow][iColumn]) ;

					uiSad += ((iAbsSum8+2)>>2) ;
		}
#endif
		return uiSad;
}

UInt32 xCalcHADs32x32( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt32 uiSad = 0;

#if SATD_USE_ASM
	uiSad += xCalcHADs8x8(pSrc				    , nStrideSrc, pRef				   , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc			     + 8, nStrideSrc, pRef			     +8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc              +16, nStrideSrc, pRef              +16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc              +24, nStrideSrc, pRef              +24, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc   , nStrideSrc, pRef+ 8*nStrideRef    , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc+ 8, nStrideSrc, pRef+ 8*nStrideRef+ 8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc+16, nStrideSrc, pRef+ 8*nStrideRef+16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc+24, nStrideSrc, pRef+ 8*nStrideRef+24, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc   , nStrideSrc, pRef+16*nStrideRef   , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc+ 8, nStrideSrc, pRef+16*nStrideRef+ 8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc+16, nStrideSrc, pRef+16*nStrideRef+16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc+24, nStrideSrc, pRef+16*nStrideRef+24, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc   , nStrideSrc, pRef+24*nStrideRef   , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc+ 8, nStrideSrc, pRef+24*nStrideRef+ 8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc+16, nStrideSrc, pRef+24*nStrideRef+16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc+24, nStrideSrc, pRef+24*nStrideRef+24, nStrideRef);
#else
	int p32[32][32] ;
	int iRow, iColumn ;
	for (iRow = 0 ; iRow < 32 ; iRow ++)
		for (iColumn = 0 ; iColumn < 32 ; iColumn ++)
			p32[iRow][iColumn] = pSrc[iRow * nStrideSrc + iColumn] - pRef[iRow * nStrideRef + iColumn] ;

	int i8x8Block_Row, i8x8Block_Column ;
	for (i8x8Block_Row = 0 ; i8x8Block_Row < 32 / 8 ; i8x8Block_Row ++)
		for (i8x8Block_Column = 0 ; i8x8Block_Column < 32 / 8 ; i8x8Block_Column ++){
			int p8[8][8] ;
			for (iRow = 0 ; iRow < 8 ; iRow ++)
				for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
					p8[iRow][iColumn] = p32[i8x8Block_Row * 8 + iRow][i8x8Block_Column * 8 + iColumn] ;

			int Temp8[8][8] ;
			int t ;
			for (iRow = 0 ; iRow < 8 ; iRow ++)
				for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
					Temp8[iRow][iColumn] = 0 ;
					for (t = 0 ; t < 8 ; t ++)
						Temp8[iRow][iColumn] += (H8[iRow][t] * p8[t][iColumn]) ;
				}

				int p8_Transformed[8][8] ;
				for (iRow = 0 ; iRow < 8 ; iRow ++)
					for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
						p8_Transformed[iRow][iColumn] = 0 ;
						for (t = 0 ; t < 8 ; t ++)
							p8_Transformed[iRow][iColumn] += Temp8[iRow][t] * H8[t][iColumn] ;
					}

					int iAbsSum8 ;
					iAbsSum8 = 0 ;
					for (iRow = 0 ; iRow < 8 ; iRow ++)
						for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
							iAbsSum8 += abs(p8_Transformed[iRow][iColumn]) ;
			
					uiSad += ((iAbsSum8+2)>>2) ;
		}
#endif
		return uiSad;
}

UInt32 xCalcHADs64x64( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt32 uiSad = 0;
	uiSad += xCalcHADs32x32(pSrc				         ,  nStrideSrc, pRef			           ,	nStrideRef);
	uiSad += xCalcHADs32x32(pSrc+32			         ,  nStrideSrc, pRef+32			         ,	nStrideRef);
	uiSad += xCalcHADs32x32(pSrc+32*nStrideSrc   ,  nStrideSrc, pRef+32*nStrideRef   ,	nStrideRef);
	uiSad += xCalcHADs32x32(pSrc+32*nStrideSrc+32,  nStrideSrc, pRef+32*nStrideRef+32,	nStrideRef);
	return uiSad;
}

// for debuging, remove this when fix this problem 
//x64 modify, WARNING: FIXME: currently, SATD_32x32 ASM from multicoreware will cause great RD decrease (0.5dB+) for intra coding //
//it will clear dLambda to be zero sometimes in xEncDecideCU_Intra in Release mode, probably by memory error
//So SATD_32x32 by YananZhao is used instead, obvious speed drop is not found, 2014-01-12, YananZhao
int xCalcHADs32x32_test( pixel *pSrc, intptr_t nStrideSrc, pixel *pRef, intptr_t nStrideRef )
{
	UInt32 uiSad = 0;

#if SATD_USE_ASM
	uiSad += xCalcHADs8x8(pSrc				    , nStrideSrc, pRef				   , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc			     + 8, nStrideSrc, pRef			     +8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc              +16, nStrideSrc, pRef              +16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc              +24, nStrideSrc, pRef              +24, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc   , nStrideSrc, pRef+ 8*nStrideRef    , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc+ 8, nStrideSrc, pRef+ 8*nStrideRef+ 8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc+16, nStrideSrc, pRef+ 8*nStrideRef+16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+ 8*nStrideSrc+24, nStrideSrc, pRef+ 8*nStrideRef+24, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc   , nStrideSrc, pRef+16*nStrideRef   , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc+ 8, nStrideSrc, pRef+16*nStrideRef+ 8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc+16, nStrideSrc, pRef+16*nStrideRef+16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+16*nStrideSrc+24, nStrideSrc, pRef+16*nStrideRef+24, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc   , nStrideSrc, pRef+24*nStrideRef   , nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc+ 8, nStrideSrc, pRef+24*nStrideRef+ 8, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc+16, nStrideSrc, pRef+24*nStrideRef+16, nStrideRef);
	uiSad += xCalcHADs8x8(pSrc+24*nStrideSrc+24, nStrideSrc, pRef+24*nStrideRef+24, nStrideRef);
#else
	int p32[32][32] ;
	int iRow, iColumn ;
	for (iRow = 0 ; iRow < 32 ; iRow ++)
		for (iColumn = 0 ; iColumn < 32 ; iColumn ++)
			p32[iRow][iColumn] = pSrc[iRow * nStrideSrc + iColumn] - pRef[iRow * nStrideRef + iColumn] ;

	int i8x8Block_Row, i8x8Block_Column ;
	for (i8x8Block_Row = 0 ; i8x8Block_Row < 32 / 8 ; i8x8Block_Row ++)
		for (i8x8Block_Column = 0 ; i8x8Block_Column < 32 / 8 ; i8x8Block_Column ++){
			int p8[8][8] ;
			for (iRow = 0 ; iRow < 8 ; iRow ++)
				for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
					p8[iRow][iColumn] = p32[i8x8Block_Row * 8 + iRow][i8x8Block_Column * 8 + iColumn] ;

			int Temp8[8][8] ;
			int t ;
			for (iRow = 0 ; iRow < 8 ; iRow ++)
				for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
					Temp8[iRow][iColumn] = 0 ;
					for (t = 0 ; t < 8 ; t ++)
						Temp8[iRow][iColumn] += (H8[iRow][t] * p8[t][iColumn]) ;
				}

				int p8_Transformed[8][8] ;
				for (iRow = 0 ; iRow < 8 ; iRow ++)
					for (iColumn = 0 ; iColumn < 8 ; iColumn ++){
						p8_Transformed[iRow][iColumn] = 0 ;
						for (t = 0 ; t < 8 ; t ++)
							p8_Transformed[iRow][iColumn] += Temp8[iRow][t] * H8[t][iColumn] ;
					}

					int iAbsSum8 ;
					iAbsSum8 = 0 ;
					for (iRow = 0 ; iRow < 8 ; iRow ++)
						for (iColumn = 0 ; iColumn < 8 ; iColumn ++)
							iAbsSum8 += abs(p8_Transformed[iRow][iColumn]) ;

					uiSad += ((iAbsSum8+2)>>2) ;
		}
#endif
		return uiSad;
}

xSatd *xSatdN[MAX_CU_DEPTH] = {
#if SATD_USE_TRUE_ASM
	x265_pixel_satd_4x4_sse4,
	x265_pixel_satd_8x8_sse4,
	x265_pixel_satd_16x16_sse4,
	//x265_pixel_satd_32x32_sse4 //x64 modify, warning, see above FIXME for xCalcHADs32x32_test, YananZhao, 2014-01-12
	xCalcHADs32x32_test
#else
	xCalcHADs4x4,
	xCalcHADs8x8,
	xCalcHADs16x16,
	xCalcHADs32x32
#endif
};
//#endif

#if (SAD_USE_ASM >= ASM_SSE2)
UInt32 xSad4x4( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt y;
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

#if (SAD_USE_ASM >= ASM_SSE2)
UInt32 xSad8x8( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	Int y;//x64 modify
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

#if (SAD_USE_ASM >= ASM_SSE2)
UInt32 xSad16x16( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt y;
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

#if (SAD_USE_ASM >= ASM_SSE2)
UInt32 xSad32x32( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
    UInt y;
    UInt32 uiSad = 0;
    CUInt8 *P = pSrc;
    CUInt8 *Q = pRef;
    __m128i sumA = _mm_setzero_si128();
    __m128i sumB = _mm_setzero_si128();

    for( y=0; y<32; y++ ) {
        sumA = _mm_add_epi32(sumA, _mm_sad_epu8(_mm_loadu_si128((__m128i*)P + 0), _mm_loadu_si128((__m128i*)Q + 0)));//x64 modify, replace load with loadu
        sumB = _mm_add_epi32(sumB, _mm_sad_epu8(_mm_loadu_si128((__m128i*)P + 1), _mm_loadu_si128((__m128i*)Q + 1)));//x64 modify, replace load with loadu
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

UInt32 xSad64x64( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	Int x, y;//x64 modify, change type from UInt to Int
	UInt32 uiSad = 0;

	for( y=0; y<64; y++ ) {
		for( x=0; x<64; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}

	//x64 modify, replace 64x64 with 32x32, avoiding signed/unsigned bug in above C code
	//uiSad += xSad32x32( pSrc, nStrideSrc, pRef, nStrideRef );
	//uiSad += xSad32x32( pSrc+32, nStrideSrc, pRef+32, nStrideRef );
	//uiSad += xSad32x32( pSrc+32*nStrideSrc, nStrideSrc, pRef+32*nStrideRef, nStrideRef );
	//uiSad += xSad32x32( pSrc+32*nStrideSrc+32, nStrideSrc, pRef+32*nStrideRef+32, nStrideRef );

	return uiSad;
}

UInt32 xSad64xN( const UInt N, const UInt8 *pSrc, const UInt nStrideSrc, const UInt8 *pRef, const UInt nStrideRef )
{
	UInt x, y;
	UInt32 uiSad = 0;

	for( y=0; y<N; y++ ) {
		for( x=0; x<64; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}
	return uiSad;
}

xSad *xSadN[MAX_CU_DEPTH] = {
  xSad4x4,    /*  4 x 4 */
  xSad8x8,    /*  8 x 8 */
  xSad16x16,   /* 16 x 16 */
	xSad32x32,   /* 32 x 32 */
	xSad64x64,   /* 64 x 64 */
};

UInt32 xSad8x16( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt x, y;
	UInt32 uiSad = 0;

	for( y=0; y<16; y++ ) {
		for( x=0; x<8; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}
	return uiSad;
}
UInt32 xSad16x8( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt x, y;
	UInt32 uiSad = 0;

	for( y=0; y<8; y++ ) {
		for( x=0; x<16; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}
	return uiSad;
}

UInt32 xSad16x32( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt x, y;
	UInt32 uiSad = 0;

	for( y=0; y<32; y++ ) {
		for( x=0; x<16; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}
	return uiSad;
}
UInt32 xSad32x16( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt x, y;
	UInt32 uiSad = 0;

	for( y=0; y<16; y++ ) {
		for( x=0; x<32; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}
	return uiSad;
}
UInt32 xSad32x64( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt x, y;
	UInt32 uiSad = 0;

	for( y=0; y<64; y++ ) {
		for( x=0; x<32; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}
	return uiSad;
}
UInt32 xSad64x32( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef )
{
	UInt x, y;
	UInt32 uiSad = 0;

	for( y=0; y<32; y++ ) {
		for( x=0; x<64; x++ ) {
			uiSad += abs(pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]);
		}
	}
	return uiSad;
}

#if SSD_USE_TRUE_ASM
extern "C" int x265_pixel_ssd_4x4_ssse3(pixel*, intptr_t, pixel*, intptr_t);//ssse3, no more higher sets available
extern "C" int x265_pixel_ssd_8x8_avx(pixel*, intptr_t, pixel*, intptr_t);//sse2, ssse3, avx
extern "C" int x265_pixel_ssd_16x16_avx(pixel*, intptr_t, pixel*, intptr_t);//sse2, ssse3, avx
extern "C" int x265_pixel_ssd_32x32_avx(pixel*, intptr_t, pixel*, intptr_t);//sse2, ssse3, avx
//extern "C" int x265_pixel_ssd_64x64_avx2(pixel*, intptr_t, pixel*, intptr_t);//sse2, ssse3, avx
int x265_pixel_ssd_64x64_avx2(pixel* pSrc, intptr_t iStrideSrc, pixel* pRef, intptr_t iStrideRef)//x64 modify, convert to Int
{
	int iSsd = 0;
	iSsd += x265_pixel_ssd_32x32_avx(pSrc   , iStrideSrc, pRef   , iStrideRef);
	iSsd += x265_pixel_ssd_32x32_avx(pSrc+32, iStrideSrc, pRef+32, iStrideRef);
	iSsd += x265_pixel_ssd_32x32_avx(pSrc+32*iStrideSrc   , iStrideSrc, pRef+32*iStrideRef   , iStrideRef);
	iSsd += x265_pixel_ssd_32x32_avx(pSrc+32*iStrideSrc+32, iStrideSrc, pRef+32*iStrideRef+32, iStrideRef);

	return iSsd;
}

xSSD *xSsdN[MAX_CU_DEPTH] = {
	x265_pixel_ssd_4x4_ssse3,    /*  4 x 4 */
	x265_pixel_ssd_8x8_avx,    /*  8 x 8 */
	x265_pixel_ssd_16x16_avx,   /* 16 x 16 */
	x265_pixel_ssd_32x32_avx,   /* 32 x 32 */
	x265_pixel_ssd_64x64_avx2,   /* 64 x 64 */
};
#else
UInt32 xSsd( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef, UInt32 nSize )
{
	UInt x, y;
	UInt32 uiSsd = 0;

	for( y=0; y<nSize; y++ ) {
		for( x=0; x<nSize; x++ ) {
			uiSsd += abs((pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]))*abs((pSrc[y * nStrideSrc + x] - pRef[y * nStrideRef + x]));
		}
	}
	return uiSsd;
}
#endif


xME_SAD  *xSad_AMP[10] = {//2*logX + logY - 9
	xSad8x8,   
	xSad8x16,
	xSad16x8,
	xSad16x16,
	xSad16x32,
	xSad32x16,
	xSad32x32,
	xSad32x64,
	xSad64x32,
	xSad64x64
};

xME_SATD  *xSatd_AMP[10] = {//2*logX + logY - 9
	xCalcHADs8x8,   
	NULL,
	NULL,
	xCalcHADs16x16,
	NULL,
	NULL,
	xCalcHADs32x32,
	NULL,
	NULL,
	xCalcHADs64x64
};

// ***************************************************************************
// * Interface Functions
// ***************************************************************************
#if (QUANT_USE_ASM >= ASM_SSE4)
UInt32 xQuant4( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight, xSliceType eSType )
{
    int y;
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
    int y;
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
    int y;
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
			uiAcSum += iLevel ;
			iLevel *= iSign;
            pDst[nBlockPos] = iLevel;
            pDst[nBlockPos] = Clip3(-32768, 32767, pDst[nBlockPos]);
        }
    }
    return uiAcSum;
}
#endif

#if (DEQUANT_USE_ASM >= ASM_SSE4)
void xDeQuant4( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight )//x64 modify, remove xSliceType
{
	int y;
	CUInt nQpDiv6 = nQP / 6;
	CUInt nQpMod6 = nQP % 6;
	Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - (MAX_TR_DYNAMIC_RANGE - 8 - 2);
	Int32 iRnd = 1 << (nShift-1);
	Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

	__m128i RND1 = _mm_set1_epi32( (iRnd<<16) | iScale );          //16bit: [iR1  iScale iR1  iScale ..] 
	__m128i RND2 = _mm_set1_epi16(                1    );          //16bit: [1     1      1     1    ..]

	for( y=0; y < iHeight; y++ ) {
		__m128i T00 = _mm_loadl_epi64((__m128i*)&pSrc[y*nStride]);//16bit: [xx  xx  xx xx 03  02 01  00]
		__m128i T01 = _mm_unpacklo_epi16(T00, RND2);              //16bit: [iR2 03 iR2 02 iR2 01 iR2 00]
		__m128i T10 = _mm_madd_epi16(T01, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T20 = _mm_srai_epi32(T10, nShift);
		__m128i T30 = _mm_packs_epi32(T20, T20);
		_mm_storel_epi64((__m128i*)&pDst[y*nStride], T30);
	}
}
void xDeQuant8( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight )//x64 modify, remove xSliceType
{
	int y;
	CUInt nQpDiv6 = nQP / 6;
	CUInt nQpMod6 = nQP % 6;
	Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - (MAX_TR_DYNAMIC_RANGE - 8 - 3);
	Int32 iRnd = 1 << (nShift-1);
	Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

	__m128i RND1 = _mm_set1_epi32( (iRnd<<16) | iScale );          //16bit: [iR1  iScale iR1  iScale ..] 
	__m128i RND2 = _mm_set1_epi16(                1    );          //16bit: [1     1      1     1    ..]

	for( y=0; y < iHeight; y++ ) {
		__m128i T00D = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+ 0]);//16bit: [07 06 05 04 03 02 01 00]

		__m128i T01G = _mm_unpackhi_epi16(T00D, RND2);              //16bit: [07 06 05 04]
		__m128i T01H = _mm_unpacklo_epi16(T00D, RND2);              //16bit: [03 02 01 00]

		__m128i T10G = _mm_madd_epi16(T01G, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10H = _mm_madd_epi16(T01H, RND1);                  //32bit: [pSrc * iScale + iRnd]

		__m128i T20G = _mm_srai_epi32(T10G, nShift);
		__m128i T20H = _mm_srai_epi32(T10H, nShift);

		__m128i T30D = _mm_packs_epi32(T20H, T20G);//16bit: [07 06 05 04 03 02 01 00]

		_mm_storeu_si128((__m128i*)&pDst[y*nStride+ 0], T30D);
	}
}
void xDeQuant16( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight )//x64 modify, remove xSliceType
{
	int y;
	CUInt nQpDiv6 = nQP / 6;
	CUInt nQpMod6 = nQP % 6;
	Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - (MAX_TR_DYNAMIC_RANGE - 8 - 4);
	Int32 iRnd = 1 << (nShift-1);
	Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

	__m128i RND1 = _mm_set1_epi32( (iRnd<<16) | iScale );          //16bit: [iR1  iScale iR1  iScale ..] 
	__m128i RND2 = _mm_set1_epi16(                1    );          //16bit: [1     1      1     1    ..]

	for( y=0; y < iHeight; y++ ) {
		__m128i T00C = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+ 8]);//16bit: [15 14 13 12 11 10 09 08]
		__m128i T00D = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+ 0]);//16bit: [07 06 05 04 03 02 01 00]

		__m128i T01E = _mm_unpackhi_epi16(T00C, RND2);              //16bit: [15 14 13 12]
		__m128i T01F = _mm_unpacklo_epi16(T00C, RND2);              //16bit: [11 10 09 08]
		__m128i T01G = _mm_unpackhi_epi16(T00D, RND2);              //16bit: [07 06 05 04]
		__m128i T01H = _mm_unpacklo_epi16(T00D, RND2);              //16bit: [03 02 01 00]

		__m128i T10E = _mm_madd_epi16(T01E, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10F = _mm_madd_epi16(T01F, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10G = _mm_madd_epi16(T01G, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10H = _mm_madd_epi16(T01H, RND1);                  //32bit: [pSrc * iScale + iRnd]

		__m128i T20E = _mm_srai_epi32(T10E, nShift);
		__m128i T20F = _mm_srai_epi32(T10F, nShift);
		__m128i T20G = _mm_srai_epi32(T10G, nShift);
		__m128i T20H = _mm_srai_epi32(T10H, nShift);

		__m128i T30C = _mm_packs_epi32(T20F, T20E);//16bit: [15 14 13 12 11 10 09 08]
		__m128i T30D = _mm_packs_epi32(T20H, T20G);//16bit: [07 06 05 04 03 02 01 00]

		_mm_storeu_si128((__m128i*)&pDst[y*nStride+ 8], T30C);
		_mm_storeu_si128((__m128i*)&pDst[y*nStride+ 0], T30D);
	}
}
void xDeQuant32( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight )//x64 modify, remove xSliceType
{
	int y;
	CUInt nQpDiv6 = nQP / 6;
	CUInt nQpMod6 = nQP % 6;
	Int nShift = QUANT_IQUANT_SHIFT - QUANT_SHIFT - (MAX_TR_DYNAMIC_RANGE - 8 - 5);
	Int32 iRnd = 1 << (nShift-1);
	Int iScale = xg_invQuantScales[nQpMod6] << nQpDiv6;

	__m128i RND1 = _mm_set1_epi32( (iRnd<<16) | iScale );          //16bit: [iR1  iScale iR1  iScale ..] 
	__m128i RND2 = _mm_set1_epi16(                1    );          //16bit: [1     1      1     1    ..]

	for( y=0; y < iHeight; y++ ) {
		__m128i T00A = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+24]);//16bit: [31 30 29 28 27 26 25 24]
		__m128i T00B = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+16]);//16bit: [23 22 21 20 19 18 17 16]
		__m128i T00C = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+ 8]);//16bit: [15 14 13 12 11 10 09 08]
		__m128i T00D = _mm_loadu_si128((__m128i*)&pSrc[y*nStride+ 0]);//16bit: [07 06 05 04 03 02 01 00]

		__m128i T01A = _mm_unpackhi_epi16(T00A, RND2);              //16bit: [v1 31 v1 30 v1 29 v1 28]
		__m128i T01B = _mm_unpacklo_epi16(T00A, RND2);              //16bit: [27 26 25 24]
		__m128i T01C = _mm_unpackhi_epi16(T00B, RND2);              //16bit: [23 22 21 20]
		__m128i T01D = _mm_unpacklo_epi16(T00B, RND2);              //16bit: [19 18 17 16]
		__m128i T01E = _mm_unpackhi_epi16(T00C, RND2);              //16bit: [15 14 13 12]
		__m128i T01F = _mm_unpacklo_epi16(T00C, RND2);              //16bit: [11 10 09 08]
		__m128i T01G = _mm_unpackhi_epi16(T00D, RND2);              //16bit: [07 06 05 04]
		__m128i T01H = _mm_unpacklo_epi16(T00D, RND2);              //16bit: [03 02 01 00]

		__m128i T10A = _mm_madd_epi16(T01A, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10B = _mm_madd_epi16(T01B, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10C = _mm_madd_epi16(T01C, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10D = _mm_madd_epi16(T01D, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10E = _mm_madd_epi16(T01E, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10F = _mm_madd_epi16(T01F, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10G = _mm_madd_epi16(T01G, RND1);                  //32bit: [pSrc * iScale + iRnd]
		__m128i T10H = _mm_madd_epi16(T01H, RND1);                  //32bit: [pSrc * iScale + iRnd]

		__m128i T20A = _mm_srai_epi32(T10A, nShift);
		__m128i T20B = _mm_srai_epi32(T10B, nShift);
		__m128i T20C = _mm_srai_epi32(T10C, nShift);
		__m128i T20D = _mm_srai_epi32(T10D, nShift);
		__m128i T20E = _mm_srai_epi32(T10E, nShift);
		__m128i T20F = _mm_srai_epi32(T10F, nShift);
		__m128i T20G = _mm_srai_epi32(T10G, nShift);
		__m128i T20H = _mm_srai_epi32(T10H, nShift);

		__m128i T30A = _mm_packs_epi32(T20B, T20A);//16bit: [31 30 29 28 27 26 25 24]
		__m128i T30B = _mm_packs_epi32(T20D, T20C);//16bit: [23 22 21 20 19 18 17 16]
		__m128i T30C = _mm_packs_epi32(T20F, T20E);//16bit: [15 14 13 12 11 10 09 08]
		__m128i T30D = _mm_packs_epi32(T20H, T20G);//16bit: [07 06 05 04 03 02 01 00]

		_mm_storeu_si128((__m128i*)&pDst[y*nStride+24], T30A);
		_mm_storeu_si128((__m128i*)&pDst[y*nStride+16], T30B);
		_mm_storeu_si128((__m128i*)&pDst[y*nStride+ 8], T30C);
		_mm_storeu_si128((__m128i*)&pDst[y*nStride+ 0], T30D);
	}
}
typedef void (*DeQuant_ptr)( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iHeight );//x64 modify, remove xSliceType
DeQuant_ptr xDeQuantN[4] = {
    xDeQuant4,
    xDeQuant8,
    xDeQuant16,
    xDeQuant32,
};
void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight )//x64 modify, remove xSliceType
{
    xDeQuantN[xLog2(iWidth-1)-2](pDst, pSrc, nStride, nQP, iHeight);//x64 modify, remove xSliceType
}
#else // ASM_NONE
void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight )//x64 modify, remove xSliceType
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
//------------------forward transform-------------------------------------
//{//transpose 4x4 32bit matrix
__m128i T00, T01, T02, T03;
#define TRANSPOSE_4x4_32BIT(I0, I1, I2, I3)\
	T00 = _mm_unpacklo_epi32(I0, I1);\
	T01 = _mm_unpackhi_epi32(I0, I1);\
	T02 = _mm_unpacklo_epi32(I2, I3);\
	T03 = _mm_unpackhi_epi32(I2, I3);\
	I0  = _mm_unpacklo_epi64(T00, T02);\
	I1  = _mm_unpackhi_epi64(T00, T02);\
	I2  = _mm_unpacklo_epi64(T01, T03);\
	I3  = _mm_unpackhi_epi64(T01, T03);

//transpose matrix 8x8 16bit. 
__m128i tr0_0, tr0_1, tr0_2, tr0_3, tr0_4, tr0_5, tr0_6, tr0_7;
__m128i tr1_0, tr1_1, tr1_2, tr1_3, tr1_4, tr1_5, tr1_6, tr1_7;
#define TRANSPOSE_8x8_16BIT(I0, I1, I2, I3, I4, I5, I6, I7, O0, O1, O2, O3, O4, O5, O6, O7)\
	tr0_0 = _mm_unpacklo_epi16(I0, I1);\
	tr0_1 = _mm_unpacklo_epi16(I2, I3);\
	tr0_2 = _mm_unpackhi_epi16(I0, I1);\
	tr0_3 = _mm_unpackhi_epi16(I2, I3);\
	tr0_4 = _mm_unpacklo_epi16(I4, I5);\
	tr0_5 = _mm_unpacklo_epi16(I6, I7);\
	tr0_6 = _mm_unpackhi_epi16(I4, I5);\
	tr0_7 = _mm_unpackhi_epi16(I6, I7);\
	tr1_0 = _mm_unpacklo_epi32(tr0_0, tr0_1);\
	tr1_1 = _mm_unpacklo_epi32(tr0_2, tr0_3);\
	tr1_2 = _mm_unpackhi_epi32(tr0_0, tr0_1);\
	tr1_3 = _mm_unpackhi_epi32(tr0_2, tr0_3);\
	tr1_4 = _mm_unpacklo_epi32(tr0_4, tr0_5);\
	tr1_5 = _mm_unpacklo_epi32(tr0_6, tr0_7);\
	tr1_6 = _mm_unpackhi_epi32(tr0_4, tr0_5);\
	tr1_7 = _mm_unpackhi_epi32(tr0_6, tr0_7);\
	O0 = _mm_unpacklo_epi64(tr1_0, tr1_4);\
	O1 = _mm_unpackhi_epi64(tr1_0, tr1_4);\
	O2 = _mm_unpacklo_epi64(tr1_2, tr1_6);\
	O3 = _mm_unpackhi_epi64(tr1_2, tr1_6);\
	O4 = _mm_unpacklo_epi64(tr1_1, tr1_5);\
	O5 = _mm_unpackhi_epi64(tr1_1, tr1_5);\
	O6 = _mm_unpacklo_epi64(tr1_3, tr1_7);\
	O7 = _mm_unpackhi_epi64(tr1_3, tr1_7);

#if (SUB_DCT_USE_ASM >= ASM_SSE4)
void xSubDCT4(
    Int16 *pDst,
    UInt8 *pSrc, UInt nStride,
    UInt8 *pRef, UInt nStrideRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
    (void *)piTmp0;//avoid warning
		(void *)piTmp1;//avoid warning

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

#if (SUB_DCT_USE_ASM >= ASM_SSE4)
void xSubDST4(
	Int16 *pDst,
	UInt8 *pSrc, UInt nStride,
	UInt8 *pRef, UInt nStrideRef,
	Int16 *piTmp0, Int16 *piTmp1
	)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning

	// Const
	__m128i zero         = _mm_setzero_si128();
	__m128i rnd1         = _mm_set1_epi32(1);
	__m128i rnd2         = _mm_set1_epi32(128);
	__m128i c16_m0       = _mm_set_epi16( 84, 74,  55,  29, 84, 74,  55,  29);
	__m128i c16_m1       = _mm_set_epi16(-74, 0 ,  74,  74,-74, 0 ,  74,  74);
	__m128i c16_m2       = _mm_set_epi16( 55, -74, -29, 84, 55, -74, -29, 84);
	__m128i c16_m3       = _mm_set_epi16(-29, 74, -84, 55, -29, 74, -84, 55);
	__m128i c32_m0       = _mm_set_epi32( 84, 74,  55,  29);
	__m128i c32_m1       = _mm_set_epi32(-74, 0 ,  74,  74);
	__m128i c32_m2       = _mm_set_epi32( 55, -74, -29, 84);
	__m128i c32_m3       = _mm_set_epi32(-29, 74, -84, 55);

	//Sub
	__m128i T00A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[0*nStride]);//Src 8bit: [03 02 01 00]
	__m128i T01A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[1*nStride]);//Src 8bit: [13 12 11 10]
	__m128i T02A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[2*nStride]);//Src 8bit: [23 22 21 20]
	__m128i T03A = _mm_cvtsi32_si128(*(UInt32*)&pSrc[3*nStride]);//Src 8bit: [33 32 31 30]
	__m128i T00B = _mm_cvtsi32_si128(*(UInt32*)&pRef[0*nStrideRef]);
	__m128i T01B = _mm_cvtsi32_si128(*(UInt32*)&pRef[1*nStrideRef]);
	__m128i T02B = _mm_cvtsi32_si128(*(UInt32*)&pRef[2*nStrideRef]);
	__m128i T03B = _mm_cvtsi32_si128(*(UInt32*)&pRef[3*nStrideRef]);

	__m128i T10A = _mm_unpacklo_epi8(T00A, zero);//Src 16bit: [03 02 01 00]
	__m128i T11A = _mm_unpacklo_epi8(T01A, zero);//Src 16bit: [13 12 11 10]
	__m128i T12A = _mm_unpacklo_epi8(T02A, zero);//Src 16bit: [23 22 21 20]
	__m128i T13A = _mm_unpacklo_epi8(T03A, zero);//Src 16bit: [33 32 31 30]
	__m128i T10B = _mm_unpacklo_epi8(T00B, zero);
	__m128i T11B = _mm_unpacklo_epi8(T01B, zero);
	__m128i T12B = _mm_unpacklo_epi8(T02B, zero);
	__m128i T13B = _mm_unpacklo_epi8(T03B, zero);
	__m128i T20  = _mm_sub_epi16(T10A, T10B);       //Res 16bit: [03 02 01 00]
	__m128i T21  = _mm_sub_epi16(T11A, T11B);       //Res 16bit: [13 12 11 10]
	__m128i T22  = _mm_sub_epi16(T12A, T12B);       //Res 16bit: [23 22 21 20]
	__m128i T23  = _mm_sub_epi16(T13A, T13B);       //Res 16bit: [33 32 31 30]

	// DST1
	__m128i T30  = _mm_unpacklo_epi64(T20, T21);        // [13 12 11 10 03 02 01 00]
	__m128i T31  = _mm_unpacklo_epi64(T22, T23);        // [33 32 31 30 23 22 21 20]
	__m128i T40a  = _mm_madd_epi16(T30, c16_m0);
	__m128i T40b  = _mm_madd_epi16(T31, c16_m0);
	__m128i T41a  = _mm_madd_epi16(T30, c16_m1);
	__m128i T41b  = _mm_madd_epi16(T31, c16_m1);
	__m128i T42a  = _mm_madd_epi16(T30, c16_m2);
	__m128i T42b  = _mm_madd_epi16(T31, c16_m2);
	__m128i T43a  = _mm_madd_epi16(T30, c16_m3);
	__m128i T43b  = _mm_madd_epi16(T31, c16_m3);
	__m128i T50   = _mm_hadd_epi32(T40a, T40b);
	__m128i T51   = _mm_hadd_epi32(T41a, T41b);
	__m128i T52   = _mm_hadd_epi32(T42a, T42b);
	__m128i T53   = _mm_hadd_epi32(T43a, T43b);
	__m128i T60  = _mm_srai_epi32(_mm_add_epi32(rnd1, T50), 1);  // [03 02 01 00]
	__m128i T61  = _mm_srai_epi32(_mm_add_epi32(rnd1, T51), 1);  // [13 12 11 10]
	__m128i T62  = _mm_srai_epi32(_mm_add_epi32(rnd1, T52), 1);  // [23 22 21 20]
	__m128i T63  = _mm_srai_epi32(_mm_add_epi32(rnd1, T53), 1);  // [33 32 31 30]

	// DCT2@
	__m128i T70a  = _mm_mullo_epi32(c32_m0, T60);            
	__m128i T71a  = _mm_mullo_epi32(c32_m1, T60);            
	__m128i T72a  = _mm_mullo_epi32(c32_m2, T60);             
	__m128i T73a  = _mm_mullo_epi32(c32_m3, T60);           
	__m128i T70b  = _mm_mullo_epi32(c32_m0, T61);             
	__m128i T71b  = _mm_mullo_epi32(c32_m1, T61);           
	__m128i T72b  = _mm_mullo_epi32(c32_m2, T61);            
	__m128i T73b  = _mm_mullo_epi32(c32_m3, T61);             
	__m128i T70c  = _mm_mullo_epi32(c32_m0, T62);             
	__m128i T71c  = _mm_mullo_epi32(c32_m1, T62);            
	__m128i T72c  = _mm_mullo_epi32(c32_m2, T62);             
	__m128i T73c  = _mm_mullo_epi32(c32_m3, T62);           
	__m128i T70d  = _mm_mullo_epi32(c32_m0, T63);            
	__m128i T71d  = _mm_mullo_epi32(c32_m1, T63);             
	__m128i T72d  = _mm_mullo_epi32(c32_m2, T63);           
	__m128i T73d  = _mm_mullo_epi32(c32_m3, T63);             
	__m128i T80a  = _mm_hadd_epi32(_mm_hadd_epi32(T70a, T70b), _mm_hadd_epi32(T70c, T70d));
	__m128i T80b  = _mm_hadd_epi32(_mm_hadd_epi32(T71a, T71b), _mm_hadd_epi32(T71c, T71d));
	__m128i T80c  = _mm_hadd_epi32(_mm_hadd_epi32(T72a, T72b), _mm_hadd_epi32(T72c, T72d));
	__m128i T80d  = _mm_hadd_epi32(_mm_hadd_epi32(T73a, T73b), _mm_hadd_epi32(T73c, T73d));

	__m128i T90  = _mm_srai_epi32(_mm_add_epi32(rnd2, T80a), 8);  // [03 02 01 00]
	__m128i T91  = _mm_srai_epi32(_mm_add_epi32(rnd2, T80b), 8);  // [13 12 11 10]
	__m128i T92  = _mm_srai_epi32(_mm_add_epi32(rnd2, T80c), 8);  // [23 22 21 20]
	__m128i T93  = _mm_srai_epi32(_mm_add_epi32(rnd2, T80d), 8);  // [33 32 31 30]

	_mm_storel_epi64((__m128i*)&pDst[0*nStride], _mm_packs_epi32(T90, T90));
	_mm_storel_epi64((__m128i*)&pDst[1*nStride], _mm_packs_epi32(T91, T91));
	_mm_storel_epi64((__m128i*)&pDst[2*nStride], _mm_packs_epi32(T92, T92));
	_mm_storel_epi64((__m128i*)&pDst[3*nStride], _mm_packs_epi32(T93, T93));
}
#else
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
#endif

#if (SUB_DCT_USE_ASM >= ASM_SSE4)
void xSubDCT8(
	Int16 *pDst,
	UInt8 *pSrc, UInt nStride,
	UInt8 *pRef, UInt nStrideRef,
	Int16 *piTmp0, Int16 *piTmp1
	)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning

	const __m128i c_zero = _mm_setzero_si128();
	__m128i in0, in1, in2, in3, in4, in5, in6, in7;
	__m128i temp32[8][2];//8 rows
	{
		const __m128i P0_01_00a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[0*nStride]),c_zero);//Org 16bit: [07 06 05 04 03 02 01 00]
		const __m128i P0_01_01a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[1*nStride]),c_zero);
		const __m128i P0_01_02a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[2*nStride]),c_zero);
		const __m128i P0_01_03a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[3*nStride]),c_zero);
		const __m128i P0_01_04a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[4*nStride]),c_zero);
		const __m128i P0_01_05a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[5*nStride]),c_zero);
		const __m128i P0_01_06a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[6*nStride]),c_zero);
		const __m128i P0_01_07a	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pSrc[7*nStride]),c_zero);
		const __m128i P0_01_00b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[0*nStrideRef]),c_zero);//Ref 16bit: [07 06 05 04 03 02 01 00]
		const __m128i P0_01_01b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[1*nStrideRef]),c_zero);
		const __m128i P0_01_02b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[2*nStrideRef]),c_zero);
		const __m128i P0_01_03b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[3*nStrideRef]),c_zero);
		const __m128i P0_01_04b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[4*nStrideRef]),c_zero);
		const __m128i P0_01_05b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[5*nStrideRef]),c_zero);
		const __m128i P0_01_06b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[6*nStrideRef]),c_zero);
		const __m128i P0_01_07b	= _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)&pRef[7*nStrideRef]),c_zero);

		in0	= _mm_sub_epi16(P0_01_00a,P0_01_00b);//Residual 16bit
		in1	= _mm_sub_epi16(P0_01_01a,P0_01_01b);
		in2	= _mm_sub_epi16(P0_01_02a,P0_01_02b);
		in3	= _mm_sub_epi16(P0_01_03a,P0_01_03b);
		in4	= _mm_sub_epi16(P0_01_04a,P0_01_04b);
		in5	= _mm_sub_epi16(P0_01_05a,P0_01_05b);
		in6	= _mm_sub_epi16(P0_01_06a,P0_01_06b);
		in7	= _mm_sub_epi16(P0_01_07a,P0_01_07b);

		TRANSPOSE_8x8_16BIT(in0, in1, in2, in3, in4, in5, in6, in7, in0, in1, in2, in3, in4, in5, in6, in7)
	}

	//DCT1. 
	{
		const __m128i c16_p36_p83  = _mm_set1_epi32(0x00240053);
		const __m128i c16_n83_p36  = _mm_set1_epi32(0xFFAD0024);
		const __m128i c16_n64_p64  = _mm_set1_epi32(0xFFC00040);
		const __m128i c16_p64_p64  = _mm_set1_epi32(0x00400040);
		const __m128i c16_p75_p89  = _mm_set1_epi32(0x004B0059);
		const __m128i c16_p18_p50  = _mm_set1_epi32(0x00120032);
		const __m128i c16_n18_p75  = _mm_set1_epi32(0xFFEE004B);
		const __m128i c16_n50_n89  = _mm_set1_epi32(0xFFCEFFA7);
		const __m128i c16_n89_p50  = _mm_set1_epi32(0xFFA70032);
		const __m128i c16_p75_p18  = _mm_set1_epi32(0x004B0012);
		const __m128i c16_n50_p18  = _mm_set1_epi32(0xFFCE0012);
		const __m128i c16_n89_p75  = _mm_set1_epi32(0xFFA7004B);
		const __m128i c32_rnd  = _mm_set1_epi32(2);

		const __m128i T_01_00  = _mm_add_epi16(in0,in7);//Row00+Row07
		const __m128i T_01_01  = _mm_add_epi16(in1,in6);//Row01+Row06
		const __m128i T_01_02  = _mm_add_epi16(in2,in5);//Row02+Row05
		const __m128i T_01_03  = _mm_add_epi16(in3,in4);//Row03+Row04
		const __m128i T_01_04  = _mm_sub_epi16(in0,in7);//
		const __m128i T_01_05  = _mm_sub_epi16(in1,in6);//
		const __m128i T_01_06  = _mm_sub_epi16(in2,in5);//
		const __m128i T_01_07  = _mm_sub_epi16(in3,in4);//
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_01_04, T_01_05);
			const __m128i T00B  = _mm_unpackhi_epi16(T_01_04, T_01_05);
			const __m128i T01A  = _mm_unpacklo_epi16(T_01_06, T_01_07);
			const __m128i T01B  = _mm_unpackhi_epi16(T_01_06, T_01_07);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, c23, rowA, rowB)\
	T10A = _mm_add_epi32(_mm_madd_epi16(c01, T00A), _mm_madd_epi16(c23, T01A));\
	T10B = _mm_add_epi32(_mm_madd_epi16(c01, T00B), _mm_madd_epi16(c23, T01B));\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 2);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 2);\

			COMPUTE_O_ROW(c16_p75_p89, c16_p18_p50, temp32[1][0], temp32[1][1])
				COMPUTE_O_ROW(c16_n18_p75, c16_n50_n89, temp32[3][0], temp32[3][1])
				COMPUTE_O_ROW(c16_n89_p50, c16_p75_p18, temp32[5][0], temp32[5][1])
				COMPUTE_O_ROW(c16_n50_p18, c16_n89_p75, temp32[7][0], temp32[7][1])
#undef COMPUTE_O_ROW
		}

		const __m128i T_02_00  = _mm_add_epi16(T_01_00,T_01_03);//Row00+Row07 + (Row03+Row04)
		const __m128i T_02_01  = _mm_add_epi16(T_01_01,T_01_02);//Row01+Row06 + (Row02+Row05)
		const __m128i T_02_02  = _mm_sub_epi16(T_01_00,T_01_03);//Row00+Row07 - (Row03+Row04)
		const __m128i T_02_03  = _mm_sub_epi16(T_01_01,T_01_02);//Row01+Row06 - (Row02+Row05)
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_02_02, T_02_03);
			const __m128i T00B  = _mm_unpackhi_epi16(T_02_02, T_02_03);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, rowA, rowB)\
	T10A = _mm_madd_epi16(c01, T00A);\
	T10B = _mm_madd_epi16(c01, T00B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 2);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 2);\

			COMPUTE_O_ROW(c16_p36_p83, temp32[2][0], temp32[2][1])
				COMPUTE_O_ROW(c16_n83_p36, temp32[6][0], temp32[6][1])
#undef COMPUTE_O_ROW
		}

		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_02_00, T_02_01);
			const __m128i T00B  = _mm_unpackhi_epi16(T_02_00, T_02_01);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, rowA, rowB)\
	T10A = _mm_madd_epi16(c01, T00A);\
	T10B = _mm_madd_epi16(c01, T00B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 2);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 2);\

			COMPUTE_O_ROW(c16_p64_p64, temp32[0][0], temp32[0][1])
				COMPUTE_O_ROW(c16_n64_p64, temp32[4][0], temp32[4][1])
#undef COMPUTE_O_ROW
		}
	}//END OF DCT1

	//DCT2
	__m128i out32[8][2];
	{
		const __m128i c32_p89 = _mm_set1_epi32( 89);
		const __m128i c32_p75 = _mm_set1_epi32( 75);
		const __m128i c32_p50 = _mm_set1_epi32( 50);
		const __m128i c32_p18 = _mm_set1_epi32( 18);
		const __m128i c32_p83 = _mm_set1_epi32( 83);
		const __m128i c32_p36 = _mm_set1_epi32( 36);
		const __m128i c32_p64 = _mm_set1_epi32( 64);
		const __m128i c32_n89 = _mm_set1_epi32(-89);
		const __m128i c32_n75 = _mm_set1_epi32(-75);
		const __m128i c32_n50 = _mm_set1_epi32(-50);
		const __m128i c32_n18 = _mm_set1_epi32(-18);
		const __m128i c32_n83 = _mm_set1_epi32(-83);
		const __m128i c32_n36 = _mm_set1_epi32(-36);
		const __m128i c32_n64 = _mm_set1_epi32(-64);
		const __m128i c32_rnd = _mm_set1_epi32(256);

		for (Int i=0; i<8; i+=4){//4 rows forms one part
			Int part = (i>>2);    

			TRANSPOSE_4x4_32BIT(temp32[i][0], temp32[i+1][0], temp32[i+2][0], temp32[i+3][0])
				TRANSPOSE_4x4_32BIT(temp32[i][1], temp32[i+1][1], temp32[i+2][1], temp32[i+3][1])

				const __m128i T_00_00 = _mm_add_epi32(temp32[i+0][0], temp32[i+3][1]);
			const __m128i T_00_01 = _mm_add_epi32(temp32[i+1][0], temp32[i+2][1]);
			const __m128i T_00_02 = _mm_add_epi32(temp32[i+2][0], temp32[i+1][1]);
			const __m128i T_00_03 = _mm_add_epi32(temp32[i+3][0], temp32[i+0][1]);
			const __m128i T_00_04 = _mm_sub_epi32(temp32[i+0][0], temp32[i+3][1]);
			const __m128i T_00_05 = _mm_sub_epi32(temp32[i+1][0], temp32[i+2][1]);
			const __m128i T_00_06 = _mm_sub_epi32(temp32[i+2][0], temp32[i+1][1]);
			const __m128i T_00_07 = _mm_sub_epi32(temp32[i+3][0], temp32[i+0][1]);
			{
				__m128i T00, T01, T10;
#define COMPUTE_ROW(c0, c1, c2, c3, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_00_04, c0), _mm_mullo_epi32(T_00_05, c1));\
	T01 = _mm_add_epi32(_mm_mullo_epi32(T_00_06, c2), _mm_mullo_epi32(T_00_07, c3));\
	T10 = _mm_add_epi32(T00, T01);\
	row = _mm_srai_epi32(_mm_add_epi32(T10, c32_rnd), 9);

				COMPUTE_ROW(c32_p89, c32_p75, c32_p50, c32_p18, out32[1][part])
					COMPUTE_ROW(c32_p75, c32_n18, c32_n89, c32_n50, out32[3][part])
					COMPUTE_ROW(c32_p50, c32_n89, c32_p18, c32_p75, out32[5][part])
					COMPUTE_ROW(c32_p18, c32_n50, c32_p75, c32_n89, out32[7][part])
#undef COMPUTE_ROW
			}

			const __m128i T_01_00 = _mm_add_epi32(T_00_00, T_00_03);
			const __m128i T_01_01 = _mm_add_epi32(T_00_01, T_00_02);
			const __m128i T_01_02 = _mm_sub_epi32(T_00_00, T_00_03);
			const __m128i T_01_03 = _mm_sub_epi32(T_00_01, T_00_02);
			{
				__m128i T00;
#define COMPUTE_ROW(c0, c1, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_01_02, c0), _mm_mullo_epi32(T_01_03, c1));\
	row = _mm_srai_epi32(_mm_add_epi32(T00, c32_rnd), 9);

				COMPUTE_ROW(c32_p83, c32_p36, out32[2][part])
					COMPUTE_ROW(c32_p36, c32_n83, out32[6][part])
#undef COMPUTE_ROW
			}

			{
				__m128i T00;
#define COMPUTE_ROW(c0, c1, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_01_00, c0), _mm_mullo_epi32(T_01_01, c1));\
	row = _mm_srai_epi32(_mm_add_epi32(T00, c32_rnd), 9);

				COMPUTE_ROW(c32_p64, c32_p64, out32[0][part])
					COMPUTE_ROW(c32_p64, c32_n64, out32[4][part])
#undef COMPUTE_ROW
			}
		}

		for (Int i=0; i<8; i++){
			const __m128i out = _mm_packs_epi32(out32[i][0], out32[i][1]);
			_mm_storeu_si128((__m128i*)&pDst[i*nStride], out);
		}
	}

}
#else
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
#endif

#if (SUB_DCT_USE_ASM >= ASM_SSE4)
void xSubDCT16(
	Int16 *pDst,
	UInt8 *pSrc, UInt nStride,
	UInt8 *pRef, UInt nStrideRef,
	Int16 *piTmp0, Int16 *piTmp1
	)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning
	nStrideRef = 0;//avoid warning

	const __m128i c_zero = _mm_setzero_si128();
	__m128i in16[16][2];
	__m128i temp32[16][4];//16 rows

	for(Int row=0; row<16; row+=8){
		Int part = (row>>3);
		const __m128i src00 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+0)*nStride] );//8bit Src
		const __m128i src01 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+1)*nStride] );
		const __m128i src02 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+2)*nStride] );
		const __m128i src03 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+3)*nStride] );
		const __m128i src04 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+4)*nStride] );
		const __m128i src05 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+5)*nStride] );
		const __m128i src06 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+6)*nStride] );
		const __m128i src07 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+7)*nStride] );

		const __m128i ref00 = _mm_loadu_si128( (const __m128i*)&pRef[(row+0)*nStride] );//8bit Ref
		const __m128i ref01 = _mm_loadu_si128( (const __m128i*)&pRef[(row+1)*nStride] );
		const __m128i ref02 = _mm_loadu_si128( (const __m128i*)&pRef[(row+2)*nStride] );
		const __m128i ref03 = _mm_loadu_si128( (const __m128i*)&pRef[(row+3)*nStride] );
		const __m128i ref04 = _mm_loadu_si128( (const __m128i*)&pRef[(row+4)*nStride] );
		const __m128i ref05 = _mm_loadu_si128( (const __m128i*)&pRef[(row+5)*nStride] );
		const __m128i ref06 = _mm_loadu_si128( (const __m128i*)&pRef[(row+6)*nStride] );
		const __m128i ref07 = _mm_loadu_si128( (const __m128i*)&pRef[(row+7)*nStride] );

		const __m128i T0000a = _mm_unpacklo_epi8( src00,c_zero );//16bit Src
		const __m128i T0001a = _mm_unpacklo_epi8( src01,c_zero );
		const __m128i T0002a = _mm_unpacklo_epi8( src02,c_zero );
		const __m128i T0003a = _mm_unpacklo_epi8( src03,c_zero );
		const __m128i T0004a = _mm_unpacklo_epi8( src04,c_zero );
		const __m128i T0005a = _mm_unpacklo_epi8( src05,c_zero );
		const __m128i T0006a = _mm_unpacklo_epi8( src06,c_zero );
		const __m128i T0007a = _mm_unpacklo_epi8( src07,c_zero );
		const __m128i T0000b = _mm_unpackhi_epi8( src00,c_zero );
		const __m128i T0001b = _mm_unpackhi_epi8( src01,c_zero );
		const __m128i T0002b = _mm_unpackhi_epi8( src02,c_zero );
		const __m128i T0003b = _mm_unpackhi_epi8( src03,c_zero );
		const __m128i T0004b = _mm_unpackhi_epi8( src04,c_zero );
		const __m128i T0005b = _mm_unpackhi_epi8( src05,c_zero );
		const __m128i T0006b = _mm_unpackhi_epi8( src06,c_zero );
		const __m128i T0007b = _mm_unpackhi_epi8( src07,c_zero );

		const __m128i T0000c = _mm_unpacklo_epi8( ref00,c_zero );//16bit Ref
		const __m128i T0001c = _mm_unpacklo_epi8( ref01,c_zero );
		const __m128i T0002c = _mm_unpacklo_epi8( ref02,c_zero );
		const __m128i T0003c = _mm_unpacklo_epi8( ref03,c_zero );
		const __m128i T0004c = _mm_unpacklo_epi8( ref04,c_zero );
		const __m128i T0005c = _mm_unpacklo_epi8( ref05,c_zero );
		const __m128i T0006c = _mm_unpacklo_epi8( ref06,c_zero );
		const __m128i T0007c = _mm_unpacklo_epi8( ref07,c_zero );
		const __m128i T0000d = _mm_unpackhi_epi8( ref00,c_zero );
		const __m128i T0001d = _mm_unpackhi_epi8( ref01,c_zero );
		const __m128i T0002d = _mm_unpackhi_epi8( ref02,c_zero );
		const __m128i T0003d = _mm_unpackhi_epi8( ref03,c_zero );
		const __m128i T0004d = _mm_unpackhi_epi8( ref04,c_zero );
		const __m128i T0005d = _mm_unpackhi_epi8( ref05,c_zero );
		const __m128i T0006d = _mm_unpackhi_epi8( ref06,c_zero );
		const __m128i T0007d = _mm_unpackhi_epi8( ref07,c_zero );

		const __m128i T0100a = _mm_sub_epi16( T0000a,T0000c );//16bit residual
		const __m128i T0101a = _mm_sub_epi16( T0001a,T0001c );
		const __m128i T0102a = _mm_sub_epi16( T0002a,T0002c );
		const __m128i T0103a = _mm_sub_epi16( T0003a,T0003c );
		const __m128i T0104a = _mm_sub_epi16( T0004a,T0004c );
		const __m128i T0105a = _mm_sub_epi16( T0005a,T0005c );
		const __m128i T0106a = _mm_sub_epi16( T0006a,T0006c );
		const __m128i T0107a = _mm_sub_epi16( T0007a,T0007c );
		const __m128i T0100b = _mm_sub_epi16( T0000b,T0000d );
		const __m128i T0101b = _mm_sub_epi16( T0001b,T0001d );
		const __m128i T0102b = _mm_sub_epi16( T0002b,T0002d );
		const __m128i T0103b = _mm_sub_epi16( T0003b,T0003d );
		const __m128i T0104b = _mm_sub_epi16( T0004b,T0004d );
		const __m128i T0105b = _mm_sub_epi16( T0005b,T0005d );
		const __m128i T0106b = _mm_sub_epi16( T0006b,T0006d );
		const __m128i T0107b = _mm_sub_epi16( T0007b,T0007d );

		TRANSPOSE_8x8_16BIT(T0100a, T0101a, T0102a, T0103a, T0104a, T0105a, T0106a, T0107a, \
			in16[ 0][part],in16[ 1][part],in16[ 2][part],in16[ 3][part],in16[ 4][part],in16[ 5][part],in16[ 6][part],in16[ 7][part])
			TRANSPOSE_8x8_16BIT(T0100b, T0101b, T0102b, T0103b, T0104b, T0105b, T0106b, T0107b, \
			in16[ 8][part],in16[ 9][part],in16[10][part],in16[11][part],in16[12][part],in16[13][part],in16[14][part],in16[15][part])
	}

	//DCT1.
	const __m128i c16_p87_p90   = _mm_set1_epi32(0x0057005A);//row0 87high - 90low address
	const __m128i c16_p70_p80   = _mm_set1_epi32(0x00460050);
	const __m128i c16_p43_p57   = _mm_set1_epi32(0x002B0039);
	const __m128i c16_p09_p25   = _mm_set1_epi32(0x00090019);
	const __m128i c16_p57_p87   = _mm_set1_epi32(0x00390057);//row1
	const __m128i c16_n43_p09   = _mm_set1_epi32(0xFFD50009);
	const __m128i c16_n90_n80   = _mm_set1_epi32(0xFFA6FFB0);
	const __m128i c16_n25_n70   = _mm_set1_epi32(0xFFE7FFBA);
	const __m128i c16_p09_p80   = _mm_set1_epi32(0x00090050);//row2
	const __m128i c16_n87_n70   = _mm_set1_epi32(0xFFA9FFBA);
	const __m128i c16_p57_n25   = _mm_set1_epi32(0x0039FFE7);
	const __m128i c16_p43_p90   = _mm_set1_epi32(0x002B005A);
	const __m128i c16_n43_p70   = _mm_set1_epi32(0xFFD50046);//row3
	const __m128i c16_p09_n87   = _mm_set1_epi32(0x0009FFA9);
	const __m128i c16_p25_p90   = _mm_set1_epi32(0x0019005A);
	const __m128i c16_n57_n80   = _mm_set1_epi32(0xFFC7FFB0);
	const __m128i c16_n80_p57   = _mm_set1_epi32(0xFFB00039);//row4
	const __m128i c16_p90_n25   = _mm_set1_epi32(0x005AFFE7);
	const __m128i c16_n87_n09   = _mm_set1_epi32(0xFFA9FFF7);
	const __m128i c16_p70_p43   = _mm_set1_epi32(0x0046002B);
	const __m128i c16_n90_p43   = _mm_set1_epi32(0xFFA6002B);//row5
	const __m128i c16_p25_p57   = _mm_set1_epi32(0x00190039);
	const __m128i c16_p70_n87   = _mm_set1_epi32(0x0046FFA9);
	const __m128i c16_n80_p09   = _mm_set1_epi32(0xFFB00009);
	const __m128i c16_n70_p25   = _mm_set1_epi32(0xFFBA0019);//row6
	const __m128i c16_n80_p90   = _mm_set1_epi32(0xFFB0005A);
	const __m128i c16_p09_p43   = _mm_set1_epi32(0x0009002B);
	const __m128i c16_p87_n57   = _mm_set1_epi32(0x0057FFC7);
	const __m128i c16_n25_p09   = _mm_set1_epi32(0xFFE70009);//row7
	const __m128i c16_n57_p43   = _mm_set1_epi32(0xFFC7002B);
	const __m128i c16_n80_p70   = _mm_set1_epi32(0xFFB00046);
	const __m128i c16_n90_p87   = _mm_set1_epi32(0xFFA60057);

	const __m128i c16_p36_p83  = _mm_set1_epi32(0x00240053);
	const __m128i c16_n83_p36  = _mm_set1_epi32(0xFFAD0024);
	const __m128i c16_n64_p64  = _mm_set1_epi32(0xFFC00040);
	const __m128i c16_p64_p64  = _mm_set1_epi32(0x00400040);
	const __m128i c16_p75_p89  = _mm_set1_epi32(0x004B0059);
	const __m128i c16_p18_p50  = _mm_set1_epi32(0x00120032);
	const __m128i c16_n18_p75  = _mm_set1_epi32(0xFFEE004B);
	const __m128i c16_n50_n89  = _mm_set1_epi32(0xFFCEFFA7);
	const __m128i c16_n89_p50  = _mm_set1_epi32(0xFFA70032);
	const __m128i c16_p75_p18  = _mm_set1_epi32(0x004B0012);
	const __m128i c16_n50_p18  = _mm_set1_epi32(0xFFCE0012);
	const __m128i c16_n89_p75  = _mm_set1_epi32(0xFFA7004B);
	__m128i c32_rnd			 = _mm_set1_epi32(4);//rnd1

	for(Int row=0; row<16; row+=8){
		Int part   = (row>>3);
		Int offset = (part<<1);
		const __m128i T_00_00  = _mm_add_epi16(in16[ 0][part],in16[15][part]);
		const __m128i T_00_01  = _mm_add_epi16(in16[ 1][part],in16[14][part]);
		const __m128i T_00_02  = _mm_add_epi16(in16[ 2][part],in16[13][part]);
		const __m128i T_00_03  = _mm_add_epi16(in16[ 3][part],in16[12][part]);
		const __m128i T_00_04  = _mm_add_epi16(in16[ 4][part],in16[11][part]);
		const __m128i T_00_05  = _mm_add_epi16(in16[ 5][part],in16[10][part]);
		const __m128i T_00_06  = _mm_add_epi16(in16[ 6][part],in16[ 9][part]);
		const __m128i T_00_07  = _mm_add_epi16(in16[ 7][part],in16[ 8][part]);
		const __m128i T_00_08  = _mm_sub_epi16(in16[ 0][part],in16[15][part]);
		const __m128i T_00_09  = _mm_sub_epi16(in16[ 1][part],in16[14][part]);
		const __m128i T_00_10  = _mm_sub_epi16(in16[ 2][part],in16[13][part]);
		const __m128i T_00_11  = _mm_sub_epi16(in16[ 3][part],in16[12][part]);
		const __m128i T_00_12  = _mm_sub_epi16(in16[ 4][part],in16[11][part]);
		const __m128i T_00_13  = _mm_sub_epi16(in16[ 5][part],in16[10][part]);
		const __m128i T_00_14  = _mm_sub_epi16(in16[ 6][part],in16[ 9][part]);
		const __m128i T_00_15  = _mm_sub_epi16(in16[ 7][part],in16[ 8][part]);
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_00_08, T_00_09);
			const __m128i T00B  = _mm_unpackhi_epi16(T_00_08, T_00_09);
			const __m128i T01A  = _mm_unpacklo_epi16(T_00_10, T_00_11);
			const __m128i T01B  = _mm_unpackhi_epi16(T_00_10, T_00_11);
			const __m128i T02A  = _mm_unpacklo_epi16(T_00_12, T_00_13);
			const __m128i T02B  = _mm_unpackhi_epi16(T_00_12, T_00_13);
			const __m128i T03A  = _mm_unpacklo_epi16(T_00_14, T_00_15);
			const __m128i T03B  = _mm_unpackhi_epi16(T_00_14, T_00_15);
			__m128i T10A, T10B, T11A, T11B, T20A, T20B;
#define COMPUTE_O_ROW(c01, c23, c45, c67, rowA, rowB)\
	T10A = _mm_add_epi32(_mm_madd_epi16(c01, T00A), _mm_madd_epi16(c23, T01A));\
	T10B = _mm_add_epi32(_mm_madd_epi16(c01, T00B), _mm_madd_epi16(c23, T01B));\
	T11A = _mm_add_epi32(_mm_madd_epi16(c45, T02A), _mm_madd_epi16(c67, T03A));\
	T11B = _mm_add_epi32(_mm_madd_epi16(c45, T02B), _mm_madd_epi16(c67, T03B));\
	T20A = _mm_add_epi32(T10A, T11A);\
	T20B = _mm_add_epi32(T10B, T11B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T20A, c32_rnd), 3);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T20B, c32_rnd), 3);\

			COMPUTE_O_ROW(c16_p87_p90,c16_p70_p80,c16_p43_p57,c16_p09_p25, temp32[ 1][offset+0], temp32[ 1][offset+1])
				COMPUTE_O_ROW(c16_p57_p87,c16_n43_p09,c16_n90_n80,c16_n25_n70, temp32[ 3][offset+0], temp32[ 3][offset+1])
				COMPUTE_O_ROW(c16_p09_p80,c16_n87_n70,c16_p57_n25,c16_p43_p90, temp32[ 5][offset+0], temp32[ 5][offset+1])
				COMPUTE_O_ROW(c16_n43_p70,c16_p09_n87,c16_p25_p90,c16_n57_n80, temp32[ 7][offset+0], temp32[ 7][offset+1])
				COMPUTE_O_ROW(c16_n80_p57,c16_p90_n25,c16_n87_n09,c16_p70_p43, temp32[ 9][offset+0], temp32[ 9][offset+1])
				COMPUTE_O_ROW(c16_n90_p43,c16_p25_p57,c16_p70_n87,c16_n80_p09, temp32[11][offset+0], temp32[11][offset+1])
				COMPUTE_O_ROW(c16_n70_p25,c16_n80_p90,c16_p09_p43,c16_p87_n57, temp32[13][offset+0], temp32[13][offset+1])
				COMPUTE_O_ROW(c16_n25_p09,c16_n57_p43,c16_n80_p70,c16_n90_p87, temp32[15][offset+0], temp32[15][offset+1])
#undef COMPUTE_O_ROW
		}

		const __m128i T_01_00  = _mm_add_epi16(T_00_00,T_00_07);
		const __m128i T_01_01  = _mm_add_epi16(T_00_01,T_00_06);
		const __m128i T_01_02  = _mm_add_epi16(T_00_02,T_00_05);
		const __m128i T_01_03  = _mm_add_epi16(T_00_03,T_00_04);
		const __m128i T_01_04  = _mm_sub_epi16(T_00_00,T_00_07);
		const __m128i T_01_05  = _mm_sub_epi16(T_00_01,T_00_06);
		const __m128i T_01_06  = _mm_sub_epi16(T_00_02,T_00_05);
		const __m128i T_01_07  = _mm_sub_epi16(T_00_03,T_00_04);
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_01_04, T_01_05);
			const __m128i T00B  = _mm_unpackhi_epi16(T_01_04, T_01_05);
			const __m128i T01A  = _mm_unpacklo_epi16(T_01_06, T_01_07);
			const __m128i T01B  = _mm_unpackhi_epi16(T_01_06, T_01_07);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, c23, rowA, rowB)\
	T10A = _mm_add_epi32(_mm_madd_epi16(c01, T00A), _mm_madd_epi16(c23, T01A));\
	T10B = _mm_add_epi32(_mm_madd_epi16(c01, T00B), _mm_madd_epi16(c23, T01B));\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 3);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 3);\

			COMPUTE_O_ROW(c16_p75_p89, c16_p18_p50, temp32[ 2][offset+0], temp32[ 2][offset+1])
				COMPUTE_O_ROW(c16_n18_p75, c16_n50_n89, temp32[ 6][offset+0], temp32[ 6][offset+1])
				COMPUTE_O_ROW(c16_n89_p50, c16_p75_p18, temp32[10][offset+0], temp32[10][offset+1])
				COMPUTE_O_ROW(c16_n50_p18, c16_n89_p75, temp32[14][offset+0], temp32[14][offset+1])
#undef COMPUTE_O_ROW
		}

		const __m128i T_02_00  = _mm_add_epi16(T_01_00,T_01_03);
		const __m128i T_02_01  = _mm_add_epi16(T_01_01,T_01_02);
		const __m128i T_02_02  = _mm_sub_epi16(T_01_00,T_01_03);
		const __m128i T_02_03  = _mm_sub_epi16(T_01_01,T_01_02);
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_02_02, T_02_03);
			const __m128i T00B  = _mm_unpackhi_epi16(T_02_02, T_02_03);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, rowA, rowB)\
	T10A = _mm_madd_epi16(c01, T00A);\
	T10B = _mm_madd_epi16(c01, T00B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 3);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 3);\

			COMPUTE_O_ROW(c16_p36_p83, temp32[ 4][offset+0], temp32[ 4][offset+1])
				COMPUTE_O_ROW(c16_n83_p36, temp32[12][offset+0], temp32[12][offset+1])
#undef COMPUTE_O_ROW
		}

		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_02_00, T_02_01);
			const __m128i T00B  = _mm_unpackhi_epi16(T_02_00, T_02_01);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, rowA, rowB)\
	T10A = _mm_madd_epi16(c01, T00A);\
	T10B = _mm_madd_epi16(c01, T00B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 3);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 3);\

			COMPUTE_O_ROW(c16_p64_p64, temp32[0][offset+0], temp32[0][offset+1])
				COMPUTE_O_ROW(c16_n64_p64, temp32[8][offset+0], temp32[8][offset+1])
#undef COMPUTE_O_ROW
		}
	}//END OF DCT1

	//DCT2
	__m128i out32[16][4];
	{
		const __m128i c32_p90 = _mm_set1_epi32( 90);
		const __m128i c32_p87 = _mm_set1_epi32( 87);
		const __m128i c32_p80 = _mm_set1_epi32( 80);
		const __m128i c32_p70 = _mm_set1_epi32( 70);
		const __m128i c32_p57 = _mm_set1_epi32( 57);
		const __m128i c32_p43 = _mm_set1_epi32( 43);
		const __m128i c32_p25 = _mm_set1_epi32( 25);
		const __m128i c32_p09 = _mm_set1_epi32(  9);
		const __m128i c32_p89 = _mm_set1_epi32( 89);
		const __m128i c32_p75 = _mm_set1_epi32( 75);
		const __m128i c32_p50 = _mm_set1_epi32( 50);
		const __m128i c32_p18 = _mm_set1_epi32( 18);
		const __m128i c32_p83 = _mm_set1_epi32( 83);
		const __m128i c32_p36 = _mm_set1_epi32( 36);
		const __m128i c32_p64 = _mm_set1_epi32( 64);
		const __m128i c32_n90 = _mm_set1_epi32(-90);
		const __m128i c32_n87 = _mm_set1_epi32(-87);
		const __m128i c32_n80 = _mm_set1_epi32(-80);
		const __m128i c32_n70 = _mm_set1_epi32(-70);
		const __m128i c32_n57 = _mm_set1_epi32(-57);
		const __m128i c32_n43 = _mm_set1_epi32(-43);
		const __m128i c32_n25 = _mm_set1_epi32(-25);
		const __m128i c32_n09 = _mm_set1_epi32(- 9);
		const __m128i c32_n89 = _mm_set1_epi32(-89);
		const __m128i c32_n75 = _mm_set1_epi32(-75);
		const __m128i c32_n50 = _mm_set1_epi32(-50);
		const __m128i c32_n18 = _mm_set1_epi32(-18);
		const __m128i c32_n83 = _mm_set1_epi32(-83);
		const __m128i c32_n36 = _mm_set1_epi32(-36);
		const __m128i c32_n64 = _mm_set1_epi32(-64);
		c32_rnd = _mm_set1_epi32(512);//rnd2

		for (Int i=0; i<16; i+=4){//4 rows forms one part
			Int part = (i>>2);    

			TRANSPOSE_4x4_32BIT(temp32[i][0], temp32[i+1][0], temp32[i+2][0], temp32[i+3][0])
				TRANSPOSE_4x4_32BIT(temp32[i][1], temp32[i+1][1], temp32[i+2][1], temp32[i+3][1])
				TRANSPOSE_4x4_32BIT(temp32[i][2], temp32[i+1][2], temp32[i+2][2], temp32[i+3][2])
				TRANSPOSE_4x4_32BIT(temp32[i][3], temp32[i+1][3], temp32[i+2][3], temp32[i+3][3])

				const __m128i T_00_00 = _mm_add_epi32(temp32[i+0][0], temp32[i+3][3]);
			const __m128i T_00_01 = _mm_add_epi32(temp32[i+1][0], temp32[i+2][3]);
			const __m128i T_00_02 = _mm_add_epi32(temp32[i+2][0], temp32[i+1][3]);
			const __m128i T_00_03 = _mm_add_epi32(temp32[i+3][0], temp32[i+0][3]);
			const __m128i T_00_04 = _mm_add_epi32(temp32[i+0][1], temp32[i+3][2]);
			const __m128i T_00_05 = _mm_add_epi32(temp32[i+1][1], temp32[i+2][2]);
			const __m128i T_00_06 = _mm_add_epi32(temp32[i+2][1], temp32[i+1][2]);
			const __m128i T_00_07 = _mm_add_epi32(temp32[i+3][1], temp32[i+0][2]);
			const __m128i T_00_08 = _mm_sub_epi32(temp32[i+0][0], temp32[i+3][3]);
			const __m128i T_00_09 = _mm_sub_epi32(temp32[i+1][0], temp32[i+2][3]);
			const __m128i T_00_10 = _mm_sub_epi32(temp32[i+2][0], temp32[i+1][3]);
			const __m128i T_00_11 = _mm_sub_epi32(temp32[i+3][0], temp32[i+0][3]);
			const __m128i T_00_12 = _mm_sub_epi32(temp32[i+0][1], temp32[i+3][2]);
			const __m128i T_00_13 = _mm_sub_epi32(temp32[i+1][1], temp32[i+2][2]);
			const __m128i T_00_14 = _mm_sub_epi32(temp32[i+2][1], temp32[i+1][2]);
			const __m128i T_00_15 = _mm_sub_epi32(temp32[i+3][1], temp32[i+0][2]);
			{
				__m128i T00, T01, T02, T03, T10;
#define COMPUTE_ROW(c0, c1, c2, c3, c4, c5, c6, c7, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_00_08, c0), _mm_mullo_epi32(T_00_09, c1));\
	T01 = _mm_add_epi32(_mm_mullo_epi32(T_00_10, c2), _mm_mullo_epi32(T_00_11, c3));\
	T02 = _mm_add_epi32(_mm_mullo_epi32(T_00_12, c4), _mm_mullo_epi32(T_00_13, c5));\
	T03 = _mm_add_epi32(_mm_mullo_epi32(T_00_14, c6), _mm_mullo_epi32(T_00_15, c7));\
	T10 = _mm_add_epi32(_mm_add_epi32(T00, T01), _mm_add_epi32(T02, T03));\
	row = _mm_srai_epi32(_mm_add_epi32(T10, c32_rnd), 10);

				COMPUTE_ROW(c32_p90, c32_p87, c32_p80, c32_p70, c32_p57, c32_p43, c32_p25, c32_p09, out32[ 1][part])
					COMPUTE_ROW(c32_p87, c32_p57, c32_p09, c32_n43, c32_n80, c32_n90, c32_n70, c32_n25, out32[ 3][part])
					COMPUTE_ROW(c32_p80, c32_p09, c32_n70, c32_n87, c32_n25, c32_p57, c32_p90, c32_p43, out32[ 5][part])
					COMPUTE_ROW(c32_p70, c32_n43, c32_n87, c32_p09, c32_p90, c32_p25, c32_n80, c32_n57, out32[ 7][part])
					COMPUTE_ROW(c32_p57, c32_n80, c32_n25, c32_p90, c32_n09, c32_n87, c32_p43, c32_p70, out32[ 9][part])
					COMPUTE_ROW(c32_p43, c32_n90, c32_p57, c32_p25, c32_n87, c32_p70, c32_p09, c32_n80, out32[11][part])
					COMPUTE_ROW(c32_p25, c32_n70, c32_p90, c32_n80, c32_p43, c32_p09, c32_n57, c32_p87, out32[13][part])
					COMPUTE_ROW(c32_p09, c32_n25, c32_p43, c32_n57, c32_p70, c32_n80, c32_p87, c32_n90, out32[15][part])
#undef COMPUTE_ROW
			}

			const __m128i T_01_00 = _mm_add_epi32(T_00_00, T_00_07);
			const __m128i T_01_01 = _mm_add_epi32(T_00_01, T_00_06);
			const __m128i T_01_02 = _mm_add_epi32(T_00_02, T_00_05);
			const __m128i T_01_03 = _mm_add_epi32(T_00_03, T_00_04);
			const __m128i T_01_04 = _mm_sub_epi32(T_00_00, T_00_07);
			const __m128i T_01_05 = _mm_sub_epi32(T_00_01, T_00_06);
			const __m128i T_01_06 = _mm_sub_epi32(T_00_02, T_00_05);
			const __m128i T_01_07 = _mm_sub_epi32(T_00_03, T_00_04);
			{
				__m128i T00, T01, T10;
#define COMPUTE_ROW(c0, c1, c2, c3, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_01_04, c0), _mm_mullo_epi32(T_01_05, c1));\
	T01 = _mm_add_epi32(_mm_mullo_epi32(T_01_06, c2), _mm_mullo_epi32(T_01_07, c3));\
	T10 = _mm_add_epi32(T00, T01);\
	row = _mm_srai_epi32(_mm_add_epi32(T10, c32_rnd), 10);

				COMPUTE_ROW(c32_p89, c32_p75, c32_p50, c32_p18, out32[ 2][part])
					COMPUTE_ROW(c32_p75, c32_n18, c32_n89, c32_n50, out32[ 6][part])
					COMPUTE_ROW(c32_p50, c32_n89, c32_p18, c32_p75, out32[10][part])
					COMPUTE_ROW(c32_p18, c32_n50, c32_p75, c32_n89, out32[14][part])
#undef COMPUTE_ROW
			}

			const __m128i T_02_00 = _mm_add_epi32(T_01_00, T_01_03);
			const __m128i T_02_01 = _mm_add_epi32(T_01_01, T_01_02);
			const __m128i T_02_02 = _mm_sub_epi32(T_01_00, T_01_03);
			const __m128i T_02_03 = _mm_sub_epi32(T_01_01, T_01_02);
			{
				__m128i T00;
#define COMPUTE_ROW(c0, c1, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_02_02, c0), _mm_mullo_epi32(T_02_03, c1));\
	row = _mm_srai_epi32(_mm_add_epi32(T00, c32_rnd), 10);

				COMPUTE_ROW(c32_p83, c32_p36, out32[ 4][part])
					COMPUTE_ROW(c32_p36, c32_n83, out32[12][part])
#undef COMPUTE_ROW
			}

			{
				__m128i T00;
#define COMPUTE_ROW(c0, c1, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_02_00, c0), _mm_mullo_epi32(T_02_01, c1));\
	row = _mm_srai_epi32(_mm_add_epi32(T00, c32_rnd), 10);

				COMPUTE_ROW(c32_p64, c32_p64, out32[ 0][part])
					COMPUTE_ROW(c32_p64, c32_n64, out32[ 8][part])
#undef COMPUTE_ROW
			}
		}

		for (Int i=0; i<16; i++){
			const __m128i out0 = _mm_packs_epi32(out32[i][0], out32[i][1]);
			const __m128i out1 = _mm_packs_epi32(out32[i][2], out32[i][3]);
			_mm_storeu_si128((__m128i*)&pDst[i*nStride  ], out0);
			_mm_storeu_si128((__m128i*)&pDst[i*nStride+8], out1);
		}
	}
}
#else
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
#endif

#if (SUB_DCT_USE_ASM >= ASM_SSE4)
void xSubDCT32(
	Int16 *pDst,
	UInt8 *pSrc, UInt nStride,
	UInt8 *pRef, UInt nStrideRef,
	Int16 *piTmp0, Int16 *piTmp1
	)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning
	nStrideRef = 0;//avoid warning

	const __m128i c_zero = _mm_setzero_si128();
	__m128i in16[32][4];
	__m128i temp32[32][8];//16 rows

	for(Int row=0; row<32; row+=8){
		for (Int column=0; column<32; column+=16){
			Int part = (row>>3);
			Int offsetY = column;

			const __m128i src00 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+0)*nStride+column] );//8bit Src//x64 modify, load->loadu
			const __m128i src01 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+1)*nStride+column] );
			const __m128i src02 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+2)*nStride+column] );
			const __m128i src03 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+3)*nStride+column] );
			const __m128i src04 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+4)*nStride+column] );
			const __m128i src05 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+5)*nStride+column] );
			const __m128i src06 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+6)*nStride+column] );
			const __m128i src07 = _mm_loadu_si128( (const __m128i*)&pSrc[(row+7)*nStride+column] );

			const __m128i ref00 = _mm_loadu_si128( (const __m128i*)&pRef[(row+0)*nStride+column] );//8bit Ref
			const __m128i ref01 = _mm_loadu_si128( (const __m128i*)&pRef[(row+1)*nStride+column] );
			const __m128i ref02 = _mm_loadu_si128( (const __m128i*)&pRef[(row+2)*nStride+column] );
			const __m128i ref03 = _mm_loadu_si128( (const __m128i*)&pRef[(row+3)*nStride+column] );
			const __m128i ref04 = _mm_loadu_si128( (const __m128i*)&pRef[(row+4)*nStride+column] );
			const __m128i ref05 = _mm_loadu_si128( (const __m128i*)&pRef[(row+5)*nStride+column] );
			const __m128i ref06 = _mm_loadu_si128( (const __m128i*)&pRef[(row+6)*nStride+column] );
			const __m128i ref07 = _mm_loadu_si128( (const __m128i*)&pRef[(row+7)*nStride+column] );

			const __m128i T0000a = _mm_unpacklo_epi8( src00,c_zero );//16bit Src
			const __m128i T0001a = _mm_unpacklo_epi8( src01,c_zero );
			const __m128i T0002a = _mm_unpacklo_epi8( src02,c_zero );
			const __m128i T0003a = _mm_unpacklo_epi8( src03,c_zero );
			const __m128i T0004a = _mm_unpacklo_epi8( src04,c_zero );
			const __m128i T0005a = _mm_unpacklo_epi8( src05,c_zero );
			const __m128i T0006a = _mm_unpacklo_epi8( src06,c_zero );
			const __m128i T0007a = _mm_unpacklo_epi8( src07,c_zero );
			const __m128i T0000b = _mm_unpackhi_epi8( src00,c_zero );
			const __m128i T0001b = _mm_unpackhi_epi8( src01,c_zero );
			const __m128i T0002b = _mm_unpackhi_epi8( src02,c_zero );
			const __m128i T0003b = _mm_unpackhi_epi8( src03,c_zero );
			const __m128i T0004b = _mm_unpackhi_epi8( src04,c_zero );
			const __m128i T0005b = _mm_unpackhi_epi8( src05,c_zero );
			const __m128i T0006b = _mm_unpackhi_epi8( src06,c_zero );
			const __m128i T0007b = _mm_unpackhi_epi8( src07,c_zero );

			const __m128i T0000c = _mm_unpacklo_epi8( ref00,c_zero );//16bit Ref
			const __m128i T0001c = _mm_unpacklo_epi8( ref01,c_zero );
			const __m128i T0002c = _mm_unpacklo_epi8( ref02,c_zero );
			const __m128i T0003c = _mm_unpacklo_epi8( ref03,c_zero );
			const __m128i T0004c = _mm_unpacklo_epi8( ref04,c_zero );
			const __m128i T0005c = _mm_unpacklo_epi8( ref05,c_zero );
			const __m128i T0006c = _mm_unpacklo_epi8( ref06,c_zero );
			const __m128i T0007c = _mm_unpacklo_epi8( ref07,c_zero );
			const __m128i T0000d = _mm_unpackhi_epi8( ref00,c_zero );
			const __m128i T0001d = _mm_unpackhi_epi8( ref01,c_zero );
			const __m128i T0002d = _mm_unpackhi_epi8( ref02,c_zero );
			const __m128i T0003d = _mm_unpackhi_epi8( ref03,c_zero );
			const __m128i T0004d = _mm_unpackhi_epi8( ref04,c_zero );
			const __m128i T0005d = _mm_unpackhi_epi8( ref05,c_zero );
			const __m128i T0006d = _mm_unpackhi_epi8( ref06,c_zero );
			const __m128i T0007d = _mm_unpackhi_epi8( ref07,c_zero );

			const __m128i T0100a = _mm_sub_epi16( T0000a,T0000c );//16bit residual
			const __m128i T0101a = _mm_sub_epi16( T0001a,T0001c );
			const __m128i T0102a = _mm_sub_epi16( T0002a,T0002c );
			const __m128i T0103a = _mm_sub_epi16( T0003a,T0003c );
			const __m128i T0104a = _mm_sub_epi16( T0004a,T0004c );
			const __m128i T0105a = _mm_sub_epi16( T0005a,T0005c );
			const __m128i T0106a = _mm_sub_epi16( T0006a,T0006c );
			const __m128i T0107a = _mm_sub_epi16( T0007a,T0007c );
			const __m128i T0100b = _mm_sub_epi16( T0000b,T0000d );
			const __m128i T0101b = _mm_sub_epi16( T0001b,T0001d );
			const __m128i T0102b = _mm_sub_epi16( T0002b,T0002d );
			const __m128i T0103b = _mm_sub_epi16( T0003b,T0003d );
			const __m128i T0104b = _mm_sub_epi16( T0004b,T0004d );
			const __m128i T0105b = _mm_sub_epi16( T0005b,T0005d );
			const __m128i T0106b = _mm_sub_epi16( T0006b,T0006d );
			const __m128i T0107b = _mm_sub_epi16( T0007b,T0007d );

			TRANSPOSE_8x8_16BIT(T0100a, T0101a, T0102a, T0103a, T0104a, T0105a, T0106a, T0107a, \
				in16[offsetY+ 0][part],in16[offsetY+ 1][part],in16[offsetY+ 2][part],in16[offsetY+ 3][part],in16[offsetY+ 4][part],in16[offsetY+ 5][part],in16[offsetY+ 6][part],in16[offsetY+ 7][part])
				TRANSPOSE_8x8_16BIT(T0100b, T0101b, T0102b, T0103b, T0104b, T0105b, T0106b, T0107b, \
				in16[offsetY+ 8][part],in16[offsetY+ 9][part],in16[offsetY+10][part],in16[offsetY+11][part],in16[offsetY+12][part],in16[offsetY+13][part],in16[offsetY+14][part],in16[offsetY+15][part])
		}
	}

	//DCT1.
	const __m128i c16_p90_p90   = _mm_set1_epi32(0x005A005A);//column 0
	const __m128i c16_p85_p88   = _mm_set1_epi32(0x00550058);
	const __m128i c16_p78_p82   = _mm_set1_epi32(0x004E0052);
	const __m128i c16_p67_p73   = _mm_set1_epi32(0x00430049);
	const __m128i c16_p54_p61   = _mm_set1_epi32(0x0036003D);
	const __m128i c16_p38_p46   = _mm_set1_epi32(0x0026002E);
	const __m128i c16_p22_p31   = _mm_set1_epi32(0x0016001F);
	const __m128i c16_p04_p13   = _mm_set1_epi32(0x0004000D);
	const __m128i c16_p82_p90   = _mm_set1_epi32(0x0052005A);//column 1
	const __m128i c16_p46_p67   = _mm_set1_epi32(0x002E0043);
	const __m128i c16_n04_p22   = _mm_set1_epi32(0xFFFC0016);
	const __m128i c16_n54_n31   = _mm_set1_epi32(0xFFCAFFE1);
	const __m128i c16_n85_n73   = _mm_set1_epi32(0xFFABFFB7);
	const __m128i c16_n88_n90   = _mm_set1_epi32(0xFFA8FFA6);
	const __m128i c16_n61_n78   = _mm_set1_epi32(0xFFC3FFB2);
	const __m128i c16_n13_n38   = _mm_set1_epi32(0xFFF3FFDA);
	const __m128i c16_p67_p88   = _mm_set1_epi32(0x00430058);//column 2
	const __m128i c16_n13_p31   = _mm_set1_epi32(0xFFF3001F);
	const __m128i c16_n82_n54   = _mm_set1_epi32(0xFFAEFFCA);
	const __m128i c16_n78_n90   = _mm_set1_epi32(0xFFB2FFA6);
	const __m128i c16_n04_n46   = _mm_set1_epi32(0xFFFCFFD2);
	const __m128i c16_p73_p38   = _mm_set1_epi32(0x00490026);
	const __m128i c16_p85_p90   = _mm_set1_epi32(0x0055005A);
	const __m128i c16_p22_p61   = _mm_set1_epi32(0x0016003D);
	const __m128i c16_p46_p85   = _mm_set1_epi32(0x002E0055);//column 3
	const __m128i c16_n67_n13   = _mm_set1_epi32(0xFFBDFFF3);
	const __m128i c16_n73_n90   = _mm_set1_epi32(0xFFB7FFA6);
	const __m128i c16_p38_n22   = _mm_set1_epi32(0x0026FFEA);
	const __m128i c16_p88_p82   = _mm_set1_epi32(0x00580052);
	const __m128i c16_n04_p54   = _mm_set1_epi32(0xFFFC0036);
	const __m128i c16_n90_n61   = _mm_set1_epi32(0xFFA6FFC3);
	const __m128i c16_n31_n78   = _mm_set1_epi32(0xFFE1FFB2);
	const __m128i c16_p22_p82   = _mm_set1_epi32(0x00160052);//column 4
	const __m128i c16_n90_n54   = _mm_set1_epi32(0xFFA6FFCA);
	const __m128i c16_p13_n61   = _mm_set1_epi32(0x000DFFC3);
	const __m128i c16_p85_p78   = _mm_set1_epi32(0x0055004E);
	const __m128i c16_n46_p31   = _mm_set1_epi32(0xFFD2001F);
	const __m128i c16_n67_n90   = _mm_set1_epi32(0xFFBDFFA6);
	const __m128i c16_p73_p04   = _mm_set1_epi32(0x00490004);
	const __m128i c16_p38_p88   = _mm_set1_epi32(0x00260058);
	const __m128i c16_n04_p78   = _mm_set1_epi32(0xFFFC004E);//column 5
	const __m128i c16_n73_n82   = _mm_set1_epi32(0xFFB7FFAE);
	const __m128i c16_p85_p13   = _mm_set1_epi32(0x0055000D);
	const __m128i c16_n22_p67   = _mm_set1_epi32(0xFFEA0043);
	const __m128i c16_n61_n88   = _mm_set1_epi32(0xFFC3FFA8);
	const __m128i c16_p90_p31   = _mm_set1_epi32(0x005A001F);
	const __m128i c16_n38_p54   = _mm_set1_epi32(0xFFDA0036);
	const __m128i c16_n46_n90   = _mm_set1_epi32(0xFFD2FFA6);
	const __m128i c16_n31_p73   = _mm_set1_epi32(0xFFE10049);//column 6
	const __m128i c16_n22_n90   = _mm_set1_epi32(0xFFEAFFA6);
	const __m128i c16_p67_p78   = _mm_set1_epi32(0x0043004E);
	const __m128i c16_n90_n38   = _mm_set1_epi32(0xFFA6FFDA);
	const __m128i c16_p82_n13   = _mm_set1_epi32(0x0052FFF3);
	const __m128i c16_n46_p61   = _mm_set1_epi32(0xFFD2003D);
	const __m128i c16_n04_n88   = _mm_set1_epi32(0xFFFCFFA8);
	const __m128i c16_p54_p85   = _mm_set1_epi32(0x00360055);
	const __m128i c16_n54_p67   = _mm_set1_epi32(0xFFCA0043);//column 7
	const __m128i c16_p38_n78   = _mm_set1_epi32(0x0026FFB2);
	const __m128i c16_n22_p85   = _mm_set1_epi32(0xFFEA0055);
	const __m128i c16_p04_n90   = _mm_set1_epi32(0x0004FFA6);
	const __m128i c16_p13_p90   = _mm_set1_epi32(0x000D005A);
	const __m128i c16_n31_n88   = _mm_set1_epi32(0xFFE1FFA8);
	const __m128i c16_p46_p82   = _mm_set1_epi32(0x002E0052);
	const __m128i c16_n61_n73   = _mm_set1_epi32(0xFFC3FFB7);
	const __m128i c16_n73_p61   = _mm_set1_epi32(0xFFB7003D);//column 8
	const __m128i c16_p82_n46   = _mm_set1_epi32(0x0052FFD2);
	const __m128i c16_n88_p31   = _mm_set1_epi32(0xFFA8001F);
	const __m128i c16_p90_n13   = _mm_set1_epi32(0x005AFFF3);
	const __m128i c16_n90_n04   = _mm_set1_epi32(0xFFA6FFFC);
	const __m128i c16_p85_p22   = _mm_set1_epi32(0x00550016);
	const __m128i c16_n78_n38   = _mm_set1_epi32(0xFFB2FFDA);
	const __m128i c16_p67_p54   = _mm_set1_epi32(0x00430036);
	const __m128i c16_n85_p54   = _mm_set1_epi32(0xFFAB0036);//column 9
	const __m128i c16_p88_n04   = _mm_set1_epi32(0x0058FFFC);
	const __m128i c16_n61_n46   = _mm_set1_epi32(0xFFC3FFD2);
	const __m128i c16_p13_p82   = _mm_set1_epi32(0x000D0052);
	const __m128i c16_p38_n90   = _mm_set1_epi32(0x0026FFA6);
	const __m128i c16_n78_p67   = _mm_set1_epi32(0xFFB20043);
	const __m128i c16_p90_n22   = _mm_set1_epi32(0x005AFFEA);
	const __m128i c16_n73_n31   = _mm_set1_epi32(0xFFB7FFE1);
	const __m128i c16_n90_p46   = _mm_set1_epi32(0xFFA6002E);//column 10
	const __m128i c16_p54_p38   = _mm_set1_epi32(0x00360026);
	const __m128i c16_p31_n90   = _mm_set1_epi32(0x001FFFA6);
	const __m128i c16_n88_p61   = _mm_set1_epi32(0xFFA8003D);
	const __m128i c16_p67_p22   = _mm_set1_epi32(0x00430016);
	const __m128i c16_p13_n85   = _mm_set1_epi32(0x000DFFAB);
	const __m128i c16_n82_p73   = _mm_set1_epi32(0xFFAE0049);
	const __m128i c16_p78_p04   = _mm_set1_epi32(0x004E0004);
	const __m128i c16_n88_p38   = _mm_set1_epi32(0xFFA80026);//column 11
	const __m128i c16_n04_p73   = _mm_set1_epi32(0xFFFC0049);
	const __m128i c16_p90_n67   = _mm_set1_epi32(0x005AFFBD);
	const __m128i c16_n31_n46   = _mm_set1_epi32(0xFFE1FFD2);
	const __m128i c16_n78_p85   = _mm_set1_epi32(0xFFB20055);
	const __m128i c16_p61_p13   = _mm_set1_epi32(0x003D000D);
	const __m128i c16_p54_n90   = _mm_set1_epi32(0x0036FFA6);
	const __m128i c16_n82_p22   = _mm_set1_epi32(0xFFAE0016);
	const __m128i c16_n78_p31   = _mm_set1_epi32(0xFFB2001F);//column 12
	const __m128i c16_n61_p90   = _mm_set1_epi32(0xFFC3005A);
	const __m128i c16_p54_p04   = _mm_set1_epi32(0x00360004);
	const __m128i c16_p82_n88   = _mm_set1_epi32(0x0052FFA8);
	const __m128i c16_n22_n38   = _mm_set1_epi32(0xFFEAFFDA);
	const __m128i c16_n90_p73   = _mm_set1_epi32(0xFFA60049);
	const __m128i c16_n13_p67   = _mm_set1_epi32(0xFFF30043);
	const __m128i c16_p85_n46   = _mm_set1_epi32(0x0055FFD2);
	const __m128i c16_n61_p22   = _mm_set1_epi32(0xFFC30016);//column 13
	const __m128i c16_n90_p85   = _mm_set1_epi32(0xFFA60055);
	const __m128i c16_n38_p73   = _mm_set1_epi32(0xFFDA0049);
	const __m128i c16_p46_n04   = _mm_set1_epi32(0x002EFFFC);
	const __m128i c16_p90_n78   = _mm_set1_epi32(0x005AFFB2);
	const __m128i c16_p54_n82   = _mm_set1_epi32(0x0036FFAE);
	const __m128i c16_n31_n13   = _mm_set1_epi32(0xFFE1FFF3);
	const __m128i c16_n88_p67   = _mm_set1_epi32(0xFFA80043);
	const __m128i c16_n38_p13   = _mm_set1_epi32(0xFFDA000D);//column 14
	const __m128i c16_n78_p61   = _mm_set1_epi32(0xFFB2003D);
	const __m128i c16_n90_p88   = _mm_set1_epi32(0xFFA60058);
	const __m128i c16_n73_p85   = _mm_set1_epi32(0xFFB70055);
	const __m128i c16_n31_p54   = _mm_set1_epi32(0xFFE10036);
	const __m128i c16_p22_p04   = _mm_set1_epi32(0x00160004);
	const __m128i c16_p67_n46   = _mm_set1_epi32(0x0043FFD2);
	const __m128i c16_p90_n82   = _mm_set1_epi32(0x005AFFAE);
	const __m128i c16_n13_p04   = _mm_set1_epi32(0xFFF30004);//column 15
	const __m128i c16_n31_p22   = _mm_set1_epi32(0xFFE10016);
	const __m128i c16_n46_p38   = _mm_set1_epi32(0xFFD20026);
	const __m128i c16_n61_p54   = _mm_set1_epi32(0xFFC30036);
	const __m128i c16_n73_p67   = _mm_set1_epi32(0xFFB70043);
	const __m128i c16_n82_p78   = _mm_set1_epi32(0xFFAE004E);
	const __m128i c16_n88_p85   = _mm_set1_epi32(0xFFA80055);
	const __m128i c16_n90_p90   = _mm_set1_epi32(0xFFA6005A);

	const __m128i c16_p87_p90   = _mm_set1_epi32(0x0057005A);//row0 87high - 90low address
	const __m128i c16_p70_p80   = _mm_set1_epi32(0x00460050);
	const __m128i c16_p43_p57   = _mm_set1_epi32(0x002B0039);
	const __m128i c16_p09_p25   = _mm_set1_epi32(0x00090019);
	const __m128i c16_p57_p87   = _mm_set1_epi32(0x00390057);//row1
	const __m128i c16_n43_p09   = _mm_set1_epi32(0xFFD50009);
	const __m128i c16_n90_n80   = _mm_set1_epi32(0xFFA6FFB0);
	const __m128i c16_n25_n70   = _mm_set1_epi32(0xFFE7FFBA);
	const __m128i c16_p09_p80   = _mm_set1_epi32(0x00090050);//row2
	const __m128i c16_n87_n70   = _mm_set1_epi32(0xFFA9FFBA);
	const __m128i c16_p57_n25   = _mm_set1_epi32(0x0039FFE7);
	const __m128i c16_p43_p90   = _mm_set1_epi32(0x002B005A);
	const __m128i c16_n43_p70   = _mm_set1_epi32(0xFFD50046);//row3
	const __m128i c16_p09_n87   = _mm_set1_epi32(0x0009FFA9);
	const __m128i c16_p25_p90   = _mm_set1_epi32(0x0019005A);
	const __m128i c16_n57_n80   = _mm_set1_epi32(0xFFC7FFB0);
	const __m128i c16_n80_p57   = _mm_set1_epi32(0xFFB00039);//row4
	const __m128i c16_p90_n25   = _mm_set1_epi32(0x005AFFE7);
	const __m128i c16_n87_n09   = _mm_set1_epi32(0xFFA9FFF7);
	const __m128i c16_p70_p43   = _mm_set1_epi32(0x0046002B);
	const __m128i c16_n90_p43   = _mm_set1_epi32(0xFFA6002B);//row5
	const __m128i c16_p25_p57   = _mm_set1_epi32(0x00190039);
	const __m128i c16_p70_n87   = _mm_set1_epi32(0x0046FFA9);
	const __m128i c16_n80_p09   = _mm_set1_epi32(0xFFB00009);
	const __m128i c16_n70_p25   = _mm_set1_epi32(0xFFBA0019);//row6
	const __m128i c16_n80_p90   = _mm_set1_epi32(0xFFB0005A);
	const __m128i c16_p09_p43   = _mm_set1_epi32(0x0009002B);
	const __m128i c16_p87_n57   = _mm_set1_epi32(0x0057FFC7);
	const __m128i c16_n25_p09   = _mm_set1_epi32(0xFFE70009);//row7
	const __m128i c16_n57_p43   = _mm_set1_epi32(0xFFC7002B);
	const __m128i c16_n80_p70   = _mm_set1_epi32(0xFFB00046);
	const __m128i c16_n90_p87   = _mm_set1_epi32(0xFFA60057);

	const __m128i c16_p36_p83  = _mm_set1_epi32(0x00240053);
	const __m128i c16_n83_p36  = _mm_set1_epi32(0xFFAD0024);
	const __m128i c16_n64_p64  = _mm_set1_epi32(0xFFC00040);
	const __m128i c16_p64_p64  = _mm_set1_epi32(0x00400040);
	const __m128i c16_p75_p89  = _mm_set1_epi32(0x004B0059);
	const __m128i c16_p18_p50  = _mm_set1_epi32(0x00120032);
	const __m128i c16_n18_p75  = _mm_set1_epi32(0xFFEE004B);
	const __m128i c16_n50_n89  = _mm_set1_epi32(0xFFCEFFA7);
	const __m128i c16_n89_p50  = _mm_set1_epi32(0xFFA70032);
	const __m128i c16_p75_p18  = _mm_set1_epi32(0x004B0012);
	const __m128i c16_n50_p18  = _mm_set1_epi32(0xFFCE0012);
	const __m128i c16_n89_p75  = _mm_set1_epi32(0xFFA7004B);
	__m128i c32_rnd			 = _mm_set1_epi32(8);//rnd1

	for(Int row=0; row<32; row+=8){
		Int part   = (row>>3);
		Int offset = (part<<1);
		const __m128i T_00_00  = _mm_add_epi16(in16[ 0][part],in16[31][part]);
		const __m128i T_00_01  = _mm_add_epi16(in16[ 1][part],in16[30][part]);
		const __m128i T_00_02  = _mm_add_epi16(in16[ 2][part],in16[29][part]);
		const __m128i T_00_03  = _mm_add_epi16(in16[ 3][part],in16[28][part]);
		const __m128i T_00_04  = _mm_add_epi16(in16[ 4][part],in16[27][part]);
		const __m128i T_00_05  = _mm_add_epi16(in16[ 5][part],in16[26][part]);
		const __m128i T_00_06  = _mm_add_epi16(in16[ 6][part],in16[25][part]);
		const __m128i T_00_07  = _mm_add_epi16(in16[ 7][part],in16[24][part]);
		const __m128i T_00_08  = _mm_add_epi16(in16[ 8][part],in16[23][part]);
		const __m128i T_00_09  = _mm_add_epi16(in16[ 9][part],in16[22][part]);
		const __m128i T_00_10  = _mm_add_epi16(in16[10][part],in16[21][part]);
		const __m128i T_00_11  = _mm_add_epi16(in16[11][part],in16[20][part]);
		const __m128i T_00_12  = _mm_add_epi16(in16[12][part],in16[19][part]);
		const __m128i T_00_13  = _mm_add_epi16(in16[13][part],in16[18][part]);
		const __m128i T_00_14  = _mm_add_epi16(in16[14][part],in16[17][part]);
		const __m128i T_00_15  = _mm_add_epi16(in16[15][part],in16[16][part]);
		const __m128i T_00_16  = _mm_sub_epi16(in16[ 0][part],in16[31][part]);
		const __m128i T_00_17  = _mm_sub_epi16(in16[ 1][part],in16[30][part]);
		const __m128i T_00_18  = _mm_sub_epi16(in16[ 2][part],in16[29][part]);
		const __m128i T_00_19  = _mm_sub_epi16(in16[ 3][part],in16[28][part]);
		const __m128i T_00_20  = _mm_sub_epi16(in16[ 4][part],in16[27][part]);
		const __m128i T_00_21  = _mm_sub_epi16(in16[ 5][part],in16[26][part]);
		const __m128i T_00_22  = _mm_sub_epi16(in16[ 6][part],in16[25][part]);
		const __m128i T_00_23  = _mm_sub_epi16(in16[ 7][part],in16[24][part]);
		const __m128i T_00_24  = _mm_sub_epi16(in16[ 8][part],in16[23][part]);
		const __m128i T_00_25  = _mm_sub_epi16(in16[ 9][part],in16[22][part]);
		const __m128i T_00_26  = _mm_sub_epi16(in16[10][part],in16[21][part]);
		const __m128i T_00_27  = _mm_sub_epi16(in16[11][part],in16[20][part]);
		const __m128i T_00_28  = _mm_sub_epi16(in16[12][part],in16[19][part]);
		const __m128i T_00_29  = _mm_sub_epi16(in16[13][part],in16[18][part]);
		const __m128i T_00_30  = _mm_sub_epi16(in16[14][part],in16[17][part]);
		const __m128i T_00_31  = _mm_sub_epi16(in16[15][part],in16[16][part]);
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_00_16, T_00_17);
			const __m128i T00B  = _mm_unpackhi_epi16(T_00_16, T_00_17);
			const __m128i T01A  = _mm_unpacklo_epi16(T_00_18, T_00_19);
			const __m128i T01B  = _mm_unpackhi_epi16(T_00_18, T_00_19);
			const __m128i T02A  = _mm_unpacklo_epi16(T_00_20, T_00_21);
			const __m128i T02B  = _mm_unpackhi_epi16(T_00_20, T_00_21);
			const __m128i T03A  = _mm_unpacklo_epi16(T_00_22, T_00_23);
			const __m128i T03B  = _mm_unpackhi_epi16(T_00_22, T_00_23);
			const __m128i T04A  = _mm_unpacklo_epi16(T_00_24, T_00_25);
			const __m128i T04B  = _mm_unpackhi_epi16(T_00_24, T_00_25);
			const __m128i T05A  = _mm_unpacklo_epi16(T_00_26, T_00_27);
			const __m128i T05B  = _mm_unpackhi_epi16(T_00_26, T_00_27);
			const __m128i T06A  = _mm_unpacklo_epi16(T_00_28, T_00_29);
			const __m128i T06B  = _mm_unpackhi_epi16(T_00_28, T_00_29);
			const __m128i T07A  = _mm_unpacklo_epi16(T_00_30, T_00_31);
			const __m128i T07B  = _mm_unpackhi_epi16(T_00_30, T_00_31);
			__m128i T10A, T10B, T11A, T11B,  T12A, T12B, T13A, T13B, T20A, T20B;
#define COMPUTE_O_ROW(c01, c23, c45, c67, c89, cAB, cCD, cEF, rowA, rowB)\
	T10A = _mm_add_epi32(_mm_madd_epi16(c01, T00A), _mm_madd_epi16(c23, T01A));\
	T10B = _mm_add_epi32(_mm_madd_epi16(c01, T00B), _mm_madd_epi16(c23, T01B));\
	T11A = _mm_add_epi32(_mm_madd_epi16(c45, T02A), _mm_madd_epi16(c67, T03A));\
	T11B = _mm_add_epi32(_mm_madd_epi16(c45, T02B), _mm_madd_epi16(c67, T03B));\
	T12A = _mm_add_epi32(_mm_madd_epi16(c89, T04A), _mm_madd_epi16(cAB, T05A));\
	T12B = _mm_add_epi32(_mm_madd_epi16(c89, T04B), _mm_madd_epi16(cAB, T05B));\
	T13A = _mm_add_epi32(_mm_madd_epi16(cCD, T06A), _mm_madd_epi16(cEF, T07A));\
	T13B = _mm_add_epi32(_mm_madd_epi16(cCD, T06B), _mm_madd_epi16(cEF, T07B));\
	T20A = _mm_add_epi32(_mm_add_epi32(T10A, T11A), _mm_add_epi32(T12A, T13A));\
	T20B = _mm_add_epi32(_mm_add_epi32(T10B, T11B), _mm_add_epi32(T12B, T13B));\
	rowA = _mm_srai_epi32(_mm_add_epi32(T20A, c32_rnd), 4);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T20B, c32_rnd), 4);\

			COMPUTE_O_ROW(c16_p90_p90, c16_p85_p88, c16_p78_p82, c16_p67_p73, c16_p54_p61, c16_p38_p46, c16_p22_p31, c16_p04_p13, temp32[ 1][offset+0], temp32[ 1][offset+1])
				COMPUTE_O_ROW(c16_p82_p90, c16_p46_p67, c16_n04_p22, c16_n54_n31, c16_n85_n73, c16_n88_n90, c16_n61_n78, c16_n13_n38, temp32[ 3][offset+0], temp32[ 3][offset+1])
				COMPUTE_O_ROW(c16_p67_p88, c16_n13_p31, c16_n82_n54, c16_n78_n90, c16_n04_n46, c16_p73_p38, c16_p85_p90, c16_p22_p61, temp32[ 5][offset+0], temp32[ 5][offset+1])
				COMPUTE_O_ROW(c16_p46_p85, c16_n67_n13, c16_n73_n90, c16_p38_n22, c16_p88_p82, c16_n04_p54, c16_n90_n61, c16_n31_n78, temp32[ 7][offset+0], temp32[ 7][offset+1])
				COMPUTE_O_ROW(c16_p22_p82, c16_n90_n54, c16_p13_n61, c16_p85_p78, c16_n46_p31, c16_n67_n90, c16_p73_p04, c16_p38_p88, temp32[ 9][offset+0], temp32[ 9][offset+1])
				COMPUTE_O_ROW(c16_n04_p78, c16_n73_n82, c16_p85_p13, c16_n22_p67, c16_n61_n88, c16_p90_p31, c16_n38_p54, c16_n46_n90, temp32[11][offset+0], temp32[11][offset+1])
				COMPUTE_O_ROW(c16_n31_p73, c16_n22_n90, c16_p67_p78, c16_n90_n38, c16_p82_n13, c16_n46_p61, c16_n04_n88, c16_p54_p85, temp32[13][offset+0], temp32[13][offset+1])
				COMPUTE_O_ROW(c16_n54_p67, c16_p38_n78, c16_n22_p85, c16_p04_n90, c16_p13_p90, c16_n31_n88, c16_p46_p82, c16_n61_n73, temp32[15][offset+0], temp32[15][offset+1])

				COMPUTE_O_ROW(c16_n73_p61, c16_p82_n46, c16_n88_p31, c16_p90_n13, c16_n90_n04, c16_p85_p22, c16_n78_n38, c16_p67_p54, temp32[17][offset+0], temp32[17][offset+1])
				COMPUTE_O_ROW(c16_n85_p54, c16_p88_n04, c16_n61_n46, c16_p13_p82, c16_p38_n90, c16_n78_p67, c16_p90_n22, c16_n73_n31, temp32[19][offset+0], temp32[19][offset+1])
				COMPUTE_O_ROW(c16_n90_p46, c16_p54_p38, c16_p31_n90, c16_n88_p61, c16_p67_p22, c16_p13_n85, c16_n82_p73, c16_p78_p04, temp32[21][offset+0], temp32[21][offset+1])
				COMPUTE_O_ROW(c16_n88_p38, c16_n04_p73, c16_p90_n67, c16_n31_n46, c16_n78_p85, c16_p61_p13, c16_p54_n90, c16_n82_p22, temp32[23][offset+0], temp32[23][offset+1])
				COMPUTE_O_ROW(c16_n78_p31, c16_n61_p90, c16_p54_p04, c16_p82_n88, c16_n22_n38, c16_n90_p73, c16_n13_p67, c16_p85_n46, temp32[25][offset+0], temp32[25][offset+1])
				COMPUTE_O_ROW(c16_n61_p22, c16_n90_p85, c16_n38_p73, c16_p46_n04, c16_p90_n78, c16_p54_n82, c16_n31_n13, c16_n88_p67, temp32[27][offset+0], temp32[27][offset+1])
				COMPUTE_O_ROW(c16_n38_p13, c16_n78_p61, c16_n90_p88, c16_n73_p85, c16_n31_p54, c16_p22_p04, c16_p67_n46, c16_p90_n82, temp32[29][offset+0], temp32[29][offset+1])
				COMPUTE_O_ROW(c16_n13_p04, c16_n31_p22, c16_n46_p38, c16_n61_p54, c16_n73_p67, c16_n82_p78, c16_n88_p85, c16_n90_p90, temp32[31][offset+0], temp32[31][offset+1])

#undef COMPUTE_O_ROW
		}

		const __m128i T_01_00  = _mm_add_epi16(T_00_00,T_00_15);
		const __m128i T_01_01  = _mm_add_epi16(T_00_01,T_00_14);
		const __m128i T_01_02  = _mm_add_epi16(T_00_02,T_00_13);
		const __m128i T_01_03  = _mm_add_epi16(T_00_03,T_00_12);
		const __m128i T_01_04  = _mm_add_epi16(T_00_04,T_00_11);
		const __m128i T_01_05  = _mm_add_epi16(T_00_05,T_00_10);
		const __m128i T_01_06  = _mm_add_epi16(T_00_06,T_00_09);
		const __m128i T_01_07  = _mm_add_epi16(T_00_07,T_00_08);
		const __m128i T_01_08  = _mm_sub_epi16(T_00_00,T_00_15);
		const __m128i T_01_09  = _mm_sub_epi16(T_00_01,T_00_14);
		const __m128i T_01_10  = _mm_sub_epi16(T_00_02,T_00_13);
		const __m128i T_01_11  = _mm_sub_epi16(T_00_03,T_00_12);
		const __m128i T_01_12  = _mm_sub_epi16(T_00_04,T_00_11);
		const __m128i T_01_13  = _mm_sub_epi16(T_00_05,T_00_10);
		const __m128i T_01_14  = _mm_sub_epi16(T_00_06,T_00_09);
		const __m128i T_01_15  = _mm_sub_epi16(T_00_07,T_00_08);
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_01_08, T_01_09);
			const __m128i T00B  = _mm_unpackhi_epi16(T_01_08, T_01_09);
			const __m128i T01A  = _mm_unpacklo_epi16(T_01_10, T_01_11);
			const __m128i T01B  = _mm_unpackhi_epi16(T_01_10, T_01_11);
			const __m128i T02A  = _mm_unpacklo_epi16(T_01_12, T_01_13);
			const __m128i T02B  = _mm_unpackhi_epi16(T_01_12, T_01_13);
			const __m128i T03A  = _mm_unpacklo_epi16(T_01_14, T_01_15);
			const __m128i T03B  = _mm_unpackhi_epi16(T_01_14, T_01_15);
			__m128i T10A, T10B, T11A, T11B, T20A, T20B;
#define COMPUTE_O_ROW(c01, c23, c45, c67, rowA, rowB)\
	T10A = _mm_add_epi32(_mm_madd_epi16(c01, T00A), _mm_madd_epi16(c23, T01A));\
	T10B = _mm_add_epi32(_mm_madd_epi16(c01, T00B), _mm_madd_epi16(c23, T01B));\
	T11A = _mm_add_epi32(_mm_madd_epi16(c45, T02A), _mm_madd_epi16(c67, T03A));\
	T11B = _mm_add_epi32(_mm_madd_epi16(c45, T02B), _mm_madd_epi16(c67, T03B));\
	T20A = _mm_add_epi32(T10A, T11A);\
	T20B = _mm_add_epi32(T10B, T11B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T20A, c32_rnd), 4);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T20B, c32_rnd), 4);\

			COMPUTE_O_ROW(c16_p87_p90,c16_p70_p80,c16_p43_p57,c16_p09_p25, temp32[ 2][offset+0], temp32[ 2][offset+1])
				COMPUTE_O_ROW(c16_p57_p87,c16_n43_p09,c16_n90_n80,c16_n25_n70, temp32[ 6][offset+0], temp32[ 6][offset+1])
				COMPUTE_O_ROW(c16_p09_p80,c16_n87_n70,c16_p57_n25,c16_p43_p90, temp32[10][offset+0], temp32[10][offset+1])
				COMPUTE_O_ROW(c16_n43_p70,c16_p09_n87,c16_p25_p90,c16_n57_n80, temp32[14][offset+0], temp32[14][offset+1])
				COMPUTE_O_ROW(c16_n80_p57,c16_p90_n25,c16_n87_n09,c16_p70_p43, temp32[18][offset+0], temp32[18][offset+1])
				COMPUTE_O_ROW(c16_n90_p43,c16_p25_p57,c16_p70_n87,c16_n80_p09, temp32[22][offset+0], temp32[22][offset+1])
				COMPUTE_O_ROW(c16_n70_p25,c16_n80_p90,c16_p09_p43,c16_p87_n57, temp32[26][offset+0], temp32[26][offset+1])
				COMPUTE_O_ROW(c16_n25_p09,c16_n57_p43,c16_n80_p70,c16_n90_p87, temp32[30][offset+0], temp32[30][offset+1])
#undef COMPUTE_O_ROW
		}

		const __m128i T_02_00  = _mm_add_epi16(T_01_00,T_01_07);
		const __m128i T_02_01  = _mm_add_epi16(T_01_01,T_01_06);
		const __m128i T_02_02  = _mm_add_epi16(T_01_02,T_01_05);
		const __m128i T_02_03  = _mm_add_epi16(T_01_03,T_01_04);
		const __m128i T_02_04  = _mm_sub_epi16(T_01_00,T_01_07);
		const __m128i T_02_05  = _mm_sub_epi16(T_01_01,T_01_06);
		const __m128i T_02_06  = _mm_sub_epi16(T_01_02,T_01_05);
		const __m128i T_02_07  = _mm_sub_epi16(T_01_03,T_01_04);
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_02_04, T_02_05);
			const __m128i T00B  = _mm_unpackhi_epi16(T_02_04, T_02_05);
			const __m128i T01A  = _mm_unpacklo_epi16(T_02_06, T_02_07);
			const __m128i T01B  = _mm_unpackhi_epi16(T_02_06, T_02_07);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, c23, rowA, rowB)\
	T10A = _mm_add_epi32(_mm_madd_epi16(c01, T00A), _mm_madd_epi16(c23, T01A));\
	T10B = _mm_add_epi32(_mm_madd_epi16(c01, T00B), _mm_madd_epi16(c23, T01B));\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 4);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 4);\

			COMPUTE_O_ROW(c16_p75_p89, c16_p18_p50, temp32[ 4][offset+0], temp32[ 4][offset+1])
				COMPUTE_O_ROW(c16_n18_p75, c16_n50_n89, temp32[12][offset+0], temp32[12][offset+1])
				COMPUTE_O_ROW(c16_n89_p50, c16_p75_p18, temp32[20][offset+0], temp32[20][offset+1])
				COMPUTE_O_ROW(c16_n50_p18, c16_n89_p75, temp32[28][offset+0], temp32[28][offset+1])
#undef COMPUTE_O_ROW
		}

		const __m128i T_03_00  = _mm_add_epi16(T_02_00,T_02_03);
		const __m128i T_03_01  = _mm_add_epi16(T_02_01,T_02_02);
		const __m128i T_03_02  = _mm_sub_epi16(T_02_00,T_02_03);
		const __m128i T_03_03  = _mm_sub_epi16(T_02_01,T_02_02);
		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_03_02, T_03_03);
			const __m128i T00B  = _mm_unpackhi_epi16(T_03_02, T_03_03);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, rowA, rowB)\
	T10A = _mm_madd_epi16(c01, T00A);\
	T10B = _mm_madd_epi16(c01, T00B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 4);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 4);\

			COMPUTE_O_ROW(c16_p36_p83, temp32[ 8][offset+0], temp32[ 8][offset+1])
				COMPUTE_O_ROW(c16_n83_p36, temp32[24][offset+0], temp32[24][offset+1])
#undef COMPUTE_O_ROW
		}

		{
			const __m128i T00A  = _mm_unpacklo_epi16(T_03_00, T_03_01);
			const __m128i T00B  = _mm_unpackhi_epi16(T_03_00, T_03_01);
			__m128i T10A, T10B;
#define COMPUTE_O_ROW(c01, rowA, rowB)\
	T10A = _mm_madd_epi16(c01, T00A);\
	T10B = _mm_madd_epi16(c01, T00B);\
	rowA = _mm_srai_epi32(_mm_add_epi32(T10A, c32_rnd), 4);\
	rowB = _mm_srai_epi32(_mm_add_epi32(T10B, c32_rnd), 4);\

			COMPUTE_O_ROW(c16_p64_p64, temp32[ 0][offset+0], temp32[ 0][offset+1])
				COMPUTE_O_ROW(c16_n64_p64, temp32[16][offset+0], temp32[16][offset+1])
#undef COMPUTE_O_ROW
		}
	}//END OF DCT1

	//DCT2
	__m128i out32[32][8];
	{
		const __m128i c32_p88 = _mm_set1_epi32( 88);
		const __m128i c32_p85 = _mm_set1_epi32( 85);
		const __m128i c32_p82 = _mm_set1_epi32( 82);
		const __m128i c32_p78 = _mm_set1_epi32( 78);
		const __m128i c32_p73 = _mm_set1_epi32( 73);
		const __m128i c32_p67 = _mm_set1_epi32( 67);
		const __m128i c32_p61 = _mm_set1_epi32( 61);
		const __m128i c32_p54 = _mm_set1_epi32( 54);
		const __m128i c32_p46 = _mm_set1_epi32( 46);
		const __m128i c32_p38 = _mm_set1_epi32( 38);
		const __m128i c32_p31 = _mm_set1_epi32( 31);
		const __m128i c32_p22 = _mm_set1_epi32( 22);
		const __m128i c32_p13 = _mm_set1_epi32( 13);
		const __m128i c32_p04 = _mm_set1_epi32( 04);
		const __m128i c32_p90 = _mm_set1_epi32( 90);
		const __m128i c32_p87 = _mm_set1_epi32( 87);
		const __m128i c32_p80 = _mm_set1_epi32( 80);
		const __m128i c32_p70 = _mm_set1_epi32( 70);
		const __m128i c32_p57 = _mm_set1_epi32( 57);
		const __m128i c32_p43 = _mm_set1_epi32( 43);
		const __m128i c32_p25 = _mm_set1_epi32( 25);
		const __m128i c32_p09 = _mm_set1_epi32(  9);
		const __m128i c32_p89 = _mm_set1_epi32( 89);
		const __m128i c32_p75 = _mm_set1_epi32( 75);
		const __m128i c32_p50 = _mm_set1_epi32( 50);
		const __m128i c32_p18 = _mm_set1_epi32( 18);
		const __m128i c32_p83 = _mm_set1_epi32( 83);
		const __m128i c32_p36 = _mm_set1_epi32( 36);
		const __m128i c32_p64 = _mm_set1_epi32( 64);
		const __m128i c32_n88 = _mm_set1_epi32(-88);
		const __m128i c32_n85 = _mm_set1_epi32(-85);
		const __m128i c32_n82 = _mm_set1_epi32(-82);
		const __m128i c32_n78 = _mm_set1_epi32(-78);
		const __m128i c32_n73 = _mm_set1_epi32(-73);
		const __m128i c32_n67 = _mm_set1_epi32(-67);
		const __m128i c32_n61 = _mm_set1_epi32(-61);
		const __m128i c32_n54 = _mm_set1_epi32(-54);
		const __m128i c32_n46 = _mm_set1_epi32(-46);
		const __m128i c32_n38 = _mm_set1_epi32(-38);
		const __m128i c32_n31 = _mm_set1_epi32(-31);
		const __m128i c32_n22 = _mm_set1_epi32(-22);
		const __m128i c32_n13 = _mm_set1_epi32(-13);
		const __m128i c32_n04 = _mm_set1_epi32(-04);
		const __m128i c32_n90 = _mm_set1_epi32(-90);
		const __m128i c32_n87 = _mm_set1_epi32(-87);
		const __m128i c32_n80 = _mm_set1_epi32(-80);
		const __m128i c32_n70 = _mm_set1_epi32(-70);
		const __m128i c32_n57 = _mm_set1_epi32(-57);
		const __m128i c32_n43 = _mm_set1_epi32(-43);
		const __m128i c32_n25 = _mm_set1_epi32(-25);
		const __m128i c32_n09 = _mm_set1_epi32(- 9);
		const __m128i c32_n89 = _mm_set1_epi32(-89);
		const __m128i c32_n75 = _mm_set1_epi32(-75);
		const __m128i c32_n50 = _mm_set1_epi32(-50);
		const __m128i c32_n18 = _mm_set1_epi32(-18);
		const __m128i c32_n83 = _mm_set1_epi32(-83);
		const __m128i c32_n36 = _mm_set1_epi32(-36);
		const __m128i c32_n64 = _mm_set1_epi32(-64);
		c32_rnd = _mm_set1_epi32(1024);//rnd2

		for (Int i=0; i<32; i+=4){//4 rows forms one part
			Int part = (i>>2);    

			TRANSPOSE_4x4_32BIT(temp32[i][0], temp32[i+1][0], temp32[i+2][0], temp32[i+3][0])
				TRANSPOSE_4x4_32BIT(temp32[i][1], temp32[i+1][1], temp32[i+2][1], temp32[i+3][1])
				TRANSPOSE_4x4_32BIT(temp32[i][2], temp32[i+1][2], temp32[i+2][2], temp32[i+3][2])
				TRANSPOSE_4x4_32BIT(temp32[i][3], temp32[i+1][3], temp32[i+2][3], temp32[i+3][3])
				TRANSPOSE_4x4_32BIT(temp32[i][4], temp32[i+1][4], temp32[i+2][4], temp32[i+3][4])
				TRANSPOSE_4x4_32BIT(temp32[i][5], temp32[i+1][5], temp32[i+2][5], temp32[i+3][5])
				TRANSPOSE_4x4_32BIT(temp32[i][6], temp32[i+1][6], temp32[i+2][6], temp32[i+3][6])
				TRANSPOSE_4x4_32BIT(temp32[i][7], temp32[i+1][7], temp32[i+2][7], temp32[i+3][7])

				const __m128i T_00_00 = _mm_add_epi32(temp32[i+0][0], temp32[i+3][7]);
			const __m128i T_00_01 = _mm_add_epi32(temp32[i+1][0], temp32[i+2][7]);
			const __m128i T_00_02 = _mm_add_epi32(temp32[i+2][0], temp32[i+1][7]);
			const __m128i T_00_03 = _mm_add_epi32(temp32[i+3][0], temp32[i+0][7]);
			const __m128i T_00_04 = _mm_add_epi32(temp32[i+0][1], temp32[i+3][6]);
			const __m128i T_00_05 = _mm_add_epi32(temp32[i+1][1], temp32[i+2][6]);
			const __m128i T_00_06 = _mm_add_epi32(temp32[i+2][1], temp32[i+1][6]);
			const __m128i T_00_07 = _mm_add_epi32(temp32[i+3][1], temp32[i+0][6]);
			const __m128i T_00_08 = _mm_add_epi32(temp32[i+0][2], temp32[i+3][5]);
			const __m128i T_00_09 = _mm_add_epi32(temp32[i+1][2], temp32[i+2][5]);
			const __m128i T_00_10 = _mm_add_epi32(temp32[i+2][2], temp32[i+1][5]);
			const __m128i T_00_11 = _mm_add_epi32(temp32[i+3][2], temp32[i+0][5]);
			const __m128i T_00_12 = _mm_add_epi32(temp32[i+0][3], temp32[i+3][4]);
			const __m128i T_00_13 = _mm_add_epi32(temp32[i+1][3], temp32[i+2][4]);
			const __m128i T_00_14 = _mm_add_epi32(temp32[i+2][3], temp32[i+1][4]);
			const __m128i T_00_15 = _mm_add_epi32(temp32[i+3][3], temp32[i+0][4]);
			const __m128i T_00_16 = _mm_sub_epi32(temp32[i+0][0], temp32[i+3][7]);
			const __m128i T_00_17 = _mm_sub_epi32(temp32[i+1][0], temp32[i+2][7]);
			const __m128i T_00_18 = _mm_sub_epi32(temp32[i+2][0], temp32[i+1][7]);
			const __m128i T_00_19 = _mm_sub_epi32(temp32[i+3][0], temp32[i+0][7]);
			const __m128i T_00_20 = _mm_sub_epi32(temp32[i+0][1], temp32[i+3][6]);
			const __m128i T_00_21 = _mm_sub_epi32(temp32[i+1][1], temp32[i+2][6]);
			const __m128i T_00_22 = _mm_sub_epi32(temp32[i+2][1], temp32[i+1][6]);
			const __m128i T_00_23 = _mm_sub_epi32(temp32[i+3][1], temp32[i+0][6]);
			const __m128i T_00_24 = _mm_sub_epi32(temp32[i+0][2], temp32[i+3][5]);
			const __m128i T_00_25 = _mm_sub_epi32(temp32[i+1][2], temp32[i+2][5]);
			const __m128i T_00_26 = _mm_sub_epi32(temp32[i+2][2], temp32[i+1][5]);
			const __m128i T_00_27 = _mm_sub_epi32(temp32[i+3][2], temp32[i+0][5]);
			const __m128i T_00_28 = _mm_sub_epi32(temp32[i+0][3], temp32[i+3][4]);
			const __m128i T_00_29 = _mm_sub_epi32(temp32[i+1][3], temp32[i+2][4]);
			const __m128i T_00_30 = _mm_sub_epi32(temp32[i+2][3], temp32[i+1][4]);
			const __m128i T_00_31 = _mm_sub_epi32(temp32[i+3][3], temp32[i+0][4]);
			{
				__m128i T00, T01, T02, T03, T04, T05, T06, T07, T10, T11, T20;
#define COMPUTE_ROW(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, cA, cB, cC, cD, cE, cF, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_00_16, c0), _mm_mullo_epi32(T_00_17, c1));\
	T01 = _mm_add_epi32(_mm_mullo_epi32(T_00_18, c2), _mm_mullo_epi32(T_00_19, c3));\
	T02 = _mm_add_epi32(_mm_mullo_epi32(T_00_20, c4), _mm_mullo_epi32(T_00_21, c5));\
	T03 = _mm_add_epi32(_mm_mullo_epi32(T_00_22, c6), _mm_mullo_epi32(T_00_23, c7));\
	T04 = _mm_add_epi32(_mm_mullo_epi32(T_00_24, c8), _mm_mullo_epi32(T_00_25, c9));\
	T05 = _mm_add_epi32(_mm_mullo_epi32(T_00_26, cA), _mm_mullo_epi32(T_00_27, cB));\
	T06 = _mm_add_epi32(_mm_mullo_epi32(T_00_28, cC), _mm_mullo_epi32(T_00_29, cD));\
	T07 = _mm_add_epi32(_mm_mullo_epi32(T_00_30, cE), _mm_mullo_epi32(T_00_31, cF));\
	T10 = _mm_add_epi32(_mm_add_epi32(T00, T01), _mm_add_epi32(T02, T03));\
	T11 = _mm_add_epi32(_mm_add_epi32(T04, T05), _mm_add_epi32(T06, T07));\
	T20 = _mm_add_epi32(T10, T11);\
	row = _mm_srai_epi32(_mm_add_epi32(T20, c32_rnd), 11);

				COMPUTE_ROW(c32_p90,c32_p90,c32_p88,c32_p85,c32_p82,c32_p78,c32_p73,c32_p67,c32_p61,c32_p54,c32_p46,c32_p38,c32_p31,c32_p22,c32_p13,c32_p04,out32[ 1][part])
					COMPUTE_ROW(c32_p90,c32_p82,c32_p67,c32_p46,c32_p22,c32_n04,c32_n31,c32_n54,c32_n73,c32_n85,c32_n90,c32_n88,c32_n78,c32_n61,c32_n38,c32_n13,out32[ 3][part])
					COMPUTE_ROW(c32_p88,c32_p67,c32_p31,c32_n13,c32_n54,c32_n82,c32_n90,c32_n78,c32_n46,c32_n04,c32_p38,c32_p73,c32_p90,c32_p85,c32_p61,c32_p22,out32[ 5][part])
					COMPUTE_ROW(c32_p85,c32_p46,c32_n13,c32_n67,c32_n90,c32_n73,c32_n22,c32_p38,c32_p82,c32_p88,c32_p54,c32_n04,c32_n61,c32_n90,c32_n78,c32_n31,out32[ 7][part])
					COMPUTE_ROW(c32_p82,c32_p22,c32_n54,c32_n90,c32_n61,c32_p13,c32_p78,c32_p85,c32_p31,c32_n46,c32_n90,c32_n67,c32_p04,c32_p73,c32_p88,c32_p38,out32[ 9][part])
					COMPUTE_ROW(c32_p78,c32_n04,c32_n82,c32_n73,c32_p13,c32_p85,c32_p67,c32_n22,c32_n88,c32_n61,c32_p31,c32_p90,c32_p54,c32_n38,c32_n90,c32_n46,out32[11][part])
					COMPUTE_ROW(c32_p73,c32_n31,c32_n90,c32_n22,c32_p78,c32_p67,c32_n38,c32_n90,c32_n13,c32_p82,c32_p61,c32_n46,c32_n88,c32_n04,c32_p85,c32_p54,out32[13][part])
					COMPUTE_ROW(c32_p67,c32_n54,c32_n78,c32_p38,c32_p85,c32_n22,c32_n90,c32_p04,c32_p90,c32_p13,c32_n88,c32_n31,c32_p82,c32_p46,c32_n73,c32_n61,out32[15][part])
					COMPUTE_ROW(c32_p61,c32_n73,c32_n46,c32_p82,c32_p31,c32_n88,c32_n13,c32_p90,c32_n04,c32_n90,c32_p22,c32_p85,c32_n38,c32_n78,c32_p54,c32_p67,out32[17][part])
					COMPUTE_ROW(c32_p54,c32_n85,c32_n04,c32_p88,c32_n46,c32_n61,c32_p82,c32_p13,c32_n90,c32_p38,c32_p67,c32_n78,c32_n22,c32_p90,c32_n31,c32_n73,out32[19][part])
					COMPUTE_ROW(c32_p46,c32_n90,c32_p38,c32_p54,c32_n90,c32_p31,c32_p61,c32_n88,c32_p22,c32_p67,c32_n85,c32_p13,c32_p73,c32_n82,c32_p04,c32_p78,out32[21][part])
					COMPUTE_ROW(c32_p38,c32_n88,c32_p73,c32_n04,c32_n67,c32_p90,c32_n46,c32_n31,c32_p85,c32_n78,c32_p13,c32_p61,c32_n90,c32_p54,c32_p22,c32_n82,out32[23][part])
					COMPUTE_ROW(c32_p31,c32_n78,c32_p90,c32_n61,c32_p04,c32_p54,c32_n88,c32_p82,c32_n38,c32_n22,c32_p73,c32_n90,c32_p67,c32_n13,c32_n46,c32_p85,out32[25][part])
					COMPUTE_ROW(c32_p22,c32_n61,c32_p85,c32_n90,c32_p73,c32_n38,c32_n04,c32_p46,c32_n78,c32_p90,c32_n82,c32_p54,c32_n13,c32_n31,c32_p67,c32_n88,out32[27][part])
					COMPUTE_ROW(c32_p13,c32_n38,c32_p61,c32_n78,c32_p88,c32_n90,c32_p85,c32_n73,c32_p54,c32_n31,c32_p04,c32_p22,c32_n46,c32_p67,c32_n82,c32_p90,out32[29][part])
					COMPUTE_ROW(c32_p04,c32_n13,c32_p22,c32_n31,c32_p38,c32_n46,c32_p54,c32_n61,c32_p67,c32_n73,c32_p78,c32_n82,c32_p85,c32_n88,c32_p90,c32_n90,out32[31][part])
#undef COMPUTE_ROW
			}

			const __m128i T_01_00 = _mm_add_epi32(T_00_00, T_00_15);
			const __m128i T_01_01 = _mm_add_epi32(T_00_01, T_00_14);
			const __m128i T_01_02 = _mm_add_epi32(T_00_02, T_00_13);
			const __m128i T_01_03 = _mm_add_epi32(T_00_03, T_00_12);
			const __m128i T_01_04 = _mm_add_epi32(T_00_04, T_00_11);
			const __m128i T_01_05 = _mm_add_epi32(T_00_05, T_00_10);
			const __m128i T_01_06 = _mm_add_epi32(T_00_06, T_00_09);
			const __m128i T_01_07 = _mm_add_epi32(T_00_07, T_00_08);
			const __m128i T_01_08 = _mm_sub_epi32(T_00_00, T_00_15);
			const __m128i T_01_09 = _mm_sub_epi32(T_00_01, T_00_14);
			const __m128i T_01_10 = _mm_sub_epi32(T_00_02, T_00_13);
			const __m128i T_01_11 = _mm_sub_epi32(T_00_03, T_00_12);
			const __m128i T_01_12 = _mm_sub_epi32(T_00_04, T_00_11);
			const __m128i T_01_13 = _mm_sub_epi32(T_00_05, T_00_10);
			const __m128i T_01_14 = _mm_sub_epi32(T_00_06, T_00_09);
			const __m128i T_01_15 = _mm_sub_epi32(T_00_07, T_00_08);
			{
				__m128i T00, T01, T02, T03, T10;
#define COMPUTE_ROW(c0, c1, c2, c3, c4, c5, c6, c7, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_01_08, c0), _mm_mullo_epi32(T_01_09, c1));\
	T01 = _mm_add_epi32(_mm_mullo_epi32(T_01_10, c2), _mm_mullo_epi32(T_01_11, c3));\
	T02 = _mm_add_epi32(_mm_mullo_epi32(T_01_12, c4), _mm_mullo_epi32(T_01_13, c5));\
	T03 = _mm_add_epi32(_mm_mullo_epi32(T_01_14, c6), _mm_mullo_epi32(T_01_15, c7));\
	T10 = _mm_add_epi32(_mm_add_epi32(T00, T01), _mm_add_epi32(T02, T03));\
	row = _mm_srai_epi32(_mm_add_epi32(T10, c32_rnd), 11);

				COMPUTE_ROW(c32_p90, c32_p87, c32_p80, c32_p70, c32_p57, c32_p43, c32_p25, c32_p09, out32[ 2][part])
					COMPUTE_ROW(c32_p87, c32_p57, c32_p09, c32_n43, c32_n80, c32_n90, c32_n70, c32_n25, out32[ 6][part])
					COMPUTE_ROW(c32_p80, c32_p09, c32_n70, c32_n87, c32_n25, c32_p57, c32_p90, c32_p43, out32[10][part])
					COMPUTE_ROW(c32_p70, c32_n43, c32_n87, c32_p09, c32_p90, c32_p25, c32_n80, c32_n57, out32[14][part])
					COMPUTE_ROW(c32_p57, c32_n80, c32_n25, c32_p90, c32_n09, c32_n87, c32_p43, c32_p70, out32[18][part])
					COMPUTE_ROW(c32_p43, c32_n90, c32_p57, c32_p25, c32_n87, c32_p70, c32_p09, c32_n80, out32[22][part])
					COMPUTE_ROW(c32_p25, c32_n70, c32_p90, c32_n80, c32_p43, c32_p09, c32_n57, c32_p87, out32[26][part])
					COMPUTE_ROW(c32_p09, c32_n25, c32_p43, c32_n57, c32_p70, c32_n80, c32_p87, c32_n90, out32[30][part])
#undef COMPUTE_ROW
			}

			const __m128i T_02_00 = _mm_add_epi32(T_01_00, T_01_07);
			const __m128i T_02_01 = _mm_add_epi32(T_01_01, T_01_06);
			const __m128i T_02_02 = _mm_add_epi32(T_01_02, T_01_05);
			const __m128i T_02_03 = _mm_add_epi32(T_01_03, T_01_04);
			const __m128i T_02_04 = _mm_sub_epi32(T_01_00, T_01_07);
			const __m128i T_02_05 = _mm_sub_epi32(T_01_01, T_01_06);
			const __m128i T_02_06 = _mm_sub_epi32(T_01_02, T_01_05);
			const __m128i T_02_07 = _mm_sub_epi32(T_01_03, T_01_04);
			{
				__m128i T00, T01, T10;
#define COMPUTE_ROW(c0, c1, c2, c3, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_02_04, c0), _mm_mullo_epi32(T_02_05, c1));\
	T01 = _mm_add_epi32(_mm_mullo_epi32(T_02_06, c2), _mm_mullo_epi32(T_02_07, c3));\
	T10 = _mm_add_epi32(T00, T01);\
	row = _mm_srai_epi32(_mm_add_epi32(T10, c32_rnd), 11);

				COMPUTE_ROW(c32_p89, c32_p75, c32_p50, c32_p18, out32[ 4][part])
					COMPUTE_ROW(c32_p75, c32_n18, c32_n89, c32_n50, out32[12][part])
					COMPUTE_ROW(c32_p50, c32_n89, c32_p18, c32_p75, out32[20][part])
					COMPUTE_ROW(c32_p18, c32_n50, c32_p75, c32_n89, out32[28][part])
#undef COMPUTE_ROW
			}

			const __m128i T_03_00 = _mm_add_epi32(T_02_00, T_02_03);
			const __m128i T_03_01 = _mm_add_epi32(T_02_01, T_02_02);
			const __m128i T_03_02 = _mm_sub_epi32(T_02_00, T_02_03);
			const __m128i T_03_03 = _mm_sub_epi32(T_02_01, T_02_02);
			{
				__m128i T00;
#define COMPUTE_ROW(c0, c1, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_03_02, c0), _mm_mullo_epi32(T_03_03, c1));\
	row = _mm_srai_epi32(_mm_add_epi32(T00, c32_rnd), 11);

				COMPUTE_ROW(c32_p83, c32_p36, out32[ 8][part])
					COMPUTE_ROW(c32_p36, c32_n83, out32[24][part])
#undef COMPUTE_ROW
			}

			{
				__m128i T00;
#define COMPUTE_ROW(c0, c1, row)\
	T00 = _mm_add_epi32(_mm_mullo_epi32(T_03_00, c0), _mm_mullo_epi32(T_03_01, c1));\
	row = _mm_srai_epi32(_mm_add_epi32(T00, c32_rnd), 11);

				COMPUTE_ROW(c32_p64, c32_p64, out32[ 0][part])
					COMPUTE_ROW(c32_p64, c32_n64, out32[16][part])
#undef COMPUTE_ROW
			}
		}

		for (Int i=0; i<32; i++){
			const __m128i out0 = _mm_packs_epi32(out32[i][0], out32[i][1]);
			const __m128i out1 = _mm_packs_epi32(out32[i][2], out32[i][3]);
			const __m128i out2 = _mm_packs_epi32(out32[i][4], out32[i][5]);
			const __m128i out3 = _mm_packs_epi32(out32[i][6], out32[i][7]);
			_mm_storeu_si128((__m128i*)&pDst[i*nStride   ], out0);
			_mm_storeu_si128((__m128i*)&pDst[i*nStride+ 8], out1);
			_mm_storeu_si128((__m128i*)&pDst[i*nStride+16], out2);
			_mm_storeu_si128((__m128i*)&pDst[i*nStride+24], out3);
		}
	}
}
#else
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
#endif

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

#if (IDCT_ADD_USE_ASM >= ASM_SSE4)
void xIDstAdd4(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning
	
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

#if (IDCT_ADD_USE_ASM >= ASM_SSE4)
void xIDctAdd4(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning

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

#if (IDCT_ADD_USE_ASM >= ASM_SSE4)
void xIDctAdd8(
    UInt8 *pDst, UInt nStrideDst,
    Int16 *pSrc, UInt nStride,
    UInt8 *pRef,
    Int16 *piTmp0, Int16 *piTmp1
)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning
 
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

#if (IDCT_ADD_USE_ASM >= ASM_SSE4)
void xIDctAdd16(
	UInt8 *pDst, UInt nStrideDst,
	Int16 *pSrc, UInt nStride,
	UInt8 *pRef,
	Int16 *piTmp0, Int16 *piTmp1
	)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning

	const __m128i zero        = _mm_setzero_si128();

	const __m128i c16_p87_p90   = _mm_set1_epi32(0x0057005A);//row0 87high - 90low address
	const __m128i c16_p70_p80   = _mm_set1_epi32(0x00460050);
	const __m128i c16_p43_p57   = _mm_set1_epi32(0x002B0039);
	const __m128i c16_p09_p25   = _mm_set1_epi32(0x00090019);
	const __m128i c16_p57_p87   = _mm_set1_epi32(0x00390057);//row1
	const __m128i c16_n43_p09   = _mm_set1_epi32(0xFFD50009);
	const __m128i c16_n90_n80   = _mm_set1_epi32(0xFFA6FFB0);
	const __m128i c16_n25_n70   = _mm_set1_epi32(0xFFE7FFBA);
	const __m128i c16_p09_p80   = _mm_set1_epi32(0x00090050);//row2
	const __m128i c16_n87_n70   = _mm_set1_epi32(0xFFA9FFBA);
	const __m128i c16_p57_n25   = _mm_set1_epi32(0x0039FFE7);
	const __m128i c16_p43_p90   = _mm_set1_epi32(0x002B005A);
	const __m128i c16_n43_p70   = _mm_set1_epi32(0xFFD50046);//row3
	const __m128i c16_p09_n87   = _mm_set1_epi32(0x0009FFA9);
	const __m128i c16_p25_p90   = _mm_set1_epi32(0x0019005A);
	const __m128i c16_n57_n80   = _mm_set1_epi32(0xFFC7FFB0);
	const __m128i c16_n80_p57   = _mm_set1_epi32(0xFFB00039);//row4
	const __m128i c16_p90_n25   = _mm_set1_epi32(0x005AFFE7);
	const __m128i c16_n87_n09   = _mm_set1_epi32(0xFFA9FFF7);
	const __m128i c16_p70_p43   = _mm_set1_epi32(0x0046002B);
	const __m128i c16_n90_p43   = _mm_set1_epi32(0xFFA6002B);//row5
	const __m128i c16_p25_p57   = _mm_set1_epi32(0x00190039);
	const __m128i c16_p70_n87   = _mm_set1_epi32(0x0046FFA9);
	const __m128i c16_n80_p09   = _mm_set1_epi32(0xFFB00009);
	const __m128i c16_n70_p25   = _mm_set1_epi32(0xFFBA0019);//row6
	const __m128i c16_n80_p90   = _mm_set1_epi32(0xFFB0005A);
	const __m128i c16_p09_p43   = _mm_set1_epi32(0x0009002B);
	const __m128i c16_p87_n57   = _mm_set1_epi32(0x0057FFC7);
	const __m128i c16_n25_p09   = _mm_set1_epi32(0xFFE70009);//row7
	const __m128i c16_n57_p43   = _mm_set1_epi32(0xFFC7002B);
	const __m128i c16_n80_p70   = _mm_set1_epi32(0xFFB00046);
	const __m128i c16_n90_p87   = _mm_set1_epi32(0xFFA60057);
	
	const __m128i c16_p75_p89   = _mm_set1_epi32(0x004B0059);
	const __m128i c16_p18_p50   = _mm_set1_epi32(0x00120032);
	const __m128i c16_n18_p75   = _mm_set1_epi32(0xFFEE004B);
	const __m128i c16_n50_n89   = _mm_set1_epi32(0xFFCEFFA7);
	const __m128i c16_n89_p50   = _mm_set1_epi32(0xFFA70032);
	const __m128i c16_p75_p18   = _mm_set1_epi32(0x004B0012);
	const __m128i c16_n50_p18   = _mm_set1_epi32(0xFFCE0012);
	const __m128i c16_n89_p75   = _mm_set1_epi32(0xFFA7004B);

	const __m128i c16_p36_p83   = _mm_set1_epi32(0x00240053);
	const __m128i c16_n83_p36   = _mm_set1_epi32(0xFFAD0024);

	const __m128i c16_n64_p64   = _mm_set1_epi32(0xFFC00040);
	const __m128i c16_p64_p64   = _mm_set1_epi32(0x00400040);
	__m128i c32_rnd      = _mm_set1_epi32(64);

	Int nShift = SHIFT_INV_1ST;

	// DCT1
	__m128i in00[2], in01[2], in02[2], in03[2], in04[2], in05[2], in06[2], in07[2];
	__m128i in08[2], in09[2], in10[2], in11[2], in12[2], in13[2], in14[2], in15[2];
	__m128i res00[2], res01[2], res02[2], res03[2], res04[2], res05[2], res06[2], res07[2];
	__m128i res08[2], res09[2], res10[2], res11[2], res12[2], res13[2], res14[2], res15[2];

	for (Int i=0; i<2; i++){
		Int offset = (i<<3);

		in00[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 0*nStride+offset]); // [07 06 05 04 03 02 01 00]
		in01[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 1*nStride+offset]); // [17 16 15 14 13 12 11 10]
		in02[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 2*nStride+offset]); // [27 26 25 24 23 22 21 20]
		in03[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 3*nStride+offset]); // [37 36 35 34 33 32 31 30]
		in04[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 4*nStride+offset]); // [47 46 45 44 43 42 41 40]
		in05[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 5*nStride+offset]); // [57 56 55 54 53 52 51 50]
		in06[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 6*nStride+offset]); // [67 66 65 64 63 62 61 60]
		in07[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 7*nStride+offset]); // [77 76 75 74 73 72 71 70]
		in08[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 8*nStride+offset]); // [x7~x0]row08
		in09[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 9*nStride+offset]); // [x7~x0]row09
		in10[i]  = _mm_loadu_si128((const __m128i*)&pSrc[10*nStride+offset]); // [x7~x0]row10
		in11[i]  = _mm_loadu_si128((const __m128i*)&pSrc[11*nStride+offset]); // [x7~x0]row11
		in12[i]  = _mm_loadu_si128((const __m128i*)&pSrc[12*nStride+offset]); // [x7~x0]row12
		in13[i]  = _mm_loadu_si128((const __m128i*)&pSrc[13*nStride+offset]); // [x7~x0]row13
		in14[i]  = _mm_loadu_si128((const __m128i*)&pSrc[14*nStride+offset]); // [x7~x0]row14
		in15[i]  = _mm_loadu_si128((const __m128i*)&pSrc[15*nStride+offset]); // [x7~x0]row15
	}
	

	for(Int pass=0; pass<2; ++pass){
		if (pass==1){
			c32_rnd = _mm_set1_epi32(2048);//rnd2
			nShift  = SHIFT_INV_2ND;  
		}

		for(Int part=0; part<2; part++){
			const __m128i T_00_00A = _mm_unpacklo_epi16(in01[part], in03[part]);             // [33 13 32 12 31 11 30 10]
			const __m128i T_00_00B = _mm_unpackhi_epi16(in01[part], in03[part]);             // [37 17 36 16 35 15 34 14]
			const __m128i T_00_01A = _mm_unpacklo_epi16(in05[part], in07[part]);             // [ ]
			const __m128i T_00_01B = _mm_unpackhi_epi16(in05[part], in07[part]);             // [ ]
			const __m128i T_00_02A = _mm_unpacklo_epi16(in09[part], in11[part]);             // [ ]
			const __m128i T_00_02B = _mm_unpackhi_epi16(in09[part], in11[part]);             // [ ]
			const __m128i T_00_03A = _mm_unpacklo_epi16(in13[part], in15[part]);             // [ ]
			const __m128i T_00_03B = _mm_unpackhi_epi16(in13[part], in15[part]);             // [ ]
			const __m128i T_00_04A = _mm_unpacklo_epi16(in02[part], in06[part]);             // [ ]
			const __m128i T_00_04B = _mm_unpackhi_epi16(in02[part], in06[part]);             // [ ]
			const __m128i T_00_05A = _mm_unpacklo_epi16(in10[part], in14[part]);             // [ ]
			const __m128i T_00_05B = _mm_unpackhi_epi16(in10[part], in14[part]);             // [ ]
			const __m128i T_00_06A = _mm_unpacklo_epi16(in04[part], in12[part]);             // [ ]row
			const __m128i T_00_06B = _mm_unpackhi_epi16(in04[part], in12[part]);             // [ ]
			const __m128i T_00_07A = _mm_unpacklo_epi16(in00[part], in08[part]);             // [83 03 82 02 81 01 81 00] row08 row00
			const __m128i T_00_07B = _mm_unpackhi_epi16(in00[part], in08[part]);             // [87 07 86 06 85 05 84 04]

			__m128i O0A, O1A, O2A, O3A, O4A, O5A, O6A, O7A;
			__m128i O0B, O1B, O2B, O3B, O4B, O5B, O6B, O7B;
			{
				__m128i T00, T01;
	#define COMPUTE_ROW(row0103, row0507, row0911, row1315, c0103, c0507, c0911, c1315, row)\
				T00 = _mm_add_epi32(_mm_madd_epi16(row0103, c0103), _mm_madd_epi16(row0507, c0507));\
				T01 = _mm_add_epi32(_mm_madd_epi16(row0911, c0911), _mm_madd_epi16(row1315, c1315));\
				row = _mm_add_epi32(T00, T01);
			
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_p87_p90,c16_p70_p80,c16_p43_p57,c16_p09_p25, O0A)
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_p57_p87,c16_n43_p09,c16_n90_n80,c16_n25_n70, O1A)
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_p09_p80,c16_n87_n70,c16_p57_n25,c16_p43_p90, O2A)
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_n43_p70,c16_p09_n87,c16_p25_p90,c16_n57_n80, O3A)
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_n80_p57,c16_p90_n25,c16_n87_n09,c16_p70_p43, O4A)
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_n90_p43,c16_p25_p57,c16_p70_n87,c16_n80_p09, O5A)
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_n70_p25,c16_n80_p90,c16_p09_p43,c16_p87_n57, O6A)
				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, c16_n25_p09,c16_n57_p43,c16_n80_p70,c16_n90_p87, O7A)

				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_p87_p90,c16_p70_p80,c16_p43_p57,c16_p09_p25, O0B)
				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_p57_p87,c16_n43_p09,c16_n90_n80,c16_n25_n70, O1B)
				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_p09_p80,c16_n87_n70,c16_p57_n25,c16_p43_p90, O2B)
				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_n43_p70,c16_p09_n87,c16_p25_p90,c16_n57_n80, O3B)
				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_n80_p57,c16_p90_n25,c16_n87_n09,c16_p70_p43, O4B)
				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_n90_p43,c16_p25_p57,c16_p70_n87,c16_n80_p09, O5B)
				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_n70_p25,c16_n80_p90,c16_p09_p43,c16_p87_n57, O6B)
				COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, c16_n25_p09,c16_n57_p43,c16_n80_p70,c16_n90_p87, O7B)
	#undef COMPUTE_ROW
			}

			__m128i EO0A, EO1A, EO2A, EO3A;
			__m128i EO0B, EO1B, EO2B, EO3B;
			EO0A = _mm_add_epi32(_mm_madd_epi16(T_00_04A, c16_p75_p89 ), _mm_madd_epi16(T_00_05A, c16_p18_p50 )); // EO0
			EO0B = _mm_add_epi32(_mm_madd_epi16(T_00_04B, c16_p75_p89 ), _mm_madd_epi16(T_00_05B, c16_p18_p50 ));
			EO1A = _mm_add_epi32(_mm_madd_epi16(T_00_04A, c16_n18_p75 ), _mm_madd_epi16(T_00_05A, c16_n50_n89 )); // EO1
			EO1B = _mm_add_epi32(_mm_madd_epi16(T_00_04B, c16_n18_p75 ), _mm_madd_epi16(T_00_05B, c16_n50_n89 ));
			EO2A = _mm_add_epi32(_mm_madd_epi16(T_00_04A, c16_n89_p50 ), _mm_madd_epi16(T_00_05A, c16_p75_p18 )); // EO2
			EO2B = _mm_add_epi32(_mm_madd_epi16(T_00_04B, c16_n89_p50 ), _mm_madd_epi16(T_00_05B, c16_p75_p18 ));
			EO3A = _mm_add_epi32(_mm_madd_epi16(T_00_04A, c16_n50_p18 ), _mm_madd_epi16(T_00_05A, c16_n89_p75 )); // EO3
			EO3B = _mm_add_epi32(_mm_madd_epi16(T_00_04B, c16_n50_p18 ), _mm_madd_epi16(T_00_05B, c16_n89_p75 ));

			__m128i EEO0A, EEO1A;
			__m128i EEO0B, EEO1B;
			EEO0A = _mm_madd_epi16(T_00_06A, c16_p36_p83 );
			EEO0B = _mm_madd_epi16(T_00_06B, c16_p36_p83 );
			EEO1A = _mm_madd_epi16(T_00_06A, c16_n83_p36 );
			EEO1B = _mm_madd_epi16(T_00_06B, c16_n83_p36 );

			__m128i EEE0A, EEE1A;
			__m128i EEE0B, EEE1B;
			EEE0A = _mm_madd_epi16(T_00_07A, c16_p64_p64 );
			EEE0B = _mm_madd_epi16(T_00_07B, c16_p64_p64 );
			EEE1A = _mm_madd_epi16(T_00_07A, c16_n64_p64 );
			EEE1B = _mm_madd_epi16(T_00_07B, c16_n64_p64 );

			const __m128i EE0A = _mm_add_epi32(EEE0A, EEO0A);                // EE0 = EEE0 + EEO0
			const __m128i EE0B = _mm_add_epi32(EEE0B, EEO0B);
			const __m128i EE1A = _mm_add_epi32(EEE1A, EEO1A);                // EE1 = EEE1 + EEO1
			const __m128i EE1B = _mm_add_epi32(EEE1B, EEO1B);
			const __m128i EE3A = _mm_sub_epi32(EEE0A, EEO0A);                // EE2 = EEE0 - EEO0
			const __m128i EE3B = _mm_sub_epi32(EEE0B, EEO0B);
			const __m128i EE2A = _mm_sub_epi32(EEE1A, EEO1A);                // EE3 = EEE1 - EEO1
			const __m128i EE2B = _mm_sub_epi32(EEE1B, EEO1B);

			const __m128i E0A = _mm_add_epi32(EE0A, EO0A);                // E0 = EE0 + EO0
			const __m128i E0B = _mm_add_epi32(EE0B, EO0B);
			const __m128i E1A = _mm_add_epi32(EE1A, EO1A);                // E1 = EE1 + EO1
			const __m128i E1B = _mm_add_epi32(EE1B, EO1B);
			const __m128i E2A = _mm_add_epi32(EE2A, EO2A);                // E2 = EE2 + EO2
			const __m128i E2B = _mm_add_epi32(EE2B, EO2B);
			const __m128i E3A = _mm_add_epi32(EE3A, EO3A);                // E3 = EE3 + EO3
			const __m128i E3B = _mm_add_epi32(EE3B, EO3B);
			const __m128i E7A = _mm_sub_epi32(EE0A, EO0A);                // E0 = EE0 - EO0
			const __m128i E7B = _mm_sub_epi32(EE0B, EO0B);
			const __m128i E6A = _mm_sub_epi32(EE1A, EO1A);                // E1 = EE1 - EO1
			const __m128i E6B = _mm_sub_epi32(EE1B, EO1B);
			const __m128i E5A = _mm_sub_epi32(EE2A, EO2A);                // E2 = EE2 - EO2
			const __m128i E5B = _mm_sub_epi32(EE2B, EO2B);
			const __m128i E4A = _mm_sub_epi32(EE3A, EO3A);                // E3 = EE3 - EO3
			const __m128i E4B = _mm_sub_epi32(EE3B, EO3B);

			const __m128i T10A = _mm_add_epi32(E0A, c32_rnd);               // E0 + rnd
			const __m128i T10B = _mm_add_epi32(E0B, c32_rnd);
			const __m128i T11A = _mm_add_epi32(E1A, c32_rnd);               // E1 + rnd
			const __m128i T11B = _mm_add_epi32(E1B, c32_rnd);
			const __m128i T12A = _mm_add_epi32(E2A, c32_rnd);               // E2 + rnd
			const __m128i T12B = _mm_add_epi32(E2B, c32_rnd);
			const __m128i T13A = _mm_add_epi32(E3A, c32_rnd);               // E3 + rnd
			const __m128i T13B = _mm_add_epi32(E3B, c32_rnd);
			const __m128i T14A = _mm_add_epi32(E4A, c32_rnd);               // E4 + rnd
			const __m128i T14B = _mm_add_epi32(E4B, c32_rnd);
			const __m128i T15A = _mm_add_epi32(E5A, c32_rnd);               // E5 + rnd
			const __m128i T15B = _mm_add_epi32(E5B, c32_rnd);
			const __m128i T16A = _mm_add_epi32(E6A, c32_rnd);               // E6 + rnd
			const __m128i T16B = _mm_add_epi32(E6B, c32_rnd);
			const __m128i T17A = _mm_add_epi32(E7A, c32_rnd);               // E7 + rnd
			const __m128i T17B = _mm_add_epi32(E7B, c32_rnd);
	
			const __m128i T20A = _mm_add_epi32(T10A, O0A);                // E0 + O0 + rnd
			const __m128i T20B = _mm_add_epi32(T10B, O0B);
			const __m128i T21A = _mm_add_epi32(T11A, O1A);                // E1 + O1 + rnd
			const __m128i T21B = _mm_add_epi32(T11B, O1B);
			const __m128i T22A = _mm_add_epi32(T12A, O2A);                // E2 + O2 + rnd
			const __m128i T22B = _mm_add_epi32(T12B, O2B);
			const __m128i T23A = _mm_add_epi32(T13A, O3A);                // E3 + O3 + rnd
			const __m128i T23B = _mm_add_epi32(T13B, O3B);
			const __m128i T24A = _mm_add_epi32(T14A, O4A);                // E4 
			const __m128i T24B = _mm_add_epi32(T14B, O4B);
			const __m128i T25A = _mm_add_epi32(T15A, O5A);                // E5 
			const __m128i T25B = _mm_add_epi32(T15B, O5B);
			const __m128i T26A = _mm_add_epi32(T16A, O6A);                // E6
			const __m128i T26B = _mm_add_epi32(T16B, O6B);
			const __m128i T27A = _mm_add_epi32(T17A, O7A);                // E7 
			const __m128i T27B = _mm_add_epi32(T17B, O7B);
			const __m128i T2FA = _mm_sub_epi32(T10A, O0A);                // E0 - O0 + rnd
			const __m128i T2FB = _mm_sub_epi32(T10B, O0B);
			const __m128i T2EA = _mm_sub_epi32(T11A, O1A);                // E1 - O1 + rnd
			const __m128i T2EB = _mm_sub_epi32(T11B, O1B);
			const __m128i T2DA = _mm_sub_epi32(T12A, O2A);                // E2 - O2 + rnd
			const __m128i T2DB = _mm_sub_epi32(T12B, O2B);
			const __m128i T2CA = _mm_sub_epi32(T13A, O3A);                // E3 - O3 + rnd
			const __m128i T2CB = _mm_sub_epi32(T13B, O3B);
			const __m128i T2BA = _mm_sub_epi32(T14A, O4A);                // E4
			const __m128i T2BB = _mm_sub_epi32(T14B, O4B);
			const __m128i T2AA = _mm_sub_epi32(T15A, O5A);                // E5
			const __m128i T2AB = _mm_sub_epi32(T15B, O5B);
			const __m128i T29A = _mm_sub_epi32(T16A, O6A);                // E6
			const __m128i T29B = _mm_sub_epi32(T16B, O6B);
			const __m128i T28A = _mm_sub_epi32(T17A, O7A);                // E7
			const __m128i T28B = _mm_sub_epi32(T17B, O7B);
		
			const __m128i T30A = _mm_srai_epi32(T20A, nShift);                   // [30 20 10 00]
			const __m128i T30B = _mm_srai_epi32(T20B, nShift);                   // [70 60 50 40]
			const __m128i T31A = _mm_srai_epi32(T21A, nShift);                   // [31 21 11 01]
			const __m128i T31B = _mm_srai_epi32(T21B, nShift);                   // [71 61 51 41]
			const __m128i T32A = _mm_srai_epi32(T22A, nShift);                   // [32 22 12 02]
			const __m128i T32B = _mm_srai_epi32(T22B, nShift);                   // [72 62 52 42]
			const __m128i T33A = _mm_srai_epi32(T23A, nShift);                   // [33 23 13 03]
			const __m128i T33B = _mm_srai_epi32(T23B, nShift);                   // [73 63 53 43]
			const __m128i T34A = _mm_srai_epi32(T24A, nShift);                   // [33 24 14 04]
			const __m128i T34B = _mm_srai_epi32(T24B, nShift);                   // [74 64 54 44]
			const __m128i T35A = _mm_srai_epi32(T25A, nShift);                   // [35 25 15 05]
			const __m128i T35B = _mm_srai_epi32(T25B, nShift);                   // [75 65 55 45]
			const __m128i T36A = _mm_srai_epi32(T26A, nShift);                   // [36 26 16 06]
			const __m128i T36B = _mm_srai_epi32(T26B, nShift);                   // [76 66 56 46]
			const __m128i T37A = _mm_srai_epi32(T27A, nShift);                   // [37 27 17 07]
			const __m128i T37B = _mm_srai_epi32(T27B, nShift);                   // [77 67 57 47]

			const __m128i T38A = _mm_srai_epi32(T28A, nShift);                   // [30 20 10 00] x8
			const __m128i T38B = _mm_srai_epi32(T28B, nShift);                   // [70 60 50 40]
			const __m128i T39A = _mm_srai_epi32(T29A, nShift);                   // [31 21 11 01] x9 
			const __m128i T39B = _mm_srai_epi32(T29B, nShift);                   // [71 61 51 41]
			const __m128i T3AA = _mm_srai_epi32(T2AA, nShift);                   // [32 22 12 02] xA
			const __m128i T3AB = _mm_srai_epi32(T2AB, nShift);                   // [72 62 52 42]
			const __m128i T3BA = _mm_srai_epi32(T2BA, nShift);                   // [33 23 13 03] xB
			const __m128i T3BB = _mm_srai_epi32(T2BB, nShift);                   // [73 63 53 43]
			const __m128i T3CA = _mm_srai_epi32(T2CA, nShift);                   // [33 24 14 04] xC
			const __m128i T3CB = _mm_srai_epi32(T2CB, nShift);                   // [74 64 54 44]
			const __m128i T3DA = _mm_srai_epi32(T2DA, nShift);                   // [35 25 15 05] xD
			const __m128i T3DB = _mm_srai_epi32(T2DB, nShift);                   // [75 65 55 45]
			const __m128i T3EA = _mm_srai_epi32(T2EA, nShift);                   // [36 26 16 06] xE
			const __m128i T3EB = _mm_srai_epi32(T2EB, nShift);                   // [76 66 56 46]
			const __m128i T3FA = _mm_srai_epi32(T2FA, nShift);                   // [37 27 17 07] xF
			const __m128i T3FB = _mm_srai_epi32(T2FB, nShift);                   // [77 67 57 47]

			res00[part]  = _mm_packs_epi32(T30A, T30B);              // [70 60 50 40 30 20 10 00]
			res01[part]  = _mm_packs_epi32(T31A, T31B);              // [71 61 51 41 31 21 11 01]
			res02[part]  = _mm_packs_epi32(T32A, T32B);              // [72 62 52 42 32 22 12 02]
			res03[part]  = _mm_packs_epi32(T33A, T33B);              // [73 63 53 43 33 23 13 03]
			res04[part]  = _mm_packs_epi32(T34A, T34B);              // [74 64 54 44 34 24 14 04]
			res05[part]  = _mm_packs_epi32(T35A, T35B);              // [75 65 55 45 35 25 15 05]
			res06[part]  = _mm_packs_epi32(T36A, T36B);              // [76 66 56 46 36 26 16 06]
			res07[part]  = _mm_packs_epi32(T37A, T37B);              // [77 67 57 47 37 27 17 07]

			res08[part]  = _mm_packs_epi32(T38A, T38B);              // [A0 ... 80]
			res09[part]  = _mm_packs_epi32(T39A, T39B);              // [A1 ... 81]
			res10[part]  = _mm_packs_epi32(T3AA, T3AB);              // [A2 ... 82]
			res11[part]  = _mm_packs_epi32(T3BA, T3BB);              // [A3 ... 83]
			res12[part]  = _mm_packs_epi32(T3CA, T3CB);              // [A4 ... 84]
			res13[part]  = _mm_packs_epi32(T3DA, T3DB);              // [A5 ... 85]
			res14[part]  = _mm_packs_epi32(T3EA, T3EB);              // [A6 ... 86]
			res15[part]  = _mm_packs_epi32(T3FA, T3FB);              // [A7 ... 87] 
		
		/*	Int offset = part*8;
			_mm_storeu_si128( (__m128i *)&piTmp0[0*nStrideDst+offset], T40);
			_mm_storeu_si128( (__m128i *)&piTmp0[1*nStrideDst+offset], T41);
			_mm_storeu_si128( (__m128i *)&piTmp0[2*nStrideDst+offset], T42);
			_mm_storeu_si128( (__m128i *)&piTmp0[3*nStrideDst+offset], T43);
			_mm_storeu_si128( (__m128i *)&piTmp0[4*nStrideDst+offset], T44);
			_mm_storeu_si128( (__m128i *)&piTmp0[5*nStrideDst+offset], T45);
			_mm_storeu_si128( (__m128i *)&piTmp0[6*nStrideDst+offset], T46);
			_mm_storeu_si128( (__m128i *)&piTmp0[7*nStrideDst+offset], T47);
			_mm_storeu_si128( (__m128i *)&piTmp0[8*nStrideDst+offset], T48);
			_mm_storeu_si128( (__m128i *)&piTmp0[9*nStrideDst+offset], T49);
			_mm_storeu_si128( (__m128i *)&piTmp0[10*nStrideDst+offset], T4A);
			_mm_storeu_si128( (__m128i *)&piTmp0[11*nStrideDst+offset], T4B);
			_mm_storeu_si128( (__m128i *)&piTmp0[12*nStrideDst+offset], T4C);
			_mm_storeu_si128( (__m128i *)&piTmp0[13*nStrideDst+offset], T4D);
			_mm_storeu_si128( (__m128i *)&piTmp0[14*nStrideDst+offset], T4E);
			_mm_storeu_si128( (__m128i *)&piTmp0[15*nStrideDst+offset], T4F);*/
		}
		//transpose matrix 8x8 16bit. 
		{
			__m128i tr0_0, tr0_1, tr0_2, tr0_3, tr0_4, tr0_5, tr0_6, tr0_7; 
			__m128i tr1_0, tr1_1, tr1_2, tr1_3, tr1_4, tr1_5, tr1_6, tr1_7; 
#define TRANSPOSE_8x8_16BIT(I0, I1, I2, I3, I4, I5, I6, I7, O0, O1, O2, O3, O4, O5, O6, O7)\
			tr0_0 = _mm_unpacklo_epi16(I0, I1);\
			tr0_1 = _mm_unpacklo_epi16(I2, I3);\
			tr0_2 = _mm_unpackhi_epi16(I0, I1);\
			tr0_3 = _mm_unpackhi_epi16(I2, I3);\
			tr0_4 = _mm_unpacklo_epi16(I4, I5);\
			tr0_5 = _mm_unpacklo_epi16(I6, I7);\
			tr0_6 = _mm_unpackhi_epi16(I4, I5);\
			tr0_7 = _mm_unpackhi_epi16(I6, I7);\
			tr1_0 = _mm_unpacklo_epi32(tr0_0, tr0_1);\
			tr1_1 = _mm_unpacklo_epi32(tr0_2, tr0_3);\
			tr1_2 = _mm_unpackhi_epi32(tr0_0, tr0_1);\
			tr1_3 = _mm_unpackhi_epi32(tr0_2, tr0_3);\
			tr1_4 = _mm_unpacklo_epi32(tr0_4, tr0_5);\
			tr1_5 = _mm_unpacklo_epi32(tr0_6, tr0_7);\
			tr1_6 = _mm_unpackhi_epi32(tr0_4, tr0_5);\
			tr1_7 = _mm_unpackhi_epi32(tr0_6, tr0_7);\
			O0 = _mm_unpacklo_epi64(tr1_0, tr1_4);\
			O1 = _mm_unpackhi_epi64(tr1_0, tr1_4);\
			O2 = _mm_unpacklo_epi64(tr1_2, tr1_6);\
			O3 = _mm_unpackhi_epi64(tr1_2, tr1_6);\
			O4 = _mm_unpacklo_epi64(tr1_1, tr1_5);\
			O5 = _mm_unpackhi_epi64(tr1_1, tr1_5);\
			O6 = _mm_unpacklo_epi64(tr1_3, tr1_7);\
			O7 = _mm_unpackhi_epi64(tr1_3, tr1_7);\

			TRANSPOSE_8x8_16BIT(res00[0],res01[0],res02[0],res03[0],res04[0],res05[0],res06[0],res07[0],in00[0],in01[0],in02[0],in03[0],in04[0],in05[0],in06[0],in07[0])
			TRANSPOSE_8x8_16BIT(res08[0],res09[0],res10[0],res11[0],res12[0],res13[0],res14[0],res15[0],in00[1],in01[1],in02[1],in03[1],in04[1],in05[1],in06[1],in07[1])
			TRANSPOSE_8x8_16BIT(res00[1],res01[1],res02[1],res03[1],res04[1],res05[1],res06[1],res07[1],in08[0],in09[0],in10[0],in11[0],in12[0],in13[0],in14[0],in15[0])
			TRANSPOSE_8x8_16BIT(res08[1],res09[1],res10[1],res11[1],res12[1],res13[1],res14[1],res15[1],in08[1],in09[1],in10[1],in11[1],in12[1],in13[1],in14[1],in15[1])

#undef TRANSPOSE_8x8_16BIT
		}
	}

	// Add
	const __m128i T180 = _mm_loadu_si128((const __m128i*)&pRef[0*nStride]);
	const __m128i T181 = _mm_loadu_si128((const __m128i*)&pRef[1*nStride]);
	const __m128i T182 = _mm_loadu_si128((const __m128i*)&pRef[2*nStride]);
	const __m128i T183 = _mm_loadu_si128((const __m128i*)&pRef[3*nStride]);
	const __m128i T184 = _mm_loadu_si128((const __m128i*)&pRef[4*nStride]);
	const __m128i T185 = _mm_loadu_si128((const __m128i*)&pRef[5*nStride]);
	const __m128i T186 = _mm_loadu_si128((const __m128i*)&pRef[6*nStride]);
	const __m128i T187 = _mm_loadu_si128((const __m128i*)&pRef[7*nStride]);
	const __m128i T188 = _mm_loadu_si128((const __m128i*)&pRef[8*nStride]);
	const __m128i T189 = _mm_loadu_si128((const __m128i*)&pRef[9*nStride]);
	const __m128i T18A = _mm_loadu_si128((const __m128i*)&pRef[10*nStride]);
	const __m128i T18B = _mm_loadu_si128((const __m128i*)&pRef[11*nStride]);
	const __m128i T18C = _mm_loadu_si128((const __m128i*)&pRef[12*nStride]);
	const __m128i T18D = _mm_loadu_si128((const __m128i*)&pRef[13*nStride]);
	const __m128i T18E = _mm_loadu_si128((const __m128i*)&pRef[14*nStride]);
	const __m128i T18F = _mm_loadu_si128((const __m128i*)&pRef[15*nStride]);
	
	const __m128i c_zero  = _mm_setzero_si128();
	const __m128i T190a = _mm_unpacklo_epi8(T180, c_zero);
	const __m128i T191a = _mm_unpacklo_epi8(T181, c_zero);
	const __m128i T192a = _mm_unpacklo_epi8(T182, c_zero);
	const __m128i T193a = _mm_unpacklo_epi8(T183, c_zero);
	const __m128i T194a = _mm_unpacklo_epi8(T184, c_zero);
	const __m128i T195a = _mm_unpacklo_epi8(T185, c_zero);
	const __m128i T196a = _mm_unpacklo_epi8(T186, c_zero);
	const __m128i T197a = _mm_unpacklo_epi8(T187, c_zero);
	const __m128i T198a = _mm_unpacklo_epi8(T188, c_zero);
	const __m128i T199a = _mm_unpacklo_epi8(T189, c_zero);
	const __m128i T19Aa = _mm_unpacklo_epi8(T18A, c_zero);
	const __m128i T19Ba = _mm_unpacklo_epi8(T18B, c_zero);
	const __m128i T19Ca = _mm_unpacklo_epi8(T18C, c_zero);
	const __m128i T19Da = _mm_unpacklo_epi8(T18D, c_zero);
	const __m128i T19Ea = _mm_unpacklo_epi8(T18E, c_zero);
	const __m128i T19Fa = _mm_unpacklo_epi8(T18F, c_zero);
	const __m128i T190b = _mm_unpackhi_epi8(T180, c_zero);
	const __m128i T191b = _mm_unpackhi_epi8(T181, c_zero);
	const __m128i T192b = _mm_unpackhi_epi8(T182, c_zero);
	const __m128i T193b = _mm_unpackhi_epi8(T183, c_zero);
	const __m128i T194b = _mm_unpackhi_epi8(T184, c_zero);
	const __m128i T195b = _mm_unpackhi_epi8(T185, c_zero);
	const __m128i T196b = _mm_unpackhi_epi8(T186, c_zero);
	const __m128i T197b = _mm_unpackhi_epi8(T187, c_zero);
	const __m128i T198b = _mm_unpackhi_epi8(T188, c_zero);
	const __m128i T199b = _mm_unpackhi_epi8(T189, c_zero);
	const __m128i T19Ab = _mm_unpackhi_epi8(T18A, c_zero);
	const __m128i T19Bb = _mm_unpackhi_epi8(T18B, c_zero);
	const __m128i T19Cb = _mm_unpackhi_epi8(T18C, c_zero);
	const __m128i T19Db = _mm_unpackhi_epi8(T18D, c_zero);
	const __m128i T19Eb = _mm_unpackhi_epi8(T18E, c_zero);
	const __m128i T19Fb = _mm_unpackhi_epi8(T18F, c_zero);

	const __m128i T200a = _mm_add_epi16(in00[0], T190a);
	const __m128i T201a = _mm_add_epi16(in01[0], T191a);
	const __m128i T202a = _mm_add_epi16(in02[0], T192a);
	const __m128i T203a = _mm_add_epi16(in03[0], T193a);
	const __m128i T204a = _mm_add_epi16(in04[0], T194a);
	const __m128i T205a = _mm_add_epi16(in05[0], T195a);
	const __m128i T206a = _mm_add_epi16(in06[0], T196a);
	const __m128i T207a = _mm_add_epi16(in07[0], T197a);
	const __m128i T208a = _mm_add_epi16(in08[0], T198a);
	const __m128i T209a = _mm_add_epi16(in09[0], T199a);
	const __m128i T20Aa = _mm_add_epi16(in10[0], T19Aa);
	const __m128i T20Ba = _mm_add_epi16(in11[0], T19Ba);
	const __m128i T20Ca = _mm_add_epi16(in12[0], T19Ca);
	const __m128i T20Da = _mm_add_epi16(in13[0], T19Da);
	const __m128i T20Ea = _mm_add_epi16(in14[0], T19Ea);
	const __m128i T20Fa = _mm_add_epi16(in15[0], T19Fa);

	const __m128i T200b = _mm_add_epi16(in00[1], T190b);
	const __m128i T201b = _mm_add_epi16(in01[1], T191b);
	const __m128i T202b = _mm_add_epi16(in02[1], T192b);
	const __m128i T203b = _mm_add_epi16(in03[1], T193b);
	const __m128i T204b = _mm_add_epi16(in04[1], T194b);
	const __m128i T205b = _mm_add_epi16(in05[1], T195b);
	const __m128i T206b = _mm_add_epi16(in06[1], T196b);
	const __m128i T207b = _mm_add_epi16(in07[1], T197b);
	const __m128i T208b = _mm_add_epi16(in08[1], T198b);
	const __m128i T209b = _mm_add_epi16(in09[1], T199b);
	const __m128i T20Ab = _mm_add_epi16(in10[1], T19Ab);
	const __m128i T20Bb = _mm_add_epi16(in11[1], T19Bb);
	const __m128i T20Cb = _mm_add_epi16(in12[1], T19Cb);
	const __m128i T20Db = _mm_add_epi16(in13[1], T19Db);
	const __m128i T20Eb = _mm_add_epi16(in14[1], T19Eb);
	const __m128i T20Fb = _mm_add_epi16(in15[1], T19Fb);


	const __m128i T210 = _mm_packus_epi16(T200a, T200b);
	const __m128i T211 = _mm_packus_epi16(T201a, T201b);
	const __m128i T212 = _mm_packus_epi16(T202a, T202b);
	const __m128i T213 = _mm_packus_epi16(T203a, T203b);
	const __m128i T214 = _mm_packus_epi16(T204a, T204b);
	const __m128i T215 = _mm_packus_epi16(T205a, T205b);
	const __m128i T216 = _mm_packus_epi16(T206a, T206b);
	const __m128i T217 = _mm_packus_epi16(T207a, T207b);
	const __m128i T218 = _mm_packus_epi16(T208a, T208b);
	const __m128i T219 = _mm_packus_epi16(T209a, T209b);
	const __m128i T21A = _mm_packus_epi16(T20Aa, T20Ab);
	const __m128i T21B = _mm_packus_epi16(T20Ba, T20Bb);
	const __m128i T21C = _mm_packus_epi16(T20Ca, T20Cb);
	const __m128i T21D = _mm_packus_epi16(T20Da, T20Db);
	const __m128i T21E = _mm_packus_epi16(T20Ea, T20Eb);
	const __m128i T21F = _mm_packus_epi16(T20Fa, T20Fb);
	_mm_storeu_si128( (__m128i *)&pDst[0*nStrideDst], T210);
	_mm_storeu_si128( (__m128i *)&pDst[1*nStrideDst], T211);
	_mm_storeu_si128( (__m128i *)&pDst[2*nStrideDst], T212);
	_mm_storeu_si128( (__m128i *)&pDst[3*nStrideDst], T213);
	_mm_storeu_si128( (__m128i *)&pDst[4*nStrideDst], T214);
	_mm_storeu_si128( (__m128i *)&pDst[5*nStrideDst], T215);
	_mm_storeu_si128( (__m128i *)&pDst[6*nStrideDst], T216);
	_mm_storeu_si128( (__m128i *)&pDst[7*nStrideDst], T217);
	_mm_storeu_si128( (__m128i *)&pDst[8*nStrideDst], T218);
	_mm_storeu_si128( (__m128i *)&pDst[9*nStrideDst], T219);
	_mm_storeu_si128( (__m128i *)&pDst[10*nStrideDst], T21A);
	_mm_storeu_si128( (__m128i *)&pDst[11*nStrideDst], T21B);
	_mm_storeu_si128( (__m128i *)&pDst[12*nStrideDst], T21C);
	_mm_storeu_si128( (__m128i *)&pDst[13*nStrideDst], T21D);
	_mm_storeu_si128( (__m128i *)&pDst[14*nStrideDst], T21E);
	_mm_storeu_si128( (__m128i *)&pDst[15*nStrideDst], T21F);
}
#else
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
#endif

#if (IDCT_ADD_USE_ASM >= ASM_SSE4 && defined(__INTEL_COMPILER)) //IDCT32 will result in inconsistancy by VisualC++ Compiler, 2013-10-25, YananZhao
void xIDctAdd32(
	UInt8 *pDst, UInt nStrideDst,
	Int16 *pSrc, UInt nStride,
	UInt8 *pRef,
	Int16 *piTmp0, Int16 *piTmp1
	)
{
	(void *)piTmp0;//avoid warning
	(void *)piTmp1;//avoid warning

	const __m128i c16_p90_p90   = _mm_set1_epi32(0x005A005A);//column 0
	const __m128i c16_p85_p88   = _mm_set1_epi32(0x00550058);
	const __m128i c16_p78_p82   = _mm_set1_epi32(0x004E0052);
	const __m128i c16_p67_p73   = _mm_set1_epi32(0x00430049);
	const __m128i c16_p54_p61   = _mm_set1_epi32(0x0036003D);
	const __m128i c16_p38_p46   = _mm_set1_epi32(0x0026002E);
	const __m128i c16_p22_p31   = _mm_set1_epi32(0x0016001F);
	const __m128i c16_p04_p13   = _mm_set1_epi32(0x0004000D);
	const __m128i c16_p82_p90   = _mm_set1_epi32(0x0052005A);//column 1
	const __m128i c16_p46_p67   = _mm_set1_epi32(0x002E0043);
	const __m128i c16_n04_p22   = _mm_set1_epi32(0xFFFC0016);
	const __m128i c16_n54_n31   = _mm_set1_epi32(0xFFCAFFE1);
	const __m128i c16_n85_n73   = _mm_set1_epi32(0xFFABFFB7);
	const __m128i c16_n88_n90   = _mm_set1_epi32(0xFFA8FFA6);
	const __m128i c16_n61_n78   = _mm_set1_epi32(0xFFC3FFB2);
	const __m128i c16_n13_n38   = _mm_set1_epi32(0xFFF3FFDA);
	const __m128i c16_p67_p88   = _mm_set1_epi32(0x00430058);//column 2
	const __m128i c16_n13_p31   = _mm_set1_epi32(0xFFF3001F);
	const __m128i c16_n82_n54   = _mm_set1_epi32(0xFFAEFFCA);
	const __m128i c16_n78_n90   = _mm_set1_epi32(0xFFB2FFA6);
	const __m128i c16_n04_n46   = _mm_set1_epi32(0xFFFCFFD2);
	const __m128i c16_p73_p38   = _mm_set1_epi32(0x00490026);
	const __m128i c16_p85_p90   = _mm_set1_epi32(0x0055005A);
	const __m128i c16_p22_p61   = _mm_set1_epi32(0x0016003D);
	const __m128i c16_p46_p85   = _mm_set1_epi32(0x002E0055);//column 3
	const __m128i c16_n67_n13   = _mm_set1_epi32(0xFFBDFFF3);
	const __m128i c16_n73_n90   = _mm_set1_epi32(0xFFB7FFA6);
	const __m128i c16_p38_n22   = _mm_set1_epi32(0x0026FFEA);
	const __m128i c16_p88_p82   = _mm_set1_epi32(0x00580052);
	const __m128i c16_n04_p54   = _mm_set1_epi32(0xFFFC0036);
	const __m128i c16_n90_n61   = _mm_set1_epi32(0xFFA6FFC3);
	const __m128i c16_n31_n78   = _mm_set1_epi32(0xFFE1FFB2);
	const __m128i c16_p22_p82   = _mm_set1_epi32(0x00160052);//column 4
	const __m128i c16_n90_n54   = _mm_set1_epi32(0xFFA6FFCA);
	const __m128i c16_p13_n61   = _mm_set1_epi32(0x000DFFC3);
	const __m128i c16_p85_p78   = _mm_set1_epi32(0x0055004E);
	const __m128i c16_n46_p31   = _mm_set1_epi32(0xFFD2001F);
	const __m128i c16_n67_n90   = _mm_set1_epi32(0xFFBDFFA6);
	const __m128i c16_p73_p04   = _mm_set1_epi32(0x00490004);
	const __m128i c16_p38_p88   = _mm_set1_epi32(0x00260058);
	const __m128i c16_n04_p78   = _mm_set1_epi32(0xFFFC004E);//column 5
	const __m128i c16_n73_n82   = _mm_set1_epi32(0xFFB7FFAE);
	const __m128i c16_p85_p13   = _mm_set1_epi32(0x0055000D);
	const __m128i c16_n22_p67   = _mm_set1_epi32(0xFFEA0043);
	const __m128i c16_n61_n88   = _mm_set1_epi32(0xFFC3FFA8);
	const __m128i c16_p90_p31   = _mm_set1_epi32(0x005A001F);
	const __m128i c16_n38_p54   = _mm_set1_epi32(0xFFDA0036);
	const __m128i c16_n46_n90   = _mm_set1_epi32(0xFFD2FFA6);
	const __m128i c16_n31_p73   = _mm_set1_epi32(0xFFE10049);//column 6
	const __m128i c16_n22_n90   = _mm_set1_epi32(0xFFEAFFA6);
	const __m128i c16_p67_p78   = _mm_set1_epi32(0x0043004E);
	const __m128i c16_n90_n38   = _mm_set1_epi32(0xFFA6FFDA);
	const __m128i c16_p82_n13   = _mm_set1_epi32(0x0052FFF3);
	const __m128i c16_n46_p61   = _mm_set1_epi32(0xFFD2003D);
	const __m128i c16_n04_n88   = _mm_set1_epi32(0xFFFCFFA8);
	const __m128i c16_p54_p85   = _mm_set1_epi32(0x00360055);
	const __m128i c16_n54_p67   = _mm_set1_epi32(0xFFCA0043);//column 7
	const __m128i c16_p38_n78   = _mm_set1_epi32(0x0026FFB2);
	const __m128i c16_n22_p85   = _mm_set1_epi32(0xFFEA0055);
	const __m128i c16_p04_n90   = _mm_set1_epi32(0x0004FFA6);
	const __m128i c16_p13_p90   = _mm_set1_epi32(0x000D005A);
	const __m128i c16_n31_n88   = _mm_set1_epi32(0xFFE1FFA8);
	const __m128i c16_p46_p82   = _mm_set1_epi32(0x002E0052);
	const __m128i c16_n61_n73   = _mm_set1_epi32(0xFFC3FFB7);
	const __m128i c16_n73_p61   = _mm_set1_epi32(0xFFB7003D);//column 8
	const __m128i c16_p82_n46   = _mm_set1_epi32(0x0052FFD2);
	const __m128i c16_n88_p31   = _mm_set1_epi32(0xFFA8001F);
	const __m128i c16_p90_n13   = _mm_set1_epi32(0x005AFFF3);
	const __m128i c16_n90_n04   = _mm_set1_epi32(0xFFA6FFFC);
	const __m128i c16_p85_p22   = _mm_set1_epi32(0x00550016);
	const __m128i c16_n78_n38   = _mm_set1_epi32(0xFFB2FFDA);
	const __m128i c16_p67_p54   = _mm_set1_epi32(0x00430036);
	const __m128i c16_n85_p54   = _mm_set1_epi32(0xFFAB0036);//column 9
	const __m128i c16_p88_n04   = _mm_set1_epi32(0x0058FFFC);
	const __m128i c16_n61_n46   = _mm_set1_epi32(0xFFC3FFD2);
	const __m128i c16_p13_p82   = _mm_set1_epi32(0x000D0052);
	const __m128i c16_p38_n90   = _mm_set1_epi32(0x0026FFA6);
	const __m128i c16_n78_p67   = _mm_set1_epi32(0xFFB20043);
	const __m128i c16_p90_n22   = _mm_set1_epi32(0x005AFFEA);
	const __m128i c16_n73_n31   = _mm_set1_epi32(0xFFB7FFE1);
	const __m128i c16_n90_p46   = _mm_set1_epi32(0xFFA6002E);//column 10
	const __m128i c16_p54_p38   = _mm_set1_epi32(0x00360026);
	const __m128i c16_p31_n90   = _mm_set1_epi32(0x001FFFA6);
	const __m128i c16_n88_p61   = _mm_set1_epi32(0xFFA8003D);
	const __m128i c16_p67_p22   = _mm_set1_epi32(0x00430016);
	const __m128i c16_p13_n85   = _mm_set1_epi32(0x000DFFAB);
	const __m128i c16_n82_p73   = _mm_set1_epi32(0xFFAE0049);
	const __m128i c16_p78_p04   = _mm_set1_epi32(0x004E0004);
	const __m128i c16_n88_p38   = _mm_set1_epi32(0xFFA80026);//column 11
	const __m128i c16_n04_p73   = _mm_set1_epi32(0xFFFC0049);
	const __m128i c16_p90_n67   = _mm_set1_epi32(0x005AFFBD);
	const __m128i c16_n31_n46   = _mm_set1_epi32(0xFFE1FFD2);
	const __m128i c16_n78_p85   = _mm_set1_epi32(0xFFB20055);
	const __m128i c16_p61_p13   = _mm_set1_epi32(0x003D000D);
	const __m128i c16_p54_n90   = _mm_set1_epi32(0x0036FFA6);
	const __m128i c16_n82_p22   = _mm_set1_epi32(0xFFAE0016);
	const __m128i c16_n78_p31   = _mm_set1_epi32(0xFFB2001F);//column 12
	const __m128i c16_n61_p90   = _mm_set1_epi32(0xFFC3005A);
	const __m128i c16_p54_p04   = _mm_set1_epi32(0x00360004);
	const __m128i c16_p82_n88   = _mm_set1_epi32(0x0052FFA8);
	const __m128i c16_n22_n38   = _mm_set1_epi32(0xFFEAFFDA);
	const __m128i c16_n90_p73   = _mm_set1_epi32(0xFFA60049);
	const __m128i c16_n13_p67   = _mm_set1_epi32(0xFFF30043);
	const __m128i c16_p85_n46   = _mm_set1_epi32(0x0055FFD2);
	const __m128i c16_n61_p22   = _mm_set1_epi32(0xFFC30016);//column 13
	const __m128i c16_n90_p85   = _mm_set1_epi32(0xFFA60055);
	const __m128i c16_n38_p73   = _mm_set1_epi32(0xFFDA0049);
	const __m128i c16_p46_n04   = _mm_set1_epi32(0x002EFFFC);
	const __m128i c16_p90_n78   = _mm_set1_epi32(0x005AFFB2);
	const __m128i c16_p54_n82   = _mm_set1_epi32(0x0036FFAE);
	const __m128i c16_n31_n13   = _mm_set1_epi32(0xFFE1FFF3);
	const __m128i c16_n88_p67   = _mm_set1_epi32(0xFFA80043);
	const __m128i c16_n38_p13   = _mm_set1_epi32(0xFFDA000D);//column 14
	const __m128i c16_n78_p61   = _mm_set1_epi32(0xFFB2003D);
	const __m128i c16_n90_p88   = _mm_set1_epi32(0xFFA60058);
	const __m128i c16_n73_p85   = _mm_set1_epi32(0xFFB70055);
	const __m128i c16_n31_p54   = _mm_set1_epi32(0xFFE10036);
	const __m128i c16_p22_p04   = _mm_set1_epi32(0x00160004);
	const __m128i c16_p67_n46   = _mm_set1_epi32(0x0043FFD2);
	const __m128i c16_p90_n82   = _mm_set1_epi32(0x005AFFAE);
	const __m128i c16_n13_p04   = _mm_set1_epi32(0xFFF30004);//column 15
	const __m128i c16_n31_p22   = _mm_set1_epi32(0xFFE10016);
	const __m128i c16_n46_p38   = _mm_set1_epi32(0xFFD20026);
	const __m128i c16_n61_p54   = _mm_set1_epi32(0xFFC30036);
	const __m128i c16_n73_p67   = _mm_set1_epi32(0xFFB70043);
	const __m128i c16_n82_p78   = _mm_set1_epi32(0xFFAE004E);
	const __m128i c16_n88_p85   = _mm_set1_epi32(0xFFA80055);
	const __m128i c16_n90_p90   = _mm_set1_epi32(0xFFA6005A);

	//EO
	const __m128i c16_p87_p90   = _mm_set1_epi32(0x0057005A);//row0 87high - 90low address
	const __m128i c16_p70_p80   = _mm_set1_epi32(0x00460050);
	const __m128i c16_p43_p57   = _mm_set1_epi32(0x002B0039);
	const __m128i c16_p09_p25   = _mm_set1_epi32(0x00090019);
	const __m128i c16_p57_p87   = _mm_set1_epi32(0x00390057);//row1
	const __m128i c16_n43_p09   = _mm_set1_epi32(0xFFD50009);
	const __m128i c16_n90_n80   = _mm_set1_epi32(0xFFA6FFB0);
	const __m128i c16_n25_n70   = _mm_set1_epi32(0xFFE7FFBA);
	const __m128i c16_p09_p80   = _mm_set1_epi32(0x00090050);//row2
	const __m128i c16_n87_n70   = _mm_set1_epi32(0xFFA9FFBA);
	const __m128i c16_p57_n25   = _mm_set1_epi32(0x0039FFE7);
	const __m128i c16_p43_p90   = _mm_set1_epi32(0x002B005A);
	const __m128i c16_n43_p70   = _mm_set1_epi32(0xFFD50046);//row3
	const __m128i c16_p09_n87   = _mm_set1_epi32(0x0009FFA9);
	const __m128i c16_p25_p90   = _mm_set1_epi32(0x0019005A);
	const __m128i c16_n57_n80   = _mm_set1_epi32(0xFFC7FFB0);
	const __m128i c16_n80_p57   = _mm_set1_epi32(0xFFB00039);//row4
	const __m128i c16_p90_n25   = _mm_set1_epi32(0x005AFFE7);
	const __m128i c16_n87_n09   = _mm_set1_epi32(0xFFA9FFF7);
	const __m128i c16_p70_p43   = _mm_set1_epi32(0x0046002B);
	const __m128i c16_n90_p43   = _mm_set1_epi32(0xFFA6002B);//row5
	const __m128i c16_p25_p57   = _mm_set1_epi32(0x00190039);
	const __m128i c16_p70_n87   = _mm_set1_epi32(0x0046FFA9);
	const __m128i c16_n80_p09   = _mm_set1_epi32(0xFFB00009);
	const __m128i c16_n70_p25   = _mm_set1_epi32(0xFFBA0019);//row6
	const __m128i c16_n80_p90   = _mm_set1_epi32(0xFFB0005A);
	const __m128i c16_p09_p43   = _mm_set1_epi32(0x0009002B);
	const __m128i c16_p87_n57   = _mm_set1_epi32(0x0057FFC7);
	const __m128i c16_n25_p09   = _mm_set1_epi32(0xFFE70009);//row7
	const __m128i c16_n57_p43   = _mm_set1_epi32(0xFFC7002B);
	const __m128i c16_n80_p70   = _mm_set1_epi32(0xFFB00046);
	const __m128i c16_n90_p87   = _mm_set1_epi32(0xFFA60057);
	//EEO
	const __m128i c16_p75_p89   = _mm_set1_epi32(0x004B0059);
	const __m128i c16_p18_p50   = _mm_set1_epi32(0x00120032);
	const __m128i c16_n18_p75   = _mm_set1_epi32(0xFFEE004B);
	const __m128i c16_n50_n89   = _mm_set1_epi32(0xFFCEFFA7);
	const __m128i c16_n89_p50   = _mm_set1_epi32(0xFFA70032);
	const __m128i c16_p75_p18   = _mm_set1_epi32(0x004B0012);
	const __m128i c16_n50_p18   = _mm_set1_epi32(0xFFCE0012);
	const __m128i c16_n89_p75   = _mm_set1_epi32(0xFFA7004B);
	//EEEO
	const __m128i c16_p36_p83   = _mm_set1_epi32(0x00240053);
	const __m128i c16_n83_p36   = _mm_set1_epi32(0xFFAD0024);
	//EEEE
	const __m128i c16_n64_p64   = _mm_set1_epi32(0xFFC00040);
	const __m128i c16_p64_p64   = _mm_set1_epi32(0x00400040);
	const __m128i c_zero        = _mm_setzero_si128();
	__m128i c32_rnd      = _mm_set1_epi32(64);

	Int nShift = SHIFT_INV_1ST;

	// DCT1
	__m128i in00[4], in01[4], in02[4], in03[4], in04[4], in05[4], in06[4], in07[4], in08[4], in09[4], in10[4], in11[4], in12[4], in13[4], in14[4], in15[4];
	__m128i in16[4], in17[4], in18[4], in19[4], in20[4], in21[4], in22[4], in23[4], in24[4], in25[4], in26[4], in27[4], in28[4], in29[4], in30[4], in31[4];
	__m128i res00[4], res01[4], res02[4], res03[4], res04[4], res05[4], res06[4], res07[4], res08[4], res09[4], res10[4], res11[4], res12[4], res13[4], res14[4], res15[4];
	__m128i res16[4], res17[4], res18[4], res19[4], res20[4], res21[4], res22[4], res23[4], res24[4], res25[4], res26[4], res27[4], res28[4], res29[4], res30[4], res31[4];

	for (Int i=0; i<4; i++){
		Int offset = (i<<3);

		in00[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 0*nStride+offset]); // [07 06 05 04 03 02 01 00]
		in01[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 1*nStride+offset]); // [17 16 15 14 13 12 11 10]
		in02[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 2*nStride+offset]); // [27 26 25 24 23 22 21 20]
		in03[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 3*nStride+offset]); // [37 36 35 34 33 32 31 30]
		in04[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 4*nStride+offset]); // [47 46 45 44 43 42 41 40]
		in05[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 5*nStride+offset]); // [57 56 55 54 53 52 51 50]
		in06[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 6*nStride+offset]); // [67 66 65 64 63 62 61 60]
		in07[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 7*nStride+offset]); // [77 76 75 74 73 72 71 70]
		in08[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 8*nStride+offset]); // [x7~x0]row08
		in09[i]  = _mm_loadu_si128((const __m128i*)&pSrc[ 9*nStride+offset]); // [x7~x0]row09
		in10[i]  = _mm_loadu_si128((const __m128i*)&pSrc[10*nStride+offset]); // [x7~x0]row10
		in11[i]  = _mm_loadu_si128((const __m128i*)&pSrc[11*nStride+offset]); // [x7~x0]row11
		in12[i]  = _mm_loadu_si128((const __m128i*)&pSrc[12*nStride+offset]); // [x7~x0]row12
		in13[i]  = _mm_loadu_si128((const __m128i*)&pSrc[13*nStride+offset]); // [x7~x0]row13
		in14[i]  = _mm_loadu_si128((const __m128i*)&pSrc[14*nStride+offset]); // [x7~x0]row14
		in15[i]  = _mm_loadu_si128((const __m128i*)&pSrc[15*nStride+offset]); // [x7~x0]row15
		in16[i]  = _mm_loadu_si128((const __m128i*)&pSrc[16*nStride+offset]); // 
		in17[i]  = _mm_loadu_si128((const __m128i*)&pSrc[17*nStride+offset]); // 
		in18[i]  = _mm_loadu_si128((const __m128i*)&pSrc[18*nStride+offset]); // 
		in19[i]  = _mm_loadu_si128((const __m128i*)&pSrc[19*nStride+offset]); //
		in20[i]  = _mm_loadu_si128((const __m128i*)&pSrc[20*nStride+offset]); // 
		in21[i]  = _mm_loadu_si128((const __m128i*)&pSrc[21*nStride+offset]); //
		in22[i]  = _mm_loadu_si128((const __m128i*)&pSrc[22*nStride+offset]); // 
		in23[i]  = _mm_loadu_si128((const __m128i*)&pSrc[23*nStride+offset]); // 
		in24[i]  = _mm_loadu_si128((const __m128i*)&pSrc[24*nStride+offset]); // 
		in25[i]  = _mm_loadu_si128((const __m128i*)&pSrc[25*nStride+offset]); // 
		in26[i]  = _mm_loadu_si128((const __m128i*)&pSrc[26*nStride+offset]); // 
		in27[i]  = _mm_loadu_si128((const __m128i*)&pSrc[27*nStride+offset]); // 
		in28[i]  = _mm_loadu_si128((const __m128i*)&pSrc[28*nStride+offset]); // 
		in29[i]  = _mm_loadu_si128((const __m128i*)&pSrc[29*nStride+offset]); // 
		in30[i]  = _mm_loadu_si128((const __m128i*)&pSrc[30*nStride+offset]); // 
		in31[i]  = _mm_loadu_si128((const __m128i*)&pSrc[31*nStride+offset]); //
	}

	for(Int pass=0; pass<2; ++pass){
		if (pass==1){
			c32_rnd = _mm_set1_epi32(2048);//rnd2
			nShift  = SHIFT_INV_2ND;  
		}

		for(Int part=0; part<4; part++){
			const __m128i T_00_00A = _mm_unpacklo_epi16(in01[part], in03[part]);             // [33 13 32 12 31 11 30 10]
			const __m128i T_00_00B = _mm_unpackhi_epi16(in01[part], in03[part]);             // [37 17 36 16 35 15 34 14]
			const __m128i T_00_01A = _mm_unpacklo_epi16(in05[part], in07[part]);             // [ ]
			const __m128i T_00_01B = _mm_unpackhi_epi16(in05[part], in07[part]);             // [ ]
			const __m128i T_00_02A = _mm_unpacklo_epi16(in09[part], in11[part]);             // [ ]
			const __m128i T_00_02B = _mm_unpackhi_epi16(in09[part], in11[part]);             // [ ]
			const __m128i T_00_03A = _mm_unpacklo_epi16(in13[part], in15[part]);             // [ ]
			const __m128i T_00_03B = _mm_unpackhi_epi16(in13[part], in15[part]);             // [ ]
			const __m128i T_00_04A = _mm_unpacklo_epi16(in17[part], in19[part]);             // [ ]
			const __m128i T_00_04B = _mm_unpackhi_epi16(in17[part], in19[part]);             // [ ]
			const __m128i T_00_05A = _mm_unpacklo_epi16(in21[part], in23[part]);             // [ ]
			const __m128i T_00_05B = _mm_unpackhi_epi16(in21[part], in23[part]);             // [ ]
			const __m128i T_00_06A = _mm_unpacklo_epi16(in25[part], in27[part]);             // [ ]
			const __m128i T_00_06B = _mm_unpackhi_epi16(in25[part], in27[part]);             // [ ]
			const __m128i T_00_07A = _mm_unpacklo_epi16(in29[part], in31[part]);             // 
			const __m128i T_00_07B = _mm_unpackhi_epi16(in29[part], in31[part]);             // [ ]

			const __m128i T_00_08A = _mm_unpacklo_epi16(in02[part], in06[part]);             // [ ]
			const __m128i T_00_08B = _mm_unpackhi_epi16(in02[part], in06[part]);             // [ ]
			const __m128i T_00_09A = _mm_unpacklo_epi16(in10[part], in14[part]);             // [ ]
			const __m128i T_00_09B = _mm_unpackhi_epi16(in10[part], in14[part]);             // [ ]
			const __m128i T_00_10A = _mm_unpacklo_epi16(in18[part], in22[part]);             // [ ]
			const __m128i T_00_10B = _mm_unpackhi_epi16(in18[part], in22[part]);             // [ ]
			const __m128i T_00_11A = _mm_unpacklo_epi16(in26[part], in30[part]);             // [ ]
			const __m128i T_00_11B = _mm_unpackhi_epi16(in26[part], in30[part]);             // [ ]

			const __m128i T_00_12A = _mm_unpacklo_epi16(in04[part], in12[part]);             // [ ]
			const __m128i T_00_12B = _mm_unpackhi_epi16(in04[part], in12[part]);             // [ ]
			const __m128i T_00_13A = _mm_unpacklo_epi16(in20[part], in28[part]);             // [ ]
			const __m128i T_00_13B = _mm_unpackhi_epi16(in20[part], in28[part]);             // [ ]

			const __m128i T_00_14A = _mm_unpacklo_epi16(in08[part], in24[part]);             // 
			const __m128i T_00_14B = _mm_unpackhi_epi16(in08[part], in24[part]);             // [ ]
			const __m128i T_00_15A = _mm_unpacklo_epi16(in00[part], in16[part]);             // 
			const __m128i T_00_15B = _mm_unpackhi_epi16(in00[part], in16[part]);             // [ ]

			__m128i O00A, O01A, O02A, O03A, O04A, O05A, O06A, O07A, O08A, O09A, O10A, O11A, O12A, O13A, O14A, O15A;
			__m128i O00B, O01B, O02B, O03B, O04B, O05B, O06B, O07B, O08B, O09B, O10B, O11B, O12B, O13B, O14B, O15B;
			{
				__m128i T00, T01, T02, T03;
#define COMPUTE_ROW(r0103, r0507, r0911, r1315, r1719, r2123, r2527, r2931, c0103, c0507, c0911, c1315, c1719, c2123, c2527, c2931, row)\
	T00 = _mm_add_epi32(_mm_madd_epi16(r0103, c0103), _mm_madd_epi16(r0507, c0507));\
	T01 = _mm_add_epi32(_mm_madd_epi16(r0911, c0911), _mm_madd_epi16(r1315, c1315));\
	T02 = _mm_add_epi32(_mm_madd_epi16(r1719, c1719), _mm_madd_epi16(r2123, c2123));\
	T03 = _mm_add_epi32(_mm_madd_epi16(r2527, c2527), _mm_madd_epi16(r2931, c2931));\
	row = _mm_add_epi32(_mm_add_epi32(T00, T01), _mm_add_epi32(T02, T03));

				COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_p90_p90, c16_p85_p88, c16_p78_p82, c16_p67_p73, c16_p54_p61, c16_p38_p46, c16_p22_p31, c16_p04_p13, O00A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_p82_p90, c16_p46_p67, c16_n04_p22, c16_n54_n31, c16_n85_n73, c16_n88_n90, c16_n61_n78, c16_n13_n38, O01A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_p67_p88, c16_n13_p31, c16_n82_n54, c16_n78_n90, c16_n04_n46, c16_p73_p38, c16_p85_p90, c16_p22_p61, O02A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_p46_p85, c16_n67_n13, c16_n73_n90, c16_p38_n22, c16_p88_p82, c16_n04_p54, c16_n90_n61, c16_n31_n78, O03A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_p22_p82, c16_n90_n54, c16_p13_n61, c16_p85_p78, c16_n46_p31, c16_n67_n90, c16_p73_p04, c16_p38_p88, O04A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n04_p78, c16_n73_n82, c16_p85_p13, c16_n22_p67, c16_n61_n88, c16_p90_p31, c16_n38_p54, c16_n46_n90, O05A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n31_p73, c16_n22_n90, c16_p67_p78, c16_n90_n38, c16_p82_n13, c16_n46_p61, c16_n04_n88, c16_p54_p85, O06A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n54_p67, c16_p38_n78, c16_n22_p85, c16_p04_n90, c16_p13_p90, c16_n31_n88, c16_p46_p82, c16_n61_n73, O07A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n73_p61, c16_p82_n46, c16_n88_p31, c16_p90_n13, c16_n90_n04, c16_p85_p22, c16_n78_n38, c16_p67_p54, O08A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n85_p54, c16_p88_n04, c16_n61_n46, c16_p13_p82, c16_p38_n90, c16_n78_p67, c16_p90_n22, c16_n73_n31, O09A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n90_p46, c16_p54_p38, c16_p31_n90, c16_n88_p61, c16_p67_p22, c16_p13_n85, c16_n82_p73, c16_p78_p04, O10A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n88_p38, c16_n04_p73, c16_p90_n67, c16_n31_n46, c16_n78_p85, c16_p61_p13, c16_p54_n90, c16_n82_p22, O11A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n78_p31, c16_n61_p90, c16_p54_p04, c16_p82_n88, c16_n22_n38, c16_n90_p73, c16_n13_p67, c16_p85_n46, O12A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n61_p22, c16_n90_p85, c16_n38_p73, c16_p46_n04, c16_p90_n78, c16_p54_n82, c16_n31_n13, c16_n88_p67, O13A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n38_p13, c16_n78_p61, c16_n90_p88, c16_n73_p85, c16_n31_p54, c16_p22_p04, c16_p67_n46, c16_p90_n82, O14A)
					COMPUTE_ROW(T_00_00A,T_00_01A,T_00_02A,T_00_03A, T_00_04A,T_00_05A,T_00_06A,T_00_07A,\
					c16_n13_p04, c16_n31_p22, c16_n46_p38, c16_n61_p54, c16_n73_p67, c16_n82_p78, c16_n88_p85, c16_n90_p90, O15A)

					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_p90_p90, c16_p85_p88, c16_p78_p82, c16_p67_p73, c16_p54_p61, c16_p38_p46, c16_p22_p31, c16_p04_p13, O00B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_p82_p90, c16_p46_p67, c16_n04_p22, c16_n54_n31, c16_n85_n73, c16_n88_n90, c16_n61_n78, c16_n13_n38, O01B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_p67_p88, c16_n13_p31, c16_n82_n54, c16_n78_n90, c16_n04_n46, c16_p73_p38, c16_p85_p90, c16_p22_p61, O02B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_p46_p85, c16_n67_n13, c16_n73_n90, c16_p38_n22, c16_p88_p82, c16_n04_p54, c16_n90_n61, c16_n31_n78, O03B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_p22_p82, c16_n90_n54, c16_p13_n61, c16_p85_p78, c16_n46_p31, c16_n67_n90, c16_p73_p04, c16_p38_p88, O04B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n04_p78, c16_n73_n82, c16_p85_p13, c16_n22_p67, c16_n61_n88, c16_p90_p31, c16_n38_p54, c16_n46_n90, O05B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n31_p73, c16_n22_n90, c16_p67_p78, c16_n90_n38, c16_p82_n13, c16_n46_p61, c16_n04_n88, c16_p54_p85, O06B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n54_p67, c16_p38_n78, c16_n22_p85, c16_p04_n90, c16_p13_p90, c16_n31_n88, c16_p46_p82, c16_n61_n73, O07B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n73_p61, c16_p82_n46, c16_n88_p31, c16_p90_n13, c16_n90_n04, c16_p85_p22, c16_n78_n38, c16_p67_p54, O08B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n85_p54, c16_p88_n04, c16_n61_n46, c16_p13_p82, c16_p38_n90, c16_n78_p67, c16_p90_n22, c16_n73_n31, O09B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n90_p46, c16_p54_p38, c16_p31_n90, c16_n88_p61, c16_p67_p22, c16_p13_n85, c16_n82_p73, c16_p78_p04, O10B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n88_p38, c16_n04_p73, c16_p90_n67, c16_n31_n46, c16_n78_p85, c16_p61_p13, c16_p54_n90, c16_n82_p22, O11B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n78_p31, c16_n61_p90, c16_p54_p04, c16_p82_n88, c16_n22_n38, c16_n90_p73, c16_n13_p67, c16_p85_n46, O12B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n61_p22, c16_n90_p85, c16_n38_p73, c16_p46_n04, c16_p90_n78, c16_p54_n82, c16_n31_n13, c16_n88_p67, O13B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n38_p13, c16_n78_p61, c16_n90_p88, c16_n73_p85, c16_n31_p54, c16_p22_p04, c16_p67_n46, c16_p90_n82, O14B)
					COMPUTE_ROW(T_00_00B,T_00_01B,T_00_02B,T_00_03B, T_00_04B,T_00_05B,T_00_06B,T_00_07B,\
					c16_n13_p04, c16_n31_p22, c16_n46_p38, c16_n61_p54, c16_n73_p67, c16_n82_p78, c16_n88_p85, c16_n90_p90, O15B)

#undef COMPUTE_ROW
			}

			__m128i EO0A, EO1A, EO2A, EO3A, EO4A, EO5A, EO6A, EO7A;
			__m128i EO0B, EO1B, EO2B, EO3B, EO4B, EO5B, EO6B, EO7B;
			{
				__m128i T00, T01;
#define COMPUTE_ROW(row0206, row1014, row1822, row2630, c0206, c1014, c1822, c2630, row)\
	T00 = _mm_add_epi32(_mm_madd_epi16(row0206, c0206), _mm_madd_epi16(row1014, c1014));\
	T01 = _mm_add_epi32(_mm_madd_epi16(row1822, c1822), _mm_madd_epi16(row2630, c2630));\
	row = _mm_add_epi32(T00, T01);

				COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_p87_p90,c16_p70_p80,c16_p43_p57,c16_p09_p25, EO0A)
					COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_p57_p87,c16_n43_p09,c16_n90_n80,c16_n25_n70, EO1A)
					COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_p09_p80,c16_n87_n70,c16_p57_n25,c16_p43_p90, EO2A)
					COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_n43_p70,c16_p09_n87,c16_p25_p90,c16_n57_n80, EO3A)
					COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_n80_p57,c16_p90_n25,c16_n87_n09,c16_p70_p43, EO4A)
					COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_n90_p43,c16_p25_p57,c16_p70_n87,c16_n80_p09, EO5A)
					COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_n70_p25,c16_n80_p90,c16_p09_p43,c16_p87_n57, EO6A)
					COMPUTE_ROW(T_00_08A,T_00_09A,T_00_10A,T_00_11A, c16_n25_p09,c16_n57_p43,c16_n80_p70,c16_n90_p87, EO7A)

					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_p87_p90,c16_p70_p80,c16_p43_p57,c16_p09_p25, EO0B)
					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_p57_p87,c16_n43_p09,c16_n90_n80,c16_n25_n70, EO1B)
					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_p09_p80,c16_n87_n70,c16_p57_n25,c16_p43_p90, EO2B)
					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_n43_p70,c16_p09_n87,c16_p25_p90,c16_n57_n80, EO3B)
					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_n80_p57,c16_p90_n25,c16_n87_n09,c16_p70_p43, EO4B)
					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_n90_p43,c16_p25_p57,c16_p70_n87,c16_n80_p09, EO5B)
					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_n70_p25,c16_n80_p90,c16_p09_p43,c16_p87_n57, EO6B)
					COMPUTE_ROW(T_00_08B,T_00_09B,T_00_10B,T_00_11B, c16_n25_p09,c16_n57_p43,c16_n80_p70,c16_n90_p87, EO7B)
#undef COMPUTE_ROW
			}

			const __m128i EEO0A = _mm_add_epi32(_mm_madd_epi16(T_00_12A, c16_p75_p89 ), _mm_madd_epi16(T_00_13A, c16_p18_p50 )); // EEO0
			const __m128i EEO0B = _mm_add_epi32(_mm_madd_epi16(T_00_12B, c16_p75_p89 ), _mm_madd_epi16(T_00_13B, c16_p18_p50 ));
			const __m128i EEO1A = _mm_add_epi32(_mm_madd_epi16(T_00_12A, c16_n18_p75 ), _mm_madd_epi16(T_00_13A, c16_n50_n89 )); // EEO1
			const __m128i EEO1B = _mm_add_epi32(_mm_madd_epi16(T_00_12B, c16_n18_p75 ), _mm_madd_epi16(T_00_13B, c16_n50_n89 ));
			const __m128i EEO2A = _mm_add_epi32(_mm_madd_epi16(T_00_12A, c16_n89_p50 ), _mm_madd_epi16(T_00_13A, c16_p75_p18 )); // EEO2
			const __m128i EEO2B = _mm_add_epi32(_mm_madd_epi16(T_00_12B, c16_n89_p50 ), _mm_madd_epi16(T_00_13B, c16_p75_p18 ));
			const __m128i EEO3A = _mm_add_epi32(_mm_madd_epi16(T_00_12A, c16_n50_p18 ), _mm_madd_epi16(T_00_13A, c16_n89_p75 )); // EEO3
			const __m128i EEO3B = _mm_add_epi32(_mm_madd_epi16(T_00_12B, c16_n50_p18 ), _mm_madd_epi16(T_00_13B, c16_n89_p75 ));

			const __m128i EEEO0A = _mm_madd_epi16(T_00_14A, c16_p36_p83 );
			const __m128i EEEO0B = _mm_madd_epi16(T_00_14B, c16_p36_p83 );
			const __m128i EEEO1A = _mm_madd_epi16(T_00_14A, c16_n83_p36 );
			const __m128i EEEO1B = _mm_madd_epi16(T_00_14B, c16_n83_p36 );

			const __m128i EEEE0A = _mm_madd_epi16(T_00_15A, c16_p64_p64 );
			const __m128i EEEE0B = _mm_madd_epi16(T_00_15B, c16_p64_p64 );
			const __m128i EEEE1A = _mm_madd_epi16(T_00_15A, c16_n64_p64 );
			const __m128i EEEE1B = _mm_madd_epi16(T_00_15B, c16_n64_p64 );

			const __m128i EEE0A = _mm_add_epi32(EEEE0A, EEEO0A);                // EEE0 = EEEE0 + EEEO0
			const __m128i EEE0B = _mm_add_epi32(EEEE0B, EEEO0B);
			const __m128i EEE1A = _mm_add_epi32(EEEE1A, EEEO1A);                // EEE1 = EEEE1 + EEEO1
			const __m128i EEE1B = _mm_add_epi32(EEEE1B, EEEO1B);
			const __m128i EEE3A = _mm_sub_epi32(EEEE0A, EEEO0A);                // EEE2 = EEEE0 - EEEO0
			const __m128i EEE3B = _mm_sub_epi32(EEEE0B, EEEO0B);
			const __m128i EEE2A = _mm_sub_epi32(EEEE1A, EEEO1A);                // EEE3 = EEEE1 - EEEO1
			const __m128i EEE2B = _mm_sub_epi32(EEEE1B, EEEO1B);

			const __m128i EE0A = _mm_add_epi32(EEE0A, EEO0A);                // EE0 = EEE0 + EEO0
			const __m128i EE0B = _mm_add_epi32(EEE0B, EEO0B);
			const __m128i EE1A = _mm_add_epi32(EEE1A, EEO1A);                // EE1 = EEE1 + EEO1
			const __m128i EE1B = _mm_add_epi32(EEE1B, EEO1B);
			const __m128i EE2A = _mm_add_epi32(EEE2A, EEO2A);                // EE2 = EEE0 + EEO0
			const __m128i EE2B = _mm_add_epi32(EEE2B, EEO2B);
			const __m128i EE3A = _mm_add_epi32(EEE3A, EEO3A);                // EE3 = EEE1 + EEO1
			const __m128i EE3B = _mm_add_epi32(EEE3B, EEO3B);
			const __m128i EE7A = _mm_sub_epi32(EEE0A, EEO0A);                // EE7 = EEE0 - EEO0
			const __m128i EE7B = _mm_sub_epi32(EEE0B, EEO0B);
			const __m128i EE6A = _mm_sub_epi32(EEE1A, EEO1A);                // EE6 = EEE1 - EEO1
			const __m128i EE6B = _mm_sub_epi32(EEE1B, EEO1B);
			const __m128i EE5A = _mm_sub_epi32(EEE2A, EEO2A);                // EE5 = EEE0 - EEO0
			const __m128i EE5B = _mm_sub_epi32(EEE2B, EEO2B);
			const __m128i EE4A = _mm_sub_epi32(EEE3A, EEO3A);                // EE4 = EEE1 - EEO1
			const __m128i EE4B = _mm_sub_epi32(EEE3B, EEO3B);

			const __m128i E0A = _mm_add_epi32(EE0A, EO0A);                // E0 = EE0 + EO0
			const __m128i E0B = _mm_add_epi32(EE0B, EO0B);
			const __m128i E1A = _mm_add_epi32(EE1A, EO1A);                // E1 = EE1 + EO1
			const __m128i E1B = _mm_add_epi32(EE1B, EO1B);
			const __m128i E2A = _mm_add_epi32(EE2A, EO2A);                // E2 = EE2 + EO2
			const __m128i E2B = _mm_add_epi32(EE2B, EO2B);
			const __m128i E3A = _mm_add_epi32(EE3A, EO3A);                // E3 = EE3 + EO3
			const __m128i E3B = _mm_add_epi32(EE3B, EO3B);
			const __m128i E4A = _mm_add_epi32(EE4A, EO4A);                // E4 = 
			const __m128i E4B = _mm_add_epi32(EE4B, EO4B);
			const __m128i E5A = _mm_add_epi32(EE5A, EO5A);                // E5 = 
			const __m128i E5B = _mm_add_epi32(EE5B, EO5B);
			const __m128i E6A = _mm_add_epi32(EE6A, EO6A);                // E6 = 
			const __m128i E6B = _mm_add_epi32(EE6B, EO6B);
			const __m128i E7A = _mm_add_epi32(EE7A, EO7A);                // E7 = 
			const __m128i E7B = _mm_add_epi32(EE7B, EO7B);
			const __m128i EFA = _mm_sub_epi32(EE0A, EO0A);                // EF = EE0 - EO0
			const __m128i EFB = _mm_sub_epi32(EE0B, EO0B);
			const __m128i EEA = _mm_sub_epi32(EE1A, EO1A);                // EE = EE1 - EO1
			const __m128i EEB = _mm_sub_epi32(EE1B, EO1B);
			const __m128i EDA = _mm_sub_epi32(EE2A, EO2A);                // ED = EE2 - EO2
			const __m128i EDB = _mm_sub_epi32(EE2B, EO2B);
			const __m128i ECA = _mm_sub_epi32(EE3A, EO3A);                // EC = EE3 - EO3
			const __m128i ECB = _mm_sub_epi32(EE3B, EO3B);
			const __m128i EBA = _mm_sub_epi32(EE4A, EO4A);                // EB =
			const __m128i EBB = _mm_sub_epi32(EE4B, EO4B);
			const __m128i EAA = _mm_sub_epi32(EE5A, EO5A);                // EA = 
			const __m128i EAB = _mm_sub_epi32(EE5B, EO5B);
			const __m128i E9A = _mm_sub_epi32(EE6A, EO6A);                // E9 =
			const __m128i E9B = _mm_sub_epi32(EE6B, EO6B);
			const __m128i E8A = _mm_sub_epi32(EE7A, EO7A);                // E8 = 
			const __m128i E8B = _mm_sub_epi32(EE7B, EO7B);

			const __m128i T10A = _mm_add_epi32(E0A, c32_rnd);               // E0 + rnd
			const __m128i T10B = _mm_add_epi32(E0B, c32_rnd);
			const __m128i T11A = _mm_add_epi32(E1A, c32_rnd);               // E1 + rnd
			const __m128i T11B = _mm_add_epi32(E1B, c32_rnd);
			const __m128i T12A = _mm_add_epi32(E2A, c32_rnd);               // E2 + rnd
			const __m128i T12B = _mm_add_epi32(E2B, c32_rnd);
			const __m128i T13A = _mm_add_epi32(E3A, c32_rnd);               // E3 + rnd
			const __m128i T13B = _mm_add_epi32(E3B, c32_rnd);
			const __m128i T14A = _mm_add_epi32(E4A, c32_rnd);               // E4 + rnd
			const __m128i T14B = _mm_add_epi32(E4B, c32_rnd);
			const __m128i T15A = _mm_add_epi32(E5A, c32_rnd);               // E5 + rnd
			const __m128i T15B = _mm_add_epi32(E5B, c32_rnd);
			const __m128i T16A = _mm_add_epi32(E6A, c32_rnd);               // E6 + rnd
			const __m128i T16B = _mm_add_epi32(E6B, c32_rnd);
			const __m128i T17A = _mm_add_epi32(E7A, c32_rnd);               // E7 + rnd
			const __m128i T17B = _mm_add_epi32(E7B, c32_rnd);
			const __m128i T18A = _mm_add_epi32(E8A, c32_rnd);               // E8 + rnd
			const __m128i T18B = _mm_add_epi32(E8B, c32_rnd);
			const __m128i T19A = _mm_add_epi32(E9A, c32_rnd);               // E9 + rnd
			const __m128i T19B = _mm_add_epi32(E9B, c32_rnd);
			const __m128i T1AA = _mm_add_epi32(EAA, c32_rnd);               // E10 + rnd
			const __m128i T1AB = _mm_add_epi32(EAB, c32_rnd);
			const __m128i T1BA = _mm_add_epi32(EBA, c32_rnd);               // E11 + rnd
			const __m128i T1BB = _mm_add_epi32(EBB, c32_rnd);
			const __m128i T1CA = _mm_add_epi32(ECA, c32_rnd);               // E12 + rnd
			const __m128i T1CB = _mm_add_epi32(ECB, c32_rnd);
			const __m128i T1DA = _mm_add_epi32(EDA, c32_rnd);               // E13 + rnd
			const __m128i T1DB = _mm_add_epi32(EDB, c32_rnd);
			const __m128i T1EA = _mm_add_epi32(EEA, c32_rnd);               // E14 + rnd
			const __m128i T1EB = _mm_add_epi32(EEB, c32_rnd);
			const __m128i T1FA = _mm_add_epi32(EFA, c32_rnd);               // E15 + rnd
			const __m128i T1FB = _mm_add_epi32(EFB, c32_rnd);

			const __m128i T2_00A = _mm_add_epi32(T10A, O00A);                // E0 + O0 + rnd
			const __m128i T2_00B = _mm_add_epi32(T10B, O00B);
			const __m128i T2_01A = _mm_add_epi32(T11A, O01A);                // E1 + O1 + rnd
			const __m128i T2_01B = _mm_add_epi32(T11B, O01B);
			const __m128i T2_02A = _mm_add_epi32(T12A, O02A);                // E2 + O2 + rnd
			const __m128i T2_02B = _mm_add_epi32(T12B, O02B);
			const __m128i T2_03A = _mm_add_epi32(T13A, O03A);                // E3 + O3 + rnd
			const __m128i T2_03B = _mm_add_epi32(T13B, O03B);
			const __m128i T2_04A = _mm_add_epi32(T14A, O04A);                // E4 
			const __m128i T2_04B = _mm_add_epi32(T14B, O04B);
			const __m128i T2_05A = _mm_add_epi32(T15A, O05A);                // E5 
			const __m128i T2_05B = _mm_add_epi32(T15B, O05B);
			const __m128i T2_06A = _mm_add_epi32(T16A, O06A);                // E6
			const __m128i T2_06B = _mm_add_epi32(T16B, O06B);
			const __m128i T2_07A = _mm_add_epi32(T17A, O07A);                // E7 
			const __m128i T2_07B = _mm_add_epi32(T17B, O07B);
			const __m128i T2_08A = _mm_add_epi32(T18A, O08A);                // E8
			const __m128i T2_08B = _mm_add_epi32(T18B, O08B);
			const __m128i T2_09A = _mm_add_epi32(T19A, O09A);                // E9
			const __m128i T2_09B = _mm_add_epi32(T19B, O09B);
			const __m128i T2_10A = _mm_add_epi32(T1AA, O10A);                // E10
			const __m128i T2_10B = _mm_add_epi32(T1AB, O10B);
			const __m128i T2_11A = _mm_add_epi32(T1BA, O11A);                // E11
			const __m128i T2_11B = _mm_add_epi32(T1BB, O11B);
			const __m128i T2_12A = _mm_add_epi32(T1CA, O12A);                // E12 
			const __m128i T2_12B = _mm_add_epi32(T1CB, O12B);
			const __m128i T2_13A = _mm_add_epi32(T1DA, O13A);                // E13 
			const __m128i T2_13B = _mm_add_epi32(T1DB, O13B);
			const __m128i T2_14A = _mm_add_epi32(T1EA, O14A);                // E14
			const __m128i T2_14B = _mm_add_epi32(T1EB, O14B);
			const __m128i T2_15A = _mm_add_epi32(T1FA, O15A);                // E15 
			const __m128i T2_15B = _mm_add_epi32(T1FB, O15B);
			const __m128i T2_31A = _mm_sub_epi32(T10A, O00A);                // E0 - O0 + rnd
			const __m128i T2_31B = _mm_sub_epi32(T10B, O00B);
			const __m128i T2_30A = _mm_sub_epi32(T11A, O01A);                // E1 - O1 + rnd
			const __m128i T2_30B = _mm_sub_epi32(T11B, O01B);
			const __m128i T2_29A = _mm_sub_epi32(T12A, O02A);                // E2 - O2 + rnd
			const __m128i T2_29B = _mm_sub_epi32(T12B, O02B);
			const __m128i T2_28A = _mm_sub_epi32(T13A, O03A);                // E3 - O3 + rnd
			const __m128i T2_28B = _mm_sub_epi32(T13B, O03B);
			const __m128i T2_27A = _mm_sub_epi32(T14A, O04A);                // E4
			const __m128i T2_27B = _mm_sub_epi32(T14B, O04B);
			const __m128i T2_26A = _mm_sub_epi32(T15A, O05A);                // E5
			const __m128i T2_26B = _mm_sub_epi32(T15B, O05B);
			const __m128i T2_25A = _mm_sub_epi32(T16A, O06A);                // E6
			const __m128i T2_25B = _mm_sub_epi32(T16B, O06B);
			const __m128i T2_24A = _mm_sub_epi32(T17A, O07A);                // E7
			const __m128i T2_24B = _mm_sub_epi32(T17B, O07B);
			const __m128i T2_23A = _mm_sub_epi32(T18A, O08A);                // 
			const __m128i T2_23B = _mm_sub_epi32(T18B, O08B);
			const __m128i T2_22A = _mm_sub_epi32(T19A, O09A);                // 
			const __m128i T2_22B = _mm_sub_epi32(T19B, O09B);
			const __m128i T2_21A = _mm_sub_epi32(T1AA, O10A);                // 
			const __m128i T2_21B = _mm_sub_epi32(T1AB, O10B);
			const __m128i T2_20A = _mm_sub_epi32(T1BA, O11A);                // 
			const __m128i T2_20B = _mm_sub_epi32(T1BB, O11B);
			const __m128i T2_19A = _mm_sub_epi32(T1CA, O12A);                // 
			const __m128i T2_19B = _mm_sub_epi32(T1CB, O12B);
			const __m128i T2_18A = _mm_sub_epi32(T1DA, O13A);                //
			const __m128i T2_18B = _mm_sub_epi32(T1DB, O13B);
			const __m128i T2_17A = _mm_sub_epi32(T1EA, O14A);                // 
			const __m128i T2_17B = _mm_sub_epi32(T1EB, O14B);
			const __m128i T2_16A = _mm_sub_epi32(T1FA, O15A);                // 
			const __m128i T2_16B = _mm_sub_epi32(T1FB, O15B);

			const __m128i T3_00A = _mm_srai_epi32(T2_00A, nShift);                   // [30 20 10 00]
			const __m128i T3_00B = _mm_srai_epi32(T2_00B, nShift);                   // [70 60 50 40]
			const __m128i T3_01A = _mm_srai_epi32(T2_01A, nShift);                   // [31 21 11 01]
			const __m128i T3_01B = _mm_srai_epi32(T2_01B, nShift);                   // [71 61 51 41]
			const __m128i T3_02A = _mm_srai_epi32(T2_02A, nShift);                   // [32 22 12 02]
			const __m128i T3_02B = _mm_srai_epi32(T2_02B, nShift);                   // [72 62 52 42]
			const __m128i T3_03A = _mm_srai_epi32(T2_03A, nShift);                   // [33 23 13 03]
			const __m128i T3_03B = _mm_srai_epi32(T2_03B, nShift);                   // [73 63 53 43]
			const __m128i T3_04A = _mm_srai_epi32(T2_04A, nShift);                   // [33 24 14 04]
			const __m128i T3_04B = _mm_srai_epi32(T2_04B, nShift);                   // [74 64 54 44]
			const __m128i T3_05A = _mm_srai_epi32(T2_05A, nShift);                   // [35 25 15 05]
			const __m128i T3_05B = _mm_srai_epi32(T2_05B, nShift);                   // [75 65 55 45]
			const __m128i T3_06A = _mm_srai_epi32(T2_06A, nShift);                   // [36 26 16 06]
			const __m128i T3_06B = _mm_srai_epi32(T2_06B, nShift);                   // [76 66 56 46]
			const __m128i T3_07A = _mm_srai_epi32(T2_07A, nShift);                   // [37 27 17 07]
			const __m128i T3_07B = _mm_srai_epi32(T2_07B, nShift);                   // [77 67 57 47]
			const __m128i T3_08A = _mm_srai_epi32(T2_08A, nShift);                   // [30 20 10 00] x8
			const __m128i T3_08B = _mm_srai_epi32(T2_08B, nShift);                   // [70 60 50 40]
			const __m128i T3_09A = _mm_srai_epi32(T2_09A, nShift);                   // [31 21 11 01] x9 
			const __m128i T3_09B = _mm_srai_epi32(T2_09B, nShift);                   // [71 61 51 41]
			const __m128i T3_10A = _mm_srai_epi32(T2_10A, nShift);                   // [32 22 12 02] xA
			const __m128i T3_10B = _mm_srai_epi32(T2_10B, nShift);                   // [72 62 52 42]
			const __m128i T3_11A = _mm_srai_epi32(T2_11A, nShift);                   // [33 23 13 03] xB
			const __m128i T3_11B = _mm_srai_epi32(T2_11B, nShift);                   // [73 63 53 43]
			const __m128i T3_12A = _mm_srai_epi32(T2_12A, nShift);                   // [33 24 14 04] xC
			const __m128i T3_12B = _mm_srai_epi32(T2_12B, nShift);                   // [74 64 54 44]
			const __m128i T3_13A = _mm_srai_epi32(T2_13A, nShift);                   // [35 25 15 05] xD
			const __m128i T3_13B = _mm_srai_epi32(T2_13B, nShift);                   // [75 65 55 45]
			const __m128i T3_14A = _mm_srai_epi32(T2_14A, nShift);                   // [36 26 16 06] xE
			const __m128i T3_14B = _mm_srai_epi32(T2_14B, nShift);                   // [76 66 56 46]
			const __m128i T3_15A = _mm_srai_epi32(T2_15A, nShift);                   // [37 27 17 07] xF
			const __m128i T3_15B = _mm_srai_epi32(T2_15B, nShift);                   // [77 67 57 47]

			const __m128i T3_16A = _mm_srai_epi32(T2_16A, nShift);                   // [30 20 10 00]
			const __m128i T3_16B = _mm_srai_epi32(T2_16B, nShift);                   // [70 60 50 40]
			const __m128i T3_17A = _mm_srai_epi32(T2_17A, nShift);                   // [31 21 11 01]
			const __m128i T3_17B = _mm_srai_epi32(T2_17B, nShift);                   // [71 61 51 41]
			const __m128i T3_18A = _mm_srai_epi32(T2_18A, nShift);                   // [32 22 12 02]
			const __m128i T3_18B = _mm_srai_epi32(T2_18B, nShift);                   // [72 62 52 42]
			const __m128i T3_19A = _mm_srai_epi32(T2_19A, nShift);                   // [33 23 13 03]
			const __m128i T3_19B = _mm_srai_epi32(T2_19B, nShift);                   // [73 63 53 43]
			const __m128i T3_20A = _mm_srai_epi32(T2_20A, nShift);                   // [33 24 14 04]
			const __m128i T3_20B = _mm_srai_epi32(T2_20B, nShift);                   // [74 64 54 44]
			const __m128i T3_21A = _mm_srai_epi32(T2_21A, nShift);                   // [35 25 15 05]
			const __m128i T3_21B = _mm_srai_epi32(T2_21B, nShift);                   // [75 65 55 45]
			const __m128i T3_22A = _mm_srai_epi32(T2_22A, nShift);                   // [36 26 16 06]
			const __m128i T3_22B = _mm_srai_epi32(T2_22B, nShift);                   // [76 66 56 46]
			const __m128i T3_23A = _mm_srai_epi32(T2_23A, nShift);                   // [37 27 17 07]
			const __m128i T3_23B = _mm_srai_epi32(T2_23B, nShift);                   // [77 67 57 47]
			const __m128i T3_24A = _mm_srai_epi32(T2_24A, nShift);                   // [30 20 10 00] x8
			const __m128i T3_24B = _mm_srai_epi32(T2_24B, nShift);                   // [70 60 50 40]
			const __m128i T3_25A = _mm_srai_epi32(T2_25A, nShift);                   // [31 21 11 01] x9 
			const __m128i T3_25B = _mm_srai_epi32(T2_25B, nShift);                   // [71 61 51 41]
			const __m128i T3_26A = _mm_srai_epi32(T2_26A, nShift);                   // [32 22 12 02] xA
			const __m128i T3_26B = _mm_srai_epi32(T2_26B, nShift);                   // [72 62 52 42]
			const __m128i T3_27A = _mm_srai_epi32(T2_27A, nShift);                   // [33 23 13 03] xB
			const __m128i T3_27B = _mm_srai_epi32(T2_27B, nShift);                   // [73 63 53 43]
			const __m128i T3_28A = _mm_srai_epi32(T2_28A, nShift);                   // [33 24 14 04] xC
			const __m128i T3_28B = _mm_srai_epi32(T2_28B, nShift);                   // [74 64 54 44]
			const __m128i T3_29A = _mm_srai_epi32(T2_29A, nShift);                   // [35 25 15 05] xD
			const __m128i T3_29B = _mm_srai_epi32(T2_29B, nShift);                   // [75 65 55 45]
			const __m128i T3_30A = _mm_srai_epi32(T2_30A, nShift);                   // [36 26 16 06] xE
			const __m128i T3_30B = _mm_srai_epi32(T2_30B, nShift);                   // [76 66 56 46]
			const __m128i T3_31A = _mm_srai_epi32(T2_31A, nShift);                   // [37 27 17 07] xF
			const __m128i T3_31B = _mm_srai_epi32(T2_31B, nShift);                   // [77 67 57 47]

			res00[part]  = _mm_packs_epi32(T3_00A, T3_00B);              // [70 60 50 40 30 20 10 00]
			res01[part]  = _mm_packs_epi32(T3_01A, T3_01B);              // [71 61 51 41 31 21 11 01]
			res02[part]  = _mm_packs_epi32(T3_02A, T3_02B);              // [72 62 52 42 32 22 12 02]
			res03[part]  = _mm_packs_epi32(T3_03A, T3_03B);              // [73 63 53 43 33 23 13 03]
			res04[part]  = _mm_packs_epi32(T3_04A, T3_04B);              // [74 64 54 44 34 24 14 04]
			res05[part]  = _mm_packs_epi32(T3_05A, T3_05B);              // [75 65 55 45 35 25 15 05]
			res06[part]  = _mm_packs_epi32(T3_06A, T3_06B);              // [76 66 56 46 36 26 16 06]
			res07[part]  = _mm_packs_epi32(T3_07A, T3_07B);              // [77 67 57 47 37 27 17 07]
			res08[part]  = _mm_packs_epi32(T3_08A, T3_08B);              // [A0 ... 80]
			res09[part]  = _mm_packs_epi32(T3_09A, T3_09B);              // [A1 ... 81]
			res10[part]  = _mm_packs_epi32(T3_10A, T3_10B);              // [A2 ... 82]
			res11[part]  = _mm_packs_epi32(T3_11A, T3_11B);              // [A3 ... 83]
			res12[part]  = _mm_packs_epi32(T3_12A, T3_12B);              // [A4 ... 84]
			res13[part]  = _mm_packs_epi32(T3_13A, T3_13B);              // [A5 ... 85]
			res14[part]  = _mm_packs_epi32(T3_14A, T3_14B);              // [A6 ... 86]
			res15[part]  = _mm_packs_epi32(T3_15A, T3_15B);              // [A7 ... 87] 
			res16[part]  = _mm_packs_epi32(T3_16A, T3_16B);              
			res17[part]  = _mm_packs_epi32(T3_17A, T3_17B);             
			res18[part]  = _mm_packs_epi32(T3_18A, T3_18B);            
			res19[part]  = _mm_packs_epi32(T3_19A, T3_19B);             
			res20[part]  = _mm_packs_epi32(T3_20A, T3_20B);             
			res21[part]  = _mm_packs_epi32(T3_21A, T3_21B);            
			res22[part]  = _mm_packs_epi32(T3_22A, T3_22B);          
			res23[part]  = _mm_packs_epi32(T3_23A, T3_23B);            
			res24[part]  = _mm_packs_epi32(T3_24A, T3_24B);            
			res25[part]  = _mm_packs_epi32(T3_25A, T3_25B);            
			res26[part]  = _mm_packs_epi32(T3_26A, T3_26B);            
			res27[part]  = _mm_packs_epi32(T3_27A, T3_27B);            
			res28[part]  = _mm_packs_epi32(T3_28A, T3_28B);          
			res29[part]  = _mm_packs_epi32(T3_29A, T3_29B);        
			res30[part]  = _mm_packs_epi32(T3_30A, T3_30B);              
			res31[part]  = _mm_packs_epi32(T3_31A, T3_31B);           
		}
		//transpose matrix 8x8 16bit. 
		{
			__m128i tr0_0, tr0_1, tr0_2, tr0_3, tr0_4, tr0_5, tr0_6, tr0_7; 
			__m128i tr1_0, tr1_1, tr1_2, tr1_3, tr1_4, tr1_5, tr1_6, tr1_7; 
#define TRANSPOSE_8x8_16BIT(I0, I1, I2, I3, I4, I5, I6, I7, O0, O1, O2, O3, O4, O5, O6, O7)\
	tr0_0 = _mm_unpacklo_epi16(I0, I1);\
	tr0_1 = _mm_unpacklo_epi16(I2, I3);\
	tr0_2 = _mm_unpackhi_epi16(I0, I1);\
	tr0_3 = _mm_unpackhi_epi16(I2, I3);\
	tr0_4 = _mm_unpacklo_epi16(I4, I5);\
	tr0_5 = _mm_unpacklo_epi16(I6, I7);\
	tr0_6 = _mm_unpackhi_epi16(I4, I5);\
	tr0_7 = _mm_unpackhi_epi16(I6, I7);\
	tr1_0 = _mm_unpacklo_epi32(tr0_0, tr0_1);\
	tr1_1 = _mm_unpacklo_epi32(tr0_2, tr0_3);\
	tr1_2 = _mm_unpackhi_epi32(tr0_0, tr0_1);\
	tr1_3 = _mm_unpackhi_epi32(tr0_2, tr0_3);\
	tr1_4 = _mm_unpacklo_epi32(tr0_4, tr0_5);\
	tr1_5 = _mm_unpacklo_epi32(tr0_6, tr0_7);\
	tr1_6 = _mm_unpackhi_epi32(tr0_4, tr0_5);\
	tr1_7 = _mm_unpackhi_epi32(tr0_6, tr0_7);\
	O0 = _mm_unpacklo_epi64(tr1_0, tr1_4);\
	O1 = _mm_unpackhi_epi64(tr1_0, tr1_4);\
	O2 = _mm_unpacklo_epi64(tr1_2, tr1_6);\
	O3 = _mm_unpackhi_epi64(tr1_2, tr1_6);\
	O4 = _mm_unpacklo_epi64(tr1_1, tr1_5);\
	O5 = _mm_unpackhi_epi64(tr1_1, tr1_5);\
	O6 = _mm_unpacklo_epi64(tr1_3, tr1_7);\
	O7 = _mm_unpackhi_epi64(tr1_3, tr1_7);\

			TRANSPOSE_8x8_16BIT(res00[0],res01[0],res02[0],res03[0],res04[0],res05[0],res06[0],res07[0],in00[0],in01[0],in02[0],in03[0],in04[0],in05[0],in06[0],in07[0])
				TRANSPOSE_8x8_16BIT(res00[1],res01[1],res02[1],res03[1],res04[1],res05[1],res06[1],res07[1],in08[0],in09[0],in10[0],in11[0],in12[0],in13[0],in14[0],in15[0])
				TRANSPOSE_8x8_16BIT(res00[2],res01[2],res02[2],res03[2],res04[2],res05[2],res06[2],res07[2],in16[0],in17[0],in18[0],in19[0],in20[0],in21[0],in22[0],in23[0])
				TRANSPOSE_8x8_16BIT(res00[3],res01[3],res02[3],res03[3],res04[3],res05[3],res06[3],res07[3],in24[0],in25[0],in26[0],in27[0],in28[0],in29[0],in30[0],in31[0])

				TRANSPOSE_8x8_16BIT(res08[0],res09[0],res10[0],res11[0],res12[0],res13[0],res14[0],res15[0],in00[1],in01[1],in02[1],in03[1],in04[1],in05[1],in06[1],in07[1])
				TRANSPOSE_8x8_16BIT(res08[1],res09[1],res10[1],res11[1],res12[1],res13[1],res14[1],res15[1],in08[1],in09[1],in10[1],in11[1],in12[1],in13[1],in14[1],in15[1])
				TRANSPOSE_8x8_16BIT(res08[2],res09[2],res10[2],res11[2],res12[2],res13[2],res14[2],res15[2],in16[1],in17[1],in18[1],in19[1],in20[1],in21[1],in22[1],in23[1])
				TRANSPOSE_8x8_16BIT(res08[3],res09[3],res10[3],res11[3],res12[3],res13[3],res14[3],res15[3],in24[1],in25[1],in26[1],in27[1],in28[1],in29[1],in30[1],in31[1])

				TRANSPOSE_8x8_16BIT(res16[0],res17[0],res18[0],res19[0],res20[0],res21[0],res22[0],res23[0],in00[2],in01[2],in02[2],in03[2],in04[2],in05[2],in06[2],in07[2])
				TRANSPOSE_8x8_16BIT(res16[1],res17[1],res18[1],res19[1],res20[1],res21[1],res22[1],res23[1],in08[2],in09[2],in10[2],in11[2],in12[2],in13[2],in14[2],in15[2])
				TRANSPOSE_8x8_16BIT(res16[2],res17[2],res18[2],res19[2],res20[2],res21[2],res22[2],res23[2],in16[2],in17[2],in18[2],in19[2],in20[2],in21[2],in22[2],in23[2])
				TRANSPOSE_8x8_16BIT(res16[3],res17[3],res18[3],res19[3],res20[3],res21[3],res22[3],res23[3],in24[2],in25[2],in26[2],in27[2],in28[2],in29[2],in30[2],in31[2])

				TRANSPOSE_8x8_16BIT(res24[0],res25[0],res26[0],res27[0],res28[0],res29[0],res30[0],res31[0],in00[3],in01[3],in02[3],in03[3],in04[3],in05[3],in06[3],in07[3])
				TRANSPOSE_8x8_16BIT(res24[1],res25[1],res26[1],res27[1],res28[1],res29[1],res30[1],res31[1],in08[3],in09[3],in10[3],in11[3],in12[3],in13[3],in14[3],in15[3])
				TRANSPOSE_8x8_16BIT(res24[2],res25[2],res26[2],res27[2],res28[2],res29[2],res30[2],res31[2],in16[3],in17[3],in18[3],in19[3],in20[3],in21[3],in22[3],in23[3])
				TRANSPOSE_8x8_16BIT(res24[3],res25[3],res26[3],res27[3],res28[3],res29[3],res30[3],res31[3],in24[3],in25[3],in26[3],in27[3],in28[3],in29[3],in30[3],in31[3])

#undef TRANSPOSE_8x8_16BIT
		}
	}

	// Add
	for (Int i=0; i<2; i++){
		__m128i T18_00,T18_01,T18_02,T18_03,T18_04,T18_05,T18_06,T18_07;
		__m128i T19_00a,T19_01a,T19_02a,T19_03a,T19_04a,T19_05a,T19_06a,T19_07a;
		__m128i T19_00b,T19_01b,T19_02b,T19_03b,T19_04b,T19_05b,T19_06b,T19_07b;
		__m128i T210,T211,T212,T213,T214,T215,T216,T217;
		__m128i T200a,T201a,T202a,T203a,T204a,T205a,T206a,T207a;
		__m128i T200b,T201b,T202b,T203b,T204b,T205b,T206b,T207b;
#define STROE_LINE(L0,L1,L2,L3,L4,L5,L6,L7, H0,H1,H2,H3,H4,H5,H6,H7, offsetV, offsetH)\
	T18_00 = _mm_loadu_si128((const __m128i*)&pRef[(0+offsetV)*nStride+offsetH]);\
	T18_01 = _mm_loadu_si128((const __m128i*)&pRef[(1+offsetV)*nStride+offsetH]);\
	T18_02 = _mm_loadu_si128((const __m128i*)&pRef[(2+offsetV)*nStride+offsetH]);\
	T18_03 = _mm_loadu_si128((const __m128i*)&pRef[(3+offsetV)*nStride+offsetH]);\
	T18_04 = _mm_loadu_si128((const __m128i*)&pRef[(4+offsetV)*nStride+offsetH]);\
	T18_05 = _mm_loadu_si128((const __m128i*)&pRef[(5+offsetV)*nStride+offsetH]);\
	T18_06 = _mm_loadu_si128((const __m128i*)&pRef[(6+offsetV)*nStride+offsetH]);\
	T18_07 = _mm_loadu_si128((const __m128i*)&pRef[(7+offsetV)*nStride+offsetH]);\
	T19_00a = _mm_unpacklo_epi8(T18_00, c_zero);\
	T19_01a = _mm_unpacklo_epi8(T18_01, c_zero);\
	T19_02a = _mm_unpacklo_epi8(T18_02, c_zero);\
	T19_03a = _mm_unpacklo_epi8(T18_03, c_zero);\
	T19_04a = _mm_unpacklo_epi8(T18_04, c_zero);\
	T19_05a = _mm_unpacklo_epi8(T18_05, c_zero);\
	T19_06a = _mm_unpacklo_epi8(T18_06, c_zero);\
	T19_07a = _mm_unpacklo_epi8(T18_07, c_zero);\
	T19_00b = _mm_unpackhi_epi8(T18_00, c_zero);\
	T19_01b = _mm_unpackhi_epi8(T18_01, c_zero);\
	T19_02b = _mm_unpackhi_epi8(T18_02, c_zero);\
	T19_03b = _mm_unpackhi_epi8(T18_03, c_zero);\
	T19_04b = _mm_unpackhi_epi8(T18_04, c_zero);\
	T19_05b = _mm_unpackhi_epi8(T18_05, c_zero);\
	T19_06b = _mm_unpackhi_epi8(T18_06, c_zero);\
	T19_07b = _mm_unpackhi_epi8(T18_07, c_zero);\
	\
	T200a = _mm_add_epi16(L0, T19_00a);\
	T201a = _mm_add_epi16(L1, T19_01a);\
	T202a = _mm_add_epi16(L2, T19_02a);\
	T203a = _mm_add_epi16(L3, T19_03a);\
	T204a = _mm_add_epi16(L4, T19_04a);\
	T205a = _mm_add_epi16(L5, T19_05a);\
	T206a = _mm_add_epi16(L6, T19_06a);\
	T207a = _mm_add_epi16(L7, T19_07a);\
	T200b = _mm_add_epi16(H0, T19_00b);\
	T201b = _mm_add_epi16(H1, T19_01b);\
	T202b = _mm_add_epi16(H2, T19_02b);\
	T203b = _mm_add_epi16(H3, T19_03b);\
	T204b = _mm_add_epi16(H4, T19_04b);\
	T205b = _mm_add_epi16(H5, T19_05b);\
	T206b = _mm_add_epi16(H6, T19_06b);\
	T207b = _mm_add_epi16(H7, T19_07b);\
	T210 = _mm_packus_epi16(T200a, T200b);\
	T211 = _mm_packus_epi16(T201a, T201b);\
	T212 = _mm_packus_epi16(T202a, T202b);\
	T213 = _mm_packus_epi16(T203a, T203b);\
	T214 = _mm_packus_epi16(T204a, T204b);\
	T215 = _mm_packus_epi16(T205a, T205b);\
	T216 = _mm_packus_epi16(T206a, T206b);\
	T217 = _mm_packus_epi16(T207a, T207b);\
	_mm_storeu_si128( (__m128i *)&pDst[(0+offsetV)*nStrideDst+offsetH], T210);\
	_mm_storeu_si128( (__m128i *)&pDst[(1+offsetV)*nStrideDst+offsetH], T211);\
	_mm_storeu_si128( (__m128i *)&pDst[(2+offsetV)*nStrideDst+offsetH], T212);\
	_mm_storeu_si128( (__m128i *)&pDst[(3+offsetV)*nStrideDst+offsetH], T213);\
	_mm_storeu_si128( (__m128i *)&pDst[(4+offsetV)*nStrideDst+offsetH], T214);\
	_mm_storeu_si128( (__m128i *)&pDst[(5+offsetV)*nStrideDst+offsetH], T215);\
	_mm_storeu_si128( (__m128i *)&pDst[(6+offsetV)*nStrideDst+offsetH], T216);\
	_mm_storeu_si128( (__m128i *)&pDst[(7+offsetV)*nStrideDst+offsetH], T217);

		Int k = i*2;
		Int offsetH = i*16;
		STROE_LINE(in00[k],in01[k],in02[k],in03[k],in04[k],in05[k],in06[k],in07[k], in00[k+1],in01[k+1],in02[k+1],in03[k+1],in04[k+1],in05[k+1],in06[k+1],in07[k+1], 0 ,offsetH)
			STROE_LINE(in08[k],in09[k],in10[k],in11[k],in12[k],in13[k],in14[k],in15[k], in08[k+1],in09[k+1],in10[k+1],in11[k+1],in12[k+1],in13[k+1],in14[k+1],in15[k+1], 8 ,offsetH)
			STROE_LINE(in16[k],in17[k],in18[k],in19[k],in20[k],in21[k],in22[k],in23[k], in16[k+1],in17[k+1],in18[k+1],in19[k+1],in20[k+1],in21[k+1],in22[k+1],in23[k+1], 16,offsetH)
			STROE_LINE(in24[k],in25[k],in26[k],in27[k],in28[k],in29[k],in30[k],in31[k], in24[k+1],in25[k+1],in26[k+1],in27[k+1],in28[k+1],in29[k+1],in30[k+1],in31[k+1], 24,offsetH)
#undef STROE_LINE
	}
}
#else
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
#endif

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


#if (INTRA_PRED_USE_ASM >= ASM_SSE4)
static const __m128i c_mask = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
void xPredIntraPlanar4(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride
	)
{
	UInt8 *pucLeft = pucRef + 2 * 4 - 1;
	UInt8 *pucTop  = pucRef + 2 * 4 + 1;
	UInt8 bottomLeft, topRight;

	// Get left and above reference column and row
	__m128i T00         = _mm_cvtsi32_si128(*(int*)pucTop);
	__m128i v_topRow    = _mm_unpacklo_epi8(T00, _mm_setzero_si128());
	__m128i T02         = _mm_cvtsi32_si128(*(int*)(pucLeft-3));
	__m128i T03         = _mm_unpacklo_epi8(T02, _mm_setzero_si128());
	__m128i v_leftColumn= _mm_shufflelo_epi16(T03, 0x1B);

	// Prepare intermediate variables used in interpolation
	bottomLeft = pucLeft[-4];
	topRight   = pucTop[4];

	__m128i v_bottomLeft    = _mm_set1_epi16(bottomLeft);
	__m128i v_topRight      = _mm_set1_epi16(topRight);
	__m128i v_bottomRow     = _mm_sub_epi16(v_bottomLeft, v_topRow);
	__m128i v_rightColumn   = _mm_sub_epi16(v_topRight, v_leftColumn);
	v_topRow     = _mm_slli_epi16(v_topRow, 2);
	v_leftColumn = _mm_slli_epi16(v_leftColumn, 2);
	__m128i v_horPred4      = _mm_add_epi16(v_leftColumn, _mm_set1_epi16(4));
	const __m128i v_multi   = _mm_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8);
	__m128i v_horPred, v_rightColumnN;
	__m128i T10, T11;

	// line0
#define COMP_PRED_PLANAR_ROW(x) \
	v_horPred = _mm_shufflelo_epi16(v_horPred4, (x)*0x55); \
	v_rightColumnN = _mm_mullo_epi16(_mm_shufflelo_epi16(v_rightColumn, (x)*0x55), v_multi); \
	v_horPred = _mm_add_epi16(v_horPred, v_rightColumnN); \
	v_topRow = _mm_add_epi16(v_topRow, v_bottomRow); \
	T10 = _mm_srai_epi16(_mm_add_epi16(v_horPred, v_topRow), 3); \
	T11 = _mm_packus_epi16(T10, T10); \
	*(int*)&pucDst[(x) * nDstStride] = _mm_cvtsi128_si32(T11);

	// Generate prediction signal
	COMP_PRED_PLANAR_ROW(0)
		COMP_PRED_PLANAR_ROW(1)
		COMP_PRED_PLANAR_ROW(2)
		COMP_PRED_PLANAR_ROW(3)

#undef COMP_PRED_PLANAR_ROW
}

#pragma warning(disable:4556)   // value of intrinsic immediate argument '510' is out of range '0 - 255'
#pragma warning(disable:4127)   // conditional expression is constant

#define BROADCAST8(d, s, x)  {                          \
	__m128i _tmp;                                       \
	if ((x)<4) {                                        \
	_tmp = _mm_shufflelo_epi16((s), (x)*0x55);      \
	(d) = _mm_unpacklo_epi64(_tmp, _tmp);           \
	}                                                   \
	else {                                              \
	_tmp = _mm_shufflehi_epi16((s), ((x)-4)*0x55);  \
	(d) = _mm_unpackhi_epi64(_tmp, _tmp);           \
	}                                                   \
	}

void xPredIntraPlanar8(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride
	)
{
	UInt8 *pucLeft = pucRef + 2 * 8 - 1;
	UInt8 *pucTop  = pucRef + 2 * 8 + 1;
	UInt8 bottomLeft, topRight;

	// Get left and above reference column and row
	__m128i T00         = _mm_loadl_epi64((__m128i*)pucTop);
	__m128i v_topRow    = _mm_unpacklo_epi8(T00, _mm_setzero_si128());
	__m128i T02         = _mm_loadl_epi64((__m128i*)(pucLeft-7));
	__m128i T03         = _mm_unpacklo_epi8(T02, _mm_setzero_si128());
	__m128i v_leftColumn;
	v_leftColumn = _mm_shufflelo_epi16(T03, 0x1B);
	v_leftColumn = _mm_shufflehi_epi16(v_leftColumn, 0x1B);
	v_leftColumn = _mm_shuffle_epi32(v_leftColumn, 0x4E);

	// Prepare intermediate variables used in interpolation
	bottomLeft = pucLeft[-8];
	topRight   = pucTop[8];

	__m128i v_bottomLeft    = _mm_set1_epi16(bottomLeft);
	__m128i v_topRight      = _mm_set1_epi16(topRight);
	__m128i v_bottomRow     = _mm_sub_epi16(v_bottomLeft, v_topRow);
	__m128i v_rightColumn   = _mm_sub_epi16(v_topRight, v_leftColumn);
	v_topRow     = _mm_slli_epi16(v_topRow, 3);
	v_leftColumn = _mm_slli_epi16(v_leftColumn, 3);
	__m128i v_horPred4      = _mm_add_epi16(v_leftColumn, _mm_set1_epi16(8));
	const __m128i v_multi   = _mm_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8);
	__m128i v_horPred, v_rightColumnN;
	__m128i T10, T11;

	// line0
#define COMP_PRED_PLANAR_ROW(x) \
	BROADCAST8(v_horPred, v_horPred4, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumn, (x)); \
	v_rightColumnN = _mm_mullo_epi16(v_rightColumnN, v_multi); \
	v_horPred = _mm_add_epi16(v_horPred, v_rightColumnN); \
	v_topRow = _mm_add_epi16(v_topRow, v_bottomRow); \
	T10 = _mm_srai_epi16(_mm_add_epi16(v_horPred, v_topRow), 4); \
	T11 = _mm_packus_epi16(T10, T10); \
	_mm_storel_epi64((__m128i*)&pucDst[(x) * nDstStride], T11);

	// Generate prediction signal
	COMP_PRED_PLANAR_ROW(0)
		COMP_PRED_PLANAR_ROW(1)
		COMP_PRED_PLANAR_ROW(2)
		COMP_PRED_PLANAR_ROW(3)
		COMP_PRED_PLANAR_ROW(4)
		COMP_PRED_PLANAR_ROW(5)
		COMP_PRED_PLANAR_ROW(6)
		COMP_PRED_PLANAR_ROW(7)

#undef COMP_PRED_PLANAR_ROW
}

void xPredIntraPlanar16(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride
	)
{
	UInt8 *pucLeft = pucRef + 2 * 16 - 1;
	UInt8 *pucTop  = pucRef + 2 * 16 + 1;
	UInt8 bottomLeft, topRight;

	//Get left and above reference colum and row
	__m128i T00             = _mm_loadu_si128((__m128i*)pucTop);
	__m128i v_topRowA       = _mm_unpacklo_epi8(T00, _mm_setzero_si128());
	__m128i v_topRowB       = _mm_unpackhi_epi8(T00, _mm_setzero_si128());
	__m128i T01             = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(pucLeft-15)), c_mask);
	__m128i v_leftColumnA   = _mm_unpacklo_epi8(T01, _mm_setzero_si128());
	__m128i v_leftColumnB   = _mm_unpackhi_epi8(T01, _mm_setzero_si128());

	//Prepare intermediate variables used in interpolation
	bottomLeft = pucLeft[-16];
	topRight   = pucTop[16];

	__m128i v_bottomLeft   = _mm_set1_epi16(bottomLeft);
	__m128i v_topRight     = _mm_set1_epi16(topRight);
	__m128i v_bottomRowA   = _mm_sub_epi16(v_bottomLeft, v_topRowA);
	__m128i v_bottomRowB   = _mm_sub_epi16(v_bottomLeft, v_topRowB);
	__m128i v_rightColumnA = _mm_sub_epi16(v_topRight, v_leftColumnA);
	__m128i v_rightColumnB = _mm_sub_epi16(v_topRight, v_leftColumnB);
	v_topRowA           = _mm_slli_epi16(v_topRowA, 4);
	v_topRowB           = _mm_slli_epi16(v_topRowB, 4);
	v_leftColumnA = _mm_slli_epi16(v_leftColumnA, 4);
	v_leftColumnB = _mm_slli_epi16(v_leftColumnB, 4);
	__m128i v_horPredBaseA  = _mm_add_epi16(v_leftColumnA, _mm_set1_epi16(16));
	__m128i v_horPredBaseB  = _mm_add_epi16(v_leftColumnB, _mm_set1_epi16(16));
	const __m128i v_multiA  = _mm_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8);
	const __m128i v_multiB  = _mm_setr_epi16(9,10,11,12,13,14,15,16);
	__m128i v_horPred, v_rightColumnN, v_horPredA, v_rightColumnNA, v_horPredB, v_rightColumnNB;
	__m128i T10A, T10B, T11;

#define COMP_PRED_PLANAR_ROW(x, part) \
	BROADCAST8(v_horPred, v_horPredBase ## part, (x)&7); \
	BROADCAST8(v_rightColumnN, v_rightColumn ## part, (x)&7); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiA); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiB); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v_topRowA  = _mm_add_epi16(v_topRowA, v_bottomRowA); \
	v_topRowB  = _mm_add_epi16(v_topRowB, v_bottomRowB); \
	T10A = _mm_srai_epi16(_mm_add_epi16(v_horPredA, v_topRowA), 5); \
	T10B = _mm_srai_epi16(_mm_add_epi16(v_horPredB, v_topRowB), 5); \
	T11  = _mm_packus_epi16(T10A, T10B); \
	_mm_storeu_si128((__m128i*)&pucDst[(x) * nDstStride], T11);

	// Generate prediction signal
	COMP_PRED_PLANAR_ROW( 0, A)
		COMP_PRED_PLANAR_ROW( 1, A)
		COMP_PRED_PLANAR_ROW( 2, A)
		COMP_PRED_PLANAR_ROW( 3, A)
		COMP_PRED_PLANAR_ROW( 4, A)
		COMP_PRED_PLANAR_ROW( 5, A)
		COMP_PRED_PLANAR_ROW( 6, A)
		COMP_PRED_PLANAR_ROW( 7, A)

		COMP_PRED_PLANAR_ROW( 8, B)
		COMP_PRED_PLANAR_ROW( 9, B)
		COMP_PRED_PLANAR_ROW(10, B)
		COMP_PRED_PLANAR_ROW(11, B)
		COMP_PRED_PLANAR_ROW(12, B)
		COMP_PRED_PLANAR_ROW(13, B)
		COMP_PRED_PLANAR_ROW(14, B)
		COMP_PRED_PLANAR_ROW(15, B)
#undef COMP_PRED_PLANAR_ROW
}

void xPredIntraPlanar32(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride
	)
{
	UInt8 *pucLeft = pucRef + 2 * 32 - 1;
	UInt8 *pucTop  = pucRef + 2 * 32 + 1;
	UInt8 bottomLeft, topRight;

	//Get left and above reference colum and row
	__m128i T00A            = _mm_loadu_si128((__m128i*)(pucTop   ));
	__m128i T00B            = _mm_loadu_si128((__m128i*)(pucTop+16));
	__m128i v_topRowA       = _mm_unpacklo_epi8(T00A, _mm_setzero_si128());
	__m128i v_topRowB       = _mm_unpackhi_epi8(T00A, _mm_setzero_si128());
	__m128i v_topRowC       = _mm_unpacklo_epi8(T00B, _mm_setzero_si128());
	__m128i v_topRowD       = _mm_unpackhi_epi8(T00B, _mm_setzero_si128());
	__m128i c_mask          = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
	__m128i T01A            = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(pucLeft-15)), c_mask);
	__m128i T01B            = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(pucLeft-31)), c_mask);
	__m128i v_leftColumnA   = _mm_unpacklo_epi8(T01A, _mm_setzero_si128());
	__m128i v_leftColumnB   = _mm_unpackhi_epi8(T01A, _mm_setzero_si128());
	__m128i v_leftColumnC   = _mm_unpacklo_epi8(T01B, _mm_setzero_si128());
	__m128i v_leftColumnD   = _mm_unpackhi_epi8(T01B, _mm_setzero_si128());

	//Prepare intermediate variables used in interpolation
	bottomLeft = pucLeft[-32];
	topRight   = pucTop[32];

	__m128i v_bottomLeft  = _mm_set1_epi16(bottomLeft);
	__m128i v_topRight    = _mm_set1_epi16(topRight);
	__m128i v_bottomRowA  = _mm_sub_epi16(v_bottomLeft, v_topRowA);
	__m128i v_bottomRowB  = _mm_sub_epi16(v_bottomLeft, v_topRowB);
	__m128i v_bottomRowC  = _mm_sub_epi16(v_bottomLeft, v_topRowC);
	__m128i v_bottomRowD  = _mm_sub_epi16(v_bottomLeft, v_topRowD);
	__m128i v_rightColumnA = _mm_sub_epi16(v_topRight, v_leftColumnA);
	__m128i v_rightColumnB = _mm_sub_epi16(v_topRight, v_leftColumnB);
	__m128i v_rightColumnC = _mm_sub_epi16(v_topRight, v_leftColumnC);
	__m128i v_rightColumnD = _mm_sub_epi16(v_topRight, v_leftColumnD);
	v_topRowA = _mm_slli_epi16(v_topRowA, 5);
	v_topRowB = _mm_slli_epi16(v_topRowB, 5);
	v_topRowC = _mm_slli_epi16(v_topRowC, 5);
	v_topRowD = _mm_slli_epi16(v_topRowD, 5);
	v_leftColumnA = _mm_slli_epi16(v_leftColumnA, 5);
	v_leftColumnB = _mm_slli_epi16(v_leftColumnB, 5);
	v_leftColumnC = _mm_slli_epi16(v_leftColumnC, 5);
	v_leftColumnD = _mm_slli_epi16(v_leftColumnD, 5);
	__m128i v_horPredBaseA = _mm_add_epi16(v_leftColumnA, _mm_set1_epi16(32));
	__m128i v_horPredBaseB = _mm_add_epi16(v_leftColumnB, _mm_set1_epi16(32));
	__m128i v_horPredBaseC = _mm_add_epi16(v_leftColumnC, _mm_set1_epi16(32));
	__m128i v_horPredBaseD = _mm_add_epi16(v_leftColumnD, _mm_set1_epi16(32));
	const __m128i v_multiA  = _mm_setr_epi16( 1, 2, 3, 4, 5, 6, 7, 8);
	const __m128i v_multiB  = _mm_setr_epi16( 9,10,11,12,13,14,15,16);
	const __m128i v_multiC  = _mm_setr_epi16(17,18,19,20,21,22,23,24);
	const __m128i v_multiD  = _mm_setr_epi16(25,26,27,28,29,30,31,32);
	__m128i v_horPred, v_rightColumnN, v_horPredA, v_rightColumnNA, v_horPredB, v_rightColumnNB;
	__m128i T10A, T10B, T11;

	//-------------y=0:16
#define COMP_PRED_PLANAR_ROW(x) \
	BROADCAST8(v_horPred, v_horPredBaseA, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnA, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiA); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiB); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v_topRowA  = _mm_add_epi16(v_topRowA, v_bottomRowA); \
	v_topRowB  = _mm_add_epi16(v_topRowB, v_bottomRowB); \
	T10A = _mm_srai_epi16(_mm_add_epi16(v_horPredA, v_topRowA), 6); \
	T10B = _mm_srai_epi16(_mm_add_epi16(v_horPredB, v_topRowB), 6); \
	T11  = _mm_packus_epi16(T10A, T10B); \
	_mm_storeu_si128((__m128i*)&pucDst[(x) * nDstStride], T11);

	// Generate prediction signal
	COMP_PRED_PLANAR_ROW(0)
		COMP_PRED_PLANAR_ROW(1)
		COMP_PRED_PLANAR_ROW(2)
		COMP_PRED_PLANAR_ROW(3)
		COMP_PRED_PLANAR_ROW(4)
		COMP_PRED_PLANAR_ROW(5)
		COMP_PRED_PLANAR_ROW(6)
		COMP_PRED_PLANAR_ROW(7)
#undef COMP_PRED_PLANAR_ROW

#define COMP_PRED_PLANAR_ROW(x) \
	BROADCAST8(v_horPred, v_horPredBaseA, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnA, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiC); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiD); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v_topRowC  = _mm_add_epi16(v_topRowC, v_bottomRowC); \
	v_topRowD  = _mm_add_epi16(v_topRowD, v_bottomRowD); \
	T10A = _mm_srai_epi16(_mm_add_epi16(v_horPredA, v_topRowC), 6); \
	T10B = _mm_srai_epi16(_mm_add_epi16(v_horPredB, v_topRowD), 6); \
	T11  = _mm_packus_epi16(T10A, T10B); \
	_mm_storeu_si128((__m128i*)&pucDst[(x) * nDstStride + 16], T11);

		// Generate prediction signal
		COMP_PRED_PLANAR_ROW(0)
		COMP_PRED_PLANAR_ROW(1)
		COMP_PRED_PLANAR_ROW(2)
		COMP_PRED_PLANAR_ROW(3)
		COMP_PRED_PLANAR_ROW(4)
		COMP_PRED_PLANAR_ROW(5)
		COMP_PRED_PLANAR_ROW(6)
		COMP_PRED_PLANAR_ROW(7)
#undef COMP_PRED_PLANAR_ROW

#define COMP_PRED_PLANAR_ROW(x) \
	BROADCAST8(v_horPred, v_horPredBaseB, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnB, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiA); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiB); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v_topRowA  = _mm_add_epi16(v_topRowA, v_bottomRowA); \
	v_topRowB  = _mm_add_epi16(v_topRowB, v_bottomRowB); \
	T10A = _mm_srai_epi16(_mm_add_epi16(v_horPredA, v_topRowA), 6); \
	T10B = _mm_srai_epi16(_mm_add_epi16(v_horPredB, v_topRowB), 6); \
	T11  = _mm_packus_epi16(T10A, T10B); \
	_mm_storeu_si128((__m128i*)&pucDst[(8+(x)) * nDstStride], T11);

		// Generate prediction signal
		COMP_PRED_PLANAR_ROW(0)
		COMP_PRED_PLANAR_ROW(1)
		COMP_PRED_PLANAR_ROW(2)
		COMP_PRED_PLANAR_ROW(3)
		COMP_PRED_PLANAR_ROW(4)
		COMP_PRED_PLANAR_ROW(5)
		COMP_PRED_PLANAR_ROW(6)
		COMP_PRED_PLANAR_ROW(7)
#undef COMP_PRED_PLANAR_ROW

#define COMP_PRED_PLANAR_ROW(x) \
	BROADCAST8(v_horPred, v_horPredBaseB, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnB, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiC); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiD); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v_topRowC  = _mm_add_epi16(v_topRowC, v_bottomRowC); \
	v_topRowD  = _mm_add_epi16(v_topRowD, v_bottomRowD); \
	T10A = _mm_srai_epi16(_mm_add_epi16(v_horPredA, v_topRowC), 6); \
	T10B = _mm_srai_epi16(_mm_add_epi16(v_horPredB, v_topRowD), 6); \
	T11  = _mm_packus_epi16(T10A, T10B); \
	_mm_storeu_si128((__m128i*)&pucDst[(8+(x)) * nDstStride + 16], T11);

		// Generate prediction signal
		COMP_PRED_PLANAR_ROW(0)
		COMP_PRED_PLANAR_ROW(1)
		COMP_PRED_PLANAR_ROW(2)
		COMP_PRED_PLANAR_ROW(3)
		COMP_PRED_PLANAR_ROW(4)
		COMP_PRED_PLANAR_ROW(5)
		COMP_PRED_PLANAR_ROW(6)
		COMP_PRED_PLANAR_ROW(7)
#undef COMP_PRED_PLANAR_ROW

		//convert data type from 16bit to 32bit
		__m128i v32_topRowA = _mm_cvtepi16_epi32(v_topRowA);
	__m128i v32_topRowB = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_topRowA, 0x4E));
	__m128i v32_topRowC = _mm_cvtepi16_epi32(v_topRowB);
	__m128i v32_topRowD = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_topRowB, 0x4E));
	__m128i v32_topRowE = _mm_cvtepi16_epi32(v_topRowC);
	__m128i v32_topRowF = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_topRowC, 0x4E));
	__m128i v32_topRowG = _mm_cvtepi16_epi32(v_topRowD);//high address
	__m128i v32_topRowH = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_topRowD, 0x4E));

	__m128i v32_bottomRowA = _mm_cvtepi16_epi32(v_bottomRowA);
	__m128i v32_bottomRowB = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_bottomRowA, 0x4E));
	__m128i v32_bottomRowC = _mm_cvtepi16_epi32(v_bottomRowB);
	__m128i v32_bottomRowD = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_bottomRowB, 0x4E));
	__m128i v32_bottomRowE = _mm_cvtepi16_epi32(v_bottomRowC);
	__m128i v32_bottomRowF = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_bottomRowC, 0x4E));
	__m128i v32_bottomRowG = _mm_cvtepi16_epi32(v_bottomRowD);//high address
	__m128i v32_bottomRowH = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_bottomRowD, 0x4E));

	__m128i v32_horPredA, v32_horPredB, v32_horPredC, v32_horPredD;
	__m128i T11A, T11B, T10C, T10D;

	//--------------------------------y=16:32---------------------------------------
#define COMP_PRED_PLANAR_ROW32(x) \
	BROADCAST8(v_horPred, v_horPredBaseC, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnC, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiA); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiB); \
	v_horPredA          = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB        = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v32_horPredA        = _mm_cvtepi16_epi32(v_horPredA);\
	v32_horPredB        = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredA, 0x4E));\
	v32_horPredC        = _mm_cvtepi16_epi32(v_horPredB);\
	v32_horPredD        = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredB, 0x4E));\
	v32_topRowA         = _mm_add_epi32(v32_topRowA, v32_bottomRowA); \
	v32_topRowB         = _mm_add_epi32(v32_topRowB, v32_bottomRowB); \
	v32_topRowC         = _mm_add_epi32(v32_topRowC, v32_bottomRowC); \
	v32_topRowD         = _mm_add_epi32(v32_topRowD, v32_bottomRowD); \
	T10A = _mm_srai_epi32(_mm_add_epi32(v32_horPredA, v32_topRowA), 6); \
	T10B = _mm_srai_epi32(_mm_add_epi32(v32_horPredB, v32_topRowB), 6); \
	T10C = _mm_srai_epi32(_mm_add_epi32(v32_horPredC, v32_topRowC), 6); \
	T10D = _mm_srai_epi32(_mm_add_epi32(v32_horPredD, v32_topRowD), 6); \
	T11A = _mm_packus_epi32(T10A, T10B); \
	T11B = _mm_packus_epi32(T10C, T10D); \
	T11 = _mm_packus_epi16(T11A, T11B); \
	_mm_storeu_si128((__m128i*)&pucDst[(16+(x)) * nDstStride], T11);

	// Generate prediction signal
	COMP_PRED_PLANAR_ROW32(0)
		COMP_PRED_PLANAR_ROW32(1)
		COMP_PRED_PLANAR_ROW32(2)
		COMP_PRED_PLANAR_ROW32(3)
		COMP_PRED_PLANAR_ROW32(4)
		COMP_PRED_PLANAR_ROW32(5)
		COMP_PRED_PLANAR_ROW32(6)
		COMP_PRED_PLANAR_ROW32(7)
#undef COMP_PRED_PLANAR_ROW32

#define COMP_PRED_PLANAR_ROW32(x) \
	BROADCAST8(v_horPred, v_horPredBaseC, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnC, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiC); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiD); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v32_horPredA = _mm_cvtepi16_epi32(v_horPredA);\
	v32_horPredB = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredA, 0x4E));\
	v32_horPredC = _mm_cvtepi16_epi32(v_horPredB);\
	v32_horPredD = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredB, 0x4E));\
	v32_topRowE = _mm_add_epi32(v32_topRowE, v32_bottomRowE); \
	v32_topRowF = _mm_add_epi32(v32_topRowF, v32_bottomRowF); \
	v32_topRowG = _mm_add_epi32(v32_topRowG, v32_bottomRowG); \
	v32_topRowH = _mm_add_epi32(v32_topRowH, v32_bottomRowH); \
	T10A = _mm_srai_epi32(_mm_add_epi32(v32_horPredA, v32_topRowE), 6); \
	T10B = _mm_srai_epi32(_mm_add_epi32(v32_horPredB, v32_topRowF), 6); \
	T10C = _mm_srai_epi32(_mm_add_epi32(v32_horPredC, v32_topRowG), 6); \
	T10D = _mm_srai_epi32(_mm_add_epi32(v32_horPredD, v32_topRowH), 6); \
	T11A = _mm_packus_epi32(T10A, T10B); \
	T11B = _mm_packus_epi32(T10C, T10D); \
	T11 = _mm_packus_epi16(T11A, T11B); \
	_mm_storeu_si128((__m128i*)&pucDst[(16+(x)) * nDstStride + 16], T11);

		// Generate prediction signal
		COMP_PRED_PLANAR_ROW32(0)
		COMP_PRED_PLANAR_ROW32(1)
		COMP_PRED_PLANAR_ROW32(2)
		COMP_PRED_PLANAR_ROW32(3)
		COMP_PRED_PLANAR_ROW32(4)
		COMP_PRED_PLANAR_ROW32(5)
		COMP_PRED_PLANAR_ROW32(6)
		COMP_PRED_PLANAR_ROW32(7)
#undef COMP_PRED_PLANAR_ROW32

#define COMP_PRED_PLANAR_ROW32(x) \
	BROADCAST8(v_horPred, v_horPredBaseD, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnD, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiA); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiB); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v32_horPredA = _mm_cvtepi16_epi32(v_horPredA);\
	v32_horPredB = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredA, 0x4E));\
	v32_horPredC = _mm_cvtepi16_epi32(v_horPredB);\
	v32_horPredD = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredB, 0x4E));\
	v32_topRowB = _mm_add_epi32(v32_topRowB, v32_bottomRowB); \
	v32_topRowA = _mm_add_epi32(v32_topRowA, v32_bottomRowA); \
	v32_topRowC = _mm_add_epi32(v32_topRowC, v32_bottomRowC); \
	v32_topRowD = _mm_add_epi32(v32_topRowD, v32_bottomRowD); \
	T10A = _mm_srai_epi32(_mm_add_epi32(v32_horPredA, v32_topRowA), 6); \
	T10B = _mm_srai_epi32(_mm_add_epi32(v32_horPredB, v32_topRowB), 6); \
	T10C = _mm_srai_epi32(_mm_add_epi32(v32_horPredC, v32_topRowC), 6); \
	T10D = _mm_srai_epi32(_mm_add_epi32(v32_horPredD, v32_topRowD), 6); \
	T11A = _mm_packus_epi32(T10A, T10B); \
	T11B = _mm_packus_epi32(T10C, T10D); \
	T11 = _mm_packus_epi16(T11A, T11B); \
	_mm_storeu_si128((__m128i*)&pucDst[(24+(x)) * nDstStride], T11);

		// Generate prediction signal
		COMP_PRED_PLANAR_ROW32(0)
		COMP_PRED_PLANAR_ROW32(1)
		COMP_PRED_PLANAR_ROW32(2)
		COMP_PRED_PLANAR_ROW32(3)
		COMP_PRED_PLANAR_ROW32(4)
		COMP_PRED_PLANAR_ROW32(5)
		COMP_PRED_PLANAR_ROW32(6)
		COMP_PRED_PLANAR_ROW32(7)
#undef COMP_PRED_PLANAR_ROW32

#define COMP_PRED_PLANAR_ROW32(x) \
	BROADCAST8(v_horPred, v_horPredBaseD, (x)); \
	BROADCAST8(v_rightColumnN, v_rightColumnD, (x)); \
	v_rightColumnNA = _mm_mullo_epi16(v_rightColumnN, v_multiC); \
	v_rightColumnNB = _mm_mullo_epi16(v_rightColumnN, v_multiD); \
	v_horPredA = _mm_add_epi16(v_horPred, v_rightColumnNA); \
	v_horPredB = _mm_add_epi16(v_horPred, v_rightColumnNB); \
	v32_horPredA = _mm_cvtepi16_epi32(v_horPredA);\
	v32_horPredB = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredA, 0x4E));\
	v32_horPredC = _mm_cvtepi16_epi32(v_horPredB);\
	v32_horPredD = _mm_cvtepi16_epi32(_mm_shuffle_epi32(v_horPredB, 0x4E));\
	v32_topRowE = _mm_add_epi32(v32_topRowE, v32_bottomRowE); \
	v32_topRowF = _mm_add_epi32(v32_topRowF, v32_bottomRowF); \
	v32_topRowG = _mm_add_epi32(v32_topRowG, v32_bottomRowG); \
	v32_topRowH = _mm_add_epi32(v32_topRowH, v32_bottomRowH); \
	T10A = _mm_srai_epi32(_mm_add_epi32(v32_horPredA, v32_topRowE), 6); \
	T10B = _mm_srai_epi32(_mm_add_epi32(v32_horPredB, v32_topRowF), 6); \
	T10C = _mm_srai_epi32(_mm_add_epi32(v32_horPredC, v32_topRowG), 6); \
	T10D = _mm_srai_epi32(_mm_add_epi32(v32_horPredD, v32_topRowH), 6); \
	T11A = _mm_packus_epi32(T10A, T10B); \
	T11B = _mm_packus_epi32(T10C, T10D); \
	T11 = _mm_packus_epi16(T11A, T11B); \
	_mm_storeu_si128((__m128i*)&pucDst[(24+(x)) * nDstStride + 16], T11);

		// Generate prediction signal
		COMP_PRED_PLANAR_ROW32(0)
		COMP_PRED_PLANAR_ROW32(1)
		COMP_PRED_PLANAR_ROW32(2)
		COMP_PRED_PLANAR_ROW32(3)
		COMP_PRED_PLANAR_ROW32(4)
		COMP_PRED_PLANAR_ROW32(5)
		COMP_PRED_PLANAR_ROW32(6)
		COMP_PRED_PLANAR_ROW32(7)
#undef COMP_PRED_PLANAR_ROW32
}

typedef void (*xPredIntraPlanar_ptr)(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride
	);
xPredIntraPlanar_ptr xPredIntraPlanarN[4] = {
	xPredIntraPlanar4,
	xPredIntraPlanar8,
	xPredIntraPlanar16,
	xPredIntraPlanar32,
};
#else
void xPredIntraPlanar(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nSize
	)
{
	UInt nLog2Size = xLog2(nSize - 1);
	UInt8 *pucLeft = pucRef + 2 * nSize - 1;
	UInt8 *pucTop  = pucRef + 2 * nSize + 1;
	Int i, j;
	UInt8 bottomLeft, topRight;
	Int16 horPred;
	Int16 leftColumn[MAX_CU_SIZE+1], topRow[MAX_CU_SIZE+1], bottomRow[MAX_CU_SIZE+1], rightColumn[MAX_CU_SIZE+1];
	UInt offset2D = nSize;
	UInt shift1D = nLog2Size;
	UInt shift2D = shift1D + 1;

	// Get left and above reference column and row
	for( i=0; i<(Int)nSize+1; i++) {
		topRow[i]     = pucTop[i];
		leftColumn[i] = pucLeft[-i];
	}

	// Prepare intermediate variables used in interpolation
	bottomLeft = pucLeft[-(Int)nSize];
	topRight   = pucTop[nSize];
	for( i=0; i<(Int)nSize; i++ ) {
		bottomRow[i]   = bottomLeft - topRow[i];
		rightColumn[i] = topRight   - leftColumn[i];
		topRow[i]      <<= shift1D;
		leftColumn[i]  <<= shift1D;
	}

	// Generate prediction signal
	for( i=0; i<(Int)nSize; i++ ) {
		horPred = leftColumn[i] + offset2D;
		for( j=0; j<(Int)nSize; j++ ) {
			horPred += rightColumn[i];
			topRow[j] += bottomRow[j];
			pucDst[i*nDstStride+j] = ( (horPred + topRow[j]) >> shift2D );
		}
	}
}
#endif

#if (INTRA_PRED_USE_ASM >= ASM_SSE2)
UInt8 xPredIntraGetDCVal(
	UInt8   *pucRef,
	UInt     nSize
	)
{
	UInt8 *pucLeft = pucRef + 2 * nSize - 1;
	UInt8 *pucTop  = pucRef + 2 * nSize + 1;
	UInt32 uiSumTop = 0;
	UInt32 uiSumLeft = 0;
	UInt8 ucDcVal;
	UInt32 uiSum;
	const __m128i zero = _mm_setzero_si128();
	__m128i sumA = _mm_setzero_si128();
	__m128i sumB = _mm_setzero_si128();
	__m128i sumC = _mm_setzero_si128();
	__m128i sumD = _mm_setzero_si128();

	switch( nSize ) {
	case 4:
		sumA = _mm_sad_epu8(zero, _mm_cvtsi32_si128(*(UInt32*)(pucLeft-3)));
		sumB = _mm_sad_epu8(zero, _mm_cvtsi32_si128(*(UInt32*)(pucTop   )));
		break;
	case 8:
		sumA = _mm_sad_epu8(zero, _mm_loadl_epi64((__m128i*)(pucLeft-7)));
		sumB = _mm_sad_epu8(zero, _mm_loadl_epi64((__m128i*)(pucTop   )));
		break;
	case 16:
		sumA = _mm_sad_epu8(zero, _mm_loadu_si128((__m128i*)(pucLeft-15)));
		sumB = _mm_sad_epu8(zero, _mm_loadu_si128((__m128i*)(pucTop    )));
		break;
		/*case 32*/default:
			sumA = _mm_sad_epu8(zero, _mm_loadu_si128((__m128i*)(pucLeft-31)));
			sumB = _mm_sad_epu8(zero, _mm_loadu_si128((__m128i*)(pucTop    )));
			sumC = _mm_sad_epu8(zero, _mm_loadu_si128((__m128i*)(pucLeft-15)));
			sumD = _mm_sad_epu8(zero, _mm_loadu_si128((__m128i*)(pucTop +16)));
			sumA = _mm_add_epi32(sumA, sumC);
			sumB = _mm_add_epi32(sumB, sumD);
			break;
	}
	sumA  = _mm_add_epi32(sumA, sumB);
	sumA  = _mm_add_epi32(_mm_srli_si128(sumA, 8), sumA);
	uiSum = _mm_cvtsi128_si32(sumA);

	ucDcVal = (uiSum + nSize) / (nSize + nSize);
	return ucDcVal;
}
#else // ASM_NONE
UInt8 xPredIntraGetDCVal(
	UInt8   *pucRef,
	UInt     nSize
	)
{
	UInt8 *pucLeft = pucRef + 2 * nSize - 1;
	UInt8 *pucTop  = pucRef + 2 * nSize + 1;
	UInt32 uiSumTop = 0;
	UInt32 uiSumLeft = 0;
	UInt8 ucDcVal;
	int i;

	for( i=0; i<(Int)nSize; i++ ) {
		uiSumTop  += pucTop [ i];
		uiSumLeft += pucLeft[-i];
	}
	ucDcVal = (uiSumTop + uiSumLeft + nSize) / (nSize + nSize);
	return ucDcVal;
}
#endif

#if (INTRA_PRED_USE_ASM >= ASM_SSE2)
void xPredIntraDc(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nSize,
	UInt     bLuma
	)
{
	UInt8 *pucLeft = pucRef + 2 * nSize - 1;
	UInt8 *pucTop  = pucRef + 2 * nSize + 1;
	UInt8 ucDcVal = xPredIntraGetDCVal( pucRef, nSize );
	int i;
	__m128i dcVal8   = _mm_set1_epi8(ucDcVal);
	__m128i dcValR16 = _mm_set1_epi16(3 * ucDcVal + 2);
	__m128i zero     = _mm_setzero_si128();
	__m128i pixT[4], pixL[4];
	UInt8 * P = pucDst;

	// Fill DC Val
	switch( nSize ) {
	case 4:
		*(UInt32*)(P+0*nDstStride) = _mm_cvtsi128_si32(dcVal8);
		*(UInt32*)(P+1*nDstStride) = _mm_cvtsi128_si32(dcVal8);
		*(UInt32*)(P+2*nDstStride) = _mm_cvtsi128_si32(dcVal8);
		*(UInt32*)(P+3*nDstStride) = _mm_cvtsi128_si32(dcVal8);
		break;
	case 8:
#pragma unroll(8)
		for( i=0; i<8; i++ ) {
			_mm_storel_epi64((__m128i*)(P+i*nDstStride), dcVal8);
		}
		break;
	case 16:
#pragma unroll(16)
		for( i=0; i<16; i++ ) {
			_mm_storeu_si128((__m128i*)(P+i*nDstStride), dcVal8);
		}
		break;
		/*case 32*/default:
#pragma unroll(16)
			for( i=0; i<32; i++ ) {
				_mm_storeu_si128((__m128i*)(P+i*nDstStride   ), dcVal8);
				_mm_storeu_si128((__m128i*)(P+i*nDstStride+16), dcVal8);
			}
			break;
	}

	// DC Filtering ( 8.4.3.1.5 )
	if( bLuma ) {
		UInt8 tmp[32];
		pixT[0] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucTop    )), zero);
		pixT[1] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucTop + 8)), zero);
		pixT[2] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucTop +16)), zero);
		pixT[3] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucTop +24)), zero);

		pixL[0] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucLeft- 7)), zero);
		pixL[1] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucLeft-15)), zero);
		pixL[2] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucLeft-23)), zero);
		pixL[3] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(pucLeft-31)), zero);

		pixT[0] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixT[0]), 2);
		pixT[1] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixT[1]), 2);
		pixT[2] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixT[2]), 2);
		pixT[3] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixT[3]), 2);

		pixT[0] = _mm_packus_epi16(pixT[0], pixT[1]);
		pixT[2] = _mm_packus_epi16(pixT[2], pixT[3]);

		pixL[0] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixL[0]), 2);
		pixL[1] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixL[1]), 2);
		pixL[2] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixL[2]), 2);
		pixL[3] = _mm_srai_epi16(_mm_add_epi16(dcValR16, pixL[3]), 2);

		pixL[0] = _mm_packus_epi16(pixL[1], pixL[0]);
		pixL[2] = _mm_packus_epi16(pixL[3], pixL[2]);

		_mm_storeu_si128((__m128i*)(tmp   ), pixL[2]);
		_mm_storeu_si128((__m128i*)(tmp+16), pixL[0]);

		switch( nSize ) {
		case 4:
			*(UInt32*)pucDst = _mm_cvtsi128_si32(pixT[0]);
			break;
		case 8:
			_mm_storel_epi64((__m128i*)pucDst, pixT[0]);
			break;
		case 32:
			_mm_storeu_si128((__m128i*)(pucDst+16), pixT[2]);
			/*case 16*/default:
				_mm_storeu_si128((__m128i*)(pucDst   ), pixT[0]);
				break;
		}
		for( i=1; i<(Int)nSize; i++ ) {
			pucDst[i*nDstStride] = tmp[31-i];
		}
		pucDst[0] = ( pucTop[0] + pucLeft[0] + 2 * ucDcVal + 2 ) >> 2;
	}
}
#else // ASM_NONE
void xPredIntraDc(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nSize,
	UInt     bLuma
	)
{
	UInt8 *pucLeft = pucRef + 2 * nSize - 1;
	UInt8 *pucTop  = pucRef + 2 * nSize + 1;
	UInt8 ucDcVal = xPredIntraGetDCVal( pucRef, nSize );
	int i;

	// Fill DC Val
	for( i=0; i<(Int)nSize; i++ ) {
		memset( &pucDst[i * nDstStride], ucDcVal, nSize );
	}

	// DC Filtering ( 8.4.3.1.5 )
	if( bLuma ) {
		pucDst[0] = ( pucTop[0] + pucLeft[0] + 2 * ucDcVal + 2 ) >> 2;
		for( i=1; i<(Int)nSize; i++ ) {
			pucDst[i           ] = ( pucTop [ i] + 3 * ucDcVal + 2 ) >> 2;
			pucDst[i*nDstStride] = ( pucLeft[-i] + 3 * ucDcVal + 2 ) >> 2;
		}
	}
}
#endif

#if (INTRA_PRED_USE_ASM >= ASM_SSE2)
void xTranspose8x8(UInt8 *pucSrc, UInt32 nSrcStride, UInt8 *pucDst, UInt32 nDstStride)
{
	__m128i T00 = _mm_loadl_epi64((__m128i*)&pucSrc[0*nSrcStride]);  // [07 06 05 04 03 02 01 00]
	__m128i T01 = _mm_loadl_epi64((__m128i*)&pucSrc[1*nSrcStride]);  // [17 16 15 14 13 12 11 10]
	__m128i T02 = _mm_loadl_epi64((__m128i*)&pucSrc[2*nSrcStride]);  // [27 26 25 24 23 22 21 20]
	__m128i T03 = _mm_loadl_epi64((__m128i*)&pucSrc[3*nSrcStride]);  // [37 36 35 34 33 32 31 30]
	__m128i T04 = _mm_loadl_epi64((__m128i*)&pucSrc[4*nSrcStride]);  // [47 46 45 44 43 42 41 40]
	__m128i T05 = _mm_loadl_epi64((__m128i*)&pucSrc[5*nSrcStride]);  // [57 56 55 54 53 52 51 50]
	__m128i T06 = _mm_loadl_epi64((__m128i*)&pucSrc[6*nSrcStride]);  // [67 66 65 64 63 62 61 60]
	__m128i T07 = _mm_loadl_epi64((__m128i*)&pucSrc[7*nSrcStride]);  // [77 76 75 74 73 72 71 70]

	__m128i T10 = _mm_unpacklo_epi8(T00, T01);      // [17 07 16 06 15 05 14 04 13 03 12 02 11 01 10 00]
	__m128i T11 = _mm_unpacklo_epi8(T02, T03);      // [37 27 36 26 35 25 34 24 33 23 32 22 31 21 30 20]
	__m128i T12 = _mm_unpacklo_epi8(T04, T05);      // [57 47 56 46 55 45 54 44 53 43 52 42 51 41 50 40]
	__m128i T13 = _mm_unpacklo_epi8(T06, T07);      // [77 67 76 66 75 65 74 64 73 63 72 62 71 61 70 60]

	__m128i T20 = _mm_unpacklo_epi16(T10, T11);     // [33 23 13 03 32 22 12 02 31 21 11 01 30 20 10 00]
	__m128i T21 = _mm_unpackhi_epi16(T10, T11);     // [37 27 17 07 36 26 16 06 35 25 15 05 34 24 14 04]
	__m128i T22 = _mm_unpacklo_epi16(T12, T13);     // [73 63 53 43 72 62 52 42 71 61 51 41 70 60 50 40]
	__m128i T23 = _mm_unpackhi_epi16(T12, T13);     // [77 67 57 47 76 66 56 46 75 65 55 45 74 64 54 44]

	__m128i T30 = _mm_unpacklo_epi32(T20, T22);     // [71 61 51 41 31 21 11 01 70 60 50 40 30 20 10 00]
	__m128i T31 = _mm_unpackhi_epi32(T20, T22);     // [73 63 53 43 33 23 13 03 72 62 52 42 32 22 12 02]
	__m128i T32 = _mm_unpacklo_epi32(T21, T23);     // [75 65 55 45 35 25 15 05 74 64 54 44 34 24 14 04]
	__m128i T33 = _mm_unpackhi_epi32(T21, T23);     // [77 67 57 47 37 27 17 07 76 66 56 46 36 26 16 06]

	_mm_storel_epi64((__m128i*)&pucDst[0*nDstStride], T30);
	_mm_storeh_pi   ((__m64*  )&pucDst[1*nDstStride], _mm_castsi128_ps(T30));
	_mm_storel_epi64((__m128i*)&pucDst[2*nDstStride], T31);
	_mm_storeh_pi   ((__m64*  )&pucDst[3*nDstStride], _mm_castsi128_ps(T31));
	_mm_storel_epi64((__m128i*)&pucDst[4*nDstStride], T32);
	_mm_storeh_pi   ((__m64*  )&pucDst[5*nDstStride], _mm_castsi128_ps(T32));
	_mm_storel_epi64((__m128i*)&pucDst[6*nDstStride], T33);
	_mm_storeh_pi   ((__m64*  )&pucDst[7*nDstStride], _mm_castsi128_ps(T33));
}
void xTranspose16x16(UInt8 *pucSrc, UInt32 nSrcStride, UInt8 *pucDst, UInt32 nDstStride)
{
	xTranspose8x8( pucSrc,                nSrcStride, pucDst,                nDstStride);
	xTranspose8x8( pucSrc             +8, nSrcStride, pucDst+8*nDstStride,   nDstStride);
	xTranspose8x8( pucSrc+8*nSrcStride,   nSrcStride, pucDst             +8, nDstStride);
	xTranspose8x8( pucSrc+8*nSrcStride+8, nSrcStride, pucDst+8*nDstStride+8, nDstStride);
}
void xTranspose32x32(UInt8 *pucSrc, UInt32 nSrcStride, UInt8 *pucDst, UInt32 nDstStride)
{
	xTranspose16x16( pucSrc,                  nSrcStride, pucDst,                  nDstStride);
	xTranspose16x16( pucSrc              +16, nSrcStride, pucDst+16*nDstStride,    nDstStride);
	xTranspose16x16( pucSrc+16*nSrcStride,    nSrcStride, pucDst              +16, nDstStride);
	xTranspose16x16( pucSrc+16*nSrcStride+16, nSrcStride, pucDst+16*nDstStride+16, nDstStride);
}

void xPredIntraAng4(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nMode,
	UInt     bFilter
	)
{
	UInt   bModeHor          = (nMode < 18);
	Int    nIntraPredAngle  = xg_aucIntraPredAngle[nMode];
	Int    nInvAngle        = xg_aucInvAngle[nMode];
	UInt8 *pucLeft          = pucRef + 2 * 4 - 1;
	UInt8 *pucTop           = pucRef + 2 * 4 + 1;
	UInt8 *pucTopLeft       = pucRef + 2 * 4;
	UInt8  ucRefBuf[2*MAX_CU_SIZE+1];
	UInt8 *pucRefMain       = ucRefBuf + ( (nIntraPredAngle < 0) ? MAX_CU_SIZE : 0 );
	Int    x, k;

	// (8-47) and (8-50)
	if ( bModeHor ) {
		*(UInt32*)pucRefMain = _bswap(*(UInt32*)(pucTopLeft-3));
		pucRefMain[4] = pucTopLeft[-4];
	}
	else {
		*(UInt32*)pucRefMain = *(UInt32*)pucTopLeft;
		pucRefMain[4] = pucTopLeft[4];
	}

	if( nIntraPredAngle < 0 ) {
		Int iSum = 128;
		// (8-48) or (8-51)
		for( x=-1; x>(nIntraPredAngle>>3); x-- ) {
			iSum += nInvAngle;
			// Fix my inv left buffer index
			Int nOffset = bModeHor ? (iSum >> 8) : -(iSum >> 8);
			pucRefMain[x] = pucTopLeft[ nOffset ];
		}
	}
	else {
		// (8-49) or (8-52)
		if ( bModeHor ) {
			*(UInt32*)(pucRefMain+5) = _bswap(*(UInt32*)(pucTopLeft-8));
		}
		else {
			*(UInt32*)(pucRefMain+5) = *(UInt32*)(pucTopLeft+5);
		}
	}

	// 8.4.3.1.6
	Int deltaPos=0;

	for( k=0; k<4; k++ ) {
		deltaPos += nIntraPredAngle;
		Int iIdx  = deltaPos >> 5;  // (8-53)
		Int iFact = deltaPos & 31;  // (8-54)
		__m128i zero = _mm_setzero_si128();
		__m128i c16  = _mm_set1_epi16(16);
		__m128i mfac = _mm_set1_epi32((iFact<<16) | (32-iFact));
		__m128i T00A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1   ])), zero);
		//__m128i T00B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2   ])), zero);
		__m128i T00B = _mm_srli_si128(T00A, 2);

		__m128i T10A  = _mm_madd_epi16(mfac, T00A);
		__m128i T10B  = _mm_madd_epi16(mfac, T00B);

		__m128i T20A  = _mm_packs_epi32(T10A, T10A);
		__m128i T20B  = _mm_packs_epi32(T10B, T10B);

		__m128i T30A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T20A, T20B)), 5);

		*(UInt32*)&pucDst[k*nDstStride] = _mm_cvtsi128_si32(_mm_packus_epi16(T30A, T30A));
	}

	// Filter if this is IntraPredAngle zero mode
	// see 8.4.3.1.3 and 8.4.3.1.4
	if( bFilter && (nIntraPredAngle == 0) ) {
		Int offset = bModeHor ? 1 : -1;
		for( x=0; x<4; x++ ) {
			pucDst[x*nDstStride] = Clip( pucDst[x*nDstStride] + (( pucTopLeft[(x+1)*offset] - pucTopLeft[0] ) >> 1) );
		}
	}

	// Matrix Transpose if this is the horizontal mode
	if( bModeHor ) {
		//__m128i T00 = _mm_set_epi32(0, 0, 0, 0x03020100);
		//__m128i T01 = _mm_set_epi32(0, 0, 0, 0x13121110);
		//__m128i T02 = _mm_set_epi32(0, 0, 0, 0x23222120);
		//__m128i T03 = _mm_set_epi32(0, 0, 0, 0x33323130);
		__m128i T00 = _mm_cvtsi32_si128(*(UInt32*)&pucDst[0*nDstStride]);  // [03 02 01 00]
		__m128i T01 = _mm_cvtsi32_si128(*(UInt32*)&pucDst[1*nDstStride]);  // [13 12 11 10]
		__m128i T02 = _mm_cvtsi32_si128(*(UInt32*)&pucDst[2*nDstStride]);  // [23 22 21 20]
		__m128i T03 = _mm_cvtsi32_si128(*(UInt32*)&pucDst[3*nDstStride]);  // [33 32 31 30]

		__m128i T10 = _mm_unpacklo_epi8(T00, T01);      // [13 03 12 02 11 01 10 00]
		__m128i T11 = _mm_unpacklo_epi8(T02, T03);      // [33 23 32 22 31 21 30 20]
		__m128i T20 = _mm_unpacklo_epi16(T10, T11);     // [33 23 13 03 32 22 12 02 31 21 11 01 30 20 10 00]
		*(UInt32*)&pucDst[0*nDstStride] = _mm_cvtsi128_si32(T20);
		*(UInt32*)&pucDst[1*nDstStride] = _mm_cvtsi128_si32(_mm_srli_si128(T20, 4));
		*(UInt32*)&pucDst[2*nDstStride] = _mm_cvtsi128_si32(_mm_unpackhi_epi32(T20, T20));
		*(UInt32*)&pucDst[3*nDstStride] = _mm_cvtsi128_si32(_mm_srli_si128(T20,12));
	}
}
void xPredIntraAng8(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nMode,
	UInt     bFilter
	)
{
	UInt   bModeHor          = (nMode < 18);
	Int    nIntraPredAngle  = xg_aucIntraPredAngle[nMode];
	Int    nInvAngle        = xg_aucInvAngle[nMode];
	UInt8 *pucLeft          = pucRef + 2 * 8 - 1;
	UInt8 *pucTop           = pucRef + 2 * 8 + 1;
	UInt8 *pucTopLeft       = pucRef + 2 * 8;
	UInt8  ucRefBuf[2*MAX_CU_SIZE+1];
	UInt8 *pucRefMain       = ucRefBuf + ( (nIntraPredAngle < 0) ? MAX_CU_SIZE : 0 );
	Int    x, k;

	// (8-47) and (8-50)
	if ( bModeHor ) {
		*(UInt64*)pucRefMain = _bswap64(*(UInt64*)(pucTopLeft-7));
		pucRefMain[8] = pucTopLeft[-8];
	}
	else {
		_mm_storel_epi64((__m128i*)pucRefMain, _mm_loadl_epi64((__m128i*)pucTopLeft));
		pucRefMain[8] = pucTopLeft[8];
	}

	if( nIntraPredAngle < 0 ) {
		Int iSum = 128;
		// (8-48) or (8-51)
		for( x=-1; x>(nIntraPredAngle>>2); x-- ) {
			iSum += nInvAngle;
			// Fix my inv left buffer index
			Int nOffset = bModeHor ? (iSum >> 8) : -(iSum >> 8);
			pucRefMain[x] = pucTopLeft[ nOffset ];
		}
	}
	else {
		// (8-49) or (8-52)
		if ( bModeHor ) {
			*(UInt64*)(pucRefMain+9) = _bswap64(*(UInt64*)(pucTopLeft-16));
		}
		else {
			*(UInt64*)(pucRefMain+9) = *(UInt64*)(pucTopLeft+9);
		}
	}

	// 8.4.3.1.6
	Int deltaPos=0;

	for( k=0; k<8; k++ ) {
		deltaPos += nIntraPredAngle;
		Int iIdx  = deltaPos >> 5;  // (8-53)
		Int iFact = deltaPos & 31;  // (8-54)
		__m128i zero = _mm_setzero_si128();
		__m128i c16  = _mm_set1_epi16(16);
		__m128i mfac = _mm_set1_epi32((iFact<<16) | (32-iFact));
		__m128i T00A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1   ])), zero);
		__m128i T00B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2   ])), zero);

		__m128i T10A  = _mm_madd_epi16(mfac, T00A);
		__m128i T10B  = _mm_madd_epi16(mfac, T00B);

		__m128i T20A  = _mm_packs_epi32(T10A, T10A);
		__m128i T20B  = _mm_packs_epi32(T10B, T10B);

		__m128i T30A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T20A, T20B)), 5);

		__m128i T40A  = _mm_packus_epi16(T30A, T30A);

		_mm_storel_epi64((__m128i*)&pucDst[k*nDstStride], T40A);
	}

	// Filter if this is IntraPredAngle zero mode
	// see 8.4.3.1.3 and 8.4.3.1.4
	if( bFilter && (nIntraPredAngle == 0) ) {
		Int offset = bModeHor ? 1 : -1;
		for( x=0; x<8; x++ ) {
			pucDst[x*nDstStride] = Clip( pucDst[x*nDstStride] + (( pucTopLeft[(x+1)*offset] - pucTopLeft[0] ) >> 1) );
		}
	}

	// Matrix Transpose if this is the horizontal mode
	if( bModeHor ) {
		xTranspose8x8(pucDst, nDstStride, pucDst, nDstStride);
	}
}
void xPredIntraAng16(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nMode,
	UInt     bFilter
	)
{
	UInt   bModeHor          = (nMode < 18);
	Int    nIntraPredAngle  = xg_aucIntraPredAngle[nMode];
	Int    nInvAngle        = xg_aucInvAngle[nMode];
	UInt8 *pucLeft          = pucRef + 2 * 16 - 1;
	UInt8 *pucTop           = pucRef + 2 * 16 + 1;
	UInt8 *pucTopLeft       = pucRef + 2 * 16;
	UInt8  ucRefBuf[2*MAX_CU_SIZE+1];
	UInt8 *pucRefMain       = ucRefBuf + ( (nIntraPredAngle < 0) ? MAX_CU_SIZE : 0 );
	Int    x, k;
	__declspec(align(16))
		UInt8 tmp[16*16];

	// (8-47) and (8-50)
	if ( bModeHor ) {
		*(UInt64*)(pucRefMain  ) = _bswap64(*(UInt64*)(pucTopLeft- 7));
		*(UInt64*)(pucRefMain+8) = _bswap64(*(UInt64*)(pucTopLeft-15));
		//__m128i T00 = _mm_set_epi32(0x0F0E0D0C, 0x0B0A0908, 0x07060504, 0x03020100);
		// [F E D C B A 9 8 7 6 5 4 3 2 1 0]
		//__m128i T00 = _mm_loadu_si128((__m128i*)(pucTopLeft-15));
		//__m128i cBSWAP = _mm_set_epi32(0x010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F);
		//__m128i T10 = _mm_shuffle_epi8(T00, cBSWAP);
		//_mm_storeu_si128((__m128i*)pucRefMain, T10);

		pucRefMain[16] = pucTopLeft[-16];
	}
	else {
		_mm_storeu_si128((__m128i*)pucRefMain, _mm_loadu_si128((__m128i*)pucTopLeft));
		pucRefMain[16] = pucTopLeft[16];
	}

	if( nIntraPredAngle < 0 ) {
		Int iSum = 128;
		// (8-48) or (8-51)
		for( x=-1; x>(nIntraPredAngle>>1); x-- ) {
			iSum += nInvAngle;
			// Fix my inv left buffer index
			Int nOffset = bModeHor ? (iSum >> 8) : -(iSum >> 8);
			pucRefMain[x] = pucTopLeft[ nOffset ];
		}
	}
	else {
		// (8-49) or (8-52)
		if ( bModeHor ) {
			*(UInt64*)(pucRefMain+17) = _bswap64(*(UInt64*)(pucTopLeft-24));
			*(UInt64*)(pucRefMain+25) = _bswap64(*(UInt64*)(pucTopLeft-32));
			//             __m128i T00 = _mm_loadu_si128((__m128i*)(pucTopLeft-32));
			//             __m128i cBSWAP = _mm_set_epi32(0x010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F);
			//             __m128i T10 = _mm_shuffle_epi8(T00, cBSWAP);
			//             _mm_storeu_si128((__m128i*)(pucRefMain+17), T10);
		}
		else {
			_mm_storeu_si128((__m128i*)(pucRefMain+17), _mm_loadu_si128((__m128i*)(pucTopLeft+17)));
		}
	}

	// 8.4.3.1.6
	Int deltaPos=0;

	for( k=0; k<16; k++ ) {
		deltaPos += nIntraPredAngle;
		Int iIdx  = deltaPos >> 5;  // (8-53)
		Int iFact = deltaPos & 31;  // (8-54)
		__m128i zero = _mm_setzero_si128();
		__m128i c16  = _mm_set1_epi16(16);
		__m128i mfac = _mm_set1_epi32((iFact<<16) | (32-iFact));
		__m128i T00A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1   ])), zero);
		__m128i T01A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1+ 8])), zero);
		__m128i T00B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2   ])), zero);
		__m128i T01B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2+ 8])), zero);

		__m128i T10A  = _mm_madd_epi16(mfac, T00A);
		__m128i T11A  = _mm_madd_epi16(mfac, T01A);
		__m128i T10B  = _mm_madd_epi16(mfac, T00B);
		__m128i T11B  = _mm_madd_epi16(mfac, T01B);

		__m128i T20A  = _mm_packs_epi32(T10A, T10A);
		__m128i T21A  = _mm_packs_epi32(T11A, T11A);
		__m128i T20B  = _mm_packs_epi32(T10B, T10B);
		__m128i T21B  = _mm_packs_epi32(T11B, T11B);

		__m128i T30A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T20A, T20B)), 5);
		__m128i T31A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T21A, T21B)), 5);

		__m128i T40A  = _mm_packus_epi16(T30A, T31A);

		_mm_store_si128((__m128i*)&tmp[k*16], T40A);
	}

	// Filter if this is IntraPredAngle zero mode
	// see 8.4.3.1.3 and 8.4.3.1.4
	if( bFilter && (nIntraPredAngle == 0) ) {
		Int offset = bModeHor ? 1 : -1;
		for( x=0; x<16; x++ ) {
			tmp[x*16] = Clip( tmp[x*16] + (( pucTopLeft[(x+1)*offset] - pucTopLeft[0] ) >> 1) );
		}
	}

	// Matrix Transpose if this is the horizontal mode
	if( bModeHor ) {
		xTranspose16x16(tmp, 16, pucDst, nDstStride);
	}
	else {
		for(x=0;x<16;x++) {
			_mm_storeu_si128((__m128i*)&pucDst[x*nDstStride], _mm_load_si128((__m128i*)&tmp[x*16]));
		}
	}
}
void xPredIntraAng32(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nMode,
	UInt     bFilter
	)
{
	UInt   bModeHor          = (nMode < 18);
	Int    nIntraPredAngle  = xg_aucIntraPredAngle[nMode];
	Int    nInvAngle        = xg_aucInvAngle[nMode];
	UInt8 *pucLeft          = pucRef + 2 * 32 - 1;
	UInt8 *pucTop           = pucRef + 2 * 32 + 1;
	UInt8 *pucTopLeft       = pucRef + 2 * 32;
	UInt8  ucRefBuf[2*MAX_CU_SIZE+1];
	UInt8 *pucRefMain       = ucRefBuf + ( (nIntraPredAngle < 0) ? MAX_CU_SIZE : 0 );
	Int    x, k;
	__declspec(align(16))
		UInt8 tmp[32*32];

	// (8-47) and (8-50)
	if ( bModeHor ) {
		*(UInt64*)(pucRefMain   ) = _bswap64(*(UInt64*)(pucTopLeft- 7));
		*(UInt64*)(pucRefMain+ 8) = _bswap64(*(UInt64*)(pucTopLeft-15));
		*(UInt64*)(pucRefMain+16) = _bswap64(*(UInt64*)(pucTopLeft-23));
		*(UInt64*)(pucRefMain+24) = _bswap64(*(UInt64*)(pucTopLeft-31));
		//         __m128i T00 = _mm_loadu_si128((__m128i*)(pucTopLeft-15));
		//         __m128i T01 = _mm_loadu_si128((__m128i*)(pucTopLeft-31));
		//         __m128i T10 = _mm_shuffle_epi8(T00, cBSWAP);
		//         __m128i T11 = _mm_shuffle_epi8(T01, cBSWAP);
		//         _mm_storeu_si128((__m128i*)(pucRefMain   ), T10);
		//         _mm_storeu_si128((__m128i*)(pucRefMain+16), T11);
		pucRefMain[32] = pucTopLeft[-32];
	}
	else {
		_mm_storeu_si128((__m128i*)(pucRefMain   ), _mm_loadu_si128((__m128i*)(pucTopLeft   )));
		_mm_storeu_si128((__m128i*)(pucRefMain+16), _mm_loadu_si128((__m128i*)(pucTopLeft+16)));
		pucRefMain[32] = pucTopLeft[32];
	}

	if( nIntraPredAngle < 0 ) {
		Int iSum = 128;
		// (8-48) or (8-51)
		for( x=-1; x>nIntraPredAngle; x-- ) {
			iSum += nInvAngle;
			// Fix my inv left buffer index
			Int nOffset = bModeHor ? (iSum >> 8) : -(iSum >> 8);
			pucRefMain[x] = pucTopLeft[ nOffset ];
		}
	}
	else {
		// (8-49) or (8-52)
		if ( bModeHor ) {
			*(UInt64*)(pucRefMain+33) = _bswap64(*(UInt64*)(pucTopLeft-40));
			*(UInt64*)(pucRefMain+41) = _bswap64(*(UInt64*)(pucTopLeft-48));
			*(UInt64*)(pucRefMain+49) = _bswap64(*(UInt64*)(pucTopLeft-56));
			*(UInt64*)(pucRefMain+57) = _bswap64(*(UInt64*)(pucTopLeft-64));
			//             __m128i T00 = _mm_loadu_si128((__m128i*)(pucTopLeft-48));
			//             __m128i T01 = _mm_loadu_si128((__m128i*)(pucTopLeft-64));
			//             __m128i T10 = _mm_shuffle_epi8(T00, cBSWAP);
			//             __m128i T11 = _mm_shuffle_epi8(T01, cBSWAP);
			//             _mm_storeu_si128((__m128i*)(pucRefMain+33), T10);
			//             _mm_storeu_si128((__m128i*)(pucRefMain+49), T11);
		}
		else {
			_mm_storeu_si128((__m128i*)(pucRefMain+33), _mm_loadu_si128((__m128i*)(pucTopLeft+33)));
			_mm_storeu_si128((__m128i*)(pucRefMain+49), _mm_loadu_si128((__m128i*)(pucTopLeft+49)));
		}
	}

	// 8.4.3.1.6
	Int deltaPos=0;

	for( k=0; k<32; k++ ) {
		deltaPos += nIntraPredAngle;
		Int iIdx  = deltaPos >> 5;  // (8-53)
		Int iFact = deltaPos & 31;  // (8-54)
		__m128i zero = _mm_setzero_si128();
		__m128i c16  = _mm_set1_epi16(16);
		__m128i mfac = _mm_set1_epi32((iFact<<16) | (32-iFact));
		__m128i T00A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1   ])), zero);
		__m128i T01A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1+ 8])), zero);
		__m128i T00B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2   ])), zero);
		__m128i T01B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2+ 8])), zero);

		__m128i T10A  = _mm_madd_epi16(mfac, T00A);
		__m128i T11A  = _mm_madd_epi16(mfac, T01A);
		__m128i T10B  = _mm_madd_epi16(mfac, T00B);
		__m128i T11B  = _mm_madd_epi16(mfac, T01B);

		__m128i T20A  = _mm_packs_epi32(T10A, T10A);
		__m128i T21A  = _mm_packs_epi32(T11A, T11A);
		__m128i T20B  = _mm_packs_epi32(T10B, T10B);
		__m128i T21B  = _mm_packs_epi32(T11B, T11B);

		__m128i T30A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T20A, T20B)), 5);
		__m128i T31A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T21A, T21B)), 5);

		__m128i T40A  = _mm_packus_epi16(T30A, T31A);

		__m128i T02A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1+16])), zero);
		__m128i T03A = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+1+24])), zero);
		__m128i T02B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2+16])), zero);
		__m128i T03B = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(&pucRefMain[iIdx+2+24])), zero);
		__m128i T12A  = _mm_madd_epi16(mfac, T02A);
		__m128i T13A  = _mm_madd_epi16(mfac, T03A);
		__m128i T22A  = _mm_packs_epi32(T12A, T12A);
		__m128i T23A  = _mm_packs_epi32(T13A, T13A);
		__m128i T12B  = _mm_madd_epi16(mfac, T02B);
		__m128i T13B  = _mm_madd_epi16(mfac, T03B);
		__m128i T22B  = _mm_packs_epi32(T12B, T12B);
		__m128i T23B  = _mm_packs_epi32(T13B, T13B);
		__m128i T32A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T22A, T22B)), 5);
		__m128i T33A  = _mm_srai_epi16(_mm_add_epi16(c16, _mm_unpacklo_epi16(T23A, T23B)), 5);
		__m128i T41A  = _mm_packus_epi16(T32A, T33A);

		_mm_store_si128((__m128i*)&tmp[k*32   ], T40A);
		_mm_store_si128((__m128i*)&tmp[k*32+16], T41A);
	}

	// Filter if this is IntraPredAngle zero mode
	// see 8.4.3.1.3 and 8.4.3.1.4
	if( bFilter && (nIntraPredAngle == 0) ) {
		Int offset = bModeHor ? 1 : -1;
		for( x=0; x<32; x++ ) {
			tmp[x*32] = Clip( tmp[x*32] + (( pucTopLeft[(x+1)*offset] - pucTopLeft[0] ) >> 1) );
		}
	}

	// Matrix Transpose if this is the horizontal mode
	if( bModeHor ) {
		xTranspose32x32(tmp, 32, pucDst, nDstStride);
	}
	else {
		for(x=0;x<32;x++) {
			_mm_storeu_si128((__m128i*)&pucDst[x*nDstStride   ], _mm_load_si128((__m128i*)&tmp[x*32   ]));
			_mm_storeu_si128((__m128i*)&pucDst[x*nDstStride+16], _mm_load_si128((__m128i*)&tmp[x*32+16]));
		}
	}
}
typedef void (*xPredIntraAng_ptr)(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	UInt     nMode,
	UInt     bFilter
	);
xPredIntraAng_ptr xPredIntraAngN[4] = {
	xPredIntraAng4,
	xPredIntraAng8,
	xPredIntraAng16,
	xPredIntraAng32,
};
#else // ASM_NONE
void xPredIntraAng(
	UInt8   *pucDst,
	UInt8   *pucRef,
	Int      nDstStride,
	Int     nSize,
	UInt     nMode,
	UInt     bFilter
	)
{
	UInt   bModeHor          = (nMode < 18);
	Int    nIntraPredAngle  = xg_aucIntraPredAngle[nMode];
	Int    nInvAngle        = xg_aucInvAngle[nMode];
	UInt8 *pucLeft          = pucRef + 2 * nSize - 1;
	UInt8 *pucTop           = pucRef + 2 * nSize + 1;
	UInt8 *pucTopLeft       = pucRef + 2 * nSize;
	UInt8  ucRefBuf[2*MAX_CU_SIZE+1];
	UInt8 *pucRefMain       = ucRefBuf + ( (nIntraPredAngle < 0) ? MAX_CU_SIZE : 0 );
	Int    x, k;

	// (8-47) and (8-50)
	for( x=0; x<nSize+1; x++ ) {
		pucRefMain[x] = bModeHor ? pucTopLeft[-x] : pucTopLeft[x];
	}

	if( nIntraPredAngle < 0 ) {
		Int iSum = 128;
		// (8-48) or (8-51)
		for( x=-1; x>(nSize*nIntraPredAngle)>>5; x-- ) {
			iSum += nInvAngle;
			// Fix my inv left buffer index
			Int nOffset = bModeHor ? (iSum >> 8) : -(iSum >> 8);
			pucRefMain[x] = pucTopLeft[ nOffset ];
		}
	}
	else {
		// (8-49) or (8-52)
		for( x=nSize+1; x<2*nSize+1; x++ ) {
			pucRefMain[x] = bModeHor ? pucTopLeft[-x] : pucTopLeft[x];
		}
	}

	// 8.4.3.1.6
	Int deltaPos=0;
	Int refMainIndex;

	for( k=0; k<nSize; k++ ) {
		deltaPos += nIntraPredAngle;
		Int iIdx  = deltaPos >> 5;  // (8-53)
		Int iFact = deltaPos & 31;  // (8-54)

		if( iFact ) {
			// Do linear filtering
			for( x=0; x<nSize; x++ ) {
				refMainIndex           = x+iIdx+1;
				pucDst[k*nDstStride+x] = ( ((32-iFact)*pucRefMain[refMainIndex]+iFact*pucRefMain[refMainIndex+1]+16) >> 5 );
			}
		}
		else {
			// Just copy the integer samples
			for( x=0; x<nSize; x++) {
				pucDst[k*nDstStride+x] = pucRefMain[iIdx+1+x];
			}
		}
	}

	// Filter if this is IntraPredAngle zero mode
	// see 8.4.3.1.3 and 8.4.3.1.4
	if( bFilter && (nIntraPredAngle == 0) )
	{
		Int offset = bModeHor ? 1 : -1;
		for( x=0; x<nSize; x++ ) {
			pucDst[x*nDstStride] = Clip ( pucDst[x*nDstStride] + (( pucTopLeft[(x+1)*offset] - pucTopLeft[0] ) >> 1) );
		}
	}

	// Matrix Transpose if this is the horizontal mode
	if( bModeHor ) {
		UInt8 tmp;
		for (k=0;k<nSize-1;k++)
		{
			for (x=k+1;x<nSize;x++)
			{
				tmp                 = pucDst[k*nDstStride+x];
				pucDst[k*nDstStride+x] = pucDst[x*nDstStride+k];
				pucDst[x*nDstStride+k] = tmp;
			}
		}
	}
}
#endif

// ***************************************************************************
// * Decide Functions
// ***************************************************************************

void xEncIntraPred( UInt8 *pucDstY, UInt8 *pucRefY, UInt nStride, UInt nMode, UInt nSize, UInt bIsLuma )
{
	if( nMode == PLANAR_IDX ) {
#if (INTRA_PRED_USE_ASM >= ASM_SSE4)
		xPredIntraPlanarN[xLog2(nSize-1)-2](
			pucDstY,
			pucRefY,
			nStride
			);
#else
		xPredIntraPlanar(
			pucDstY,
			pucRefY,
			nStride,
			nSize
			);
#endif
	}
	else if( nMode == DC_IDX ) {
		xPredIntraDc(
			pucDstY,
			pucRefY,
			nStride,
			nSize,
			bIsLuma
			);
	}
	else {
#if (INTRA_PRED_USE_ASM >= ASM_SSE2)
		xPredIntraAngN[xLog2(nSize-1)-2](
			pucDstY,
			pucRefY,
			nStride,
			nMode,
			bIsLuma
			);
#else // ASM_NONE
		xPredIntraAng(
			pucDstY,
			pucRefY,
			nStride,
			nSize,
			nMode,
			bIsLuma
			);
#endif
	}
}


//===========================================================Interpolate==================================================================
#if INTERP_USE_TRUE_ASM

// ***************************************************************************
// * Interpolate Function Declares
// ***************************************************************************
//Horizontal
extern "C" void x265_interp_8tap_horiz_pp_64x64_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_8tap_horiz_pp_32x32_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_8tap_horiz_pp_16x16_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_8tap_horiz_pp_8x8_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
//Vertical
extern "C" void x265_interp_8tap_vert_pp_64x64_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_8tap_vert_pp_32x32_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_8tap_vert_pp_16x16_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_8tap_vert_pp_8x8_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
//Horizontal-Vertical
extern "C" void x265_interp_8tap_hv_pp_64x64_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);
extern "C" void x265_interp_8tap_hv_pp_32x32_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);
extern "C" void x265_interp_8tap_hv_pp_16x16_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);
extern "C" void x265_interp_8tap_hv_pp_8x8_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);

//Horizontal
extern "C" void x265_interp_4tap_horiz_pp_32x32_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_4tap_horiz_pp_16x16_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_4tap_horiz_pp_8x8_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_4tap_horiz_pp_4x4_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
//Vertical
extern "C" void x265_interp_4tap_vert_pp_32x32_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_4tap_vert_pp_16x16_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_4tap_vert_pp_8x8_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
extern "C" void x265_interp_4tap_vert_pp_4x4_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);
//Horizontal-Vertical
extern "C" void x265_interp_4tap_hv_pp_32x32_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);
extern "C" void x265_interp_4tap_hv_pp_16x16_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);
extern "C" void x265_interp_4tap_hv_pp_8x8_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);
extern "C" void x265_interp_4tap_hv_pp_4x4_sse4(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int idxX, int idxY);

typedef void (*xInterpolate8H_ptr)(pixel * src, intptr_t srcStride, pixel * dst, intptr_t dstStride, int coeffIdx);

xInterpolate8H_ptr xInterpolate8H_NxN[5] = {
	NULL,
	x265_interp_8tap_horiz_pp_8x8_sse4,
	x265_interp_8tap_horiz_pp_16x16_sse4,
	x265_interp_8tap_horiz_pp_32x32_sse4,
	x265_interp_8tap_horiz_pp_64x64_sse4
};

#else

#if (INTERPOLATE_USE_ASM >= ASM_SSE4)
void xInterpolate8H_8x8(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,//nSize == 8
	CUInt       nSizeY,//nSize == 8
	CUInt       nFrac
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	Int32 xg0123 = ((xg3<<24)|((xg2&0x00FF)<<16)|((xg1&0x00FF)<<8)|(xg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((xg7<<24)|((xg6&0x00FF)<<16)|((xg5&0x00FF)<<8)|(xg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i	c_32	  = _mm_set1_epi16(32);
	const __m128i	xg_luma = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);

	for (Int i=0; i<8; i+=1){
		Int offset0 = i*(int)nRefStride-(NTAPS_LUMA/2-1);//x64 modify
		Int offset1 = i*MAX_CU_SIZE;
		const __m128i T_00_00		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0  ));//8bit: Ref [xx xx xx xx xx xx xx xx 04 03 02 01 00 -01 -02 -03]
		const __m128i T_00_01		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+1));//8bit: Ref [xx xx xx xx xx xx xx xx 05 04 03 02 01  00 -01 -02]
		const __m128i T_00_02		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+2));//8bit: Ref [xx xx xx xx xx xx xx xx 06 05 04 03 02  01  00 -01]
		const __m128i T_00_03		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+3));//8bit: Ref [xx xx xx xx xx xx xx xx 07 06 05 04 03  02  01  00]
		const __m128i T_00_04		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+4));//8bit: Ref [xx xx xx xx xx xx xx xx 08 07 06 05 04  03  02  01]
		const __m128i T_00_05		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+5));//8bit: Ref [xx xx xx xx xx xx xx xx 09 08 07 06 05  04  03  02]
		const __m128i T_00_06		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+6));//8bit: Ref [xx xx xx xx xx xx xx xx 10 09 08 07 06  05  04  03]
		const __m128i T_00_07		= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+7));//8bit: Ref [xx xx xx xx xx xx xx xx 11 10 09 08 07  06  05  04]

		const __m128i T_01_00	= _mm_unpacklo_epi64(T_00_00, T_00_01);	    
		const __m128i T_01_01	= _mm_unpacklo_epi64(T_00_02, T_00_03);		
		const __m128i T_01_02	= _mm_unpacklo_epi64(T_00_04, T_00_05);		
		const __m128i T_01_03	= _mm_unpacklo_epi64(T_00_06, T_00_07);		
		const __m128i T_02_00	= _mm_maddubs_epi16(T_01_00, xg_luma);	    //16bit: 01 00 abcd
		const __m128i T_02_01	= _mm_maddubs_epi16(T_01_01, xg_luma);		//16bit: 03 02 abcd
		const __m128i T_02_02	= _mm_maddubs_epi16(T_01_02, xg_luma);		//16bit: 05 04 abcd
		const __m128i T_02_03	= _mm_maddubs_epi16(T_01_03, xg_luma);		//16bit: 07 06 abcd
		const __m128i T_03_00a	= _mm_hadd_epi16(T_02_00, T_02_01);
		const __m128i T_03_00b	= _mm_hadd_epi16(T_02_02, T_02_03);  
		const __m128i T_03_00	= _mm_hadd_epi16(T_03_00a, T_03_00b);//16bit: 07 06 05 04 03 02 01 00
		const __m128i T_04_00	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00), 6);
		const __m128i T_05_00	= _mm_packus_epi16(T_04_00, T_04_00);
		_mm_storeu_si128((__m128i *)(pucPred+offset1), T_05_00);
	}
}
void xInterpolate8H_16x16(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	Int32 xg0123 = ((xg3<<24)|((xg2&0x00FF)<<16)|((xg1&0x00FF)<<8)|(xg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((xg7<<24)|((xg6&0x00FF)<<16)|((xg5&0x00FF)<<8)|(xg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i	c_32	  = _mm_set1_epi16(32);
	const __m128i	xg_luma = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);

	for (Int i=0; i<16; i+=1){
		Int offset0 = i*(int)nRefStride-(NTAPS_LUMA/2-1);//x64 modify
		Int offset1 = i*MAX_CU_SIZE;
		const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+offset0  )); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
		const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+1)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
		const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+2)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
		const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+3)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
		const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+4));//8bit: [16 15 14 13 12 11 10 09 08 07 06 05 04  03  02  01]
		const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+5));//8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05  04  03  02]
		const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+6));//8bit: [18 17 16 15 14 13 12 11 10 09 08 07 06  05  04  03]
		const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+7));//8bit: [19 18 17 16 15 14 13 12 11 10 09 08 07  06  05  04]

		const __m128i T_01_00	= _mm_maddubs_epi16(T_00_00, xg_luma);	    //16bit: 08 00 abcd
		const __m128i T_01_01	= _mm_maddubs_epi16(T_00_01, xg_luma);		//16bit: 09 01 abcd
		const __m128i T_01_02	= _mm_maddubs_epi16(T_00_02, xg_luma);		//16bit: 10 02 abcd
		const __m128i T_01_03	= _mm_maddubs_epi16(T_00_03, xg_luma);		//16bit: 11 03 abcd
		const __m128i T_01_04	= _mm_maddubs_epi16(T_00_04, xg_luma);	    //16bit: 12 04 abcd
		const __m128i T_01_05	= _mm_maddubs_epi16(T_00_05, xg_luma);		//16bit: 13 05 abcd
		const __m128i T_01_06	= _mm_maddubs_epi16(T_00_06, xg_luma);		//16bit: 14 06 abcd
		const __m128i T_01_07	= _mm_maddubs_epi16(T_00_07, xg_luma);		//16bit: 15 07 abcd

		const __m128i T_02_00	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_00, T_01_02), _mm_hadd_epi16(T_01_04, T_01_06));//16bit: 14 06 12 04 10 02 08 00
		const __m128i T_02_01	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_01, T_01_03), _mm_hadd_epi16(T_01_05, T_01_07));//16bit: 15 07 13 05 11 03 09 01
		const __m128i T_03_00   = _mm_unpacklo_epi16(T_02_00, T_02_01);	//16bit: 11 10 03 02 09 08 01 00
		const __m128i T_03_01   = _mm_unpackhi_epi16(T_02_00, T_02_01);	//16bit: 15 14 07 06 13 12 05 04
		const __m128i T_04_00   = _mm_shuffle_epi32(T_03_00, 0xD8);		//16bit: 11 10 09 08 03 02 01 00
		const __m128i T_04_01   = _mm_shuffle_epi32(T_03_01, 0xD8);		//16bit: 15 14 13 12 07 06 05 04
		const __m128i T_05_00	= _mm_unpacklo_epi64(T_04_00, T_04_01); //16bit: 07 06 05 04 03 02 01 00
		const __m128i T_05_01	= _mm_unpackhi_epi64(T_04_00, T_04_01); //16bit: 15 14 13 12 11 10 09 08
		const __m128i T_06_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_05_00), 6);
		const __m128i T_06_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_05_01), 6);
		const __m128i T_07_00	= _mm_packus_epi16(T_06_00a, T_06_00b);
		_mm_storeu_si128((__m128i *)(pucPred+offset1), T_07_00);
	}
}

void xInterpolate8H_32x32(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	Int32 xg0123 = ((xg3<<24)|((xg2&0x00FF)<<16)|((xg1&0x00FF)<<8)|(xg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((xg7<<24)|((xg6&0x00FF)<<16)|((xg5&0x00FF)<<8)|(xg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i	c_32	  = _mm_set1_epi16(32);
	const __m128i	xg_luma = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);

	for (Int i=0; i<32; i+=1){
		for (Int j=0; j<2; j++){
			Int offset0 = i*(int)nRefStride-(NTAPS_LUMA/2-1)+j*16;//x64 modify
			Int offset1 = i*MAX_CU_SIZE+j*16;
			const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+offset0  )); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
			const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+1)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
			const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+2)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
			const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+3)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
			const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+4));//8bit: [16 15 14 13 12 11 10 09 08 07 06 05 04  03  02  01]
			const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+5));//8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05  04  03  02]
			const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+6));//8bit: [18 17 16 15 14 13 12 11 10 09 08 07 06  05  04  03]
			const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+7));//8bit: [19 18 17 16 15 14 13 12 11 10 09 08 07  06  05  04]

			const __m128i T_01_00	= _mm_maddubs_epi16(T_00_00, xg_luma);	    //16bit: 08 00 abcd
			const __m128i T_01_01	= _mm_maddubs_epi16(T_00_01, xg_luma);		//16bit: 09 01 abcd
			const __m128i T_01_02	= _mm_maddubs_epi16(T_00_02, xg_luma);		//16bit: 10 02 abcd
			const __m128i T_01_03	= _mm_maddubs_epi16(T_00_03, xg_luma);		//16bit: 11 03 abcd
			const __m128i T_01_04	= _mm_maddubs_epi16(T_00_04, xg_luma);	    //16bit: 12 04 abcd
			const __m128i T_01_05	= _mm_maddubs_epi16(T_00_05, xg_luma);		//16bit: 13 05 abcd
			const __m128i T_01_06	= _mm_maddubs_epi16(T_00_06, xg_luma);		//16bit: 14 06 abcd
			const __m128i T_01_07	= _mm_maddubs_epi16(T_00_07, xg_luma);		//16bit: 15 07 abcd

			const __m128i T_02_00	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_00, T_01_02), _mm_hadd_epi16(T_01_04, T_01_06));//16bit: 14 06 12 04 10 02 08 00
			const __m128i T_02_01	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_01, T_01_03), _mm_hadd_epi16(T_01_05, T_01_07));//16bit: 15 07 13 05 11 03 09 01
			const __m128i T_03_00   = _mm_unpacklo_epi16(T_02_00, T_02_01);	//16bit: 11 10 03 02 09 08 01 00
			const __m128i T_03_01   = _mm_unpackhi_epi16(T_02_00, T_02_01);	//16bit: 15 14 07 06 13 12 05 04
			const __m128i T_04_00   = _mm_shuffle_epi32(T_03_00, 0xD8);		//16bit: 11 10 09 08 03 02 01 00
			const __m128i T_04_01   = _mm_shuffle_epi32(T_03_01, 0xD8);		//16bit: 15 14 13 12 07 06 05 04
			const __m128i T_05_00	= _mm_unpacklo_epi64(T_04_00, T_04_01); //16bit: 07 06 05 04 03 02 01 00
			const __m128i T_05_01	= _mm_unpackhi_epi64(T_04_00, T_04_01); //16bit: 15 14 13 12 11 10 09 08
			const __m128i T_06_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_05_00), 6);
			const __m128i T_06_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_05_01), 6);
			const __m128i T_07_00	= _mm_packus_epi16(T_06_00a, T_06_00b);
			_mm_storeu_si128((__m128i *)(pucPred+offset1), T_07_00);
		}
	}
}
void xInterpolate8H_64x64(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	Int32 xg0123 = ((xg3<<24)|((xg2&0x00FF)<<16)|((xg1&0x00FF)<<8)|(xg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((xg7<<24)|((xg6&0x00FF)<<16)|((xg5&0x00FF)<<8)|(xg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i	c_32	  = _mm_set1_epi16(32);
	const __m128i	xg_luma = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);

	for (Int i=0; i<64; i+=1){
		for (Int j=0; j<4; j++){
			Int offset0 = i*(int)nRefStride-(NTAPS_LUMA/2-1)+j*16;//x64 modify, force conversion to int(nRefStride)
			Int offset1 = i*MAX_CU_SIZE+j*16;
			const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+offset0  )); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
			const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+1)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
			const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+2)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
			const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+3)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
			const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+4));//8bit: [16 15 14 13 12 11 10 09 08 07 06 05 04  03  02  01]
			const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+5));//8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05  04  03  02]
			const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+6));//8bit: [18 17 16 15 14 13 12 11 10 09 08 07 06  05  04  03]
			const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+7));//8bit: [19 18 17 16 15 14 13 12 11 10 09 08 07  06  05  04]

			const __m128i T_01_00	= _mm_maddubs_epi16(T_00_00, xg_luma);	    //16bit: 08 00 abcd
			const __m128i T_01_01	= _mm_maddubs_epi16(T_00_01, xg_luma);		//16bit: 09 01 abcd
			const __m128i T_01_02	= _mm_maddubs_epi16(T_00_02, xg_luma);		//16bit: 10 02 abcd
			const __m128i T_01_03	= _mm_maddubs_epi16(T_00_03, xg_luma);		//16bit: 11 03 abcd
			const __m128i T_01_04	= _mm_maddubs_epi16(T_00_04, xg_luma);	    //16bit: 12 04 abcd
			const __m128i T_01_05	= _mm_maddubs_epi16(T_00_05, xg_luma);		//16bit: 13 05 abcd
			const __m128i T_01_06	= _mm_maddubs_epi16(T_00_06, xg_luma);		//16bit: 14 06 abcd
			const __m128i T_01_07	= _mm_maddubs_epi16(T_00_07, xg_luma);		//16bit: 15 07 abcd

			const __m128i T_02_00	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_00, T_01_02), _mm_hadd_epi16(T_01_04, T_01_06));//16bit: 14 06 12 04 10 02 08 00
			const __m128i T_02_01	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_01, T_01_03), _mm_hadd_epi16(T_01_05, T_01_07));//16bit: 15 07 13 05 11 03 09 01
			const __m128i T_03_00   = _mm_unpacklo_epi16(T_02_00, T_02_01);	//16bit: 11 10 03 02 09 08 01 00
			const __m128i T_03_01   = _mm_unpackhi_epi16(T_02_00, T_02_01);	//16bit: 15 14 07 06 13 12 05 04
			const __m128i T_04_00   = _mm_shuffle_epi32(T_03_00, 0xD8);		//16bit: 11 10 09 08 03 02 01 00
			const __m128i T_04_01   = _mm_shuffle_epi32(T_03_01, 0xD8);		//16bit: 15 14 13 12 07 06 05 04
			const __m128i T_05_00	= _mm_unpacklo_epi64(T_04_00, T_04_01); //16bit: 07 06 05 04 03 02 01 00
			const __m128i T_05_01	= _mm_unpackhi_epi64(T_04_00, T_04_01); //16bit: 15 14 13 12 11 10 09 08
			const __m128i T_06_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_05_00), 6);
			const __m128i T_06_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_05_01), 6);
			const __m128i T_07_00	= _mm_packus_epi16(T_06_00a, T_06_00b);
			_mm_storeu_si128((__m128i *)(pucPred+offset1), T_07_00);
		}
	}
	//xInterpolate8H_32x32(pucRef,	pucPred,	nRefStride, 32, 32, nFrac);
	//xInterpolate8H_32x32(pucRef+32, pucPred+32, nRefStride, 32, 32, nFrac);
	//xInterpolate8H_32x32(pucRef+32*nRefStride,		pucPred+32*MAX_CU_SIZE,		nRefStride, 32, 32, nFrac);
	//xInterpolate8H_32x32(pucRef+32*nRefStride+32,	pucPred+32*MAX_CU_SIZE+32,	nRefStride, 32, 32, nFrac);
}

typedef void (*xInterpolate8H_ptr)(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
);

xInterpolate8H_ptr xInterpolate8H_NxN[5] = {
	NULL,
	xInterpolate8H_8x8,
	xInterpolate8H_16x16,
	xInterpolate8H_32x32,
	xInterpolate8H_64x64
};

#else

void xInterpolate8H(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	//assert( nFrac != 0 );
	int i, j;

	for( i=0; i<(int)nSizeY; i++ ) {
		CUInt8 *P = pucRef + i * nRefStride - (NTAPS_LUMA/2-1);
		UInt8 *Q = pucPred + i * MAX_CU_SIZE;

		for( j=0; j<(int)nSizeX; j++ ) {
			Int16 iSum;
			iSum  = P[0+j] * xg_lumaFilter[nFrac][0];
			iSum += P[1+j] * xg_lumaFilter[nFrac][1];
			iSum += P[2+j] * xg_lumaFilter[nFrac][2];
			iSum += P[3+j] * xg_lumaFilter[nFrac][3];
			iSum += P[4+j] * xg_lumaFilter[nFrac][4];
			iSum += P[5+j] * xg_lumaFilter[nFrac][5];
			iSum += P[6+j] * xg_lumaFilter[nFrac][6];
			iSum += P[7+j] * xg_lumaFilter[nFrac][7];
			Q[j] = Clip3( 0, 255, (iSum + 32) >> 6);
		}
	}
}
#endif
#endif//end of USE_TRUE_ASM

#if (INTERPOLATE_USE_ASM >= ASM_SSE4)
void xInterpolate8V_8x8(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	const __m128i	c_32	= _mm_set1_epi16(32);
	const __m128i	xg_luma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_luma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );
	const __m128i	xg_luma_45	= _mm_set1_epi16( (xg5<<8) | (xg4 & 0x00FF) );
	const __m128i	xg_luma_67	= _mm_set1_epi16( (xg7<<8) | (xg6 & 0x00FF) );

	Int iRefStride = Int(nRefStride);//x64 modify, for temporary use, try to change all input parameters to signed types if possible
	const __m128i T_00_00		= _mm_loadl_epi64((const __m128i*)(pucRef-3*iRefStride));							 //8bit: Ref [-03 ] xx .. xx 07~00
	const __m128i T_00_01		= _mm_loadl_epi64((const __m128i*)(pucRef-2*iRefStride));							 //8bit: Ref [-02 ] xx .. xx 07~00
	const __m128i T_00_02		= _mm_loadl_epi64((const __m128i*)(pucRef-1*iRefStride));							 //8bit: Ref [-01 ] xx .. xx 07~00
	const __m128i T_00_03		= _mm_loadl_epi64((const __m128i*)(pucRef-0*iRefStride));							 //8bit: Ref [ 00 ] xx .. xx 07~00
	const __m128i T_00_04		= _mm_loadl_epi64((const __m128i*)(pucRef+1*iRefStride));							 //8bit: Ref [ 01 ] xx .. xx 07~00
	const __m128i T_00_05		= _mm_loadl_epi64((const __m128i*)(pucRef+2*iRefStride));							 //8bit: Ref [ 02 ] xx .. xx 07~00
	const __m128i T_00_06		= _mm_loadl_epi64((const __m128i*)(pucRef+3*iRefStride));							 //8bit: Ref [ 03 ] xx .. xx 07~00
	const __m128i T_00_07		= _mm_loadl_epi64((const __m128i*)(pucRef+4*iRefStride));							 //8bit: Ref [ 04 ] xx .. xx 07~00
	const __m128i T_00_08		= _mm_loadl_epi64((const __m128i*)(pucRef+5*iRefStride));							 //8bit: Ref [ 05 ] xx .. xx 07~00
	const __m128i T_00_09		= _mm_loadl_epi64((const __m128i*)(pucRef+6*iRefStride));							 //8bit: Ref [ 06 ] xx .. xx 07~00
	const __m128i T_00_10		= _mm_loadl_epi64((const __m128i*)(pucRef+7*iRefStride));							 //8bit: Ref [ 07 ] xx .. xx 07~00
	const __m128i T_00_11		= _mm_loadl_epi64((const __m128i*)(pucRef+8*iRefStride));							 //8bit: Ref [ 08 ] xx .. xx 07~00
	const __m128i T_00_12		= _mm_loadl_epi64((const __m128i*)(pucRef+9*iRefStride));							 //8bit: Ref [ 09 ] xx .. xx 07~00
	const __m128i T_00_13		= _mm_loadl_epi64((const __m128i*)(pucRef+10*iRefStride));						   //8bit: Ref [ 10 ] xx .. xx 07~00
	const __m128i T_00_14		= _mm_loadl_epi64((const __m128i*)(pucRef+11*iRefStride));						   //8bit: Ref [ 11 ] xx .. xx 07~00

	__m128i T_01_00a, T_01_01a, T_01_02a, T_01_03a, T_02_00a, T_02_01a, T_02_02a, T_02_03a, T_03_00a, T_04_00a;
	__m128i T_01_00b, T_01_01b, T_01_02b, T_01_03b, T_02_00b, T_02_01b, T_02_02b, T_02_03b, T_03_00b, T_04_00b;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, row4, row5, row6, row7, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_01_02a	= _mm_unpacklo_epi8(row4, row5);\
	T_01_03a	= _mm_unpacklo_epi8(row6, row7);\
	T_01_00b	= _mm_unpackhi_epi8(row0, row1);\
	T_01_01b	= _mm_unpackhi_epi8(row2, row3);\
	T_01_02b	= _mm_unpackhi_epi8(row4, row5);\
	T_01_03b	= _mm_unpackhi_epi8(row6, row7);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_luma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_luma_23);\
	T_02_02a	= _mm_maddubs_epi16(T_01_02a, xg_luma_45);\
	T_02_03a	= _mm_maddubs_epi16(T_01_03a, xg_luma_67);\
	T_02_00b	= _mm_maddubs_epi16(T_01_00b, xg_luma_01);\
	T_02_01b	= _mm_maddubs_epi16(T_01_01b, xg_luma_23);\
	T_02_02b	= _mm_maddubs_epi16(T_01_02b, xg_luma_45);\
	T_02_03b	= _mm_maddubs_epi16(T_01_03b, xg_luma_67);\
	T_03_00a	= _mm_add_epi16(_mm_add_epi16(T_02_00a,T_02_01a),_mm_add_epi16(T_02_02a,T_02_03a));\
	T_03_00b	= _mm_add_epi16(_mm_add_epi16(T_02_00b,T_02_01b),_mm_add_epi16(T_02_02b,T_02_03b));\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00b), 6);\
	_mm_storeu_si128((__m128i *)(pucPred+(k) *MAX_CU_SIZE), _mm_packus_epi16(T_04_00a, T_04_00b));

	INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,0)
		INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,1)
		INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,2)
		INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,3)
		INTERPOLATE_ONE_LINE(T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,4)
		INTERPOLATE_ONE_LINE(T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,5)
		INTERPOLATE_ONE_LINE(T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,6)
		INTERPOLATE_ONE_LINE(T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,7)
#undef INTERPOLATE_ONE_LINE
}
void xInterpolate8V_16x16(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	const __m128i	c_32	= _mm_set1_epi16(32);
	const __m128i	xg_luma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_luma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );
	const __m128i	xg_luma_45	= _mm_set1_epi16( (xg5<<8) | (xg4 & 0x00FF) );
	const __m128i	xg_luma_67	= _mm_set1_epi16( (xg7<<8) | (xg6 & 0x00FF) );

	Int iRefStride = Int(nRefStride);//x64 modify, for temporary use, try to change all input parameters to signed types if possible
	const __m128i T_00_00		= _mm_loadu_si128((const __m128i*)(pucRef-3*iRefStride));							 //8bit: Ref [-03 ] 15~00
	const __m128i T_00_01		= _mm_loadu_si128((const __m128i*)(pucRef-2*iRefStride));							 //8bit: Ref [-02 ] 15~00
	const __m128i T_00_02		= _mm_loadu_si128((const __m128i*)(pucRef-1*iRefStride));							 //8bit: Ref [-01 ] 15~00
	const __m128i T_00_03		= _mm_loadu_si128((const __m128i*)(pucRef-0*iRefStride));							 //8bit: Ref [ 00 ] 15~00
	const __m128i T_00_04		= _mm_loadu_si128((const __m128i*)(pucRef+1*iRefStride));							 //8bit: Ref [ 01 ] 15~00
	const __m128i T_00_05		= _mm_loadu_si128((const __m128i*)(pucRef+2*iRefStride));							 //8bit: Ref [ 02 ] 15~00
	const __m128i T_00_06		= _mm_loadu_si128((const __m128i*)(pucRef+3*iRefStride));							 //8bit: Ref [ 03 ] 15~00
	const __m128i T_00_07		= _mm_loadu_si128((const __m128i*)(pucRef+4*iRefStride));							 //8bit: Ref [ 04 ] 15~00
	const __m128i T_00_08		= _mm_loadu_si128((const __m128i*)(pucRef+5*iRefStride));							 //8bit: Ref [ 05 ] 15~00
	const __m128i T_00_09		= _mm_loadu_si128((const __m128i*)(pucRef+6*iRefStride));							 //8bit: Ref [ 06 ] 15~00
	const __m128i T_00_10		= _mm_loadu_si128((const __m128i*)(pucRef+7*iRefStride));							 //8bit: Ref [ 07 ] 15~00
	const __m128i T_00_11		= _mm_loadu_si128((const __m128i*)(pucRef+8*iRefStride));							 //8bit: Ref [ 08 ] 15~00
	const __m128i T_00_12		= _mm_loadu_si128((const __m128i*)(pucRef+9*iRefStride));							 //8bit: Ref [ 09 ] 15~00
	const __m128i T_00_13		= _mm_loadu_si128((const __m128i*)(pucRef+10*iRefStride));						   //8bit: Ref [ 10 ] 15~00
	const __m128i T_00_14		= _mm_loadu_si128((const __m128i*)(pucRef+11*iRefStride));						   //8bit: Ref [ 11 ] 15~00
	const __m128i T_00_15		= _mm_loadu_si128((const __m128i*)(pucRef+12*iRefStride));							 //8bit: Ref [-03 ] 15~00
	const __m128i T_00_16		= _mm_loadu_si128((const __m128i*)(pucRef+13*iRefStride));							 //8bit: Ref [-02 ] 15~00
	const __m128i T_00_17		= _mm_loadu_si128((const __m128i*)(pucRef+14*iRefStride));							 //8bit: Ref [-01 ] 15~00
	const __m128i T_00_18		= _mm_loadu_si128((const __m128i*)(pucRef+15*iRefStride));							 //8bit: Ref [ 00 ] 15~00
	const __m128i T_00_19		= _mm_loadu_si128((const __m128i*)(pucRef+16*iRefStride));							 //8bit: Ref [ 01 ] 15~00
	const __m128i T_00_20		= _mm_loadu_si128((const __m128i*)(pucRef+17*iRefStride));							 //8bit: Ref [ 02 ] 15~00
	const __m128i T_00_21		= _mm_loadu_si128((const __m128i*)(pucRef+18*iRefStride));							 //8bit: Ref [ 03 ] 15~00
	const __m128i T_00_22		= _mm_loadu_si128((const __m128i*)(pucRef+19*iRefStride));							 //8bit: Ref [ 04 ] 15~00

	__m128i T_01_00a, T_01_01a, T_01_02a, T_01_03a, T_02_00a, T_02_01a, T_02_02a, T_02_03a, T_03_00a, T_04_00a;
	__m128i T_01_00b, T_01_01b, T_01_02b, T_01_03b, T_02_00b, T_02_01b, T_02_02b, T_02_03b, T_03_00b, T_04_00b;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, row4, row5, row6, row7, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_01_02a	= _mm_unpacklo_epi8(row4, row5);\
	T_01_03a	= _mm_unpacklo_epi8(row6, row7);\
	T_01_00b	= _mm_unpackhi_epi8(row0, row1);\
	T_01_01b	= _mm_unpackhi_epi8(row2, row3);\
	T_01_02b	= _mm_unpackhi_epi8(row4, row5);\
	T_01_03b	= _mm_unpackhi_epi8(row6, row7);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_luma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_luma_23);\
	T_02_02a	= _mm_maddubs_epi16(T_01_02a, xg_luma_45);\
	T_02_03a	= _mm_maddubs_epi16(T_01_03a, xg_luma_67);\
	T_02_00b	= _mm_maddubs_epi16(T_01_00b, xg_luma_01);\
	T_02_01b	= _mm_maddubs_epi16(T_01_01b, xg_luma_23);\
	T_02_02b	= _mm_maddubs_epi16(T_01_02b, xg_luma_45);\
	T_02_03b	= _mm_maddubs_epi16(T_01_03b, xg_luma_67);\
	T_03_00a	= _mm_add_epi16(_mm_add_epi16(T_02_00a,T_02_01a),_mm_add_epi16(T_02_02a,T_02_03a));\
	T_03_00b	= _mm_add_epi16(_mm_add_epi16(T_02_00b,T_02_01b),_mm_add_epi16(T_02_02b,T_02_03b));\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00b), 6);\
	_mm_storeu_si128((__m128i *)(pucPred+(k) *MAX_CU_SIZE), _mm_packus_epi16(T_04_00a, T_04_00b));

	INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,0)
		INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,1)
		INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,2)
		INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,3)
		INTERPOLATE_ONE_LINE(T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,4)
		INTERPOLATE_ONE_LINE(T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,5)
		INTERPOLATE_ONE_LINE(T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,6)
		INTERPOLATE_ONE_LINE(T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,7)
		INTERPOLATE_ONE_LINE(T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,8)
		INTERPOLATE_ONE_LINE(T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,9)
		INTERPOLATE_ONE_LINE(T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,10)
		INTERPOLATE_ONE_LINE(T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,11)
		INTERPOLATE_ONE_LINE(T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,12)
		INTERPOLATE_ONE_LINE(T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,13)
		INTERPOLATE_ONE_LINE(T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,T_00_21,14)
		INTERPOLATE_ONE_LINE(T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,T_00_21,T_00_22,15)
#undef INTERPOLATE_ONE_LINE
}

void xInterpolate8V_32x32(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	// new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	const __m128i	c_32	= _mm_set1_epi16(32);
	const __m128i	xg_luma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_luma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );
	const __m128i	xg_luma_45	= _mm_set1_epi16( (xg5<<8) | (xg4 & 0x00FF) );
	const __m128i	xg_luma_67	= _mm_set1_epi16( (xg7<<8) | (xg6 & 0x00FF) );

	Int iRefStride = Int(nRefStride);//x64 modify, for temporary use, try to change all input parameters to signed types if possible
	for (Int i=0; i<32; i+=16){
		for (Int j=0; j<2; j++)	{
			Int offset = j*16;
			const __m128i T_00_00		= _mm_loadu_si128((const __m128i*)(pucRef+(i-3)*iRefStride+offset));							 //8bit: Ref [-03 ] 15~00
			const __m128i T_00_01		= _mm_loadu_si128((const __m128i*)(pucRef+(i-2)*iRefStride+offset));							 //8bit: Ref [-02 ] 15~00
			const __m128i T_00_02		= _mm_loadu_si128((const __m128i*)(pucRef+(i-1)*iRefStride+offset));							 //8bit: Ref [-01 ] 15~00
			const __m128i T_00_03		= _mm_loadu_si128((const __m128i*)(pucRef+(i-0)*iRefStride+offset));							 //8bit: Ref [ 00 ] 15~00
			const __m128i T_00_04		= _mm_loadu_si128((const __m128i*)(pucRef+(i+1)*iRefStride+offset));							 //8bit: Ref [ 01 ] 15~00
			const __m128i T_00_05		= _mm_loadu_si128((const __m128i*)(pucRef+(i+2)*iRefStride+offset));							 //8bit: Ref [ 02 ] 15~00
			const __m128i T_00_06		= _mm_loadu_si128((const __m128i*)(pucRef+(i+3)*iRefStride+offset));							 //8bit: Ref [ 03 ] 15~00
			const __m128i T_00_07		= _mm_loadu_si128((const __m128i*)(pucRef+(i+4)*iRefStride+offset));							 //8bit: Ref [ 04 ] 15~00
			const __m128i T_00_08		= _mm_loadu_si128((const __m128i*)(pucRef+(i+5)*iRefStride+offset));							 //8bit: Ref [ 05 ] 15~00
			const __m128i T_00_09		= _mm_loadu_si128((const __m128i*)(pucRef+(i+6)*iRefStride+offset));							 //8bit: Ref [ 06 ] 15~00
			const __m128i T_00_10		= _mm_loadu_si128((const __m128i*)(pucRef+(i+7)*iRefStride+offset));							 //8bit: Ref [ 07 ] 15~00
			const __m128i T_00_11		= _mm_loadu_si128((const __m128i*)(pucRef+(i+8)*iRefStride+offset));							 //8bit: Ref [ 08 ] 15~00
			const __m128i T_00_12		= _mm_loadu_si128((const __m128i*)(pucRef+(i+9)*iRefStride+offset));							 //8bit: Ref [ 09 ] 15~00
			const __m128i T_00_13		= _mm_loadu_si128((const __m128i*)(pucRef+(i+10)*iRefStride+offset));						   //8bit: Ref [ 10 ] 15~00
			const __m128i T_00_14		= _mm_loadu_si128((const __m128i*)(pucRef+(i+11)*iRefStride+offset));						   //8bit: Ref [ 11 ] 15~00
			const __m128i T_00_15		= _mm_loadu_si128((const __m128i*)(pucRef+(i+12)*iRefStride+offset));							 //8bit: Ref [-03 ] 15~00
			const __m128i T_00_16		= _mm_loadu_si128((const __m128i*)(pucRef+(i+13)*iRefStride+offset));							 //8bit: Ref [-02 ] 15~00
			const __m128i T_00_17		= _mm_loadu_si128((const __m128i*)(pucRef+(i+14)*iRefStride+offset));							 //8bit: Ref [-01 ] 15~00
			const __m128i T_00_18		= _mm_loadu_si128((const __m128i*)(pucRef+(i+15)*iRefStride+offset));							 //8bit: Ref [ 00 ] 15~00
			const __m128i T_00_19		= _mm_loadu_si128((const __m128i*)(pucRef+(i+16)*iRefStride+offset));							 //8bit: Ref [ 01 ] 15~00
			const __m128i T_00_20		= _mm_loadu_si128((const __m128i*)(pucRef+(i+17)*iRefStride+offset));							 //8bit: Ref [ 02 ] 15~00
			const __m128i T_00_21		= _mm_loadu_si128((const __m128i*)(pucRef+(i+18)*iRefStride+offset));							 //8bit: Ref [ 03 ] 15~00
			const __m128i T_00_22		= _mm_loadu_si128((const __m128i*)(pucRef+(i+19)*iRefStride+offset));							 //8bit: Ref [ 04 ] 15~00

			__m128i T_01_00a, T_01_01a, T_01_02a, T_01_03a, T_02_00a, T_02_01a, T_02_02a, T_02_03a, T_03_00a, T_04_00a;
			__m128i T_01_00b, T_01_01b, T_01_02b, T_01_03b, T_02_00b, T_02_01b, T_02_02b, T_02_03b, T_03_00b, T_04_00b;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, row4, row5, row6, row7, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_01_02a	= _mm_unpacklo_epi8(row4, row5);\
	T_01_03a	= _mm_unpacklo_epi8(row6, row7);\
	T_01_00b	= _mm_unpackhi_epi8(row0, row1);\
	T_01_01b	= _mm_unpackhi_epi8(row2, row3);\
	T_01_02b	= _mm_unpackhi_epi8(row4, row5);\
	T_01_03b	= _mm_unpackhi_epi8(row6, row7);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_luma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_luma_23);\
	T_02_02a	= _mm_maddubs_epi16(T_01_02a, xg_luma_45);\
	T_02_03a	= _mm_maddubs_epi16(T_01_03a, xg_luma_67);\
	T_02_00b	= _mm_maddubs_epi16(T_01_00b, xg_luma_01);\
	T_02_01b	= _mm_maddubs_epi16(T_01_01b, xg_luma_23);\
	T_02_02b	= _mm_maddubs_epi16(T_01_02b, xg_luma_45);\
	T_02_03b	= _mm_maddubs_epi16(T_01_03b, xg_luma_67);\
	T_03_00a	= _mm_add_epi16(_mm_add_epi16(T_02_00a,T_02_01a),_mm_add_epi16(T_02_02a,T_02_03a));\
	T_03_00b	= _mm_add_epi16(_mm_add_epi16(T_02_00b,T_02_01b),_mm_add_epi16(T_02_02b,T_02_03b));\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00b), 6);\
	_mm_storeu_si128((__m128i *)(pucPred+(i+k) *MAX_CU_SIZE + offset), _mm_packus_epi16(T_04_00a, T_04_00b));

			INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,0)
				INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,1)
				INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,2)
				INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,3)
				INTERPOLATE_ONE_LINE(T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,4)
				INTERPOLATE_ONE_LINE(T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,5)
				INTERPOLATE_ONE_LINE(T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,6)
				INTERPOLATE_ONE_LINE(T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,7)
				INTERPOLATE_ONE_LINE(T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,8)
				INTERPOLATE_ONE_LINE(T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,9)
				INTERPOLATE_ONE_LINE(T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,10)
				INTERPOLATE_ONE_LINE(T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,11)
				INTERPOLATE_ONE_LINE(T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,12)
				INTERPOLATE_ONE_LINE(T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,13)
				INTERPOLATE_ONE_LINE(T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,T_00_21,14)
				INTERPOLATE_ONE_LINE(T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,T_00_21,T_00_22,15)
#undef INTERPOLATE_ONE_LINE
		}
	}
}

void xInterpolate8V_64x64(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,//x64 modify, try to change all input parameters to signed types if possible
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning

	// new interpolate, 2013-12-6
	Int16 xg0 = (xg_lumaFilter[nFrac][0]);
	Int16 xg1 = (xg_lumaFilter[nFrac][1]);
	Int16 xg2 = (xg_lumaFilter[nFrac][2]);
	Int16 xg3 = (xg_lumaFilter[nFrac][3]);
	Int16 xg4 = (xg_lumaFilter[nFrac][4]);
	Int16 xg5 = (xg_lumaFilter[nFrac][5]);
	Int16 xg6 = (xg_lumaFilter[nFrac][6]);
	Int16 xg7 = (xg_lumaFilter[nFrac][7]);
	const __m128i	c_32	= _mm_set1_epi16(32);
	const __m128i	xg_luma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_luma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );
	const __m128i	xg_luma_45	= _mm_set1_epi16( (xg5<<8) | (xg4 & 0x00FF) );
	const __m128i	xg_luma_67	= _mm_set1_epi16( (xg7<<8) | (xg6 & 0x00FF) );

	Int iRefStride = Int(nRefStride);//x64 modify, for temporary use, try to change all input parameters to signed types if possible
	for (Int i=0; i<64; i+=16){
		for (Int j=0; j<4; j++)	{
			Int offset = j*16;
			const __m128i T_00_00		= _mm_loadu_si128((const __m128i*)(pucRef+(i-3)*iRefStride+offset));							 //8bit: Ref [-03 ] 15~00
			const __m128i T_00_01		= _mm_loadu_si128((const __m128i*)(pucRef+(i-2)*iRefStride+offset));							 //8bit: Ref [-02 ] 15~00
			const __m128i T_00_02		= _mm_loadu_si128((const __m128i*)(pucRef+(i-1)*iRefStride+offset));							 //8bit: Ref [-01 ] 15~00
			const __m128i T_00_03		= _mm_loadu_si128((const __m128i*)(pucRef+(i-0)*iRefStride+offset));							 //8bit: Ref [ 00 ] 15~00
			const __m128i T_00_04		= _mm_loadu_si128((const __m128i*)(pucRef+(i+1)*iRefStride+offset));							 //8bit: Ref [ 01 ] 15~00
			const __m128i T_00_05		= _mm_loadu_si128((const __m128i*)(pucRef+(i+2)*iRefStride+offset));							 //8bit: Ref [ 02 ] 15~00
			const __m128i T_00_06		= _mm_loadu_si128((const __m128i*)(pucRef+(i+3)*iRefStride+offset));							 //8bit: Ref [ 03 ] 15~00
			const __m128i T_00_07		= _mm_loadu_si128((const __m128i*)(pucRef+(i+4)*iRefStride+offset));							 //8bit: Ref [ 04 ] 15~00
			const __m128i T_00_08		= _mm_loadu_si128((const __m128i*)(pucRef+(i+5)*iRefStride+offset));							 //8bit: Ref [ 05 ] 15~00
			const __m128i T_00_09		= _mm_loadu_si128((const __m128i*)(pucRef+(i+6)*iRefStride+offset));							 //8bit: Ref [ 06 ] 15~00
			const __m128i T_00_10		= _mm_loadu_si128((const __m128i*)(pucRef+(i+7)*iRefStride+offset));							 //8bit: Ref [ 07 ] 15~00
			const __m128i T_00_11		= _mm_loadu_si128((const __m128i*)(pucRef+(i+8)*iRefStride+offset));							 //8bit: Ref [ 08 ] 15~00
			const __m128i T_00_12		= _mm_loadu_si128((const __m128i*)(pucRef+(i+9)*iRefStride+offset));							 //8bit: Ref [ 09 ] 15~00
			const __m128i T_00_13		= _mm_loadu_si128((const __m128i*)(pucRef+(i+10)*iRefStride+offset));						   //8bit: Ref [ 10 ] 15~00
			const __m128i T_00_14		= _mm_loadu_si128((const __m128i*)(pucRef+(i+11)*iRefStride+offset));						   //8bit: Ref [ 11 ] 15~00
			const __m128i T_00_15		= _mm_loadu_si128((const __m128i*)(pucRef+(i+12)*iRefStride+offset));							 //8bit: Ref [-03 ] 15~00
			const __m128i T_00_16		= _mm_loadu_si128((const __m128i*)(pucRef+(i+13)*iRefStride+offset));							 //8bit: Ref [-02 ] 15~00
			const __m128i T_00_17		= _mm_loadu_si128((const __m128i*)(pucRef+(i+14)*iRefStride+offset));							 //8bit: Ref [-01 ] 15~00
			const __m128i T_00_18		= _mm_loadu_si128((const __m128i*)(pucRef+(i+15)*iRefStride+offset));							 //8bit: Ref [ 00 ] 15~00
			const __m128i T_00_19		= _mm_loadu_si128((const __m128i*)(pucRef+(i+16)*iRefStride+offset));							 //8bit: Ref [ 01 ] 15~00
			const __m128i T_00_20		= _mm_loadu_si128((const __m128i*)(pucRef+(i+17)*iRefStride+offset));							 //8bit: Ref [ 02 ] 15~00
			const __m128i T_00_21		= _mm_loadu_si128((const __m128i*)(pucRef+(i+18)*iRefStride+offset));							 //8bit: Ref [ 03 ] 15~00
			const __m128i T_00_22		= _mm_loadu_si128((const __m128i*)(pucRef+(i+19)*iRefStride+offset));							 //8bit: Ref [ 04 ] 15~00

			__m128i T_01_00a, T_01_01a, T_01_02a, T_01_03a, T_02_00a, T_02_01a, T_02_02a, T_02_03a, T_03_00a, T_04_00a;
			__m128i T_01_00b, T_01_01b, T_01_02b, T_01_03b, T_02_00b, T_02_01b, T_02_02b, T_02_03b, T_03_00b, T_04_00b;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, row4, row5, row6, row7, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_01_02a	= _mm_unpacklo_epi8(row4, row5);\
	T_01_03a	= _mm_unpacklo_epi8(row6, row7);\
	T_01_00b	= _mm_unpackhi_epi8(row0, row1);\
	T_01_01b	= _mm_unpackhi_epi8(row2, row3);\
	T_01_02b	= _mm_unpackhi_epi8(row4, row5);\
	T_01_03b	= _mm_unpackhi_epi8(row6, row7);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_luma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_luma_23);\
	T_02_02a	= _mm_maddubs_epi16(T_01_02a, xg_luma_45);\
	T_02_03a	= _mm_maddubs_epi16(T_01_03a, xg_luma_67);\
	T_02_00b	= _mm_maddubs_epi16(T_01_00b, xg_luma_01);\
	T_02_01b	= _mm_maddubs_epi16(T_01_01b, xg_luma_23);\
	T_02_02b	= _mm_maddubs_epi16(T_01_02b, xg_luma_45);\
	T_02_03b	= _mm_maddubs_epi16(T_01_03b, xg_luma_67);\
	T_03_00a	= _mm_add_epi16(_mm_add_epi16(T_02_00a,T_02_01a),_mm_add_epi16(T_02_02a,T_02_03a));\
	T_03_00b	= _mm_add_epi16(_mm_add_epi16(T_02_00b,T_02_01b),_mm_add_epi16(T_02_02b,T_02_03b));\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00b), 6);\
	_mm_storeu_si128((__m128i *)(pucPred+(i+k) *MAX_CU_SIZE + offset), _mm_packus_epi16(T_04_00a, T_04_00b));

			INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,0)
				INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,1)
				INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,2)
				INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,3)
				INTERPOLATE_ONE_LINE(T_00_04,T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,4)
				INTERPOLATE_ONE_LINE(T_00_05,T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,5)
				INTERPOLATE_ONE_LINE(T_00_06,T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,6)
				INTERPOLATE_ONE_LINE(T_00_07,T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,7)
				INTERPOLATE_ONE_LINE(T_00_08,T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,8)
				INTERPOLATE_ONE_LINE(T_00_09,T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,9)
				INTERPOLATE_ONE_LINE(T_00_10,T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,10)
				INTERPOLATE_ONE_LINE(T_00_11,T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,11)
				INTERPOLATE_ONE_LINE(T_00_12,T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,12)
				INTERPOLATE_ONE_LINE(T_00_13,T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,13)
				INTERPOLATE_ONE_LINE(T_00_14,T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,T_00_21,14)
				INTERPOLATE_ONE_LINE(T_00_15,T_00_16,T_00_17,T_00_18,T_00_19,T_00_20,T_00_21,T_00_22,15)
#undef INTERPOLATE_ONE_LINE
		}
	}
	//xInterpolate8V_32x32(pucRef,	pucPred,	nRefStride, 32, 32, nFrac);
	//xInterpolate8V_32x32(pucRef+32, pucPred+32, nRefStride, 32, 32, nFrac);
	//xInterpolate8V_32x32(pucRef+32*nRefStride,		pucPred+32*MAX_CU_SIZE,		nRefStride, 32, 32, nFrac);
	//xInterpolate8V_32x32(pucRef+32*nRefStride+32,	pucPred+32*MAX_CU_SIZE+32,	nRefStride, 32, 32, nFrac);
}

typedef void (*xInterpolate8V_ptr)(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	);

xInterpolate8V_ptr xInterpolate8V_NxN[5] = {
	NULL,
	xInterpolate8V_8x8,
	xInterpolate8V_16x16,
	xInterpolate8V_32x32,
	xInterpolate8V_64x64
};

#else
void xInterpolate8V(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	//assert( nFrac != 0 );
	int i, j;

	for( i=0; i<(int)nSizeX; i++ ) {
		CUInt8 *P = pucRef  + i;
		UInt8 *Q = pucPred + i;

		for( j=0; j<(int)nSizeY; j++ ) {
			Int16 iSum;
			iSum  = P[(j-3) * nRefStride] * xg_lumaFilter[nFrac][0];
			iSum += P[(j-2) * nRefStride] * xg_lumaFilter[nFrac][1];
			iSum += P[(j-1) * nRefStride] * xg_lumaFilter[nFrac][2];
			iSum += P[(j+0) * nRefStride] * xg_lumaFilter[nFrac][3];
			iSum += P[(j+1) * nRefStride] * xg_lumaFilter[nFrac][4];
			iSum += P[(j+2) * nRefStride] * xg_lumaFilter[nFrac][5];
			iSum += P[(j+3) * nRefStride] * xg_lumaFilter[nFrac][6];
			iSum += P[(j+4) * nRefStride] * xg_lumaFilter[nFrac][7];
			Q[j*MAX_CU_SIZE] = Clip3( 0, 255, (iSum + 32) >> 6);
		}
	}
}
#endif

#if (INTERPOLATE_USE_ASM >= ASM_SSE4)
void xInterpolate8HV_8x8(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16      *psTmp
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning
	(void *)psTmp;//avoid warning

	//new interpolate, 2013-12-6
	Int16 Xxg0 = (xg_lumaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_lumaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_lumaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_lumaFilter[xFrac][3]);
	Int16 Xxg4 = (xg_lumaFilter[xFrac][4]);
	Int16 Xxg5 = (xg_lumaFilter[xFrac][5]);
	Int16 Xxg6 = (xg_lumaFilter[xFrac][6]);
	Int16 Xxg7 = (xg_lumaFilter[xFrac][7]);
	Int16 Yxg0 = (xg_lumaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_lumaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_lumaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_lumaFilter[yFrac][3]);
	Int16 Yxg4 = (xg_lumaFilter[yFrac][4]);
	Int16 Yxg5 = (xg_lumaFilter[yFrac][5]);
	Int16 Yxg6 = (xg_lumaFilter[yFrac][6]);
	Int16 Yxg7 = (xg_lumaFilter[yFrac][7]);
	Int32 xg0123 = ((Xxg3<<24)|((Xxg2&0x00FF)<<16)|((Xxg1&0x00FF)<<8)|(Xxg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((Xxg7<<24)|((Xxg6&0x00FF)<<16)|((Xxg5&0x00FF)<<8)|(Xxg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i xg_luma	  = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);
	const __m128i xg16_luma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_luma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	const __m128i xg16_luma45 = _mm_set1_epi32( (Yxg5<<16) | (Yxg4 & 0xFFFF) );
	const __m128i xg16_luma67 = _mm_set1_epi32( (Yxg7<<16) | (Yxg6 & 0xFFFF) );

	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	__m128i	tmp16[8+NTAPS_LUMA-1];

	//Stage1. Interpolate H
	int nSize = 8;
	for(int i=0; i<(int)nSize+NTAPS_LUMA-1; i++ ) {
		Int offset0 = (i-3)*nRefStride;
		const __m128i T_00_00	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0-3));//8bit: Ref [xx xx xx xx xx xx xx xx 04 03 02 01 00 -01 -02 -03]
		const __m128i T_00_01	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0-2));//8bit: Ref [xx xx xx xx xx xx xx xx 05 04 03 02 01  00 -01 -02]
		const __m128i T_00_02	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0-1));//8bit: Ref [xx xx xx xx xx xx xx xx 06 05 04 03 02  01  00 -01]
		const __m128i T_00_03	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+0));//8bit: Ref [xx xx xx xx xx xx xx xx 07 06 05 04 03  02  01  00]
		const __m128i T_00_04	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+1));//8bit: Ref [xx xx xx xx xx xx xx xx 08 07 06 05 04  03  02  01]
		const __m128i T_00_05	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+2));//8bit: Ref [xx xx xx xx xx xx xx xx 09 08 07 06 05  04  03  02]
		const __m128i T_00_06	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+3));//8bit: Ref [xx xx xx xx xx xx xx xx 10 09 08 07 06  05  04  03]
		const __m128i T_00_07	= _mm_loadl_epi64((const __m128i*)(pucRef+offset0+4));//8bit: Ref [xx xx xx xx xx xx xx xx 11 10 09 08 07  06  05  04]
		const __m128i T_01_00	= _mm_unpacklo_epi64(T_00_00, T_00_01);	    
		const __m128i T_01_01	= _mm_unpacklo_epi64(T_00_02, T_00_03);		
		const __m128i T_01_02	= _mm_unpacklo_epi64(T_00_04, T_00_05);		
		const __m128i T_01_03	= _mm_unpacklo_epi64(T_00_06, T_00_07);		
		const __m128i T_02_00	= _mm_maddubs_epi16(T_01_00, xg_luma);	    //16bit: 01 00 abcd
		const __m128i T_02_01	= _mm_maddubs_epi16(T_01_01, xg_luma);		//16bit: 03 02 abcd
		const __m128i T_02_02	= _mm_maddubs_epi16(T_01_02, xg_luma);		//16bit: 05 04 abcd
		const __m128i T_02_03	= _mm_maddubs_epi16(T_01_03, xg_luma);		//16bit: 07 06 abcd
		const __m128i T_03_00a	= _mm_hadd_epi16(T_02_00, T_02_01);
		const __m128i T_03_00b	= _mm_hadd_epi16(T_02_02, T_02_03);  
		const __m128i T_03_00	= _mm_hadd_epi16(T_03_00a, T_03_00b);//16bit: 07 06 05 04 03 02 01 00
		tmp16[i] = _mm_sub_epi16(T_03_00, c_H);			
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<(int)nSize; i++ ) {
		Int offset1 = i*MAX_CU_SIZE;
		const __m128i	T_00_00d	= _mm_unpackhi_epi16(tmp16[i  ], tmp16[i+1]);//16bit: Ref [07 07 06 06 05 05 04 04] Row: -2,-3
		const __m128i	T_00_01d	= _mm_unpacklo_epi16(tmp16[i  ], tmp16[i+1]);//16bit: Ref [03 03 02 02 01 01 00 00] Row: -2,-3
		const __m128i	T_00_02d	= _mm_unpackhi_epi16(tmp16[i+2], tmp16[i+3]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  0,-1
		const __m128i	T_00_03d	= _mm_unpacklo_epi16(tmp16[i+2], tmp16[i+3]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  0,-1
		const __m128i	T_00_04d	= _mm_unpackhi_epi16(tmp16[i+4], tmp16[i+5]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  2, 1
		const __m128i	T_00_05d	= _mm_unpacklo_epi16(tmp16[i+4], tmp16[i+5]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  2, 1
		const __m128i	T_00_06d	= _mm_unpackhi_epi16(tmp16[i+6], tmp16[i+7]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  4, 3
		const __m128i	T_00_07d	= _mm_unpacklo_epi16(tmp16[i+6], tmp16[i+7]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  4, 3
		const __m128i	T_03_24		= _mm_madd_epi16(xg16_luma01, T_00_00d);//32bit: Ref [07 0605 04]
		const __m128i	T_03_25		= _mm_madd_epi16(xg16_luma01, T_00_01d);//32bit: Ref [03 02 01 00]
		const __m128i	T_03_26		= _mm_madd_epi16(xg16_luma23, T_00_02d);//32bit: Ref []
		const __m128i	T_03_27		= _mm_madd_epi16(xg16_luma23, T_00_03d);//32bit: Ref []
		const __m128i	T_03_28		= _mm_madd_epi16(xg16_luma45, T_00_04d);//32bit: Ref []
		const __m128i	T_03_29		= _mm_madd_epi16(xg16_luma45, T_00_05d);//32bit: Ref []
		const __m128i	T_03_30		= _mm_madd_epi16(xg16_luma67, T_00_06d);//32bit: Ref []
		const __m128i	T_03_31		= _mm_madd_epi16(xg16_luma67, T_00_07d);//32bit: Ref []
		const __m128i	T_05_01		= _mm_add_epi32(_mm_add_epi32(T_03_24, T_03_26), _mm_add_epi32(T_03_28, T_03_30));//32bit: [07 06 05 04] 
		const __m128i	T_05_00		= _mm_add_epi32(_mm_add_epi32(T_03_25, T_03_27), _mm_add_epi32(T_03_29, T_03_31));//32bit: [03 02 01 00] 
		const __m128i	T_07_00a	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_00), 12);
		const __m128i	T_07_00b	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_01), 12);
		const __m128i	T_08_00a    = _mm_packus_epi32(T_07_00a, T_07_00b); //16bit: Pred:07~00 needed
		const __m128i	T_09_00a    = _mm_packus_epi16(T_08_00a, T_08_00a); // 8bit: Pred:07~00 needed
		_mm_storel_epi64((__m128i *)(pucPred+offset1), T_09_00a);
	}
}
void xInterpolate8HV_16x16(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16      *psTmp
	)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning
	(void *)psTmp;//x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 Xxg0 = (xg_lumaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_lumaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_lumaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_lumaFilter[xFrac][3]);
	Int16 Xxg4 = (xg_lumaFilter[xFrac][4]);
	Int16 Xxg5 = (xg_lumaFilter[xFrac][5]);
	Int16 Xxg6 = (xg_lumaFilter[xFrac][6]);
	Int16 Xxg7 = (xg_lumaFilter[xFrac][7]);
	Int16 Yxg0 = (xg_lumaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_lumaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_lumaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_lumaFilter[yFrac][3]);
	Int16 Yxg4 = (xg_lumaFilter[yFrac][4]);
	Int16 Yxg5 = (xg_lumaFilter[yFrac][5]);
	Int16 Yxg6 = (xg_lumaFilter[yFrac][6]);
	Int16 Yxg7 = (xg_lumaFilter[yFrac][7]);
	Int32 xg0123 = ((Xxg3<<24)|((Xxg2&0x00FF)<<16)|((Xxg1&0x00FF)<<8)|(Xxg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((Xxg7<<24)|((Xxg6&0x00FF)<<16)|((Xxg5&0x00FF)<<8)|(Xxg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i xg_luma	  = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);
	const __m128i xg16_luma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_luma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	const __m128i xg16_luma45 = _mm_set1_epi32( (Yxg5<<16) | (Yxg4 & 0xFFFF) );
	const __m128i xg16_luma67 = _mm_set1_epi32( (Yxg7<<16) | (Yxg6 & 0xFFFF) );

	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	__m128i	tmp16[16+NTAPS_LUMA-1][2];

	//Stage1. Interpolate H
	int nSize = 16;
	for (Int i=0; i<(int)nSize+NTAPS_LUMA-1; i+=1){
		Int offset0 = (i-3)*nRefStride;
		const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-3)); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
		const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-2)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
		const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-1)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
		const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-0)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
		const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+1)); //8bit: [16 15 14 13 12 11 10 09 08 07 06 05 04  03  02  01]
		const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+2)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05  04  03  02]
		const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+3)); //8bit: [18 17 16 15 14 13 12 11 10 09 08 07 06  05  04  03]
		const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+4)); //8bit: [19 18 17 16 15 14 13 12 11 10 09 08 07  06  05  04]
		const __m128i T_01_00	= _mm_maddubs_epi16(T_00_00, xg_luma);	    //16bit: 08 00 abcd
		const __m128i T_01_01	= _mm_maddubs_epi16(T_00_01, xg_luma);		//16bit: 09 01 abcd
		const __m128i T_01_02	= _mm_maddubs_epi16(T_00_02, xg_luma);		//16bit: 10 02 abcd
		const __m128i T_01_03	= _mm_maddubs_epi16(T_00_03, xg_luma);		//16bit: 11 03 abcd
		const __m128i T_01_04	= _mm_maddubs_epi16(T_00_04, xg_luma);	    //16bit: 12 04 abcd
		const __m128i T_01_05	= _mm_maddubs_epi16(T_00_05, xg_luma);		//16bit: 13 05 abcd
		const __m128i T_01_06	= _mm_maddubs_epi16(T_00_06, xg_luma);		//16bit: 14 06 abcd
		const __m128i T_01_07	= _mm_maddubs_epi16(T_00_07, xg_luma);		//16bit: 15 07 abcd
		const __m128i T_02_00	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_00, T_01_02), _mm_hadd_epi16(T_01_04, T_01_06));//16bit: 14 06 12 04 10 02 08 00
		const __m128i T_02_01	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_01, T_01_03), _mm_hadd_epi16(T_01_05, T_01_07));//16bit: 15 07 13 05 11 03 09 01
		const __m128i T_03_00   = _mm_unpacklo_epi16(T_02_00, T_02_01);	//16bit: 11 10 03 02 09 08 01 00
		const __m128i T_03_01   = _mm_unpackhi_epi16(T_02_00, T_02_01);	//16bit: 15 14 07 06 13 12 05 04
		const __m128i T_04_00   = _mm_shuffle_epi32(T_03_00, 0xD8);		//16bit: 11 10 09 08 03 02 01 00
		const __m128i T_04_01   = _mm_shuffle_epi32(T_03_01, 0xD8);		//16bit: 15 14 13 12 07 06 05 04
		const __m128i T_05_00	= _mm_unpacklo_epi64(T_04_00, T_04_01); //16bit: 07 06 05 04 03 02 01 00
		const __m128i T_05_01	= _mm_unpackhi_epi64(T_04_00, T_04_01); //16bit: 15 14 13 12 11 10 09 08
		tmp16[i][0]	= _mm_sub_epi16(T_05_00, c_H);
		tmp16[i][1]	= _mm_sub_epi16(T_05_01, c_H);
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<16; i++ ) {
		Int offset1 = i*MAX_CU_SIZE;
		const __m128i	T_00_00c	= _mm_unpackhi_epi16(tmp16[i  ][1], tmp16[i+1][1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row: -2,-3
		const __m128i	T_00_01c	= _mm_unpacklo_epi16(tmp16[i  ][1], tmp16[i+1][1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row: -2,-3
		const __m128i	T_00_02c	= _mm_unpackhi_epi16(tmp16[i+2][1], tmp16[i+3][1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  0,-1
		const __m128i	T_00_03c	= _mm_unpacklo_epi16(tmp16[i+2][1], tmp16[i+3][1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  0,-1
		const __m128i	T_00_04c	= _mm_unpackhi_epi16(tmp16[i+4][1], tmp16[i+5][1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  2, 1
		const __m128i	T_00_05c	= _mm_unpacklo_epi16(tmp16[i+4][1], tmp16[i+5][1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  2, 1
		const __m128i	T_00_06c	= _mm_unpackhi_epi16(tmp16[i+6][1], tmp16[i+7][1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  4, 3
		const __m128i	T_00_07c	= _mm_unpacklo_epi16(tmp16[i+6][1], tmp16[i+7][1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  4, 3
		const __m128i	T_00_00d	= _mm_unpackhi_epi16(tmp16[i  ][0], tmp16[i+1][0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row: -2,-3
		const __m128i	T_00_01d	= _mm_unpacklo_epi16(tmp16[i  ][0], tmp16[i+1][0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row: -2,-3
		const __m128i	T_00_02d	= _mm_unpackhi_epi16(tmp16[i+2][0], tmp16[i+3][0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  0,-1
		const __m128i	T_00_03d	= _mm_unpacklo_epi16(tmp16[i+2][0], tmp16[i+3][0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  0,-1
		const __m128i	T_00_04d	= _mm_unpackhi_epi16(tmp16[i+4][0], tmp16[i+5][0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  2, 1
		const __m128i	T_00_05d	= _mm_unpacklo_epi16(tmp16[i+4][0], tmp16[i+5][0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  2, 1
		const __m128i	T_00_06d	= _mm_unpackhi_epi16(tmp16[i+6][0], tmp16[i+7][0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  4, 3
		const __m128i	T_00_07d	= _mm_unpacklo_epi16(tmp16[i+6][0], tmp16[i+7][0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  4, 3
		const __m128i	T_03_16		= _mm_madd_epi16(xg16_luma01, T_00_00c);//32bit: Ref [15 14 13 12]
		const __m128i	T_03_17		= _mm_madd_epi16(xg16_luma01, T_00_01c);//32bit: Ref [11 10 09 08]
		const __m128i	T_03_18		= _mm_madd_epi16(xg16_luma23, T_00_02c);//32bit: Ref []
		const __m128i	T_03_19		= _mm_madd_epi16(xg16_luma23, T_00_03c);//32bit: Ref []
		const __m128i	T_03_20		= _mm_madd_epi16(xg16_luma45, T_00_04c);//32bit: Ref []
		const __m128i	T_03_21		= _mm_madd_epi16(xg16_luma45, T_00_05c);//32bit: Ref []
		const __m128i	T_03_22		= _mm_madd_epi16(xg16_luma67, T_00_06c);//32bit: Ref []
		const __m128i	T_03_23		= _mm_madd_epi16(xg16_luma67, T_00_07c);//32bit: Ref []
		const __m128i	T_03_24		= _mm_madd_epi16(xg16_luma01, T_00_00d);//32bit: Ref [07 0605 04]
		const __m128i	T_03_25		= _mm_madd_epi16(xg16_luma01, T_00_01d);//32bit: Ref [03 02 01 00]
		const __m128i	T_03_26		= _mm_madd_epi16(xg16_luma23, T_00_02d);//32bit: Ref []
		const __m128i	T_03_27		= _mm_madd_epi16(xg16_luma23, T_00_03d);//32bit: Ref []
		const __m128i	T_03_28		= _mm_madd_epi16(xg16_luma45, T_00_04d);//32bit: Ref []
		const __m128i	T_03_29		= _mm_madd_epi16(xg16_luma45, T_00_05d);//32bit: Ref []
		const __m128i	T_03_30		= _mm_madd_epi16(xg16_luma67, T_00_06d);//32bit: Ref []
		const __m128i	T_03_31		= _mm_madd_epi16(xg16_luma67, T_00_07d);//32bit: Ref []
		const __m128i	T_05_03		= _mm_add_epi32(_mm_add_epi32(T_03_16, T_03_18), _mm_add_epi32(T_03_20, T_03_22));//32bit: [15 14 13 12] 
		const __m128i	T_05_02		= _mm_add_epi32(_mm_add_epi32(T_03_17, T_03_19), _mm_add_epi32(T_03_21, T_03_23));//32bit: [11 10 09 08] 
		const __m128i	T_05_01		= _mm_add_epi32(_mm_add_epi32(T_03_24, T_03_26), _mm_add_epi32(T_03_28, T_03_30));//32bit: [07 06 05 04] 
		const __m128i	T_05_00		= _mm_add_epi32(_mm_add_epi32(T_03_25, T_03_27), _mm_add_epi32(T_03_29, T_03_31));//32bit: [03 02 01 00] 
		const __m128i	T_07_00a	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_00), 12);
		const __m128i	T_07_00b	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_01), 12);
		const __m128i	T_07_00c	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_02), 12);
		const __m128i	T_07_00d    = _mm_srai_epi32(_mm_add_epi32(c_V, T_05_03), 12);
		const __m128i	T_08_00a    = _mm_packus_epi32(T_07_00a, T_07_00b); //16bit: Pred:07~00 needed
		const __m128i	T_08_00b    = _mm_packus_epi32(T_07_00c, T_07_00d); //16bit: Pred:15~08 needed
		const __m128i	T_09_00a    = _mm_packus_epi16(T_08_00a, T_08_00b); // 8bit: Pred:15~00 needed
		_mm_storeu_si128((__m128i *)(pucPred+offset1), T_09_00a);
	}
}
void xInterpolate8HV_32x32(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16      *psTmp)

{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning
	(void *)psTmp;//avoid warning

	//new interpolate, 2013-12-6
	Int16 Xxg0 = (xg_lumaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_lumaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_lumaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_lumaFilter[xFrac][3]);
	Int16 Xxg4 = (xg_lumaFilter[xFrac][4]);
	Int16 Xxg5 = (xg_lumaFilter[xFrac][5]);
	Int16 Xxg6 = (xg_lumaFilter[xFrac][6]);
	Int16 Xxg7 = (xg_lumaFilter[xFrac][7]);
	Int16 Yxg0 = (xg_lumaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_lumaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_lumaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_lumaFilter[yFrac][3]);
	Int16 Yxg4 = (xg_lumaFilter[yFrac][4]);
	Int16 Yxg5 = (xg_lumaFilter[yFrac][5]);
	Int16 Yxg6 = (xg_lumaFilter[yFrac][6]);
	Int16 Yxg7 = (xg_lumaFilter[yFrac][7]);
	Int32 xg0123 = ((Xxg3<<24)|((Xxg2&0x00FF)<<16)|((Xxg1&0x00FF)<<8)|(Xxg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((Xxg7<<24)|((Xxg6&0x00FF)<<16)|((Xxg5&0x00FF)<<8)|(Xxg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i xg_luma	  = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);
	const __m128i xg16_luma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_luma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	const __m128i xg16_luma45 = _mm_set1_epi32( (Yxg5<<16) | (Yxg4 & 0xFFFF) );
	const __m128i xg16_luma67 = _mm_set1_epi32( (Yxg7<<16) | (Yxg6 & 0xFFFF) );

	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	__m128i	tmp16[32+NTAPS_LUMA-1][4];

	//Stage1. Interpolate H
	int nSize = 32;
	for (Int i=0; i<(int)nSize+NTAPS_LUMA-1; i+=1){
		for (Int j=0; j<2; j++){
			Int offset0 = (i-3)*nRefStride+j*16;;

			const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-3)); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
			const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-2)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
			const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-1)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
			const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-0)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
			const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+1)); //8bit: [16 15 14 13 12 11 10 09 08 07 06 05 04  03  02  01]
			const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+2)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05  04  03  02]
			const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+3)); //8bit: [18 17 16 15 14 13 12 11 10 09 08 07 06  05  04  03]
			const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+4)); //8bit: [19 18 17 16 15 14 13 12 11 10 09 08 07  06  05  04]

			const __m128i T_01_00	= _mm_maddubs_epi16(T_00_00, xg_luma);	    //16bit: 08 00 abcd
			const __m128i T_01_01	= _mm_maddubs_epi16(T_00_01, xg_luma);		//16bit: 09 01 abcd
			const __m128i T_01_02	= _mm_maddubs_epi16(T_00_02, xg_luma);		//16bit: 10 02 abcd
			const __m128i T_01_03	= _mm_maddubs_epi16(T_00_03, xg_luma);		//16bit: 11 03 abcd
			const __m128i T_01_04	= _mm_maddubs_epi16(T_00_04, xg_luma);	    //16bit: 12 04 abcd
			const __m128i T_01_05	= _mm_maddubs_epi16(T_00_05, xg_luma);		//16bit: 13 05 abcd
			const __m128i T_01_06	= _mm_maddubs_epi16(T_00_06, xg_luma);		//16bit: 14 06 abcd
			const __m128i T_01_07	= _mm_maddubs_epi16(T_00_07, xg_luma);		//16bit: 15 07 abcd

			const __m128i T_02_00	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_00, T_01_02), _mm_hadd_epi16(T_01_04, T_01_06));//16bit: 14 06 12 04 10 02 08 00
			const __m128i T_02_01	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_01, T_01_03), _mm_hadd_epi16(T_01_05, T_01_07));//16bit: 15 07 13 05 11 03 09 01
			const __m128i T_03_00   = _mm_unpacklo_epi16(T_02_00, T_02_01);	//16bit: 11 10 03 02 09 08 01 00
			const __m128i T_03_01   = _mm_unpackhi_epi16(T_02_00, T_02_01);	//16bit: 15 14 07 06 13 12 05 04
			const __m128i T_04_00   = _mm_shuffle_epi32(T_03_00, 0xD8);		//16bit: 11 10 09 08 03 02 01 00
			const __m128i T_04_01   = _mm_shuffle_epi32(T_03_01, 0xD8);		//16bit: 15 14 13 12 07 06 05 04
			const __m128i T_05_00	= _mm_unpacklo_epi64(T_04_00, T_04_01); //16bit: 07 06 05 04 03 02 01 00
			const __m128i T_05_01	= _mm_unpackhi_epi64(T_04_00, T_04_01); //16bit: 15 14 13 12 11 10 09 08
			tmp16[i][j*2+0]	= _mm_sub_epi16(T_05_00, c_H);
			tmp16[i][j*2+1]	= _mm_sub_epi16(T_05_01, c_H);
		}
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<(int)nSize; i++ ) {
		for (Int16 j=0; j<2; j++){
			Int k = j*2;
			Int offset1 = i*MAX_CU_SIZE+j*16;
			__m128i	T_00_00c		= _mm_unpackhi_epi16(tmp16[i  ][k+1], tmp16[i+1][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row: -2,-3
			__m128i	T_00_01c		= _mm_unpacklo_epi16(tmp16[i  ][k+1], tmp16[i+1][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row: -2,-3
			__m128i	T_00_02c		= _mm_unpackhi_epi16(tmp16[i+2][k+1], tmp16[i+3][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  0,-1
			__m128i	T_00_03c		= _mm_unpacklo_epi16(tmp16[i+2][k+1], tmp16[i+3][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  0,-1
			__m128i	T_00_04c		= _mm_unpackhi_epi16(tmp16[i+4][k+1], tmp16[i+5][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  2, 1
			__m128i	T_00_05c		= _mm_unpacklo_epi16(tmp16[i+4][k+1], tmp16[i+5][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  2, 1
			__m128i	T_00_06c		= _mm_unpackhi_epi16(tmp16[i+6][k+1], tmp16[i+7][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  4, 3
			__m128i	T_00_07c		= _mm_unpacklo_epi16(tmp16[i+6][k+1], tmp16[i+7][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  4, 3
			__m128i	T_00_00d		= _mm_unpackhi_epi16(tmp16[i  ][k+0], tmp16[i+1][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row: -2,-3
			__m128i	T_00_01d		= _mm_unpacklo_epi16(tmp16[i  ][k+0], tmp16[i+1][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row: -2,-3
			__m128i	T_00_02d		= _mm_unpackhi_epi16(tmp16[i+2][k+0], tmp16[i+3][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  0,-1
			__m128i	T_00_03d		= _mm_unpacklo_epi16(tmp16[i+2][k+0], tmp16[i+3][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  0,-1
			__m128i	T_00_04d		= _mm_unpackhi_epi16(tmp16[i+4][k+0], tmp16[i+5][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  2, 1
			__m128i	T_00_05d		= _mm_unpacklo_epi16(tmp16[i+4][k+0], tmp16[i+5][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  2, 1
			__m128i	T_00_06d		= _mm_unpackhi_epi16(tmp16[i+6][k+0], tmp16[i+7][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  4, 3
			__m128i	T_00_07d		= _mm_unpacklo_epi16(tmp16[i+6][k+0], tmp16[i+7][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  4, 3

			__m128i	T_03_16		= _mm_madd_epi16(xg16_luma01, T_00_00c);//32bit: Ref [15 14 13 12]
			__m128i	T_03_17		= _mm_madd_epi16(xg16_luma01, T_00_01c);//32bit: Ref [11 10 09 08]
			__m128i	T_03_18		= _mm_madd_epi16(xg16_luma23, T_00_02c);//32bit: Ref []
			__m128i	T_03_19		= _mm_madd_epi16(xg16_luma23, T_00_03c);//32bit: Ref []
			__m128i	T_03_20		= _mm_madd_epi16(xg16_luma45, T_00_04c);//32bit: Ref []
			__m128i	T_03_21		= _mm_madd_epi16(xg16_luma45, T_00_05c);//32bit: Ref []
			__m128i	T_03_22		= _mm_madd_epi16(xg16_luma67, T_00_06c);//32bit: Ref []
			__m128i	T_03_23		= _mm_madd_epi16(xg16_luma67, T_00_07c);//32bit: Ref []
			__m128i	T_03_24		= _mm_madd_epi16(xg16_luma01, T_00_00d);//32bit: Ref [07 0605 04]
			__m128i	T_03_25		= _mm_madd_epi16(xg16_luma01, T_00_01d);//32bit: Ref [03 02 01 00]
			__m128i	T_03_26		= _mm_madd_epi16(xg16_luma23, T_00_02d);//32bit: Ref []
			__m128i	T_03_27		= _mm_madd_epi16(xg16_luma23, T_00_03d);//32bit: Ref []
			__m128i	T_03_28		= _mm_madd_epi16(xg16_luma45, T_00_04d);//32bit: Ref []
			__m128i	T_03_29		= _mm_madd_epi16(xg16_luma45, T_00_05d);//32bit: Ref []
			__m128i	T_03_30		= _mm_madd_epi16(xg16_luma67, T_00_06d);//32bit: Ref []
			__m128i	T_03_31		= _mm_madd_epi16(xg16_luma67, T_00_07d);//32bit: Ref []

			const __m128i	T_05_03		= _mm_add_epi32(_mm_add_epi32(T_03_16, T_03_18), _mm_add_epi32(T_03_20, T_03_22));//32bit: [15 14 13 12] 
			const __m128i	T_05_02		= _mm_add_epi32(_mm_add_epi32(T_03_17, T_03_19), _mm_add_epi32(T_03_21, T_03_23));//32bit: [11 10 09 08] 
			const __m128i	T_05_01		= _mm_add_epi32(_mm_add_epi32(T_03_24, T_03_26), _mm_add_epi32(T_03_28, T_03_30));//32bit: [07 06 05 04] 
			const __m128i	T_05_00		= _mm_add_epi32(_mm_add_epi32(T_03_25, T_03_27), _mm_add_epi32(T_03_29, T_03_31));//32bit: [03 02 01 00] 

			const __m128i	T_07_00a	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_00), 12);				  //16bit: Pred:03~00 needed
			const __m128i	T_07_00b	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_01), 12);				  //16bit: Pred:07~04 needed
			const __m128i	T_07_00c	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_02), 12);				  //16bit: Pred:11~08 needed
			const __m128i	T_07_00d	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_03), 12);				  //16bit: Pred:15~12 needed
			const __m128i	T_08_00a  = _mm_packus_epi32(T_07_00a, T_07_00b); //16bit: Pred:07~00 needed
			const __m128i	T_08_00b  = _mm_packus_epi32(T_07_00c, T_07_00d); //16bit: Pred:15~08 needed
			const __m128i	T_09_00a  = _mm_packus_epi16(T_08_00a, T_08_00b); // 8bit: Pred:15~00 needed
			_mm_storeu_si128((__m128i *)(pucPred+offset1), T_09_00a);
		}
	}
}
void xInterpolate8HV_64x64(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16      *psTmp)
{
	(void)nSizeX; //x64 modify, avoid warning
	(void)nSizeY; //x64 modify, avoid warning
	(void *)psTmp;//x64 modify, avoid warning

	//new interpolate, 2013-12-6 
	Int16 Xxg0 = (xg_lumaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_lumaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_lumaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_lumaFilter[xFrac][3]);
	Int16 Xxg4 = (xg_lumaFilter[xFrac][4]);
	Int16 Xxg5 = (xg_lumaFilter[xFrac][5]);
	Int16 Xxg6 = (xg_lumaFilter[xFrac][6]);
	Int16 Xxg7 = (xg_lumaFilter[xFrac][7]);
	Int16 Yxg0 = (xg_lumaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_lumaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_lumaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_lumaFilter[yFrac][3]);
	Int16 Yxg4 = (xg_lumaFilter[yFrac][4]);
	Int16 Yxg5 = (xg_lumaFilter[yFrac][5]);
	Int16 Yxg6 = (xg_lumaFilter[yFrac][6]);
	Int16 Yxg7 = (xg_lumaFilter[yFrac][7]);
	Int32 xg0123 = ((Xxg3<<24)|((Xxg2&0x00FF)<<16)|((Xxg1&0x00FF)<<8)|(Xxg0&0x00FF)) & 0xFFFFFFFF;
	Int32 xg4567 = ((Xxg7<<24)|((Xxg6&0x00FF)<<16)|((Xxg5&0x00FF)<<8)|(Xxg4&0x00FF)) & 0xFFFFFFFF;
	const __m128i xg_luma	  = _mm_setr_epi32(xg0123, xg4567, xg0123, xg4567);
	const __m128i xg16_luma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_luma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	const __m128i xg16_luma45 = _mm_set1_epi32( (Yxg5<<16) | (Yxg4 & 0xFFFF) );
	const __m128i xg16_luma67 = _mm_set1_epi32( (Yxg7<<16) | (Yxg6 & 0xFFFF) );

	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	__m128i	tmp16[64+NTAPS_LUMA-1][8];

	//Stage1. Interpolate H
	int nSize = 64;
	for (Int i=0; i<(int)nSize+NTAPS_LUMA-1; i+=1){
		for (Int j=0; j<4; j++){
			Int offset0 = (i-3)*nRefStride+j*16;;

			const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-3)); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
			const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-2)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
			const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-1)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
			const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+offset0-0)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
			const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+1)); //8bit: [16 15 14 13 12 11 10 09 08 07 06 05 04  03  02  01]
			const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+2)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05  04  03  02]
			const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+3)); //8bit: [18 17 16 15 14 13 12 11 10 09 08 07 06  05  04  03]
			const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+offset0+4)); //8bit: [19 18 17 16 15 14 13 12 11 10 09 08 07  06  05  04]

			const __m128i T_01_00	= _mm_maddubs_epi16(T_00_00, xg_luma);	    //16bit: 08 00 abcd
			const __m128i T_01_01	= _mm_maddubs_epi16(T_00_01, xg_luma);		//16bit: 09 01 abcd
			const __m128i T_01_02	= _mm_maddubs_epi16(T_00_02, xg_luma);		//16bit: 10 02 abcd
			const __m128i T_01_03	= _mm_maddubs_epi16(T_00_03, xg_luma);		//16bit: 11 03 abcd
			const __m128i T_01_04	= _mm_maddubs_epi16(T_00_04, xg_luma);	    //16bit: 12 04 abcd
			const __m128i T_01_05	= _mm_maddubs_epi16(T_00_05, xg_luma);		//16bit: 13 05 abcd
			const __m128i T_01_06	= _mm_maddubs_epi16(T_00_06, xg_luma);		//16bit: 14 06 abcd
			const __m128i T_01_07	= _mm_maddubs_epi16(T_00_07, xg_luma);		//16bit: 15 07 abcd

			const __m128i T_02_00	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_00, T_01_02), _mm_hadd_epi16(T_01_04, T_01_06));//16bit: 14 06 12 04 10 02 08 00
			const __m128i T_02_01	= _mm_hadd_epi16(_mm_hadd_epi16(T_01_01, T_01_03), _mm_hadd_epi16(T_01_05, T_01_07));//16bit: 15 07 13 05 11 03 09 01
			const __m128i T_03_00   = _mm_unpacklo_epi16(T_02_00, T_02_01);	//16bit: 11 10 03 02 09 08 01 00
			const __m128i T_03_01   = _mm_unpackhi_epi16(T_02_00, T_02_01);	//16bit: 15 14 07 06 13 12 05 04
			const __m128i T_04_00   = _mm_shuffle_epi32(T_03_00, 0xD8);		//16bit: 11 10 09 08 03 02 01 00
			const __m128i T_04_01   = _mm_shuffle_epi32(T_03_01, 0xD8);		//16bit: 15 14 13 12 07 06 05 04
			const __m128i T_05_00	= _mm_unpacklo_epi64(T_04_00, T_04_01); //16bit: 07 06 05 04 03 02 01 00
			const __m128i T_05_01	= _mm_unpackhi_epi64(T_04_00, T_04_01); //16bit: 15 14 13 12 11 10 09 08
			tmp16[i][j*2+0]	= _mm_sub_epi16(T_05_00, c_H);
			tmp16[i][j*2+1]	= _mm_sub_epi16(T_05_01, c_H);
		}
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<(int)nSize; i++ ) {
		for (Int16 j=0; j<4; j++){
			Int k = j*2;
			Int offset1 = i*MAX_CU_SIZE+j*16;
			__m128i	T_00_00c		= _mm_unpackhi_epi16(tmp16[i  ][k+1], tmp16[i+1][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row: -2,-3
			__m128i	T_00_01c		= _mm_unpacklo_epi16(tmp16[i  ][k+1], tmp16[i+1][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row: -2,-3
			__m128i	T_00_02c		= _mm_unpackhi_epi16(tmp16[i+2][k+1], tmp16[i+3][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  0,-1
			__m128i	T_00_03c		= _mm_unpacklo_epi16(tmp16[i+2][k+1], tmp16[i+3][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  0,-1
			__m128i	T_00_04c		= _mm_unpackhi_epi16(tmp16[i+4][k+1], tmp16[i+5][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  2, 1
			__m128i	T_00_05c		= _mm_unpacklo_epi16(tmp16[i+4][k+1], tmp16[i+5][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  2, 1
			__m128i	T_00_06c		= _mm_unpackhi_epi16(tmp16[i+6][k+1], tmp16[i+7][k+1]);//16bit: Ref [15 15 14 14 13 13 12 12] Row:  4, 3
			__m128i	T_00_07c		= _mm_unpacklo_epi16(tmp16[i+6][k+1], tmp16[i+7][k+1]);//16bit: Ref [11 11 10 10 09 09 08 08] Row:  4, 3
			__m128i	T_00_00d		= _mm_unpackhi_epi16(tmp16[i  ][k+0], tmp16[i+1][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row: -2,-3
			__m128i	T_00_01d		= _mm_unpacklo_epi16(tmp16[i  ][k+0], tmp16[i+1][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row: -2,-3
			__m128i	T_00_02d		= _mm_unpackhi_epi16(tmp16[i+2][k+0], tmp16[i+3][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  0,-1
			__m128i	T_00_03d		= _mm_unpacklo_epi16(tmp16[i+2][k+0], tmp16[i+3][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  0,-1
			__m128i	T_00_04d		= _mm_unpackhi_epi16(tmp16[i+4][k+0], tmp16[i+5][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  2, 1
			__m128i	T_00_05d		= _mm_unpacklo_epi16(tmp16[i+4][k+0], tmp16[i+5][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  2, 1
			__m128i	T_00_06d		= _mm_unpackhi_epi16(tmp16[i+6][k+0], tmp16[i+7][k+0]);//16bit: Ref [07 07 06 06 05 05 04 04] Row:  4, 3
			__m128i	T_00_07d		= _mm_unpacklo_epi16(tmp16[i+6][k+0], tmp16[i+7][k+0]);//16bit: Ref [03 03 02 02 01 01 00 00] Row:  4, 3

			__m128i	T_03_16		= _mm_madd_epi16(xg16_luma01, T_00_00c);//32bit: Ref [15 14 13 12]
			__m128i	T_03_17		= _mm_madd_epi16(xg16_luma01, T_00_01c);//32bit: Ref [11 10 09 08]
			__m128i	T_03_18		= _mm_madd_epi16(xg16_luma23, T_00_02c);//32bit: Ref []
			__m128i	T_03_19		= _mm_madd_epi16(xg16_luma23, T_00_03c);//32bit: Ref []
			__m128i	T_03_20		= _mm_madd_epi16(xg16_luma45, T_00_04c);//32bit: Ref []
			__m128i	T_03_21		= _mm_madd_epi16(xg16_luma45, T_00_05c);//32bit: Ref []
			__m128i	T_03_22		= _mm_madd_epi16(xg16_luma67, T_00_06c);//32bit: Ref []
			__m128i	T_03_23		= _mm_madd_epi16(xg16_luma67, T_00_07c);//32bit: Ref []
			__m128i	T_03_24		= _mm_madd_epi16(xg16_luma01, T_00_00d);//32bit: Ref [07 0605 04]
			__m128i	T_03_25		= _mm_madd_epi16(xg16_luma01, T_00_01d);//32bit: Ref [03 02 01 00]
			__m128i	T_03_26		= _mm_madd_epi16(xg16_luma23, T_00_02d);//32bit: Ref []
			__m128i	T_03_27		= _mm_madd_epi16(xg16_luma23, T_00_03d);//32bit: Ref []
			__m128i	T_03_28		= _mm_madd_epi16(xg16_luma45, T_00_04d);//32bit: Ref []
			__m128i	T_03_29		= _mm_madd_epi16(xg16_luma45, T_00_05d);//32bit: Ref []
			__m128i	T_03_30		= _mm_madd_epi16(xg16_luma67, T_00_06d);//32bit: Ref []
			__m128i	T_03_31		= _mm_madd_epi16(xg16_luma67, T_00_07d);//32bit: Ref []

			const __m128i	T_05_03		= _mm_add_epi32(_mm_add_epi32(T_03_16, T_03_18), _mm_add_epi32(T_03_20, T_03_22));//32bit: [15 14 13 12] 
			const __m128i	T_05_02		= _mm_add_epi32(_mm_add_epi32(T_03_17, T_03_19), _mm_add_epi32(T_03_21, T_03_23));//32bit: [11 10 09 08] 
			const __m128i	T_05_01		= _mm_add_epi32(_mm_add_epi32(T_03_24, T_03_26), _mm_add_epi32(T_03_28, T_03_30));//32bit: [07 06 05 04] 
			const __m128i	T_05_00		= _mm_add_epi32(_mm_add_epi32(T_03_25, T_03_27), _mm_add_epi32(T_03_29, T_03_31));//32bit: [03 02 01 00] 

			const __m128i	T_07_00a	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_00), 12);				  //16bit: Pred:03~00 needed
			const __m128i	T_07_00b	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_01), 12);				  //16bit: Pred:07~04 needed
			const __m128i	T_07_00c	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_02), 12);				  //16bit: Pred:11~08 needed
			const __m128i	T_07_00d	= _mm_srai_epi32(_mm_add_epi32(c_V, T_05_03), 12);				  //16bit: Pred:15~12 needed
			const __m128i	T_08_00a  = _mm_packus_epi32(T_07_00a, T_07_00b); //16bit: Pred:07~00 needed
			const __m128i	T_08_00b  = _mm_packus_epi32(T_07_00c, T_07_00d); //16bit: Pred:15~08 needed
			const __m128i	T_09_00a  = _mm_packus_epi16(T_08_00a, T_08_00b); // 8bit: Pred:15~00 needed
			_mm_storeu_si128((__m128i *)(pucPred+offset1), T_09_00a);
		}
	}
	//xInterpolate8HV_32x32( pucRef,		pucPred,	nRefStride, 32, 32, xFrac, yFrac, psTmp);
	//xInterpolate8HV_32x32( pucRef+32,	pucPred+32, nRefStride, 32, 32, xFrac, yFrac, psTmp);
	//xInterpolate8HV_32x32( pucRef+32*nRefStride,	pucPred+32*MAX_CU_SIZE,		nRefStride, 32, 32, xFrac, yFrac, psTmp);
	//xInterpolate8HV_32x32( pucRef+32*nRefStride+32, pucPred+32*MAX_CU_SIZE+32,	nRefStride, 32, 32, xFrac, yFrac, psTmp);
}

	
typedef void (*xInterpolate8HV_ptr)(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16      *psTmp
	);

xInterpolate8HV_ptr xInterpolate8HV_NxN[5] = {
	NULL,
	xInterpolate8HV_8x8,
	xInterpolate8HV_16x16,
	xInterpolate8HV_32x32,
	xInterpolate8HV_64x64
};

#else
void xInterpolate8HV(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16    *psTmp
	)
{
	// TODO: Check the buffer size of psTmp
	//assert( nFrac != 0 );
	int i, j;

	// InterpolateH
	for( i=0; i<(int)nSizeY+NTAPS_LUMA-1; i++ ) {
		CUInt8 *P = pucRef + (i-(NTAPS_LUMA/2-1)) * nRefStride - (NTAPS_LUMA/2-1);

		for( j=0; j<(int)nSizeX; j++ ) {
			Int16 iSum;
			iSum  = P[0+j] * xg_lumaFilter[xFrac][0];
			iSum += P[1+j] * xg_lumaFilter[xFrac][1];
			iSum += P[2+j] * xg_lumaFilter[xFrac][2];
			iSum += P[3+j] * xg_lumaFilter[xFrac][3];
			iSum += P[4+j] * xg_lumaFilter[xFrac][4];
			iSum += P[5+j] * xg_lumaFilter[xFrac][5];
			iSum += P[6+j] * xg_lumaFilter[xFrac][6];
			iSum += P[7+j] * xg_lumaFilter[xFrac][7];
			psTmp[i*MAX_CU_SIZE+j] = (iSum - 8192);
		}
	}

	// InterpolateV
	for( i=0; i<(int)nSizeX; i++ ) {
		CInt16 *P = psTmp + ((NTAPS_LUMA/2-1) * MAX_CU_SIZE) + i;
		UInt8 *Q = pucPred + i;

		for( j=0; j<(int)nSizeY; j++ ) {
			Int32 iSum;
			iSum  = P[(j-3) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][0];
			iSum += P[(j-2) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][1];
			iSum += P[(j-1) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][2];
			iSum += P[(j+0) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][3];
			iSum += P[(j+1) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][4];
			iSum += P[(j+2) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][5];
			iSum += P[(j+3) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][6];
			iSum += P[(j+4) * MAX_CU_SIZE] * xg_lumaFilter[yFrac][7];
			Q[j*MAX_CU_SIZE] = Clip3( 0, 255, (iSum + 8192*64+2048) >> 12);
		}
	}
}

#endif

void xPredInterLumaBlk(
	CUInt8     *pucRef,
	UInt8      *pucPred,
	CUInt32     nRefStride,
	//CUInt32		  nPredStride,  //this parameter should always set to MAX_CU_SIZE //x64 modify, remove unused parameter
	CUInt       nSizeX,
	CUInt       nSizeY,
	xMV         bmv,
	Int16      *psTmp
	)
{
	Int  dx = bmv.x >> 2;
	Int  dy = bmv.y >> 2;
	UInt xFrac = bmv.x & 3;
	UInt yFrac = bmv.y & 3;
	UInt xyType = (yFrac != 0 ? 2 : 0) + (xFrac != 0 ? 1 : 0);

	pucRef += dy * (Int)nRefStride + dx;//x64 modify, force type conversion
	switch( xyType ) {
	case 0:
		for(UInt i=0; i<nSizeY; i++ ) {//x64 modify, type conversion
			memcpy( pucPred + i * MAX_CU_SIZE, pucRef + i * nRefStride, nSizeX );
		}
		break;
	case 1:
#if INTERP_USE_TRUE_ASM
		xInterpolate8H_NxN[xLog2(nSizeX-1)-2]( (pixel*)pucRef, nRefStride, (pixel*)pucPred, MAX_CU_SIZE, xFrac );
#else
#if (INTERPOLATE_USE_ASM >= ASM_SSE4)
		xInterpolate8H_NxN[xLog2(nSizeX-1)-2]( pucRef, pucPred, nRefStride, nSizeX, nSizeY, xFrac );
#else
		xInterpolate8H( pucRef, pucPred, nRefStride, nSizeX, nSizeY, xFrac );
#endif
#endif
		break;
	case 2:
#if (INTERPOLATE_USE_ASM >= ASM_SSE4)
		xInterpolate8V_NxN[xLog2(nSizeX-1)-2]( pucRef, pucPred, nRefStride, nSizeX, nSizeY, yFrac );
#else
		xInterpolate8V( pucRef, pucPred, nRefStride, nSizeX, nSizeY, yFrac );
#endif
		break;
		/*case 3*/default:
#if (INTERPOLATE_USE_ASM >= ASM_SSE4)
			xInterpolate8HV_NxN[xLog2(nSizeX-1)-2]( pucRef, pucPred, nRefStride, nSizeX, nSizeY, xFrac, yFrac, psTmp );
#else
			xInterpolate8HV( pucRef, pucPred, nRefStride, nSizeX, nSizeY, xFrac, yFrac, psTmp );
#endif
			break;
	}
}


#if (INTERPOLATE_CHROMA_USE_ASM >= ASM_SSE4)
static
	void xInterpolate4H_4x4(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void)nSize; //x64 modify, avoid warning

	//new interpolate
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32	  = _mm_set1_epi16(32);
	const __m128i	xg_chroma = _mm_set1_epi32( (xg3<<24) | ((xg2&0x00FF)<<16) | ((xg1&0x00FF)<<8) | (xg0&0x00FF));

	for (Int i=0; i<4; i+=1){
		Int offset	   = i*(Int)nRefStride-(NTAPS_CHROMA/2-1);//x64 modify, type conversion
		Int nStrideDst = MAX_CU_SIZE/2;
		const __m128i T_00_00	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset  ));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 02 01 00 -01]
		const __m128i T_00_01	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset+1));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 03 02 01  00]
		const __m128i T_00_02	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset+2));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 04 03 02  01]
		const __m128i T_00_03	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset+3));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 05 04 03  02]
		const __m128i T_01_00	= _mm_unpacklo_epi32(T_00_00, T_00_01);
		const __m128i T_01_01	= _mm_unpacklo_epi32(T_00_02, T_00_03);
		const __m128i T_02_00   = _mm_unpacklo_epi64(T_01_00, T_01_01);  // 8bit: 05 04 03  02 04 03 02  01 03 02 01  00 02 01 00 -01
		const __m128i T_03_00	= _mm_maddubs_epi16(T_02_00, xg_chroma); //16bit: 03a 03b 02a 02b 01a 01b 00a 00b
		const __m128i T_04_00	= _mm_hadd_epi16(T_03_00, T_03_00);	     //16bit: 03 02 01 00 03 02 01 00
		const __m128i T_05_00	= _mm_srai_epi16(_mm_add_epi16(c_32, T_04_00), 6);
		const __m128i T_06_00	= _mm_packus_epi16(T_05_00, T_05_00);
		*(UInt32 *)(pucPred+i*nStrideDst) = _mm_cvtsi128_si32(T_06_00);
	}
}

static
void xInterpolate4H_8x8(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void)nSize; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32		= _mm_set1_epi16(32);
	const __m128i	xg_chroma	= _mm_set1_epi32( (xg3<<24) | ((xg2&0x00FF)<<16) | ((xg1&0x00FF)<<8) | (xg0&0x00FF));

	for (Int i=0; i<8; i+=1){
		Int offset	   = i*(Int)nRefStride-(NTAPS_CHROMA/2-1);//x64 modify, type conversion
		Int nStrideDst = MAX_CU_SIZE/2;
		const __m128i T_00_00	= _mm_loadl_epi64((const __m128i*)(pucRef+offset  ));//8bit: Ref [xx xx xx xx xx xx xx xx 06 05 04 03 02 01 00 -01]
		const __m128i T_00_01	= _mm_loadl_epi64((const __m128i*)(pucRef+offset+1));//8bit: Ref [xx xx xx xx xx xx xx xx 07 06 05 04 03 02 01  00]
		const __m128i T_00_02	= _mm_loadl_epi64((const __m128i*)(pucRef+offset+2));//8bit: Ref [xx xx xx xx xx xx xx xx 08 07 06 05 04 03 02  01]
		const __m128i T_00_03	= _mm_loadl_epi64((const __m128i*)(pucRef+offset+3));//8bit: Ref [xx xx xx xx xx xx xx xx 09 08 07 06 05 04 03  02]
		const __m128i T_01_00	= _mm_unpacklo_epi64(T_00_00, T_00_02);
		const __m128i T_01_01	= _mm_unpacklo_epi64(T_00_01, T_00_03);
		const __m128i T_02_00a	= _mm_maddubs_epi16(T_01_00, xg_chroma); //16bit: 06a 06b 02a 02b 04a 04b 00a 00b
		const __m128i T_02_00b	= _mm_maddubs_epi16(T_01_01, xg_chroma); //16bit: 07a 07b 03a 03b 05a 05b 01a 01b
		const __m128i T_03_00a	= _mm_unpacklo_epi32(T_02_00a, T_02_00b);//16bit: 05 05 04 04 01 01 00 00
		const __m128i T_03_00b	= _mm_unpackhi_epi32(T_02_00a, T_02_00b);//16bit: 07 07 06 06 03 03 02 02
		const __m128i T_04_00	= _mm_hadd_epi16(T_03_00a, T_03_00b);	 //16bit: 07 06 03 02 05 04 01 00
		const __m128i T_05_00	= _mm_shuffle_epi32(T_04_00, 0xD8);      //16bit: 07 06 05 04 03 02 01 00
		const __m128i T_06_00	= _mm_srai_epi16(_mm_add_epi16(c_32, T_05_00), 6);
		const __m128i T_07_00	= _mm_packus_epi16(T_06_00, T_06_00);
		_mm_storel_epi64((__m128i *)(pucPred+i*nStrideDst), T_07_00);
	}
}

void xInterpolate4H_16x16(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void)nSize; //x64 modify, avoid warning

	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32	  = _mm_set1_epi16(32);
	const __m128i	xg_chroma = _mm_set1_epi32( (xg3<<24) | ((xg2&0x00FF)<<16) | ((xg1&0x00FF)<<8) | (xg0&0x00FF));

	for (Int i=0; i<16; i+=1){
		Int offset = i*(Int)nRefStride-(NTAPS_CHROMA/2-1);//x64 modify, type conversion
		Int nStrideDst = MAX_CU_SIZE/2;
		__m128i T_00_00		= _mm_loadu_si128((__m128i*)(pucRef+offset  )); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
		__m128i T_00_01		= _mm_loadu_si128((__m128i*)(pucRef+offset+1)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
		__m128i T_00_02		= _mm_loadu_si128((__m128i*)(pucRef+offset+2)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
		__m128i T_00_03		= _mm_loadu_si128((__m128i*)(pucRef+offset+3)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
		__m128i	T_01_00		= _mm_maddubs_epi16(T_00_00, xg_chroma);	    //16bit: 12a 12b 08a 08b 04a 04b 00a 00b
		__m128i	T_01_01		= _mm_maddubs_epi16(T_00_01, xg_chroma);		//16bit: 13a 13b 09a 09b 05a 05b 01a 01b
		__m128i	T_01_02		= _mm_maddubs_epi16(T_00_02, xg_chroma);		//16bit: 14a 14b 10a 10b 06a 06b 02a 02b
		__m128i	T_01_03		= _mm_maddubs_epi16(T_00_03, xg_chroma);		//16bit: 15a 15b 11a 11b 07a 07b 03a 03b
		__m128i	T_02_00		= _mm_hadd_epi16(T_01_00, T_01_03);				//16bit: 15 11 07 03 12 08 04 00
		__m128i	T_02_01		= _mm_hadd_epi16(T_01_01, T_01_02);				//16bit: 14 10 06 02 13 09 05 01
		__m128i T_03_00     = _mm_unpacklo_epi16(T_02_00, T_02_01);			//16bit: 13 12 09 08 05 04 01 00
		__m128i T_03_01     = _mm_unpackhi_epi16(T_02_01, T_02_00);			//16bit: 15 14 11 10 07 06 03 02
		__m128i	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00), 6);
		__m128i	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_01), 6);
		__m128i	T_05_00a	= _mm_unpacklo_epi32(T_04_00a, T_04_00b);
		__m128i	T_05_00b	= _mm_unpackhi_epi32(T_04_00a, T_04_00b);
		__m128i	T_06_00		= _mm_packus_epi16(T_05_00a, T_05_00b);
		_mm_storeu_si128((__m128i *)(pucPred+i*nStrideDst), T_06_00);
	}
}

void xInterpolate4H_32x32(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void )nSize;//x64 modify, avoid warning
	
	//new interpolate, 2013-12-6
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32	  = _mm_set1_epi16(32);
	const __m128i	xg_chroma = _mm_set1_epi32( (xg3<<24) | ((xg2&0x00FF)<<16) | ((xg1&0x00FF)<<8) | (xg0&0x00FF));

	for (Int i=0; i<32; i+=1){
		for (Int j=0; j<2; j++) {
			Int offset = i*(Int)nRefStride-(NTAPS_CHROMA/2-1)+j*16;//x64 modify, type conversion
			Int nStrideDst = MAX_CU_SIZE/2;
			__m128i T_00_00		= _mm_loadu_si128((__m128i*)(pucRef+offset  )); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
			__m128i T_00_01		= _mm_loadu_si128((__m128i*)(pucRef+offset+1)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
			__m128i T_00_02		= _mm_loadu_si128((__m128i*)(pucRef+offset+2)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
			__m128i T_00_03		= _mm_loadu_si128((__m128i*)(pucRef+offset+3)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
			__m128i	T_01_00		= _mm_maddubs_epi16(T_00_00, xg_chroma);	    //16bit: 12a 12b 08a 08b 04a 04b 00a 00b
			__m128i	T_01_01		= _mm_maddubs_epi16(T_00_01, xg_chroma);		//16bit: 13a 13b 09a 09b 05a 05b 01a 01b
			__m128i	T_01_02		= _mm_maddubs_epi16(T_00_02, xg_chroma);		//16bit: 14a 14b 10a 10b 06a 06b 02a 02b
			__m128i	T_01_03		= _mm_maddubs_epi16(T_00_03, xg_chroma);		//16bit: 15a 15b 11a 11b 07a 07b 03a 03b
			__m128i	T_02_00		= _mm_hadd_epi16(T_01_00, T_01_03);				//16bit: 15 11 07 03 12 08 04 00
			__m128i	T_02_01		= _mm_hadd_epi16(T_01_01, T_01_02);				//16bit: 14 10 06 02 13 09 05 01
			__m128i T_03_00     = _mm_unpacklo_epi16(T_02_00, T_02_01);			//16bit: 13 12 09 08 05 04 01 00
			__m128i T_03_01     = _mm_unpackhi_epi16(T_02_01, T_02_00);			//16bit: 15 14 11 10 07 06 03 02
			__m128i	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00), 6);
			__m128i	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_01), 6);
			__m128i	T_05_00a	= _mm_unpacklo_epi32(T_04_00a, T_04_00b);
			__m128i	T_05_00b	= _mm_unpackhi_epi32(T_04_00a, T_04_00b);
			__m128i	T_06_00		= _mm_packus_epi16(T_05_00a, T_05_00b);
			_mm_storeu_si128((__m128i *)(pucPred+i*nStrideDst+j*16), T_06_00);
		}
	}
	//xInterpolate4H_16x16(pucRef                 , pucPred                    , nRefStride, nSize/2, nFrac);
	//xInterpolate4H_16x16(pucRef+16              , pucPred+16                 , nRefStride, nSize/2, nFrac);
	//xInterpolate4H_16x16(pucRef+16*nRefStride   , pucPred+16*MAX_CU_SIZE/2   , nRefStride, nSize/2, nFrac);
	//xInterpolate4H_16x16(pucRef+16*nRefStride+16, pucPred+16*MAX_CU_SIZE/2+16, nRefStride, nSize/2, nFrac);
}

typedef void (*xInterpolate4H_ptr)(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	);

xInterpolate4H_ptr xInterpolate4H_NxN[4] = {
	xInterpolate4H_4x4,
	xInterpolate4H_8x8,
	xInterpolate4H_16x16,
	xInterpolate4H_32x32,
};
#else
static
	void xInterpolate4H(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	//assert( nFrac != 0 );
	int i, j;

	for( i=0; i<(int)nSizeY; i++ ) {
		CUInt8 *P = pucRef + i * nRefStride - (NTAPS_CHROMA/2-1);
		UInt8 *Q = pucPred + i * MAX_CU_SIZE/2;

		for( j=0; j<(int)nSizeX; j++ ) {
			Int16 iSum;
			iSum  = P[0+j] * xg_chromaFilter[nFrac][0];
			iSum += P[1+j] * xg_chromaFilter[nFrac][1];
			iSum += P[2+j] * xg_chromaFilter[nFrac][2];
			iSum += P[3+j] * xg_chromaFilter[nFrac][3];
			Q[j] = Clip3( 0, 255, (iSum + 32) >> 6);
		}
	}
}
#endif

#if (INTERPOLATE_CHROMA_USE_ASM >= ASM_SSE4)
static
	void xInterpolate4V_4x4(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void)nSize; //x64 modify, avoid warning

	//new interpolate
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32		= _mm_set1_epi16(32);
	const __m128i	xg_chroma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_chroma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );

	Int iRefStride = (Int)nRefStride;//x64 modify, force conversion, !!! try to change input parameters to unsigned !!! 
	const __m128i T_00_00		= _mm_cvtsi32_si128(*(UInt32 *)(pucRef+(-1)*iRefStride));//8bit: Ref -01 -01 -01 -01] 03~00
	const __m128i T_00_01		= _mm_cvtsi32_si128(*(UInt32 *)(pucRef+(0)*iRefStride));//8bit: Ref [ 00  00  00  00] 03~00
	const __m128i T_00_02		= _mm_cvtsi32_si128(*(UInt32 *)(pucRef+(1)*iRefStride));//8bit: Ref [ 01  01  01  01] 03~00
	const __m128i T_00_03		= _mm_cvtsi32_si128(*(UInt32 *)(pucRef+(2)*iRefStride));//8bit: Ref [ 02  02  02  02] 03~00
	const __m128i T_00_04		= _mm_cvtsi32_si128(*(UInt32 *)(pucRef+(3)*iRefStride));//8bit: Ref [ 03 ] 03~00
	const __m128i T_00_05		= _mm_cvtsi32_si128(*(UInt32 *)(pucRef+(4)*iRefStride));//8bit: Ref [ 04 ] 03~00
	const __m128i T_00_06		= _mm_cvtsi32_si128(*(UInt32 *)(pucRef+(5)*iRefStride));//8bit: Ref [ 05 ] 03~00

	__m128i T_01_00a, T_01_01a, T_02_00a, T_02_01a, T_03_00a, T_04_00a;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_chroma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_chroma_23);\
	T_03_00a	= _mm_add_epi16(T_02_00a,T_02_01a);\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	*(UInt32 *)(pucPred+ (k) *MAX_CU_SIZE/2) = _mm_cvtsi128_si32(_mm_packus_epi16(T_04_00a, T_04_00a));

	INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03, 0)
		INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04, 1)
		INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05, 2)
		INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06, 3)
#undef INTERPOLATE_ONE_LINE
}

static
	void xInterpolate4V_8x8(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void)nSize; //x64 modify, avoid warning

	//new interpolate
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32		= _mm_set1_epi16(32);
	const __m128i	xg_chroma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_chroma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );

	Int iRefStride = (Int)nRefStride;//x64 modify, force conversion, !!! try to change input parameters to unsigned !!! 
	const __m128i T_00_00		= _mm_loadl_epi64((const __m128i*)(pucRef+(-1)*iRefStride));//8bit: Ref [-01 -01 -01 -01 -01 -01 -01 -01] 07~00
	const __m128i T_00_01		= _mm_loadl_epi64((const __m128i*)(pucRef+(0)*iRefStride));//8bit: Ref [ 00  00  00  00  00  00  00  00] 07~00
	const __m128i T_00_02		= _mm_loadl_epi64((const __m128i*)(pucRef+(1)*iRefStride));//8bit: Ref [ 01  01  01  01  01  01  01  01] 07~00
	const __m128i T_00_03		= _mm_loadl_epi64((const __m128i*)(pucRef+(2)*iRefStride));//8bit: Ref [ 02  02  02  02  02  02  02  02] 07~00
	const __m128i T_00_04		= _mm_loadl_epi64((const __m128i*)(pucRef+(3)*iRefStride));//8bit: Ref [ 03 ] 07~00
	const __m128i T_00_05		= _mm_loadl_epi64((const __m128i*)(pucRef+(4)*iRefStride));//8bit: Ref [ 04 ] 07~00
	const __m128i T_00_06		= _mm_loadl_epi64((const __m128i*)(pucRef+(5)*iRefStride));//8bit: Ref [ 05 ] 07~00
	const __m128i T_00_07		= _mm_loadl_epi64((const __m128i*)(pucRef+(6)*iRefStride));//8bit: Ref [ 06 ] 07~00
	const __m128i T_00_08		= _mm_loadl_epi64((const __m128i*)(pucRef+(7)*iRefStride));//8bit: Ref [ 07 ] 07~00
	const __m128i T_00_09		= _mm_loadl_epi64((const __m128i*)(pucRef+(8)*iRefStride));//8bit: Ref [ 08 ] 07~00
	const __m128i T_00_10		= _mm_loadl_epi64((const __m128i*)(pucRef+(9)*iRefStride));//8bit: Ref [ 09 ] 07~00

	__m128i T_01_00a, T_01_01a, T_02_00a, T_02_01a, T_03_00a, T_04_00a;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_chroma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_chroma_23);\
	T_03_00a	= _mm_add_epi16(T_02_00a,T_02_01a);\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	_mm_storel_epi64((__m128i *)(pucPred + (k) *MAX_CU_SIZE/2), _mm_packus_epi16(T_04_00a, T_04_00a));

	INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03, 0)
		INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04, 1)
		INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05, 2)
		INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06, 3)
		INTERPOLATE_ONE_LINE(T_00_04,T_00_05,T_00_06,T_00_07, 4)
		INTERPOLATE_ONE_LINE(T_00_05,T_00_06,T_00_07,T_00_08, 5)
		INTERPOLATE_ONE_LINE(T_00_06,T_00_07,T_00_08,T_00_09, 6)
		INTERPOLATE_ONE_LINE(T_00_07,T_00_08,T_00_09,T_00_10, 7)
#undef INTERPOLATE_ONE_LINE
}

static
void xInterpolate4V_16x16(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void)nSize; //x64 modify, avoid warning

	//new interpolate
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32		= _mm_set1_epi16(32);
	const __m128i	xg_chroma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_chroma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );

	Int iRefStride = (Int)nRefStride;//x64 modify, force conversion, !!! try to change input parameters to unsigned !!! 
	for (Int i=0; i<16; i+=8){
		const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+(i-1)*iRefStride));//8bit: Ref [-01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01] 15~00
		const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+(i+0)*iRefStride));//8bit: Ref [ 00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00] 15~00
		const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+(i+1)*iRefStride));//8bit: Ref [ 01  01  01  01  01  01  01  01  01  01  01  01  01  01  01  01] 15~00
		const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+(i+2)*iRefStride));//8bit: Ref [ 02  02  02  02  02  02  02  02  02  02  02  02  02  02  02  02] 15~00
		const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+(i+3)*iRefStride));//8bit: Ref [ 03 ] 15~00
		const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+(i+4)*iRefStride));//8bit: Ref [ 04 ] 15~00
		const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+(i+5)*iRefStride));//8bit: Ref [ 05 ] 15~00
		const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+(i+6)*iRefStride));//8bit: Ref [ 06 ] 15~00
		const __m128i T_00_08	= _mm_loadu_si128((const __m128i*)(pucRef+(i+7)*iRefStride));//8bit: Ref [ 07 ] 15~00
		const __m128i T_00_09	= _mm_loadu_si128((const __m128i*)(pucRef+(i+8)*iRefStride));//8bit: Ref [ 08 ] 15~00
		const __m128i T_00_10	= _mm_loadu_si128((const __m128i*)(pucRef+(i+9)*iRefStride));//8bit: Ref [ 09 ] 15~00

		__m128i T_01_00a, T_01_01a, T_01_00b, T_01_01b, T_02_00a, T_02_01a, T_02_00b, T_02_01b, T_03_00a, T_03_00b, T_04_00a, T_04_00b;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_01_00b	= _mm_unpackhi_epi8(row0, row1);\
	T_01_01b	= _mm_unpackhi_epi8(row2, row3);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_chroma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_chroma_23);\
	T_02_00b	= _mm_maddubs_epi16(T_01_00b, xg_chroma_01);\
	T_02_01b	= _mm_maddubs_epi16(T_01_01b, xg_chroma_23);\
	T_03_00a	= _mm_add_epi16(T_02_00a,T_02_01a);\
	T_03_00b	= _mm_add_epi16(T_02_00b,T_02_01b);\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00b), 6);\
	_mm_storeu_si128((__m128i *)(pucPred + (i+k) *MAX_CU_SIZE/2), _mm_packus_epi16(T_04_00a, T_04_00b));

		INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03, 0)
			INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04, 1)
			INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05, 2)
			INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06, 3)
			INTERPOLATE_ONE_LINE(T_00_04,T_00_05,T_00_06,T_00_07, 4)
			INTERPOLATE_ONE_LINE(T_00_05,T_00_06,T_00_07,T_00_08, 5)
			INTERPOLATE_ONE_LINE(T_00_06,T_00_07,T_00_08,T_00_09, 6)
			INTERPOLATE_ONE_LINE(T_00_07,T_00_08,T_00_09,T_00_10, 7)
#undef INTERPOLATE_ONE_LINE
	}
}

void xInterpolate4V_32x32(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	)
{
	(void)nSize; //x64 modify, avoid warning
	
	//new interpolate
	Int16 xg0 = (xg_chromaFilter[nFrac][0]);
	Int16 xg1 = (xg_chromaFilter[nFrac][1]);
	Int16 xg2 = (xg_chromaFilter[nFrac][2]);
	Int16 xg3 = (xg_chromaFilter[nFrac][3]);
	const __m128i	c_32		= _mm_set1_epi16(32);
	const __m128i	xg_chroma_01	= _mm_set1_epi16( (xg1<<8) | (xg0 & 0x00FF) );
	const __m128i	xg_chroma_23	= _mm_set1_epi16( (xg3<<8) | (xg2 & 0x00FF) );

	Int iRefStride = (Int)nRefStride;//x64 modify, force conversion, !!! try to change input parameters to unsigned !!! 
	for (Int i=0; i<32; i+=8){
		for (Int j=0; j<2; j++)	{
			Int nOffset = j*16;
			const __m128i T_00_00	= _mm_loadu_si128((const __m128i*)(pucRef+(i-1)*iRefStride+nOffset));//8bit: Ref [-01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01 -01] 15~00
			const __m128i T_00_01	= _mm_loadu_si128((const __m128i*)(pucRef+(i+0)*iRefStride+nOffset));//8bit: Ref [ 00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00] 15~00
			const __m128i T_00_02	= _mm_loadu_si128((const __m128i*)(pucRef+(i+1)*iRefStride+nOffset));//8bit: Ref [ 01  01  01  01  01  01  01  01  01  01  01  01  01  01  01  01] 15~00
			const __m128i T_00_03	= _mm_loadu_si128((const __m128i*)(pucRef+(i+2)*iRefStride+nOffset));//8bit: Ref [ 02  02  02  02  02  02  02  02  02  02  02  02  02  02  02  02] 15~00
			const __m128i T_00_04	= _mm_loadu_si128((const __m128i*)(pucRef+(i+3)*iRefStride+nOffset));//8bit: Ref [ 03 ] 15~00
			const __m128i T_00_05	= _mm_loadu_si128((const __m128i*)(pucRef+(i+4)*iRefStride+nOffset));//8bit: Ref [ 04 ] 15~00
			const __m128i T_00_06	= _mm_loadu_si128((const __m128i*)(pucRef+(i+5)*iRefStride+nOffset));//8bit: Ref [ 05 ] 15~00
			const __m128i T_00_07	= _mm_loadu_si128((const __m128i*)(pucRef+(i+6)*iRefStride+nOffset));//8bit: Ref [ 06 ] 15~00
			const __m128i T_00_08	= _mm_loadu_si128((const __m128i*)(pucRef+(i+7)*iRefStride+nOffset));//8bit: Ref [ 07 ] 15~00
			const __m128i T_00_09	= _mm_loadu_si128((const __m128i*)(pucRef+(i+8)*iRefStride+nOffset));//8bit: Ref [ 08 ] 15~00
			const __m128i T_00_10	= _mm_loadu_si128((const __m128i*)(pucRef+(i+9)*iRefStride+nOffset));//8bit: Ref [ 09 ] 15~00

			__m128i T_01_00a, T_01_01a, T_01_00b, T_01_01b, T_02_00a, T_02_01a, T_02_00b, T_02_01b, T_03_00a, T_03_00b, T_04_00a, T_04_00b;
#define INTERPOLATE_ONE_LINE( row0, row1, row2, row3, k ) \
	T_01_00a	= _mm_unpacklo_epi8(row0, row1);\
	T_01_01a	= _mm_unpacklo_epi8(row2, row3);\
	T_01_00b	= _mm_unpackhi_epi8(row0, row1);\
	T_01_01b	= _mm_unpackhi_epi8(row2, row3);\
	T_02_00a	= _mm_maddubs_epi16(T_01_00a, xg_chroma_01);\
	T_02_01a	= _mm_maddubs_epi16(T_01_01a, xg_chroma_23);\
	T_02_00b	= _mm_maddubs_epi16(T_01_00b, xg_chroma_01);\
	T_02_01b	= _mm_maddubs_epi16(T_01_01b, xg_chroma_23);\
	T_03_00a	= _mm_add_epi16(T_02_00a,T_02_01a);\
	T_03_00b	= _mm_add_epi16(T_02_00b,T_02_01b);\
	T_04_00a	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00a), 6);\
	T_04_00b	= _mm_srai_epi16(_mm_add_epi16(c_32, T_03_00b), 6);\
	_mm_storeu_si128((__m128i *)(pucPred + (i+k) *MAX_CU_SIZE/2 + nOffset), _mm_packus_epi16(T_04_00a, T_04_00b));

			INTERPOLATE_ONE_LINE(T_00_00,T_00_01,T_00_02,T_00_03, 0)
				INTERPOLATE_ONE_LINE(T_00_01,T_00_02,T_00_03,T_00_04, 1)
				INTERPOLATE_ONE_LINE(T_00_02,T_00_03,T_00_04,T_00_05, 2)
				INTERPOLATE_ONE_LINE(T_00_03,T_00_04,T_00_05,T_00_06, 3)
				INTERPOLATE_ONE_LINE(T_00_04,T_00_05,T_00_06,T_00_07, 4)
				INTERPOLATE_ONE_LINE(T_00_05,T_00_06,T_00_07,T_00_08, 5)
				INTERPOLATE_ONE_LINE(T_00_06,T_00_07,T_00_08,T_00_09, 6)
				INTERPOLATE_ONE_LINE(T_00_07,T_00_08,T_00_09,T_00_10, 7)
#undef INTERPOLATE_ONE_LINE
		}
	}
	//xInterpolate4V_16x16(pucRef                 , pucPred                    , nRefStride, nSize/2, nFrac);
	//xInterpolate4V_16x16(pucRef+16              , pucPred+16                 , nRefStride, nSize/2, nFrac);
	//xInterpolate4V_16x16(pucRef+16*nRefStride   , pucPred+16*MAX_CU_SIZE/2   , nRefStride, nSize/2, nFrac);
	//xInterpolate4V_16x16(pucRef+16*nRefStride+16, pucPred+16*MAX_CU_SIZE/2+16, nRefStride, nSize/2, nFrac);
}

typedef void (*xInterpolate4V_ptr)(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       nFrac
	);

xInterpolate4V_ptr xInterpolate4V_NxN[4] = {
	xInterpolate4V_4x4,
	xInterpolate4V_8x8,
	xInterpolate4V_16x16,
	xInterpolate4V_32x32,
};
#else
static
	void xInterpolate4V(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       nFrac
	)
{
	//assert( nFrac != 0 );
	int i, j;

	for( i=0; i<(int)nSizeX; i++ ) {
		CUInt8 *P = pucRef  + i;
		UInt8 *Q = pucPred + i;

		for( j=0; j<(int)nSizeY; j++ ) {
			Int16 iSum;
			iSum  = P[(j-1) * nRefStride] * xg_chromaFilter[nFrac][0];
			iSum += P[(j-0) * nRefStride] * xg_chromaFilter[nFrac][1];
			iSum += P[(j+1) * nRefStride] * xg_chromaFilter[nFrac][2];
			iSum += P[(j+2) * nRefStride] * xg_chromaFilter[nFrac][3];
			Q[j*MAX_CU_SIZE/2] = Clip3( 0, 255, (iSum + 32) >> 6);
		}
	}
}
#endif

#if (INTERPOLATE_CHROMA_USE_ASM >= ASM_SSE4)
static
void xInterpolate4HV_4x4(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16    *psTmp
	)
{
	(void *)psTmp;//x64 modify, avoid warning

	//new interpolate
	Int16 Xxg0 = (xg_chromaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_chromaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_chromaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_chromaFilter[xFrac][3]);
	Int16 Yxg0 = (xg_chromaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_chromaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_chromaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_chromaFilter[yFrac][3]);
	const __m128i c_32  = _mm_set1_epi16(32);
	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	const __m128i xg_chroma		= _mm_set1_epi32( (Xxg3<<24) | ((Xxg2&0x00FF)<<16) | ((Xxg1&0x00FF)<<8) | (Xxg0&0x00FF));
	const __m128i xg16_chroma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_chroma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	__m128i	tmp16[4+NTAPS_CHROMA-1];

	//Stage1. Interpolate H
	for (Int i=0; i<(int)nSize+NTAPS_CHROMA-1; i+=1){
		Int offset0	   = (i-1)*(int)nRefStride;//x64 modify, force conversion -- (int)nRefStride
		const __m128i T_00_00	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset0-1));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 02 01 00 -01]
		const __m128i T_00_01	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset0+0));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 03 02 01  00]
		const __m128i T_00_02	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset0+1));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 04 03 02  01]
		const __m128i T_00_03	= _mm_cvtsi32_si128(*(const UInt32 *)(pucRef+offset0+2));//8bit: [xx xx xx xx xx xx xx xx xx xx xx xx 05 04 03  02]
		const __m128i T_01_00	= _mm_unpacklo_epi32(T_00_00, T_00_01);
		const __m128i T_01_01	= _mm_unpacklo_epi32(T_00_02, T_00_03);
		const __m128i T_02_00   = _mm_unpacklo_epi64(T_01_00, T_01_01);  // 8bit: 05 04 03  02 04 03 02  01 03 02 01  00 02 01 00 -01
		const __m128i T_03_00	= _mm_maddubs_epi16(T_02_00, xg_chroma); //16bit: 03a 03b 02a 02b 01a 01b 00a 00b
		const __m128i T_04_00	= _mm_hadd_epi16(T_03_00, T_03_00);	     //16bit: 03 02 01 00 03 02 01 00
		tmp16[i] = _mm_sub_epi16(T_04_00, c_H);
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<(int)nSize; i++ ) {
		Int offset1 = i*MAX_CU_SIZE/2;
		__m128i	T_00_00b	= _mm_unpacklo_epi16(tmp16[i  ], tmp16[i+1]); //16bit: Ref [03 03 02 02 01 01 00 00] Row: 0,-1
		__m128i	T_00_02b	= _mm_unpacklo_epi16(tmp16[i+2], tmp16[i+3]); //16bit: Ref [03 03 02 02 01 01 00 00] Row: 2, 1
		__m128i	T_01_00b	= _mm_madd_epi16(xg16_chroma01, T_00_00b);	  //32bit: [03 02 01 00] 
		__m128i	T_01_02b	= _mm_madd_epi16(xg16_chroma23, T_00_02b);	  //32bit: [  ]
		__m128i	T_02_02 	= _mm_add_epi32(T_01_00b, T_01_02b);		  //32bit: [03 02 01 00]
		__m128i	T_04_00c	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_02), 12);  //32bit: [03 02 01 00]
		__m128i	T_05_00a	= _mm_packus_epi32(T_04_00c, T_04_00c); //16bit: [07 06 05 04 03 02 01 00]
		*(UInt32 *)(pucPred+offset1) = _mm_cvtsi128_si32(_mm_packus_epi16(T_05_00a, T_05_00a));
	}
}

static
void xInterpolate4HV_8x8(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16    *psTmp
	)
{
	(void *)psTmp;//x64 modify, avoid warning

	//new interpolate
	Int16 Xxg0 = (xg_chromaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_chromaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_chromaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_chromaFilter[xFrac][3]);
	Int16 Yxg0 = (xg_chromaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_chromaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_chromaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_chromaFilter[yFrac][3]);
	const __m128i c_32  = _mm_set1_epi16(32);
	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	const __m128i xg_chroma		= _mm_set1_epi32( (Xxg3<<24) | ((Xxg2&0x00FF)<<16) | ((Xxg1&0x00FF)<<8) | (Xxg0&0x00FF));
	const __m128i xg16_chroma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_chroma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	__m128i	tmp16[8+NTAPS_CHROMA-1];

	//Stage1. Interpolate H
	for (Int i=0; i<(int)nSize+NTAPS_CHROMA-1; i+=1){
		Int offset0 = (i-1)*(int)nRefStride;//x64 modify, force conversion -- (int)nRefStride
		const __m128i T_00_00  = _mm_loadl_epi64((const __m128i*)(pucRef+offset0-1));//8bit: [xx xx xx xx xx xx xx xx 06 05 04 03 02 01 00 -01]
		const __m128i T_00_01  = _mm_loadl_epi64((const __m128i*)(pucRef+offset0-0));//8bit: [xx xx xx xx xx xx xx xx 07 06 05 04 03 02 01  00]
		const __m128i T_00_02  = _mm_loadl_epi64((const __m128i*)(pucRef+offset0+1));//8bit: [xx xx xx xx xx xx xx xx 08 07 06 05 04 03 02  01]
		const __m128i T_00_03  = _mm_loadl_epi64((const __m128i*)(pucRef+offset0+2));//8bit: [xx xx xx xx xx xx xx xx 09 08 07 06 05 04 03  02]
		const __m128i T_01_00  = _mm_unpacklo_epi64(T_00_00, T_00_02);
		const __m128i T_01_01  = _mm_unpacklo_epi64(T_00_01, T_00_03);
		const __m128i T_02_00a = _mm_maddubs_epi16(T_01_00, xg_chroma); //16bit: 06a 06b 02a 02b 04a 04b 00a 00b
		const __m128i T_02_00b = _mm_maddubs_epi16(T_01_01, xg_chroma); //16bit: 07a 07b 03a 03b 05a 05b 01a 01b
		const __m128i T_03_00a = _mm_unpacklo_epi32(T_02_00a, T_02_00b);//16bit: 05 05 04 04 01 01 00 00
		const __m128i T_03_00b = _mm_unpackhi_epi32(T_02_00a, T_02_00b);//16bit: 07 07 06 06 03 03 02 02
		const __m128i T_04_00  = _mm_hadd_epi16(T_03_00a, T_03_00b);	//16bit: 07 06 03 02 05 04 01 00
		const __m128i T_05_00  = _mm_shuffle_epi32(T_04_00, 0xD8);      //16bit: 07 06 05 04 03 02 01 00
		tmp16[i] = _mm_sub_epi16(T_05_00, c_H);
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<(int)nSize; i++ ) {
		Int offset1 = i*MAX_CU_SIZE/2;
		__m128i	T_00_00b	= _mm_unpacklo_epi16(tmp16[i  ], tmp16[i+1]);//16bit: [03 03 02 02 01 01 00 00] Row: 0,-1
		__m128i	T_00_01b	= _mm_unpackhi_epi16(tmp16[i  ], tmp16[i+1]);//16bit: [07 07 06 06 05 05 04 04] Row: 0,-1
		__m128i	T_00_02b	= _mm_unpacklo_epi16(tmp16[i+2], tmp16[i+3]);//16bit: [03 03 02 02 01 01 00 00] Row: 2, 1
		__m128i	T_00_03b	= _mm_unpackhi_epi16(tmp16[i+2], tmp16[i+3]);//16bit: [07 07 06 06 05 05 04 04] Row: 2, 1
		__m128i	T_01_00b	= _mm_madd_epi16(xg16_chroma01, T_00_00b);	 //32bit: [03 02 01 00] 
		__m128i	T_01_01b	= _mm_madd_epi16(xg16_chroma01, T_00_01b);	 //32bit: [07 06 05 04]
		__m128i	T_01_02b	= _mm_madd_epi16(xg16_chroma23, T_00_02b);	 //32bit: [  ]
		__m128i	T_01_03b	= _mm_madd_epi16(xg16_chroma23, T_00_03b);	 //32bit: [  ]
		__m128i	T_02_02 	= _mm_add_epi32(T_01_00b, T_01_02b);		 //32bit: [03 02 01 00]
		__m128i	T_02_03 	= _mm_add_epi32(T_01_01b, T_01_03b);		 //32bit: [07 06 05 04]
		__m128i	T_04_00c	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_02), 12);  //32bit: [03 02 01 00]
		__m128i	T_04_00d	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_03), 12);  //32bit: [07 06 05 04]
		__m128i	T_05_00a	= _mm_packus_epi32(T_04_00c, T_04_00d); //16bit: [07 06 05 04 03 02 01 00]

		_mm_storel_epi64((__m128i *)(pucPred+offset1), _mm_packus_epi16(T_05_00a, T_05_00a));
	}
}

static
void xInterpolate4HV_16x16(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16    *psTmp
	)
{
	(void *)psTmp;//x64 modify, avoid warning
	
	//new interpolate
	Int16 Xxg0 = (xg_chromaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_chromaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_chromaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_chromaFilter[xFrac][3]);
	Int16 Yxg0 = (xg_chromaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_chromaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_chromaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_chromaFilter[yFrac][3]);
	const __m128i c_32  = _mm_set1_epi16(32);
	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	const __m128i xg_chroma		= _mm_set1_epi32( (Xxg3<<24) | ((Xxg2&0x00FF)<<16) | ((Xxg1&0x00FF)<<8) | (Xxg0&0x00FF));
	const __m128i xg16_chroma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_chroma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	__m128i	tmp16[16+NTAPS_CHROMA-1][2];

	//Stage1. Interpolate H
	for (Int i=0; i<(int)nSize+NTAPS_CHROMA-1; i+=1){
		Int offset0 = (i-1)*(int)nRefStride;//x64 modify, force conversion -- (int)nRefStride
		const __m128i T_00_00  = _mm_loadu_si128((const __m128i*)(pucRef+offset0-1)); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
		const __m128i T_00_01  = _mm_loadu_si128((const __m128i*)(pucRef+offset0+0)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
		const __m128i T_00_02  = _mm_loadu_si128((const __m128i*)(pucRef+offset0+1)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
		const __m128i T_00_03  = _mm_loadu_si128((const __m128i*)(pucRef+offset0+2)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
		const __m128i T_01_00  = _mm_maddubs_epi16(T_00_00, xg_chroma);	    //16bit: 12a 12b 08a 08b 04a 04b 00a 00b
		const __m128i T_01_01  = _mm_maddubs_epi16(T_00_01, xg_chroma);		//16bit: 13a 13b 09a 09b 05a 05b 01a 01b
		const __m128i T_01_02  = _mm_maddubs_epi16(T_00_02, xg_chroma);		//16bit: 14a 14b 10a 10b 06a 06b 02a 02b
		const __m128i T_01_03  = _mm_maddubs_epi16(T_00_03, xg_chroma);		//16bit: 15a 15b 11a 11b 07a 07b 03a 03b
		const __m128i T_02_00  = _mm_hadd_epi16(T_01_00, T_01_03);			//16bit: 15 11 07 03 12 08 04 00
		const __m128i T_02_01  = _mm_hadd_epi16(T_01_01, T_01_02);			//16bit: 14 10 06 02 13 09 05 01
		const __m128i T_03_00  = _mm_unpacklo_epi16(T_02_00, T_02_01);			//16bit: 13 12 09 08 05 04 01 00
		const __m128i T_03_01  = _mm_unpackhi_epi16(T_02_01, T_02_00);			//16bit: 15 14 11 10 07 06 03 02
		const __m128i T_04_00a = _mm_sub_epi16(T_03_00, c_H);
		const __m128i T_04_00b = _mm_sub_epi16(T_03_01, c_H);
		tmp16[i][0]	= _mm_unpacklo_epi32(T_04_00a, T_04_00b);
		tmp16[i][1]	= _mm_unpackhi_epi32(T_04_00a, T_04_00b);
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<(int)nSize; i++ ) {
		Int offset1 = i*MAX_CU_SIZE/2;
		const __m128i	T_00_00a	= _mm_unpacklo_epi16(tmp16[i  ][1], tmp16[i+1][1]);//16bit: [11 11 10 10 09 09 08 08] Row: 0,-1
		const __m128i	T_00_01a	= _mm_unpackhi_epi16(tmp16[i  ][1], tmp16[i+1][1]);//16bit: [15 15 14 14 13 13 12 12] Row: 0,-1
		const __m128i	T_00_02a	= _mm_unpacklo_epi16(tmp16[i+2][1], tmp16[i+3][1]);//16bit: [11 11 10 10 09 09 08 08] Row: 2, 1
		const __m128i	T_00_03a	= _mm_unpackhi_epi16(tmp16[i+2][1], tmp16[i+3][1]);//16bit: [15 15 14 14 13 13 12 12] Row: 2, 1
		const __m128i	T_00_00b	= _mm_unpacklo_epi16(tmp16[i  ][0], tmp16[i+1][0]);//16bit: [03 03 02 02 01 01 00 00] Row: 0,-1
		const __m128i	T_00_01b	= _mm_unpackhi_epi16(tmp16[i  ][0], tmp16[i+1][0]);//16bit: [07 07 06 06 05 05 04 04] Row: 0,-1
		const __m128i	T_00_02b	= _mm_unpacklo_epi16(tmp16[i+2][0], tmp16[i+3][0]);//16bit: [03 03 02 02 01 01 00 00] Row: 2, 1
		const __m128i	T_00_03b	= _mm_unpackhi_epi16(tmp16[i+2][0], tmp16[i+3][0]);//16bit: [07 07 06 06 05 05 04 04] Row: 2, 1
		const __m128i	T_01_00a	= _mm_madd_epi16(xg16_chroma01, T_00_00a);//32bit: [11 10 09 08] 
		const __m128i	T_01_01a	= _mm_madd_epi16(xg16_chroma01, T_00_01a);//32bit: [15 14 13 12]
		const __m128i	T_01_02a	= _mm_madd_epi16(xg16_chroma23, T_00_02a);//32bit: [  ]
		const __m128i	T_01_03a	= _mm_madd_epi16(xg16_chroma23, T_00_03a);//32bit: [  ]
		const __m128i	T_01_00b	= _mm_madd_epi16(xg16_chroma01, T_00_00b);//32bit: [03 02 01 00] 
		const __m128i	T_01_01b	= _mm_madd_epi16(xg16_chroma01, T_00_01b);//32bit: [07 06 05 04]
		const __m128i	T_01_02b	= _mm_madd_epi16(xg16_chroma23, T_00_02b);//32bit: [  ]
		const __m128i	T_01_03b	= _mm_madd_epi16(xg16_chroma23, T_00_03b);//32bit: [  ]
		const __m128i	T_02_00 	= _mm_add_epi32(T_01_00a, T_01_02a);	  //32bit: [11 10 09 08] 
		const __m128i	T_02_01 	= _mm_add_epi32(T_01_01a, T_01_03a);	  //32bit: [15 14 13 12]
		const __m128i	T_02_02 	= _mm_add_epi32(T_01_00b, T_01_02b);	  //32bit: [03 02 01 00]
		const __m128i	T_02_03 	= _mm_add_epi32(T_01_01b, T_01_03b);	  //32bit: [07 06 05 04]
		const __m128i	T_04_00a	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_00), 12);	//32bit: [11 10 09 08]
		const __m128i	T_04_00b	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_01), 12);	//32bit: [15 14 13 12]
		const __m128i	T_04_00c	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_02), 12);	//32bit: [03 02 01 00]
		const __m128i	T_04_00d	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_03), 12);	//32bit: [07 06 05 04]
		const __m128i	T_05_00a	= _mm_packus_epi32(T_04_00c, T_04_00d); //16bit: [07 06 05 04 03 02 01 00]
		const __m128i	T_05_00b	= _mm_packus_epi32(T_04_00a, T_04_00b); //16bit: [15 14 13 12 11 10 09 08]

		_mm_storeu_si128((__m128i *)(pucPred+offset1), _mm_packus_epi16(T_05_00a, T_05_00b));
	}
}

void xInterpolate4HV_32x32(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16    *psTmp
	)
{
	(void *)psTmp;//x64 modify, avoid warning

	//new interpolate
	Int16 Xxg0 = (xg_chromaFilter[xFrac][0]);
	Int16 Xxg1 = (xg_chromaFilter[xFrac][1]);
	Int16 Xxg2 = (xg_chromaFilter[xFrac][2]);
	Int16 Xxg3 = (xg_chromaFilter[xFrac][3]);
	Int16 Yxg0 = (xg_chromaFilter[yFrac][0]);
	Int16 Yxg1 = (xg_chromaFilter[yFrac][1]);
	Int16 Yxg2 = (xg_chromaFilter[yFrac][2]);
	Int16 Yxg3 = (xg_chromaFilter[yFrac][3]);
	const __m128i c_32  = _mm_set1_epi16(32);
	const __m128i c_H	= _mm_set1_epi16(8192);
	const __m128i c_V	= _mm_set1_epi32(8192*64+2048);
	const __m128i xg_chroma		= _mm_set1_epi32( (Xxg3<<24) | ((Xxg2&0x00FF)<<16) | ((Xxg1&0x00FF)<<8) | (Xxg0&0x00FF));
	const __m128i xg16_chroma01 = _mm_set1_epi32( (Yxg1<<16) | (Yxg0 & 0xFFFF) );
	const __m128i xg16_chroma23 = _mm_set1_epi32( (Yxg3<<16) | (Yxg2 & 0xFFFF) );
	__m128i	tmp16[32+NTAPS_CHROMA-1][4];

	//Stage1. Interpolate H
	for (Int i=0; i<(int)nSize+NTAPS_CHROMA-1; i+=1){
		for (Int j=0; j<2; j++)	{
			Int offset0 = (i-1)*(int)nRefStride;//x64 modify, force conversion -- (int)nRefStride
			const __m128i T_00_00  = _mm_loadu_si128((const __m128i*)(pucRef+offset0-1+j*16)); //8bit: [14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 -01]
			const __m128i T_00_01  = _mm_loadu_si128((const __m128i*)(pucRef+offset0+0+j*16)); //8bit: [15 14 13 12 11 10 09 08 07 06 05 04 03 02 01  00]
			const __m128i T_00_02  = _mm_loadu_si128((const __m128i*)(pucRef+offset0+1+j*16)); //8bit: [16 17 14 13 12 11 10 09 08 07 06 05 04 03 02  01]
			const __m128i T_00_03  = _mm_loadu_si128((const __m128i*)(pucRef+offset0+2+j*16)); //8bit: [17 16 15 14 13 12 11 10 09 08 07 06 05 04 03  02]
			const __m128i T_01_00  = _mm_maddubs_epi16(T_00_00, xg_chroma);	    //16bit: 12a 12b 08a 08b 04a 04b 00a 00b
			const __m128i T_01_01  = _mm_maddubs_epi16(T_00_01, xg_chroma);		//16bit: 13a 13b 09a 09b 05a 05b 01a 01b
			const __m128i T_01_02  = _mm_maddubs_epi16(T_00_02, xg_chroma);		//16bit: 14a 14b 10a 10b 06a 06b 02a 02b
			const __m128i T_01_03  = _mm_maddubs_epi16(T_00_03, xg_chroma);		//16bit: 15a 15b 11a 11b 07a 07b 03a 03b
			const __m128i T_02_00  = _mm_hadd_epi16(T_01_00, T_01_03);			//16bit: 15 11 07 03 12 08 04 00
			const __m128i T_02_01  = _mm_hadd_epi16(T_01_01, T_01_02);			//16bit: 14 10 06 02 13 09 05 01
			const __m128i T_03_00  = _mm_unpacklo_epi16(T_02_00, T_02_01);			//16bit: 13 12 09 08 05 04 01 00
			const __m128i T_03_01  = _mm_unpackhi_epi16(T_02_01, T_02_00);			//16bit: 15 14 11 10 07 06 03 02
			const __m128i T_04_00a = _mm_sub_epi16(T_03_00, c_H);
			const __m128i T_04_00b = _mm_sub_epi16(T_03_01, c_H);
			tmp16[i][j*2+0]	= _mm_unpacklo_epi32(T_04_00a, T_04_00b);
			tmp16[i][j*2+1]	= _mm_unpackhi_epi32(T_04_00a, T_04_00b);
		}
	}

	//Stage2. Interpolate V
	for(Int16 i=0; i<(int)nSize; i++ ) {
		for (Int j=0; j<2; j++)	{
			Int offset1 = i*MAX_CU_SIZE/2+j*16;
			Int k = j*2;
			const __m128i	T_00_00a	= _mm_unpacklo_epi16(tmp16[i  ][k+1], tmp16[i+1][k+1]);//16bit: [11 11 10 10 09 09 08 08] Row: 0,-1
			const __m128i	T_00_01a	= _mm_unpackhi_epi16(tmp16[i  ][k+1], tmp16[i+1][k+1]);//16bit: [15 15 14 14 13 13 12 12] Row: 0,-1
			const __m128i	T_00_02a	= _mm_unpacklo_epi16(tmp16[i+2][k+1], tmp16[i+3][k+1]);//16bit: [11 11 10 10 09 09 08 08] Row: 2, 1
			const __m128i	T_00_03a	= _mm_unpackhi_epi16(tmp16[i+2][k+1], tmp16[i+3][k+1]);//16bit: [15 15 14 14 13 13 12 12] Row: 2, 1
			const __m128i	T_00_00b	= _mm_unpacklo_epi16(tmp16[i  ][k+0], tmp16[i+1][k+0]);//16bit: [03 03 02 02 01 01 00 00] Row: 0,-1
			const __m128i	T_00_01b	= _mm_unpackhi_epi16(tmp16[i  ][k+0], tmp16[i+1][k+0]);//16bit: [07 07 06 06 05 05 04 04] Row: 0,-1
			const __m128i	T_00_02b	= _mm_unpacklo_epi16(tmp16[i+2][k+0], tmp16[i+3][k+0]);//16bit: [03 03 02 02 01 01 00 00] Row: 2, 1
			const __m128i	T_00_03b	= _mm_unpackhi_epi16(tmp16[i+2][k+0], tmp16[i+3][k+0]);//16bit: [07 07 06 06 05 05 04 04] Row: 2, 1
			const __m128i	T_01_00a	= _mm_madd_epi16(xg16_chroma01, T_00_00a);//32bit: [11 10 09 08] 
			const __m128i	T_01_01a	= _mm_madd_epi16(xg16_chroma01, T_00_01a);//32bit: [15 14 13 12]
			const __m128i	T_01_02a	= _mm_madd_epi16(xg16_chroma23, T_00_02a);//32bit: [  ]
			const __m128i	T_01_03a	= _mm_madd_epi16(xg16_chroma23, T_00_03a);//32bit: [  ]
			const __m128i	T_01_00b	= _mm_madd_epi16(xg16_chroma01, T_00_00b);//32bit: [03 02 01 00] 
			const __m128i	T_01_01b	= _mm_madd_epi16(xg16_chroma01, T_00_01b);//32bit: [07 06 05 04]
			const __m128i	T_01_02b	= _mm_madd_epi16(xg16_chroma23, T_00_02b);//32bit: [  ]
			const __m128i	T_01_03b	= _mm_madd_epi16(xg16_chroma23, T_00_03b);//32bit: [  ]
			const __m128i	T_02_00 	= _mm_add_epi32(T_01_00a, T_01_02a);	  //32bit: [11 10 09 08] 
			const __m128i	T_02_01 	= _mm_add_epi32(T_01_01a, T_01_03a);	  //32bit: [15 14 13 12]
			const __m128i	T_02_02 	= _mm_add_epi32(T_01_00b, T_01_02b);	  //32bit: [03 02 01 00]
			const __m128i	T_02_03 	= _mm_add_epi32(T_01_01b, T_01_03b);	  //32bit: [07 06 05 04]
			const __m128i	T_04_00a	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_00), 12);	//32bit: [11 10 09 08]
			const __m128i	T_04_00b	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_01), 12);	//32bit: [15 14 13 12]
			const __m128i	T_04_00c	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_02), 12);	//32bit: [03 02 01 00]
			const __m128i	T_04_00d	= _mm_srai_epi32(_mm_add_epi32(c_V, T_02_03), 12);	//32bit: [07 06 05 04]
			const __m128i	T_05_00a	= _mm_packus_epi32(T_04_00c, T_04_00d); //16bit: [07 06 05 04 03 02 01 00]
			const __m128i	T_05_00b	= _mm_packus_epi32(T_04_00a, T_04_00b); //16bit: [15 14 13 12 11 10 09 08]

			_mm_storeu_si128((__m128i *)(pucPred+offset1), _mm_packus_epi16(T_05_00a, T_05_00b));
		}
	}
	//xInterpolate4HV_16x16(pucRef                 , pucPred                    , nRefStride, nSize/2, xFrac, yFrac, psTmp);
	//xInterpolate4HV_16x16(pucRef+16              , pucPred+16                 , nRefStride, nSize/2, xFrac, yFrac, psTmp);
	//xInterpolate4HV_16x16(pucRef+16*nRefStride   , pucPred+16*MAX_CU_SIZE/2   , nRefStride, nSize/2, xFrac, yFrac, psTmp);
	//xInterpolate4HV_16x16(pucRef+16*nRefStride+16, pucPred+16*MAX_CU_SIZE/2+16, nRefStride, nSize/2, xFrac, yFrac, psTmp);
}

typedef void (*xInterpolate4HV_ptr)(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSize,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16    *psTmp
	);

xInterpolate4HV_ptr xInterpolate4HV_NxN[4] = {
	xInterpolate4HV_4x4,
	xInterpolate4HV_8x8,
	xInterpolate4HV_16x16,
	xInterpolate4HV_32x32,
};

#else
static
	void xInterpolate4HV(
	CUInt8     *pucRef,
	UInt8     *pucPred,
	CUInt32     nRefStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt       xFrac,
	CUInt       yFrac,
	Int16    *psTmp
	)
{
	// TODO: Check the buffer size of psTmp
	//assert( nFrac != 0 );
	int i, j;

	// InterpolateH
	for( i=0; i<(int)nSizeY+NTAPS_CHROMA-1; i++ ) {
		CUInt8 *P = pucRef + (i-(NTAPS_CHROMA/2-1)) * nRefStride - (NTAPS_CHROMA/2-1);

		for( j=0; j<(int)nSizeX; j++ ) {
			Int16 iSum;
			iSum  = P[0+j] * xg_chromaFilter[xFrac][0];
			iSum += P[1+j] * xg_chromaFilter[xFrac][1];
			iSum += P[2+j] * xg_chromaFilter[xFrac][2];
			iSum += P[3+j] * xg_chromaFilter[xFrac][3];
			psTmp[i*MAX_CU_SIZE/2+j] = (iSum - 8192);
		}
	}

	// InterpolateV
	for( i=0; i<(int)nSizeX; i++ ) {
		CInt16 *P = psTmp + ((NTAPS_CHROMA/2-1) * MAX_CU_SIZE/2) + i;
		UInt8 *Q = pucPred + i;

		for( j=0; j<(int)nSizeY; j++ ) {
			Int32 iSum;
			iSum  = P[(j-1) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][0];
			iSum += P[(j+0) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][1];
			iSum += P[(j+1) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][2];
			iSum += P[(j+2) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][3];
			Q[j*MAX_CU_SIZE/2] = Clip3( 0, 255, (iSum + 8192*64+2048) >> 12);
		}
	}
}
#endif

void xPredInterChromaBlk(
	CUInt8     *pucRefC,
	UInt8     *pucPredC,
	CUInt32     nRefStrideC,
	CUInt       nSizeCX,
	CUInt       nSizeCY,
	xMV       bmv,
	Int16    *psTmp
	)
{
	Int  dx = bmv.x >> 3;
	Int  dy = bmv.y >> 3;
	UInt xFrac = bmv.x & 7;
	UInt yFrac = bmv.y & 7;
	UInt xyType = (yFrac != 0 ? 2 : 0) + (xFrac != 0 ? 1 : 0);
	int i;

	pucRefC += dy * (Int)nRefStrideC + dx;//x64 modify, converto to Int 
	switch( xyType ) {
	case 0:
		for( i=0; i<(int)nSizeCY; i++ ) {
			memcpy( pucPredC + i * MAX_CU_SIZE/2, pucRefC + i * nRefStrideC, nSizeCX );
		}
		break;
	case 1:
#if (INTERPOLATE_CHROMA_USE_ASM >= ASM_SSE4)
		xInterpolate4H_NxN[xLog2(nSizeCX-1)-2]( pucRefC, pucPredC, nRefStrideC, nSizeCX, xFrac );
#else
		xInterpolate4H( pucRefC, pucPredC, nRefStrideC, nSizeCX, nSizeCY, xFrac );
#endif
		break;
	case 2:
#if (INTERPOLATE_CHROMA_USE_ASM >= ASM_SSE4)
		xInterpolate4V_NxN[xLog2(nSizeCX-1)-2]( pucRefC, pucPredC, nRefStrideC, nSizeCX, yFrac );
#else
		xInterpolate4V( pucRefC, pucPredC, nRefStrideC, nSizeCX, nSizeCY, yFrac );
#endif
		break;
		/*case 3*/default:
#if (INTERPOLATE_CHROMA_USE_ASM >= ASM_SSE4)
		xInterpolate4HV_NxN[xLog2(nSizeCX-1)-2]( pucRefC, pucPredC, nRefStrideC, nSizeCX, xFrac, yFrac, psTmp );
#else
		xInterpolate4HV( pucRefC, pucPredC, nRefStrideC, nSizeCX, nSizeCY, xFrac, yFrac, psTmp );
#endif
			break;
	}
}

