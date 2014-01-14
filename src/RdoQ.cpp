/*****************************************************************************
 * preProcess.cpp: Pre-processing functions
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Zhengyi Luo, Yanan Zhao, Xiangwen Wang <wxw21st@163.com>
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
//lzy_RDOQ 在文件RdoQ.cpp中，包括头文件x265.h
#include "x265.h"

#include <math.h>


//lzy_RDOQ 在文件RdoQ.cpp中，把#ifdef RDOQ改为#if RDOQ。
//#ifdef RDOQ
#if RDOQ

#include "RdoQ.h"


//lzy_RDOQ 在文件RdoQ.cpp中加上函数estCBFBit()、estSignificantCoeffGroupMapBit()、estSignificantMapBit()、estSignificantCoefficientsBit()的声明。
void estCBFBit(estBitsSbacStruct* pcEstBitsSbac) ;
void estSignificantCoeffGroupMapBit(estBitsSbacStruct* pcEstBitsSbac, UInt8 bIsLuma) ;
void estSignificantMapBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, UInt8 bIsLuma) ;
void estSignificantCoefficientsBit(estBitsSbacStruct* pcEstBitsSbac, UInt8 bIsLuma) ;

//lzy_RDOQ 在文件RdoQ.cpp中，加上外部全局变量xCabac *g_pCabac的extern声明。
extern xCabac *g_pCabac ;

//lzy_RDOQ 在文件RdoQ.cpp中，拷贝g_entropyBits[128]的定义。
const Int g_entropyBits[128] =
{
    // Corrected table, most notably for last state
    0x07b23, 0x085f9, 0x074a0, 0x08cbc, 0x06ee4, 0x09354, 0x067f4, 0x09c1b, 0x060b0, 0x0a62a, 0x05a9c, 0x0af5b, 0x0548d, 0x0b955, 0x04f56, 0x0c2a9,
    0x04a87, 0x0cbf7, 0x045d6, 0x0d5c3, 0x04144, 0x0e01b, 0x03d88, 0x0e937, 0x039e0, 0x0f2cd, 0x03663, 0x0fc9e, 0x03347, 0x10600, 0x03050, 0x10f95,
    0x02d4d, 0x11a02, 0x02ad3, 0x12333, 0x0286e, 0x12cad, 0x02604, 0x136df, 0x02425, 0x13f48, 0x021f4, 0x149c4, 0x0203e, 0x1527b, 0x01e4d, 0x15d00,
    0x01c99, 0x166de, 0x01b18, 0x17017, 0x019a5, 0x17988, 0x01841, 0x18327, 0x016df, 0x18d50, 0x015d9, 0x19547, 0x0147c, 0x1a083, 0x0138e, 0x1a8a3,
    0x01251, 0x1b418, 0x01166, 0x1bd27, 0x01068, 0x1c77b, 0x00f7f, 0x1d18e, 0x00eda, 0x1d91a, 0x00e19, 0x1e254, 0x00d4f, 0x1ec9a, 0x00c90, 0x1f6e0,
    0x00c01, 0x1fef8, 0x00b5f, 0x208b1, 0x00ab6, 0x21362, 0x00a15, 0x21e46, 0x00988, 0x2285d, 0x00934, 0x22ea8, 0x008a8, 0x239b2, 0x0081d, 0x24577,
    0x007c9, 0x24ce6, 0x00763, 0x25663, 0x00710, 0x25e8f, 0x006a0, 0x26a26, 0x00672, 0x26f23, 0x005e8, 0x27ef8, 0x005ba, 0x284b5, 0x0055e, 0x29057,
    0x0050c, 0x29bab, 0x004c1, 0x2a674, 0x004a7, 0x2aa5e, 0x0046f, 0x2b32f, 0x0041f, 0x2c0ad, 0x003e7, 0x2ca8d, 0x003ba, 0x2d323, 0x0010c, 0x3bfbb
};


/*!
 ****************************************************************************
 * \brief
 *   estimate bit cost for CBP, significant map and significant coefficients
 ****************************************************************************
 */
void estimateBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, UInt8 bIsLuma)
{
    estCBFBit(pcEstBitsSbac);

    estSignificantCoeffGroupMapBit(pcEstBitsSbac, bIsLuma);

    // encode significance map
    estSignificantMapBit(pcEstBitsSbac, width, height, bIsLuma);

    // encode significant coefficients
    estSignificantCoefficientsBit(pcEstBitsSbac, bIsLuma);
}
/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost for each CBP bit
 ****************************************************************************
 */
void estCBFBit(estBitsSbacStruct* pcEstBitsSbac)
{
    //ContextModel *pCtx = m_cCUQtCbfSCModel.get(0);

    //lzy_RDOQ 在函数estCBFBit()中，加上变量ucState的定义。
    UInt8 ucState ;


    for (UInt uiCtxInc = 0; uiCtxInc < 3 * NUM_QT_CBF_CTX; uiCtxInc++)
    {
		ucState = g_pCabac->contextModels[OFF_QT_CBF_CTX + uiCtxInc];
		pcEstBitsSbac->blockCbpBits[uiCtxInc][0] = g_entropyBits[ucState ^ (short)0];
		pcEstBitsSbac->blockCbpBits[uiCtxInc][1] = g_entropyBits[ucState ^ (short)1];
        //pcEstBitsSbac->blockCbpBits[uiCtxInc][0] = pCtx[uiCtxInc].getEntropyBits(0);
        //pcEstBitsSbac->blockCbpBits[uiCtxInc][1] = pCtx[uiCtxInc].getEntropyBits(1);
    }

    //pCtx = m_cCUQtRootCbfSCModel.get(0);

    for (UInt uiCtxInc = 0; uiCtxInc < 4; uiCtxInc++)
    {
		ucState = g_pCabac->contextModels[OFF_QT_ROOT_CBF_CTX + uiCtxInc];
		pcEstBitsSbac->blockRootCbpBits[uiCtxInc][0] = g_entropyBits[ucState ^ (short)0];
		pcEstBitsSbac->blockRootCbpBits[uiCtxInc][1] = g_entropyBits[ucState ^ (short)1];
       //pcEstBitsSbac->blockRootCbpBits[uiCtxInc][0] = pCtx[uiCtxInc].getEntropyBits(0);
        //pcEstBitsSbac->blockRootCbpBits[uiCtxInc][1] = pCtx[uiCtxInc].getEntropyBits(1);
    }
}

/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient group map
 ****************************************************************************
 */
void estSignificantCoeffGroupMapBit(estBitsSbacStruct* pcEstBitsSbac, UInt8 bIsLuma)
{
    Int firstCtx = 0, numCtx = NUM_SIG_CG_FLAG_CTX;
	UInt8 ucState;

    for (Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++)
    {
        for (UInt uiBin = 0; uiBin < 2; uiBin++)
        {
			ucState = g_pCabac->contextModels[OFF_SIG_CG_FLAG_CTX + ctxIdx + (bIsLuma ? 0 : 1) * NUM_SIG_CG_FLAG_CTX];
            //pcEstBitsSbac->significantCoeffGroupBits[ctxIdx][uiBin] = m_cCUSigCoeffGroupSCModel.get(0, bIsLuma, ctxIdx).getEntropyBits(uiBin);
			pcEstBitsSbac->significantCoeffGroupBits[ctxIdx][uiBin] = g_entropyBits[ucState ^ (short)uiBin];
        }
    }
}

/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient map
 ****************************************************************************
 */
void estSignificantMapBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, UInt8 bIsLuma)
{
    Int firstCtx = 1, numCtx = 8;

    //lzy_RDOQ 在函数estSignificantMapBit()中，加上变量ucState的定义。
    UInt8 ucState;
	
    if (max(width, height) >= 16)
    {
        firstCtx = (bIsLuma == 1) ? 21 : 12;
        numCtx = (bIsLuma == 1) ? 6 : 3;
    }
    else if (width == 8)
    {
        firstCtx = 9;
        numCtx = (bIsLuma == 1) ? 12 : 3;
    }

    if (bIsLuma == 1)
    {
        for (UInt bin = 0; bin < 2; bin++)
        {//
			ucState = g_pCabac->contextModels[OFF_SIG_FLAG_CTX];
			pcEstBitsSbac->significantBits[0][bin] = g_entropyBits[ucState ^ (short)bin];
			//pcEstBitsSbac->significantBits[0][bin] = m_cCUSigSCModel.get(0, 0, 0).getEntropyBits(bin);
        }

        for (Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++)
        {
			ucState = g_pCabac->contextModels[OFF_SIG_FLAG_CTX + ctxIdx];
            for (UInt uiBin = 0; uiBin < 2; uiBin++)
            {
				pcEstBitsSbac->significantBits[ctxIdx][uiBin] = g_entropyBits[ucState ^ (short)uiBin];
				//pcEstBitsSbac->significantBits[ctxIdx][uiBin] = m_cCUSigSCModel.get(0, 0, ctxIdx).getEntropyBits(uiBin);
            }
        }
    }
    else
    {
        for (UInt bin = 0; bin < 2; bin++)
        {
			ucState = g_pCabac->contextModels[OFF_SIG_FLAG_CTX + NUM_SIG_FLAG_CTX_LUMA];
			pcEstBitsSbac->significantBits[0][bin] = g_entropyBits[ucState ^ (short)bin];
			//pcEstBitsSbac->significantBits[0][bin] = m_cCUSigSCModel.get(0, 0, NUM_SIG_FLAG_CTX_LUMA + 0).getEntropyBits(bin);
        }

        for (Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++)
        {
			ucState = g_pCabac->contextModels[OFF_SIG_FLAG_CTX + NUM_SIG_FLAG_CTX_LUMA + ctxIdx];
            for (UInt uiBin = 0; uiBin < 2; uiBin++)
            {
				pcEstBitsSbac->significantBits[ctxIdx][uiBin] = g_entropyBits[ucState ^ (short)uiBin];
                //pcEstBitsSbac->significantBits[ctxIdx][uiBin] = m_cCUSigSCModel.get(0, 0, ctxIdx).getEntropyBits(uiBin);
            }
        }
    }


	CUInt   nLog2Size = xLog2( width - 1 );
	UInt	nCtxLast;
	UInt	nCtxX = OFF_LAST_X_CTX + (bIsLuma ? 0 : NUM_LAST_FLAG_XY_CTX);
	UInt	nCtxY = OFF_LAST_Y_CTX + (bIsLuma ? 0 : NUM_LAST_FLAG_XY_CTX);
    
    //lzy_RDOQ 在函数estSignificantMapBit()中，因用于初始化uiGroupIdxX、uiGroupIdxY的nPosX、nPosY未定义，且uiGroupIdxX、uiGroupIdxY在后面未用到，所以注掉uiGroupIdxX、uiGroupIdxY的定义。
	//UInt	uiGroupIdxX    = xg_uiGroupIdx[ nPosX ];
	//UInt	uiGroupIdxY    = xg_uiGroupIdx[ nPosY ];

	Int blkSizeOffsetXY = bIsLuma ? ((nLog2Size-2)*3 + ((nLog2Size-1)>>2)) : 0;
	Int shiftXY         = bIsLuma ? ((nLog2Size+1)>>2) : nLog2Size-2;



    Int iBitsX = 0, iBitsY = 0;
    Int blkSizeOffsetX, blkSizeOffsetY, shiftX, shiftY;

    //blkSizeOffsetX = bIsLuma ? 0 : (g_convertToBit[width] * 3 + ((g_convertToBit[width] + 1) >> 2));
    //blkSizeOffsetY = bIsLuma ? 0 : (g_convertToBit[height] * 3 + ((g_convertToBit[height] + 1) >> 2));
    //shiftX = bIsLuma ? g_convertToBit[width] : ((g_convertToBit[width] + 3) >> 2);
    //shiftY = bIsLuma ? g_convertToBit[height] : ((g_convertToBit[height] + 3) >> 2);

    Int ctx;
    //ContextModel *pCtxX      = m_cCuCtxLastX.get(0, bIsLuma);
	
    for (ctx = 0; ctx < xg_uiGroupIdx[width - 1]; ctx++)
    {
        Int ctxOffset = blkSizeOffsetXY + (ctx >> shiftXY);
		ucState = g_pCabac->contextModels[nCtxX+ctxOffset];
        //pcEstBitsSbac->lastXBits[ctx] = iBitsX + pCtxX[ctxOffset].getEntropyBits(0);
		pcEstBitsSbac->lastXBits[ctx] = iBitsX + g_entropyBits[ucState ^ (short)0];
        //iBitsX += pCtxX[ctxOffset].getEntropyBits(1);
		iBitsX += g_entropyBits[ucState ^ (short)1];
    }
    pcEstBitsSbac->lastXBits[ctx] = iBitsX;

	//ContextModel *pCtxY      = m_cCuCtxLastY.get(0, bIsLuma);
    for (ctx = 0; ctx < xg_uiGroupIdx[height - 1]; ctx++)
    {
        Int ctxOffset = blkSizeOffsetXY + (ctx >> shiftXY);
        ucState = g_pCabac->contextModels[nCtxY+ctxOffset];
		//pcEstBitsSbac->lastYBits[ctx] = iBitsY + pCtxY[ctxOffset].getEntropyBits(0);
		pcEstBitsSbac->lastYBits[ctx] = iBitsY + g_entropyBits[ucState ^ (short)0];
        //iBitsY += pCtxY[ctxOffset].getEntropyBits(1);
		iBitsY += g_entropyBits[ucState ^ (short)1];
    }
    pcEstBitsSbac->lastYBits[ctx] = iBitsY;
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost of significant coefficient
 ****************************************************************************
 */
void estSignificantCoefficientsBit(estBitsSbacStruct* pcEstBitsSbac, UInt8 bIsLuma)
{
	UInt8 ucState;
    if (bIsLuma == 1)
    {
        //ContextModel *ctxOne = m_cCUOneSCModel.get(0, 0);
        //ContextModel *ctxAbs = m_cCUAbsSCModel.get(0, 0);

        for (Int ctxIdx = 0; ctxIdx < NUM_ONE_FLAG_CTX_LUMA; ctxIdx++)
        {
            //pcEstBitsSbac->greaterOneBits[ctxIdx][0] = ctxOne[ctxIdx].getEntropyBits(0);
			//pcEstBitsSbac->greaterOneBits[ctxIdx][1] = ctxOne[ctxIdx].getEntropyBits(1);
		    //Int getEntropyBits(Short val) { return m_entropyBits[m_ucState ^ val]; }
			ucState = g_pCabac->contextModels[OFF_ONE_FLAG_CTX + ctxIdx];
			pcEstBitsSbac->greaterOneBits[ctxIdx][0] = g_entropyBits[ucState ^ (short)0];
			pcEstBitsSbac->greaterOneBits[ctxIdx][1] = g_entropyBits[ucState ^ (short)1];
        }

        for (Int ctxIdx = 0; ctxIdx < NUM_ABS_FLAG_CTX_LUMA; ctxIdx++)
        {
            //pcEstBitsSbac->levelAbsBits[ctxIdx][0] = ctxAbs[ctxIdx].getEntropyBits(0);
            //pcEstBitsSbac->levelAbsBits[ctxIdx][1] = ctxAbs[ctxIdx].getEntropyBits(1);
			ucState = g_pCabac->contextModels[OFF_ABS_FLAG_CTX + ctxIdx];
			pcEstBitsSbac->levelAbsBits[ctxIdx][0] = g_entropyBits[ucState ^ (short)0];
			pcEstBitsSbac->levelAbsBits[ctxIdx][1] = g_entropyBits[ucState ^ (short)1];
        }
    }
    else
    {
        //ContextModel *ctxOne = m_cCUOneSCModel.get(0, 0) + NUM_ONE_FLAG_CTX_LUMA;
        //ContextModel *ctxAbs = m_cCUAbsSCModel.get(0, 0) + NUM_ABS_FLAG_CTX_LUMA;

        for (Int ctxIdx = 0; ctxIdx < NUM_ONE_FLAG_CTX_CHROMA; ctxIdx++)
        {
            //pcEstBitsSbac->greaterOneBits[ctxIdx][0] = ctxOne[ctxIdx].getEntropyBits(0);
            //pcEstBitsSbac->greaterOneBits[ctxIdx][1] = ctxOne[ctxIdx].getEntropyBits(1);
			ucState = g_pCabac->contextModels[OFF_ONE_FLAG_CTX + NUM_ONE_FLAG_CTX_LUMA + ctxIdx];
			pcEstBitsSbac->greaterOneBits[ctxIdx][0] = g_entropyBits[ucState ^ (short)0];
			pcEstBitsSbac->greaterOneBits[ctxIdx][1] = g_entropyBits[ucState ^ (short)1];
        }

        for (Int ctxIdx = 0; ctxIdx < NUM_ABS_FLAG_CTX_CHROMA; ctxIdx++)
        {
            //pcEstBitsSbac->levelAbsBits[ctxIdx][0] = ctxAbs[ctxIdx].getEntropyBits(0);
            //pcEstBitsSbac->levelAbsBits[ctxIdx][1] = ctxAbs[ctxIdx].getEntropyBits(1);
			ucState = g_pCabac->contextModels[OFF_ABS_FLAG_CTX + NUM_ABS_FLAG_CTX_LUMA + ctxIdx];
			pcEstBitsSbac->levelAbsBits[ctxIdx][0] = g_entropyBits[ucState ^ (short)0];
			pcEstBitsSbac->levelAbsBits[ctxIdx][1] = g_entropyBits[ucState ^ (short)1];
        }
    }
}

/*
// brief Encode bin
void xEncodeBin_BitEstimate( UInt32 **pCmd, UInt binValue, UInt nCtxState ) //(UInt binValue, ContextModel &rcCtxModel)
{
	UInt8 ucState = g_pCabac->contextModels[nCtxState];
	g_fracBits += g_entropyBits[ucState ^ (short)binValue];
	ucState = g_nextState[ucState][binValue];
	g_pCabac->contextModels[nCtxState] = ucState;
}

*/

#endif