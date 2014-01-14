/*****************************************************************************
 * RDOQ.h: 
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Zhengyi Luo, Xiangwen Wang <wxw21st@163.com>, Yanan Zhao
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
#ifndef __RDOQ_H__
#define __RDOQ_H__

#include "x265.h"
// ====================================================================================================================
// Constants
// ====================================================================================================================

#define QP_BITS 15

// ====================================================================================================================
// Type definition
// ====================================================================================================================

typedef struct
{
	Int significantCoeffGroupBits[NUM_SIG_CG_FLAG_CTX][2];
	Int significantBits[NUM_SIG_FLAG_CTX][2];
	Int lastXBits[32];
	Int lastYBits[32];
	Int greaterOneBits[NUM_ONE_FLAG_CTX][2];
	Int levelAbsBits[NUM_ABS_FLAG_CTX][2];

	Int blockCbpBits[3 * NUM_QT_CBF_CTX][2];
	Int blockRootCbpBits[4][2];
} estBitsSbacStruct;


Int  g_eTTable[4] = { 0, 3, 1, 2 };

//lzy
void My_xRateDistOptQuant(Int16 *pSrc, Int16 *pDst, UInt uiWidth, UInt uiHeight, UInt32 *puiAcSum, TextType eTType, UInt nQP, Int isIntra, UInt nMode, double dLambda, UInt uiTrDepth) ;
void My_xRateDistOptQuant_Init() ;
void My_xRateDistOptQuant_Destroy() ;


#endif