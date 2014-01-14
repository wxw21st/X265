/*****************************************************************************
 * encode.cpp: Main for encode
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Min Chen <chenm003@163.com>,  Xiangwen Wang <wxw21st@163.com>
 *          Yanan Zhao and Zhengyi Luo
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
#include <math.h>
#ifdef CHECK_SEI
#include "md5.h"
#endif


void xBuildNextStateTable(xRdoParam *rdoParam);

// ***************************************************************************
// * Internal Function Declares
// ***************************************************************************
static UInt32 xWriteMVD( UInt32 *pCmd, xMV mvd);
void xEncIntraPred( UInt8 *pucDstY, UInt8 *pucRefY, UInt nStride, UInt nMode, UInt nSize, UInt bIsLuma );
void xEncCahceInitLine( xCache *pCache );
void xEncCacheLoadCU( X265_t *h, xCache *pCache, UInt uiCUX, UInt uiCUY );
void xEncCacheStoreCU( X265_t *h, xCache *pCache, UInt uiCUX, UInt uiCUY );
void xEncCacheUpdate(	X265_t *h,	xCache *pCache,	Int    nSize,	UInt32  uiOffset,	CUInt32 idxTiles	);

//x64 modify, re-place functions in encode.cpp, place more efficiently

#ifdef DEBUG_VIS
void xDrawVisCU(
	xFrame *frm_vis,
	UInt8  *pucPixY,
	UInt8  *pucPixU,
	UInt8  *pucPixV,
	UInt16 *pusModeYInfo0,
	UInt8  *pucModeYInfo1,
	UInt    nX,
	UInt    nY,
	UInt    nWidth
	);
#endif

void xPredInterLumaBlk(
	CUInt8     *pucRef,
	UInt8	   *pucPred,
	CUInt32     nRefStride,
	//CUInt32		nPredStride,
	CUInt       nSizeX,
	CUInt       nSizeY,
	xMV         bmv,
	Int16      *psTmp
	);

void xPredInterChromaBlk(
	CUInt8     *pucRefC,
	UInt8     *pucPredC,
	CUInt32     nRefStrideC,
	CUInt       nSizeCX,
	CUInt       nSizeCY,
	xMV       bmv,
	Int16    *psTmp
	);

UInt getInterleaveBits( UInt8 x );

inline void xEncodeBin_pCmd(UInt32 **pCmd, UInt32 binValue, UInt32 nCtxState) 
{
	*(*pCmd)++ = FLAG_TYPE_BIN | ((binValue) << FLAG_OFF_VAL) | ((nCtxState) << FLAG_OFF_CTX);
}
inline void xEncodeBinsEP_pCmd(UInt32 **pCmd, UInt32 binValue, UInt32 numBins)
{
	*(*pCmd)++ = FLAG_TYPE_EPS | ((binValue) << FLAG_OFF_VAL) | ((numBins)   << FLAG_OFF_CTX);
}
inline void xEncodeTerminatingBit_pCmd(UInt32 **pCmd, UInt32 bLastCU) 
{
	*(*pCmd) ++ = FLAG_TYPE_TRM | ((bLastCU) << FLAG_OFF_VAL);
}

	const UInt8 g_aucNextStateMPS[128] =
	{
		2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
		34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
		50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
		66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
		82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
		98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
		114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 124, 125, 126, 127
	};

	const UInt8 g_aucNextStateLPS[128] =
	{
		1, 0, 0, 1, 2, 3, 4, 5, 4, 5, 8, 9, 8, 9, 10, 11,
		12, 13, 14, 15, 16, 17, 18, 19, 18, 19, 22, 23, 22, 23, 24, 25,
		26, 27, 26, 27, 30, 31, 30, 31, 32, 33, 32, 33, 36, 37, 36, 37,
		38, 39, 38, 39, 42, 43, 42, 43, 44, 45, 44, 45, 46, 47, 48, 49,
		48, 49, 50, 51, 52, 53, 52, 53, 54, 55, 54, 55, 56, 57, 58, 59,
		58, 59, 60, 61, 60, 61, 60, 61, 62, 63, 64, 65, 64, 65, 66, 67,
		66, 67, 66, 67, 68, 69, 68, 69, 70, 71, 70, 71, 70, 71, 72, 73,
		72, 73, 72, 73, 74, 75, 74, 75, 74, 75, 76, 77, 76, 77, 126, 127
	};

	void xBuildNextStateTable(xRdoParam *rdoParam)
	{
		for (Int i = 0; i < 128; i++)
		{
			for (Int j = 0; j < 2; j++)
			{
				rdoParam->nextState[i][j] = ((i & 1) == j) ? g_aucNextStateMPS[i] : g_aucNextStateLPS[i];
			}
		}
	}

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

	void xEncodeBin_BitEstimate( UInt binValue, UInt nCtxState, xRdoParam *rdoParam) //x64 modify, remove unused parameters
	{
		UInt8 ucState = rdoParam->pCabacRdo->contextModels[nCtxState];
		rdoParam->fracBits += g_entropyBits[ucState ^ (short)binValue];
		ucState = rdoParam->nextState[ucState][binValue];
		rdoParam->pCabacRdo->contextModels[nCtxState] = ucState;
	}

	void xEncodeBinsEP_BitEstimate( UInt numBins, xRdoParam *rdoParam)//x64 modify, remove unused parameters
	{
		rdoParam->fracBits += 32768 * numBins;
	}

	void xEncodeTerminatingBit_BitEstimate( UInt binValue, xRdoParam *rdoParam)
	{
		rdoParam->fracBits += g_entropyBits[126 ^ binValue];
	}

	void xEstimateCmdsBit( UInt32 *pCmd, Int32 iCmdNum, xRdoParam *rdoParam )
	{
		while( iCmdNum-- > 0 ) {
			UInt32  uiCmd   = *pCmd++;

			UInt    nVal    = (uiCmd >> FLAG_OFF_VAL);
			UInt    nCtx    = (uiCmd >> FLAG_OFF_CTX) & FLAG_MASK_CTX;

			if ( uiCmd & FLAG_TYPE_BIN ) {
				xEncodeBin_BitEstimate( nVal, nCtx, rdoParam );
			}
			else if ( uiCmd & FLAG_TYPE_TRM ) {
				xEncodeTerminatingBit_BitEstimate( nVal, rdoParam );
			}
			else {
				xEncodeBinsEP_BitEstimate( nCtx, rdoParam );
			}
		}
	}

	UInt xGetNumWrittenBits(xRdoParam *rdoParam)
	{
		return UInt(rdoParam->fracBits >> 15);
	}

	UInt xGetWrittenCmdRate(UInt32 *pCmd, UInt32 iCmdNum, xRdoParam *rdoParam)
	{
		rdoParam->fracBits &= 32767;
		xEstimateCmdsBit( pCmd, iCmdNum, rdoParam );
		return UInt(rdoParam->fracBits >> 15);
	}

UInt xGetComponentBits( Int iVal )
{
	UInt uiLength = 1;
	UInt uiTemp   = ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);
	assert ( uiTemp );
	while ( 1 != uiTemp )
	{
		uiTemp >>= 1;
		uiLength += 2;
	}
	return uiLength;
}

#if SSD_USE_TRUE_ASM
extern xSSD *xSsdN[MAX_CU_DEPTH];
#else
UInt32 xSsd( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef, UInt32 nSize );
#endif

UInt xGetMvpIdxBits(Int iIdx, Int iNum)
{
	assert(iIdx >= 0 && iNum >= 0 && iIdx < iNum);
	if (iNum == 1)
		return 0;
	UInt uiLength = 1;
	Int iTemp = iIdx;
	if ( iTemp == 0 )
	{
		return uiLength;
	}
	//lzy
	//int  bCodeLast = ( iNum-1 > iTemp );
	int bCodeLast = ( iNum-1 > iTemp );
	uiLength += (iTemp-1);
	if( bCodeLast )
	{
		uiLength++;
	}
	return uiLength;
}

//#if DEBLOCK //x64 modify
void ff_hevc_deblocking_boundary_strengths( 
	UInt8	*horizontal_bs, 
	UInt8	*vertical_bs, 
	CUInt16 nWidth, 
	int		x0, 
	int		y0, 
	int		log2_trafo_size, 
	int 	b64x64Flag, 
	xMV		*pasMV, 
	UInt8	*puclModeYInfo1, 
	xSliceType eSliceType
	);

void xDeblockFrame( 
	X265_t *h, 
	CUInt32 nCUWidth, 
	CUInt32 nCUHeight, 
	Int32 offX, 
	Int32 offY, 
	Int32 bsOff, 
	Int32 idxTiles 
	);
//#endif

#if SAO
//double lumaLambda = 19.05;
//double chromaLambda = 19.05;

/* move the following parameters into x265_t, YananZhao, 2013-12-14
UInt8 g_lumaTableBo[(1 << 8)];
UInt8 g_chromaTableBo[(1 << 8)];
UInt8 g_clipTableBase[2 * ((1 << 8) - 1)];
UInt8 g_chromaClipTableBase[2 * ((1 << 8) - 1)];
UInt8 *g_clipTable = NULL;
UInt8 *g_chromaClipTable = NULL;*/

#define MAX_DOUBLE 1.7e+308 

const UInt g_eoTable[9] = {
    1, //0
    2, //1
    0, //2
    3, //3
    4, //4
    0, //5
    0, //6
    0, //7
    0
};

const int g_numClass[MAX_NUM_SAO_TYPE] =
{
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_BO_LEN
};

inline double xRoundIbdi(double x)
{
    return ((x) >= 0 ? ((int)((x) + 0.5)) : ((int)((x) - 0.5)));
}

inline int xSign(int x)
{
    return (x >> 31) | ((int)((((UInt) - x)) >> 31));
}

void rdoSaoUnitRowInit(X265_t *h)
{
    const int depth = h->sao_depth;
    SAOParam *saoParam = &h->sao_param;

    saoParam->bSaoFlag[0] = true;
    saoParam->bSaoFlag[1] = true;

    h->numNoSao[0] = 0; // Luma
    h->numNoSao[1] = 0; // Chroma
    if (depth > 0 && h->m_depthSaoRate[0][depth - 1] > SAO_ENCODING_RATE)
    {
        saoParam->bSaoFlag[0] = FALSE;
    }
    if (depth > 0 && h->m_depthSaoRate[1][depth - 1] > SAO_ENCODING_RATE_CHROMA)
    {
        saoParam->bSaoFlag[1] = FALSE;
    }
}

void rdoSaoUnitRowEnd(X265_t *h)
{
    const int depth = h->sao_depth;
    SAOParam *saoParam = &h->sao_param;

    if (!saoParam->bSaoFlag[0])
    {
        h->m_depthSaoRate[0][depth] = 1.0;
    }
    else
    {
        h->m_depthSaoRate[0][depth] = h->numNoSao[0] / (h->numlcus);
    }
    if (!saoParam->bSaoFlag[1])
    {
        h->m_depthSaoRate[1][depth] = 1.0;
    }
    else
    {
        h->m_depthSaoRate[1][depth] = h->numNoSao[1] / (h->numlcus * 2);
    }
}

UInt8* getPicYuvAddr(xFrame *pFrame, int yCbCr, UInt uiX, UInt uiY, UInt CUSize)
{
    UInt8 *pRef = NULL;

    if (yCbCr == 0) {
        pRef = pFrame->pucY + uiY * CUSize * pFrame->iStrideY + uiX * CUSize;
    }
    else if (yCbCr == 1) {
        pRef = pFrame->pucU + uiY * CUSize / 2 * pFrame->iStrideC + uiX * CUSize / 2;
    }
    else {
        pRef = pFrame->pucV + uiY * CUSize / 2 * pFrame->iStrideC + uiX * CUSize / 2;
    }
    return pRef;
}

void calcSaoStatsCu(X265_t *h, UInt32 uiX, UInt32 uiY, int partIdx, int yCbCr)
{
    int x, y;
    UInt8* fenc;
    UInt8* pRec;
    int stride;
	int stride_fenc;
    int iLcuHeight = h->ucMaxCUWidth;
    int iLcuWidth  = h->ucMaxCUWidth;
    Int lpelx   = uiX * h->ucMaxCUWidth; //x64 modify, convert to Int
    Int tpely   = uiY * h->ucMaxCUWidth; //x64 modify, convert to Int
    Int rpelx;//x64 modify, convert to Int
    Int bpely;//x64 modify, convert to Int 
    Int64* iStats;
    Int64* iCount;
    int iClassIdx;
    int iPicWidthTmp;
    int iPicHeightTmp;
    int iStartX;
    int iStartY;
    int iEndX;
    int iEndY;
    UInt8* pTableBo = (yCbCr == 0) ? h->g_lumaTableBo : h->g_chromaTableBo;
    int *tmp_swap;

    int iIsChroma = (yCbCr != 0) ? 1 : 0;
    int numSkipLine = iIsChroma ? 2 : 4;
    int numSkipLineRight = iIsChroma ? 3 : 5;

    iPicWidthTmp  = h->iWidth  >> iIsChroma;
    iPicHeightTmp = h->iHeight >> iIsChroma;
    iLcuWidth     = iLcuWidth    >> iIsChroma;
    iLcuHeight    = iLcuHeight   >> iIsChroma;
    lpelx       = lpelx      >> iIsChroma;
    tpely       = tpely      >> iIsChroma;
    rpelx       = lpelx + iLcuWidth;
    bpely       = tpely + iLcuHeight;
    rpelx       = rpelx > iPicWidthTmp  ? iPicWidthTmp  : rpelx;
    bpely       = bpely > iPicHeightTmp ? iPicHeightTmp : bpely;
    iLcuWidth     = rpelx - lpelx;
    iLcuHeight    = bpely - tpely;

    stride    =  (yCbCr == 0) ? h->pFrameRec->iStrideY : h->pFrameRec->iStrideC;
	stride_fenc = (yCbCr == 0) ? h->pFrameCur->iStrideY : h->pFrameCur->iStrideC;
//if(iSaoType == BO_0 || iSaoType == BO_1)
    {
        if (0)
        {
            numSkipLine = iIsChroma ? 1 : 3;
            numSkipLineRight = iIsChroma ? 2 : 4;
        }
        iStats = h->m_offsetOrg[partIdx][SAO_BO];
        iCount = h->m_count[partIdx][SAO_BO];

        fenc = getPicYuvAddr(h->pFrameCur, yCbCr, uiX, uiY, h->ucMaxCUWidth);
        pRec = getPicYuvAddr(h->pFrameRec, yCbCr, uiX, uiY, h->ucMaxCUWidth);

        iEndX   = (rpelx == iPicWidthTmp) ? iLcuWidth : iLcuWidth - numSkipLineRight;
        iEndY   = (bpely == iPicHeightTmp) ? iLcuHeight : iLcuHeight - numSkipLine;
        for (y = 0; y < iEndY; y++)
        {
            for (x = 0; x < iEndX; x++)
            {
                iClassIdx = pTableBo[pRec[x]];
                if (iClassIdx)
                {
                    iStats[iClassIdx] += (fenc[x] - pRec[x]);
                    iCount[iClassIdx]++;
                }
            }

            fenc += stride_fenc;
            pRec += stride;
        }
    }
    int iSignLeft;
    int iSignRight;
    int iSignDown;
    int iSignDown1;
    int iSignDown2;

    UInt uiEdgeType;

//if (iSaoType == EO_0  || iSaoType == EO_1 || iSaoType == EO_2 || iSaoType == EO_3)
    {
        //if (iSaoType == EO_0)
        {
            if (0)
            {
                numSkipLine = iIsChroma ? 1 : 3;
                numSkipLineRight = iIsChroma ? 3 : 5;
            }
            iStats = h->m_offsetOrg[partIdx][SAO_EO_0];
            iCount = h->m_count[partIdx][SAO_EO_0];

            fenc = getPicYuvAddr(h->pFrameCur, yCbCr, uiX, uiY, h->ucMaxCUWidth);
            pRec = getPicYuvAddr(h->pFrameRec, yCbCr, uiX, uiY, h->ucMaxCUWidth);

            iStartX = (lpelx == 0) ? 1 : 0;
            iEndX   = (rpelx == iPicWidthTmp) ? iLcuWidth - 1 : iLcuWidth - numSkipLineRight;
            for (y = 0; y < iLcuHeight - numSkipLine; y++)
            {
                iSignLeft = xSign(pRec[iStartX] - pRec[iStartX - 1]);
                for (x = iStartX; x < iEndX; x++)
                {
                    iSignRight =  xSign(pRec[x] - pRec[x + 1]);
                    uiEdgeType =  iSignRight + iSignLeft + 2;
                    iSignLeft  = -iSignRight;

                    iStats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                    iCount[g_eoTable[uiEdgeType]]++;
                }

                fenc += stride_fenc;
                pRec += stride;
            }
        }

        //if (iSaoType == EO_1)
        {
            if (0)
            {
                numSkipLine = iIsChroma ? 2 : 4;
                numSkipLineRight = iIsChroma ? 2 : 4;
            }
            iStats = h->m_offsetOrg[partIdx][SAO_EO_1];
            iCount = h->m_count[partIdx][SAO_EO_1];

            fenc = getPicYuvAddr(h->pFrameCur, yCbCr, uiX, uiY, h->ucMaxCUWidth);
            pRec = getPicYuvAddr(h->pFrameRec, yCbCr, uiX, uiY, h->ucMaxCUWidth);

            iStartY = (tpely == 0) ? 1 : 0;
            iEndX   = (rpelx == iPicWidthTmp) ? iLcuWidth : iLcuWidth - numSkipLineRight;
            iEndY   = (bpely == iPicHeightTmp) ? iLcuHeight - 1 : iLcuHeight - numSkipLine;
            if (tpely == 0)
            {
                fenc += stride_fenc;
                pRec += stride;
            }

            for (x = 0; x < iLcuWidth; x++)
            {
                h->m_upBuff1[x] = xSign(pRec[x] - pRec[x - stride]);
            }

            for (y = iStartY; y < iEndY; y++)
            {
                for (x = 0; x < iEndX; x++)
                {
                    iSignDown     =  xSign(pRec[x] - pRec[x + stride]);
                    uiEdgeType    =  iSignDown + h->m_upBuff1[x] + 2;
                    h->m_upBuff1[x] = -iSignDown;

                    iStats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                    iCount[g_eoTable[uiEdgeType]]++;
                }

                fenc += stride_fenc;
                pRec += stride;
            }
        }
        //if (iSaoType == EO_2)
        {
            if (0)
            {
                numSkipLine = iIsChroma ? 2 : 4;
                numSkipLineRight = iIsChroma ? 3 : 5;
            }
            iStats = h->m_offsetOrg[partIdx][SAO_EO_2];
            iCount = h->m_count[partIdx][SAO_EO_2];

            fenc = getPicYuvAddr(h->pFrameCur, yCbCr, uiX, uiY, h->ucMaxCUWidth);
            pRec = getPicYuvAddr(h->pFrameRec, yCbCr, uiX, uiY, h->ucMaxCUWidth);

            iStartX = (lpelx == 0) ? 1 : 0;
            iEndX   = (rpelx == iPicWidthTmp) ? iLcuWidth - 1 : iLcuWidth - numSkipLineRight;

            iStartY = (tpely == 0) ? 1 : 0;
            iEndY   = (bpely == iPicHeightTmp) ? iLcuHeight - 1 : iLcuHeight - numSkipLine;
            if (tpely == 0)
            {
                fenc += stride_fenc;
                pRec += stride;
            }

            for (x = iStartX; x < iEndX; x++)
            {
                h->m_upBuff1[x] = xSign(pRec[x] - pRec[x - stride - 1]);
            }

            for (y = iStartY; y < iEndY; y++)
            {
                iSignDown2 = xSign(pRec[stride + iStartX] - pRec[iStartX - 1]);
                for (x = iStartX; x < iEndX; x++)
                {
                    iSignDown1      =  xSign(pRec[x] - pRec[x + stride + 1]);
                    uiEdgeType      =  iSignDown1 + h->m_upBuff1[x] + 2;
                    h->m_upBufft[x + 1] = -iSignDown1;
                    iStats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                    iCount[g_eoTable[uiEdgeType]]++;
                }

                h->m_upBufft[iStartX] = iSignDown2;
                tmp_swap     = h->m_upBuff1;
                h->m_upBuff1 = h->m_upBufft;
                h->m_upBufft = tmp_swap;

                pRec += stride;
                fenc += stride_fenc;
            }
        }
        //if (iSaoType == EO_3  )
        {
            if (0)
            {
                numSkipLine = iIsChroma ? 2 : 4;
                numSkipLineRight = iIsChroma ? 3 : 5;
            }
            iStats = h->m_offsetOrg[partIdx][SAO_EO_3];
            iCount = h->m_count[partIdx][SAO_EO_3];

            fenc = getPicYuvAddr(h->pFrameCur, yCbCr, uiX, uiY, h->ucMaxCUWidth);
            pRec = getPicYuvAddr(h->pFrameRec, yCbCr, uiX, uiY, h->ucMaxCUWidth);

            iStartX = (lpelx == 0) ? 1 : 0;
            iEndX   = (rpelx == iPicWidthTmp) ? iLcuWidth - 1 : iLcuWidth - numSkipLineRight;

            iStartY = (tpely == 0) ? 1 : 0;
            iEndY   = (bpely == iPicHeightTmp) ? iLcuHeight - 1 : iLcuHeight - numSkipLine;
            if (iStartY == 1)
            {
                fenc += stride_fenc;
                pRec += stride;
            }

            for (x = iStartX - 1; x < iEndX; x++)
            {
                h->m_upBuff1[x] = xSign(pRec[x] - pRec[x - stride + 1]);
            }

            for (y = iStartY; y < iEndY; y++)
            {
                for (x = iStartX; x < iEndX; x++)
                {
                    iSignDown1      =  xSign(pRec[x] - pRec[x + stride - 1]);
                    uiEdgeType      =  iSignDown1 + h->m_upBuff1[x] + 2;
                    h->m_upBuff1[x - 1] = -iSignDown1;
                    iStats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                    iCount[g_eoTable[uiEdgeType]]++;
                }

                h->m_upBuff1[iEndX - 1] = xSign(pRec[iEndX - 1 + stride] - pRec[iEndX]);

                pRec += stride;
                fenc += stride_fenc;
            }
        }
    }
}

void calcSaoStatsRowCus_BeforeDblk(X265_t *h)
{
    int addr, yCbCr;
    int x, y;

    UInt8* fenc;
    UInt8* pRec;
    int stride;
    int lcuHeight = h->ucMaxCUWidth;
    int lcuWidth  = h->ucMaxCUWidth;
    Int rPelX;//x64 modify, convert to Int
    Int bPelY;//x64 modify, convert to Int
    Int64* stats;
    Int64* count;
    int classIdx;
    int picWidthTmp = 0;
    int picHeightTmp = 0;
    int startX;
    int startY;
    int endX;
    int endY;
    int firstX, firstY;

    int idxX, idxY;
    int frameWidthInCU  = h->PicWidthInCUs;
    int frameHeightInCU = h->PicHeightInCUs;

    int isChroma;
    int numSkipLine, numSkipLineRight;

    Int lPelX, tPelY;//x64 modify, convert to Int
    UInt8* pTableBo;
    int *tmp_swap;

    memset(h->m_countPreDblk, 0, sizeof(h->m_countPreDblk));
    memset(h->m_offsetOrgPreDblk, 0, sizeof(h->m_offsetOrgPreDblk));
    for (idxY = 0; idxY < frameHeightInCU; idxY++)
    {
        for (idxX = 0; idxX < frameWidthInCU; idxX++)
        {
            lcuHeight = h->ucMaxCUWidth;
            lcuWidth  = h->ucMaxCUWidth;
            addr     = idxX  + frameWidthInCU * idxY;
            lPelX   = idxX * h->ucMaxCUWidth;
            tPelY   = idxY * h->ucMaxCUWidth;

            for (yCbCr = 0; yCbCr < 3; yCbCr++)
            {
                isChroma = (yCbCr != 0) ? 1 : 0;

                if (yCbCr == 0)
                {
                    picWidthTmp  = h->iWidth;
                    picHeightTmp = h->iHeight;
                }
                else if (yCbCr == 1)
                {
                    picWidthTmp  = h->iWidth  >> isChroma;
                    picHeightTmp = h->iHeight >> isChroma;
                    lcuWidth     = lcuWidth    >> isChroma;
                    lcuHeight    = lcuHeight   >> isChroma;
                    lPelX       = lPelX      >> isChroma;
                    tPelY       = tPelY      >> isChroma;
                }
                rPelX       = lPelX + lcuWidth;
                bPelY       = tPelY + lcuHeight;
                rPelX       = rPelX > picWidthTmp  ? picWidthTmp  : rPelX;
                bPelY       = bPelY > picHeightTmp ? picHeightTmp : bPelY;
                lcuWidth     = rPelX - lPelX;
                lcuHeight    = bPelY - tPelY;

                stride   =  (yCbCr == 0) ? h->pFrameRec->iStrideY : h->pFrameRec->iStrideC;
                pTableBo = (yCbCr == 0) ? h->g_lumaTableBo : h->g_chromaTableBo;

                //if(iSaoType == BO)

                numSkipLine = isChroma ? 1 : 3;
                numSkipLineRight = isChroma ? 2 : 4;

                stats = h->m_offsetOrgPreDblk[addr][yCbCr][SAO_BO];
                count = h->m_countPreDblk[addr][yCbCr][SAO_BO];

                fenc = getPicYuvAddr(h->pFrameCur, yCbCr, idxX, idxY, h->ucMaxCUWidth);
                pRec = getPicYuvAddr(h->pFrameRec, yCbCr, idxX, idxY, h->ucMaxCUWidth);

                startX   = (rPelX == picWidthTmp) ? lcuWidth : lcuWidth - numSkipLineRight;
                startY   = (bPelY == picHeightTmp) ? lcuHeight : lcuHeight - numSkipLine;

                for (y = 0; y < lcuHeight; y++)
                {
                    for (x = 0; x < lcuWidth; x++)
                    {
                        if (x < startX && y < startY)
                            continue;

                        classIdx = pTableBo[pRec[x]];
                        if (classIdx)
                        {
                            stats[classIdx] += (fenc[x] - pRec[x]);
                            count[classIdx]++;
                        }
                    }

                    fenc += stride;
                    pRec += stride;
                }

                int signLeft;
                int signRight;
                int signDown;
                int signDown1;
                int signDown2;

                UInt uiEdgeType;

                //if (iSaoType == EO_0)

                numSkipLine = isChroma ? 1 : 3;
                numSkipLineRight = isChroma ? 3 : 5;

                stats = h->m_offsetOrgPreDblk[addr][yCbCr][SAO_EO_0];
                count = h->m_countPreDblk[addr][yCbCr][SAO_EO_0];

                fenc = getPicYuvAddr(h->pFrameCur, yCbCr, idxX, idxY, h->ucMaxCUWidth);
                pRec = getPicYuvAddr(h->pFrameRec, yCbCr, idxX, idxY, h->ucMaxCUWidth);

                startX   = (rPelX == picWidthTmp) ? lcuWidth - 1 : lcuWidth - numSkipLineRight;
                startY   = (bPelY == picHeightTmp) ? lcuHeight : lcuHeight - numSkipLine;
                firstX   = (lPelX == 0) ? 1 : 0;
                endX   = (rPelX == picWidthTmp) ? lcuWidth - 1 : lcuWidth;

                for (y = 0; y < lcuHeight; y++)
                {
                    signLeft = xSign(pRec[firstX] - pRec[firstX - 1]);
                    for (x = firstX; x < endX; x++)
                    {
                        signRight =  xSign(pRec[x] - pRec[x + 1]);
                        uiEdgeType =  signRight + signLeft + 2;
                        signLeft  = -signRight;

                        if (x < startX && y < startY)
                            continue;

                        stats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                        count[g_eoTable[uiEdgeType]]++;
                    }

                    fenc += stride;
                    pRec += stride;
                }

                //if (iSaoType == EO_1)

                numSkipLine = isChroma ? 2 : 4;
                numSkipLineRight = isChroma ? 2 : 4;

                stats = h->m_offsetOrgPreDblk[addr][yCbCr][SAO_EO_1];
                count = h->m_countPreDblk[addr][yCbCr][SAO_EO_1];

                fenc = getPicYuvAddr(h->pFrameCur, yCbCr, idxX, idxY, h->ucMaxCUWidth);
                pRec = getPicYuvAddr(h->pFrameRec, yCbCr, idxX, idxY, h->ucMaxCUWidth);

                startX   = (rPelX == picWidthTmp) ? lcuWidth : lcuWidth - numSkipLineRight;
                startY   = (bPelY == picHeightTmp) ? lcuHeight - 1 : lcuHeight - numSkipLine;
                firstY = (tPelY == 0) ? 1 : 0;
                endY   = (bPelY == picHeightTmp) ? lcuHeight - 1 : lcuHeight;
                if (firstY == 1)
                {
                    fenc += stride;
                    pRec += stride;
                }

                for (x = 0; x < lcuWidth; x++)
                {
                    h->m_upBuff1[x] = xSign(pRec[x] - pRec[x - stride]);
                }

                for (y = firstY; y < endY; y++)
                {
                    for (x = 0; x < lcuWidth; x++)
                    {
                        signDown     =  xSign(pRec[x] - pRec[x + stride]);
                        uiEdgeType    =  signDown + h->m_upBuff1[x] + 2;
                        h->m_upBuff1[x] = -signDown;

                        if (x < startX && y < startY)
                            continue;

                        stats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                        count[g_eoTable[uiEdgeType]]++;
                    }

                    fenc += stride;
                    pRec += stride;
                }

                //if (iSaoType == EO_2)

                numSkipLine = isChroma ? 2 : 4;
                numSkipLineRight = isChroma ? 3 : 5;

                stats = h->m_offsetOrgPreDblk[addr][yCbCr][SAO_EO_2];
                count = h->m_countPreDblk[addr][yCbCr][SAO_EO_2];

                fenc = getPicYuvAddr(h->pFrameCur, yCbCr, idxX, idxY, h->ucMaxCUWidth);
                pRec = getPicYuvAddr(h->pFrameRec, yCbCr, idxX, idxY, h->ucMaxCUWidth);

                startX   = (rPelX == picWidthTmp) ? lcuWidth - 1 : lcuWidth - numSkipLineRight;
                startY   = (bPelY == picHeightTmp) ? lcuHeight - 1 : lcuHeight - numSkipLine;
                firstX   = (lPelX == 0) ? 1 : 0;
                firstY = (tPelY == 0) ? 1 : 0;
                endX   = (rPelX == picWidthTmp) ? lcuWidth - 1 : lcuWidth;
                endY   = (bPelY == picHeightTmp) ? lcuHeight - 1 : lcuHeight;
                if (firstY == 1)
                {
                    fenc += stride;
                    pRec += stride;
                }

                for (x = firstX; x < endX; x++)
                {
                    h->m_upBuff1[x] = xSign(pRec[x] - pRec[x - stride - 1]);
                }

                for (y = firstY; y < endY; y++)
                {
                    signDown2 = xSign(pRec[stride + startX] - pRec[startX - 1]);
                    for (x = firstX; x < endX; x++)
                    {
                        signDown1      =  xSign(pRec[x] - pRec[x + stride + 1]);
                        uiEdgeType      =  signDown1 + h->m_upBuff1[x] + 2;
                        h->m_upBufft[x + 1] = -signDown1;

                        if (x < startX && y < startY)
                            continue;

                        stats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                        count[g_eoTable[uiEdgeType]]++;
                    }

                    h->m_upBufft[firstX] = signDown2;
                    tmp_swap  = h->m_upBuff1;
                    h->m_upBuff1 = h->m_upBufft;
                    h->m_upBufft = tmp_swap;

                    pRec += stride;
                    fenc += stride;
                }

                //if (iSaoType == EO_3)

                numSkipLine = isChroma ? 2 : 4;
                numSkipLineRight = isChroma ? 3 : 5;

                stats = h->m_offsetOrgPreDblk[addr][yCbCr][SAO_EO_3];
                count = h->m_countPreDblk[addr][yCbCr][SAO_EO_3];

                fenc = getPicYuvAddr(h->pFrameCur, yCbCr, idxX, idxY, h->ucMaxCUWidth);
                pRec = getPicYuvAddr(h->pFrameRec, yCbCr, idxX, idxY, h->ucMaxCUWidth);

                startX   = (rPelX == picWidthTmp) ? lcuWidth - 1 : lcuWidth - numSkipLineRight;
                startY   = (bPelY == picHeightTmp) ? lcuHeight - 1 : lcuHeight - numSkipLine;
                firstX   = (lPelX == 0) ? 1 : 0;
                firstY = (tPelY == 0) ? 1 : 0;
                endX   = (rPelX == picWidthTmp) ? lcuWidth - 1 : lcuWidth;
                endY   = (bPelY == picHeightTmp) ? lcuHeight - 1 : lcuHeight;
                if (firstY == 1)
                {
                    fenc += stride;
                    pRec += stride;
                }

                for (x = firstX - 1; x < endX; x++)
                {
                    h->m_upBuff1[x] = xSign(pRec[x] - pRec[x - stride + 1]);
                }

                for (y = firstY; y < endY; y++)
                {
                    for (x = firstX; x < endX; x++)
                    {
                        signDown1      =  xSign(pRec[x] - pRec[x + stride - 1]);
                        uiEdgeType     =  signDown1 + h->m_upBuff1[x] + 2;
                        h->m_upBuff1[x - 1] = -signDown1;

                        if (x < startX && y < startY)
                            continue;

                        stats[g_eoTable[uiEdgeType]] += (fenc[x] - pRec[x]);
                        count[g_eoTable[uiEdgeType]]++;
                    }

                    h->m_upBuff1[endX - 1] = xSign(pRec[endX - 1 + stride] - pRec[endX]);

                    pRec += stride;
                    fenc += stride;
                }
            }
        }
    }
}

void resetSaoUnit(SaoLcuParam* saoUnit)
{
    saoUnit->mergeLeftFlag = 0;
    saoUnit->mergeUpFlag   = 0;
    saoUnit->typeIdx       = -1;
    saoUnit->length        = 0;
    saoUnit->subTypeIdx    = 0;

    for (int i = 0; i < 4; i++)
    {
        saoUnit->offset[i] = 0;
    }
}

void copySaoUnit(SaoLcuParam* saoUnitDst, SaoLcuParam* saoUnitSrc)
{
    saoUnitDst->mergeLeftFlag = saoUnitSrc->mergeLeftFlag;
    saoUnitDst->mergeUpFlag   = saoUnitSrc->mergeUpFlag;
    saoUnitDst->typeIdx       = saoUnitSrc->typeIdx;
    saoUnitDst->length        = saoUnitSrc->length;

    saoUnitDst->subTypeIdx  = saoUnitSrc->subTypeIdx;
    for (int i = 0; i < 4; i++)
    {
        saoUnitDst->offset[i] = saoUnitSrc->offset[i];
    }
}

inline Int64 estSaoDist(Int64 count, Int64 offset, Int64 offsetOrg, int shift)
{
	return ((count * offset * offset - offsetOrg * offset * 2) >> shift);
}

inline Int64 estIterOffset(int typeIdx, int classIdx, double lambda, Int64 offsetInput, Int64 count, Int64 offsetOrg, int shift, int bitIncrease, int *currentDistortionTableBo, double *currentRdCostTableBo, int offsetTh)
{
    //Clean up, best_q_offset.
    Int64 iterOffset, tempOffset;
    Int64 tempDist, tempRate;
    double tempCost, tempMinCost;
    Int64 offsetOutput = 0;

    iterOffset = offsetInput;
    // Assuming sending quantized value 0 results in zero offset and sending the value zero needs 1 bit. entropy coder can be used to measure the exact rate here.
    tempMinCost = lambda;
    while (iterOffset != 0)
    {
        // Calculate the bits required for signalling the offset
        tempRate = (typeIdx == SAO_BO) ? (abs((int)iterOffset) + 2) : (abs((int)iterOffset) + 1);
        if (abs((int)iterOffset) == offsetTh - 1)
        {
            tempRate--;
        }
        // Do the dequntization before distorion calculation
        tempOffset  = iterOffset << bitIncrease;
        tempDist    = estSaoDist(count, tempOffset, offsetOrg, shift);
        tempCost    = ((double)tempDist + lambda * (double)tempRate);
        if (tempCost < tempMinCost)
        {
            tempMinCost = tempCost;
            offsetOutput = iterOffset;
            if (typeIdx == SAO_BO)
            {
                currentDistortionTableBo[classIdx - 1] = (int)tempDist;
                currentRdCostTableBo[classIdx - 1] = tempCost;
            }
        }
        iterOffset = (iterOffset > 0) ? (iterOffset - 1) : (iterOffset + 1);
    }

    return offsetOutput;
}

inline Int64 estSaoTypeDist(X265_t *h, int compIdx, int typeIdx, int shift, double lambda, int *currentDistortionTableBo, double *currentRdCostTableBo)
{
    Int64 estDist = 0;
    int classIdx;
    int saoBitIncrease = 0;
    int saoOffsetTh = (1 << (8-5));//(compIdx == 0) ? m_offsetThY : m_offsetThC;

    for (classIdx = 1; classIdx < ((typeIdx < SAO_BO) ?  g_numClass[typeIdx] + 1 : SAO_MAX_BO_CLASSES + 1); classIdx++)
    {
        if (typeIdx == SAO_BO)
        {
            currentDistortionTableBo[classIdx - 1] = 0;
            currentRdCostTableBo[classIdx - 1] = lambda;
        }
        if (h->m_count[compIdx][typeIdx][classIdx])
        {
            h->m_offset[compIdx][typeIdx][classIdx] = (Int64)xRoundIbdi((double)(h->m_offsetOrg[compIdx][typeIdx][classIdx] << (8 - 8)) / (double)(h->m_count[compIdx][typeIdx][classIdx] << saoBitIncrease));
            h->m_offset[compIdx][typeIdx][classIdx] = Clip3(-saoOffsetTh + 1, saoOffsetTh - 1, (int)h->m_offset[compIdx][typeIdx][classIdx]);
            if (typeIdx < 4)
            {
                if (h->m_offset[compIdx][typeIdx][classIdx] < 0 && classIdx < 3)
                {
                    h->m_offset[compIdx][typeIdx][classIdx] = 0;
                }
                if (h->m_offset[compIdx][typeIdx][classIdx] > 0 && classIdx >= 3)
                {
                    h->m_offset[compIdx][typeIdx][classIdx] = 0;
                }
            }
            h->m_offset[compIdx][typeIdx][classIdx] = estIterOffset(typeIdx, classIdx, lambda, h->m_offset[compIdx][typeIdx][classIdx], h->m_count[compIdx][typeIdx][classIdx], h->m_offsetOrg[compIdx][typeIdx][classIdx], shift, saoBitIncrease, currentDistortionTableBo, currentRdCostTableBo, saoOffsetTh);
        }
        else
        {
            h->m_offsetOrg[compIdx][typeIdx][classIdx] = 0;
            h->m_offset[compIdx][typeIdx][classIdx] = 0;
        }
        if (typeIdx != SAO_BO)
        {
            estDist += estSaoDist(h->m_count[compIdx][typeIdx][classIdx], h->m_offset[compIdx][typeIdx][classIdx] << saoBitIncrease, h->m_offsetOrg[compIdx][typeIdx][classIdx], shift);
        }
    }

    return estDist;
}

void encodeSaoOffset(UInt32 **pCmd, SaoLcuParam* saoLcuParam, UInt compIdx);

void saoComponentParamDist(	X265_t *h, int allowMergeLeft, int allowMergeUp, SAOParam *saoParam, int addr, int addrUp, int addrLeft, int yCbCr, double lambda, SaoLcuParam *compSaoParam, double *compDistortion,
							UInt32* pCmd, xCabac* pcCabac_TempBest, xRdoParam *rdoParam)
{
    int typeIdx;

    Int64 estDist;
    int classIdx;
    int shift = 2 * 0;
    Int64 bestDist;

    SaoLcuParam*  saoLcuParam = &(saoParam->saoLcuParam[yCbCr][addr]);
    SaoLcuParam*  saoLcuParamNeighbor = NULL;

    resetSaoUnit(saoLcuParam);
    resetSaoUnit(&compSaoParam[0]);
    resetSaoUnit(&compSaoParam[1]);

    double dCostPartBest = MAX_DOUBLE;

    double  bestRDCostTableBo = MAX_DOUBLE;
    int     bestClassTableBo    = 0;
    int     currentDistortionTableBo[MAX_NUM_SAO_CLASS];
    double  currentRdCostTableBo[MAX_NUM_SAO_CLASS];

    SaoLcuParam   saoLcuParamRdo;
    double   estRate = 0;

    resetSaoUnit(&saoLcuParamRdo);

    dCostPartBest = 0;

		memcpy(&h->cabac, pcCabac_TempBest, sizeof(xCabac)); // 
		UInt32 *pCmd0 = pCmd;
		encodeSaoOffset( &pCmd, &saoLcuParamRdo, yCbCr);
		dCostPartBest = xGetWrittenCmdRate( pCmd0, pCmd - pCmd0, rdoParam) * lambda;
	
		copySaoUnit(saoLcuParam, &saoLcuParamRdo);
    bestDist = 0;

    for (typeIdx = 0; typeIdx < MAX_NUM_SAO_TYPE; typeIdx++) {
        estDist = estSaoTypeDist(h, yCbCr, typeIdx, shift, lambda, currentDistortionTableBo, currentRdCostTableBo);

        if (typeIdx == SAO_BO) {
            // Estimate Best Position
            double currentRDCost = 0.0;

            for (int i = 0; i < SAO_MAX_BO_CLASSES - SAO_BO_LEN + 1; i++) {
                currentRDCost = 0.0;
                for (Int uj = i; uj < i + SAO_BO_LEN; uj++) {//x64 modify, convert to Int
                    currentRDCost += currentRdCostTableBo[uj];
                }

                if (currentRDCost < bestRDCostTableBo) {
                    bestRDCostTableBo = currentRDCost;
                    bestClassTableBo  = i;
                }
            }

            // Re code all Offsets
            // Code Center
            estDist = 0;
            for (classIdx = bestClassTableBo; classIdx < bestClassTableBo + SAO_BO_LEN; classIdx++) {
                estDist += currentDistortionTableBo[classIdx];
            }
        }
        resetSaoUnit(&saoLcuParamRdo);
        saoLcuParamRdo.length = g_numClass[typeIdx];
        saoLcuParamRdo.typeIdx = typeIdx;
        saoLcuParamRdo.mergeLeftFlag = 0;
        saoLcuParamRdo.mergeUpFlag   = 0;
        saoLcuParamRdo.subTypeIdx = (typeIdx == SAO_BO) ? bestClassTableBo : 0;
        for (classIdx = 0; classIdx < saoLcuParamRdo.length; classIdx++) {
            saoLcuParamRdo.offset[classIdx] = (int)h->m_offset[yCbCr][typeIdx][classIdx + saoLcuParamRdo.subTypeIdx + 1];
        }

        estRate = 0;

				memcpy(&h->cabac, pcCabac_TempBest, sizeof(xCabac)); // 
				pCmd0 = pCmd;
				encodeSaoOffset( &pCmd, &saoLcuParamRdo, yCbCr);
				estRate = xGetWrittenCmdRate( pCmd0, pCmd - pCmd0, rdoParam);
		
				h->m_cost[yCbCr][typeIdx] = (double)((double)estDist + lambda * (double)estRate);

        if (h->m_cost[yCbCr][typeIdx] < dCostPartBest) {
            dCostPartBest = h->m_cost[yCbCr][typeIdx];
            copySaoUnit(saoLcuParam, &saoLcuParamRdo);
            bestDist = estDist;
        }
    }

    compDistortion[0] += ((double)bestDist / lambda);
		memcpy(&h->cabac, pcCabac_TempBest, sizeof(xCabac)); // 
		pCmd0 = pCmd;
		encodeSaoOffset( &pCmd, &saoLcuParamRdo, yCbCr);
		memcpy(pcCabac_TempBest, &h->cabac, sizeof(xCabac)); // 

    // merge left or merge up

    for (int idxNeighbor = 0; idxNeighbor < 2; idxNeighbor++) {
        saoLcuParamNeighbor = NULL;
        if (allowMergeLeft && addrLeft >= 0 && idxNeighbor == 0) {
            saoLcuParamNeighbor = &(saoParam->saoLcuParam[yCbCr][addrLeft]);
        }
        else if (allowMergeUp && addrUp >= 0 && idxNeighbor == 1) {
            saoLcuParamNeighbor = &(saoParam->saoLcuParam[yCbCr][addrUp]);
        }
        if (saoLcuParamNeighbor != NULL) {
            estDist = 0;
            typeIdx = saoLcuParamNeighbor->typeIdx;
            if (typeIdx >= 0) {
                int mergeBandPosition = (typeIdx == SAO_BO) ? saoLcuParamNeighbor->subTypeIdx : 0;
                int   merge_iOffset;
                for (classIdx = 0; classIdx < g_numClass[typeIdx]; classIdx++) {
                    merge_iOffset = saoLcuParamNeighbor->offset[classIdx];
                    estDist   += estSaoDist(h->m_count[yCbCr][typeIdx][classIdx + mergeBandPosition + 1], merge_iOffset, h->m_offsetOrg[yCbCr][typeIdx][classIdx + mergeBandPosition + 1],  shift);
                }
            }
            else {
                estDist = 0;
            }

            copySaoUnit(&compSaoParam[idxNeighbor], saoLcuParamNeighbor);
            compSaoParam[idxNeighbor].mergeUpFlag   = idxNeighbor;
            compSaoParam[idxNeighbor].mergeLeftFlag = !idxNeighbor;

            compDistortion[idxNeighbor + 1] += ((double)estDist / lambda);
        }
    }
}

void sao2ChromaParamDist(	X265_t *h, int allowMergeLeft, int allowMergeUp, SAOParam *saoParam, int addr, int addrUp, int addrLeft, double lambda, SaoLcuParam *crSaoParam, SaoLcuParam *cbSaoParam, double *distortion,
							UInt32* pCmd, xCabac* pcCabac_TempBest, xRdoParam *rdoParam)
{
    int typeIdx;

    Int64 estDist[2];
    int classIdx;
    int shift = 2 * 0;
    Int64 bestDist = 0;

    SaoLcuParam*  saoLcuParam[2] = { &(saoParam->saoLcuParam[1][addr]), &(saoParam->saoLcuParam[2][addr]) };
    SaoLcuParam*  saoLcuParamNeighbor[2] = { NULL, NULL };
    SaoLcuParam*  saoMergeParam[2][2];

    saoMergeParam[0][0] = &crSaoParam[0];
    saoMergeParam[0][1] = &crSaoParam[1];
    saoMergeParam[1][0] = &cbSaoParam[0];
    saoMergeParam[1][1] = &cbSaoParam[1];

    resetSaoUnit(saoLcuParam[0]);
    resetSaoUnit(saoLcuParam[1]);
    resetSaoUnit(saoMergeParam[0][0]);
    resetSaoUnit(saoMergeParam[0][1]);
    resetSaoUnit(saoMergeParam[1][0]);
    resetSaoUnit(saoMergeParam[1][1]);

    double costPartBest = MAX_DOUBLE;

    double  bestRDCostTableBo;
    int     bestClassTableBo[2]    = { 0, 0 };
    int     currentDistortionTableBo[MAX_NUM_SAO_CLASS];
    double  currentRdCostTableBo[MAX_NUM_SAO_CLASS];

    SaoLcuParam   saoLcuParamRdo[2];
    double   estRate = 0;

    resetSaoUnit(&saoLcuParamRdo[0]);
    resetSaoUnit(&saoLcuParamRdo[1]);

    costPartBest = 0;

		memcpy(&h->cabac, pcCabac_TempBest, sizeof(xCabac)); // 
		UInt32 *pCmd0 = pCmd;
		encodeSaoOffset( &pCmd, &saoLcuParamRdo[0], 1);
		encodeSaoOffset( &pCmd, &saoLcuParamRdo[1], 2);
		costPartBest = xGetWrittenCmdRate( pCmd0, pCmd - pCmd0, rdoParam) * lambda;
	
	copySaoUnit(saoLcuParam[0], &saoLcuParamRdo[0]);
    copySaoUnit(saoLcuParam[1], &saoLcuParamRdo[1]);

    for (typeIdx = 0; typeIdx < MAX_NUM_SAO_TYPE; typeIdx++)
    {
        if (typeIdx == SAO_BO)
        {
            // Estimate Best Position
            for (int compIdx = 0; compIdx < 2; compIdx++)
            {
                double currentRDCost = 0.0;
                bestRDCostTableBo = MAX_DOUBLE;
                estDist[compIdx] = estSaoTypeDist(h, compIdx + 1, typeIdx, shift, lambda, currentDistortionTableBo, currentRdCostTableBo);
                for (int i = 0; i < SAO_MAX_BO_CLASSES - SAO_BO_LEN + 1; i++)
                {
                    currentRDCost = 0.0;
                    for (Int uj = i; uj < i + SAO_BO_LEN; uj++)//x64 modify, convert to Int
                    {
                        currentRDCost += currentRdCostTableBo[uj];
                    }

                    if (currentRDCost < bestRDCostTableBo)
                    {
                        bestRDCostTableBo = currentRDCost;
                        bestClassTableBo[compIdx]  = i;
                    }
                }

                // Re code all Offsets
                // Code Center
                estDist[compIdx] = 0;
                for (classIdx = bestClassTableBo[compIdx]; classIdx < bestClassTableBo[compIdx] + SAO_BO_LEN; classIdx++)
                {
                    estDist[compIdx] += currentDistortionTableBo[classIdx];
                }
            }
        }
        else
        {
            estDist[0] = estSaoTypeDist(h, 1, typeIdx, shift, lambda, currentDistortionTableBo, currentRdCostTableBo);
            estDist[1] = estSaoTypeDist(h, 2, typeIdx, shift, lambda, currentDistortionTableBo, currentRdCostTableBo);
        }

				memcpy(&h->cabac, pcCabac_TempBest, sizeof(xCabac)); // 
				UInt32 *pCmd0 = pCmd;

        for (int compIdx = 0; compIdx < 2; compIdx++)
        {
            resetSaoUnit(&saoLcuParamRdo[compIdx]);
            saoLcuParamRdo[compIdx].length = g_numClass[typeIdx];
            saoLcuParamRdo[compIdx].typeIdx = typeIdx;
            saoLcuParamRdo[compIdx].mergeLeftFlag = 0;
            saoLcuParamRdo[compIdx].mergeUpFlag   = 0;
            saoLcuParamRdo[compIdx].subTypeIdx = (typeIdx == SAO_BO) ? bestClassTableBo[compIdx] : 0;
            for (classIdx = 0; classIdx < saoLcuParamRdo[compIdx].length; classIdx++)
            {
                saoLcuParamRdo[compIdx].offset[classIdx] = (int)h->m_offset[compIdx + 1][typeIdx][classIdx + saoLcuParamRdo[compIdx].subTypeIdx + 1];
            }

						encodeSaoOffset( &pCmd, &saoLcuParamRdo[compIdx], compIdx + 1);
        }
				estRate = xGetWrittenCmdRate( pCmd0, pCmd - pCmd0, rdoParam);

				h->m_cost[1][typeIdx] = (double)((double)(estDist[0] + estDist[1])  + lambda * (double)estRate);

        if (h->m_cost[1][typeIdx] < costPartBest)
        {
            costPartBest = h->m_cost[1][typeIdx];
            copySaoUnit(saoLcuParam[0], &saoLcuParamRdo[0]);
            copySaoUnit(saoLcuParam[1], &saoLcuParamRdo[1]);
            bestDist = (estDist[0] + estDist[1]);
        }
    }

    distortion[0] += ((double)bestDist / lambda);
		memcpy(&h->cabac, pcCabac_TempBest, sizeof(xCabac)); // 
		pCmd0 = pCmd;
		encodeSaoOffset( &pCmd, saoLcuParam[0], 1);
		encodeSaoOffset( &pCmd, saoLcuParam[1], 2);
		memcpy(pcCabac_TempBest, &h->cabac, sizeof(xCabac)); // 

    // merge left or merge up
    for (int idxNeighbor = 0; idxNeighbor < 2; idxNeighbor++)
    {
        for (int compIdx = 0; compIdx < 2; compIdx++)
        {
            saoLcuParamNeighbor[compIdx] = NULL;
            if (allowMergeLeft && addrLeft >= 0 && idxNeighbor == 0)
            {
                saoLcuParamNeighbor[compIdx] = &(saoParam->saoLcuParam[compIdx + 1][addrLeft]);
            }
            else if (allowMergeUp && addrUp >= 0 && idxNeighbor == 1)
            {
                saoLcuParamNeighbor[compIdx] = &(saoParam->saoLcuParam[compIdx + 1][addrUp]);
            }
            if (saoLcuParamNeighbor[compIdx] != NULL)
            {
                estDist[compIdx] = 0;
                typeIdx = saoLcuParamNeighbor[compIdx]->typeIdx;
                if (typeIdx >= 0)
                {
                    int mergeBandPosition = (typeIdx == SAO_BO) ? saoLcuParamNeighbor[compIdx]->subTypeIdx : 0;
                    int   merge_iOffset;
                    for (classIdx = 0; classIdx < g_numClass[typeIdx]; classIdx++)
                    {
                        merge_iOffset = saoLcuParamNeighbor[compIdx]->offset[classIdx];
                        estDist[compIdx]   += estSaoDist(h->m_count[compIdx + 1][typeIdx][classIdx + mergeBandPosition + 1], merge_iOffset, h->m_offsetOrg[compIdx + 1][typeIdx][classIdx + mergeBandPosition + 1],  shift);
                    }
                }
                else
                {
                    estDist[compIdx] = 0;
                }

                copySaoUnit(saoMergeParam[compIdx][idxNeighbor], saoLcuParamNeighbor[compIdx]);
                saoMergeParam[compIdx][idxNeighbor]->mergeUpFlag   = idxNeighbor;
                saoMergeParam[compIdx][idxNeighbor]->mergeLeftFlag = !idxNeighbor;
                distortion[idxNeighbor + 1] += ((double)estDist[compIdx] / lambda);
            }
        }
    }
}

void encodeSaoOffset(UInt32 **pCmd, SaoLcuParam* saoLcuParam, UInt compIdx);

void rdoSaoUnitAll(X265_t *h)
{
		CInt	nQP  = h->iQP;
		CInt  nQPC = xg_aucChromaScale[nQP];
 		
		//FIX ME: This initialization for pcCabac_TempBest and CurrBest maybe incorrect, YananZhao, 2013-11-15
		Int nIdxCabacRdo = h->PicHeightInCUs - 1;
		xCabac *pcCabac_TempBest = &h->cabacRdo[nIdxCabacRdo][MAX_CU_DEPTH-1 ];    //Immediately Cabac buff 
		xCabac *pcCabac_CurrBest = &h->cabacRdo[nIdxCabacRdo][MAX_CU_DEPTH ];    //Immediately Cabac buff 
	
		double lumaLambda = 0.578 * pow(2.0, (nQP - 12) / 3.0);// from HM
		if ( h->eSliceType == SLICE_I){
			lumaLambda *= 0.3;           //Is it right --fixME --wxw
		}
		double chromaLambda = lumaLambda * pow(2.0, (nQPC - nQP) / 3.0) ;//注意：这样计算对吗？
		memcpy(pcCabac_TempBest, &h->cabac, sizeof(xCabac)); //  
		
		SAOParam *saoParam = &h->sao_param;
		int idxX, idxY;
		int frameWidthInCU = h->PicWidthInCUs;
		int frameHeightInCU = h->PicHeightInCUs;
		int j, k;
		int addr = 0;
		int addrUp = -1;
		int addrLeft = -1;
		int compIdx = 0;
		SaoLcuParam mergeSaoParam[3][2];
		double compDistortion[3];

    for (idxY = 0; idxY < frameHeightInCU; idxY++) {
        for (idxX = 0; idxX < frameWidthInCU; idxX++) {

						UInt32 *pCmd0 = h->auiSaoCmd[idxY][idxX]; 
						UInt32 *pCmd  = pCmd0;

						addr     = idxX  + frameWidthInCU * idxY;
            addrUp   = addr < frameWidthInCU ? -1 : idxX   + frameWidthInCU * (idxY - 1);
            addrLeft = idxX == 0             ? -1 : idxX - 1 + frameWidthInCU * idxY;
            int allowMergeLeft = 1;
            int allowMergeUp   = 1;
            UInt rate;
            double bestCost, mergeCost;
            if (idxX == 0) {
                allowMergeLeft = 0;
            }
            if (idxY == 0) {
                allowMergeUp = 0;
            }

            compDistortion[0] = 0;
            compDistortion[1] = 0;
            compDistortion[2] = 0;
            // reset stats Y, Cb, Cr
						
						memcpy(&h->cabac, pcCabac_CurrBest, sizeof(xCabac)); // 
						if ( allowMergeLeft)
							xEncodeBin_pCmd( &pCmd, 0, OFF_SAO_MERGE_FLAG_CTX );//  g_pCabac
						if ( allowMergeUp)
							xEncodeBin_pCmd( &pCmd, 0, OFF_SAO_MERGE_FLAG_CTX );
						memcpy(pcCabac_TempBest, &h->cabac, sizeof(xCabac)); //CABAC ctx store 

            for (compIdx = 0; compIdx < 3; compIdx++) {
                for (j = 0; j < MAX_NUM_SAO_TYPE; j++) {
                    for (k = 0; k < MAX_NUM_SAO_CLASS; k++) {
                        h->m_offset[compIdx][j][k] = 0;
#if 0
                        h->m_count[compIdx][j][k] = h->m_countPreDblk[addr][compIdx][j][k];
                        h->m_offsetOrg[compIdx][j][k] = h->m_offsetOrgPreDblk[addr][compIdx][j][k];
#else
                        h->m_count[compIdx][j][k] = 0;
                        h->m_offsetOrg[compIdx][j][k] = 0;
#endif
                    }
                }

                saoParam->saoLcuParam[compIdx][addr].typeIdx       =  -1;
                saoParam->saoLcuParam[compIdx][addr].mergeUpFlag   = 0;
                saoParam->saoLcuParam[compIdx][addr].mergeLeftFlag = 0;
                saoParam->saoLcuParam[compIdx][addr].subTypeIdx    = 0;
                if ((compIdx == 0 && saoParam->bSaoFlag[0]) || (compIdx > 0 && saoParam->bSaoFlag[1])) {
                    calcSaoStatsCu(h, idxX, idxY, compIdx,  compIdx);
                }
            }

            saoComponentParamDist(	h, 
									allowMergeLeft, 
									allowMergeUp, 
									saoParam, 
									addr, 
									addrUp, 
									addrLeft, 
									0,  
									lumaLambda, 
									&mergeSaoParam[0][0], 
									&compDistortion[0],
									pCmd,
									pcCabac_TempBest,
									&(h->rdoParam[nIdxCabacRdo]));

            sao2ChromaParamDist(	h, 
									allowMergeLeft, 
									allowMergeUp, 
									saoParam, 
									addr, 
									addrUp, 
									addrLeft, 
									chromaLambda, 
									&mergeSaoParam[1][0], 
									&mergeSaoParam[2][0], 
									&compDistortion[0],
									pCmd,
									pcCabac_TempBest,
									&(h->rdoParam[nIdxCabacRdo]));

            if (saoParam->bSaoFlag[0] || saoParam->bSaoFlag[1]) {
                // Cost of new SAO_params
								rate = 0;
								memcpy(&h->cabac, pcCabac_CurrBest, sizeof(xCabac)); // 
								pCmd0 = pCmd;
								if ( allowMergeLeft)
									xEncodeBin_pCmd( &pCmd, 0, OFF_SAO_MERGE_FLAG_CTX);
								if ( allowMergeUp)
									xEncodeBin_pCmd( &pCmd, 0, OFF_SAO_MERGE_FLAG_CTX );
								for (compIdx = 0; compIdx < 3; compIdx++)	{
									if ((compIdx == 0 && saoParam->bSaoFlag[0]) || (compIdx > 0 && saoParam->bSaoFlag[1])) {
										encodeSaoOffset( &pCmd, &saoParam->saoLcuParam[compIdx][addr], compIdx );
									}
								}

								rate = xGetWrittenCmdRate( pCmd0, pCmd - pCmd0, &(h->rdoParam[nIdxCabacRdo]));
                bestCost = (compDistortion[0]) + (double)rate;
								memcpy(pcCabac_TempBest, &h->cabac,  sizeof(xCabac));
      
								// Cost of Merge
                for (int mergeUp = 0; mergeUp < 2; ++mergeUp) {
                    if ((allowMergeLeft && (mergeUp == 0)) || (allowMergeUp && (mergeUp == 1))) {
                        rate = 0;//m_entropyCoder->getNumberOfWrittenBits();
												memcpy(&h->cabac, pcCabac_CurrBest,  sizeof(xCabac)); //  
												pCmd0 = pCmd;
												if ( allowMergeLeft){
													xEncodeBin_pCmd( &pCmd, 1 - mergeUp, OFF_SAO_MERGE_FLAG_CTX);
												}
												if (allowMergeUp && (mergeUp == 1)){
													xEncodeBin_pCmd( &pCmd, 1, OFF_SAO_MERGE_FLAG_CTX );
												}
												rate = xGetWrittenCmdRate( pCmd0, pCmd - pCmd0, &(h->rdoParam[nIdxCabacRdo]));

												mergeCost = (compDistortion[mergeUp + 1]) + (double)rate;
												if (mergeCost < bestCost) {
													bestCost = mergeCost;
													memcpy(pcCabac_TempBest, &h->cabac,  sizeof(xCabac)); // 

													for (compIdx = 0; compIdx < 3; compIdx++) {
															mergeSaoParam[compIdx][mergeUp].mergeLeftFlag = 1 - mergeUp;
															mergeSaoParam[compIdx][mergeUp].mergeUpFlag = mergeUp;
															if ((compIdx == 0 && saoParam->bSaoFlag[0]) || (compIdx > 0 && saoParam->bSaoFlag[1])) {
																	copySaoUnit(&saoParam->saoLcuParam[compIdx][addr], &mergeSaoParam[compIdx][mergeUp]);
															}
													}
												}
                    }
                }

                if (saoParam->saoLcuParam[0][addr].typeIdx == -1) {
                    h->numNoSao[0]++;
                }
                if (saoParam->saoLcuParam[1][addr].typeIdx == -1) {
                    h->numNoSao[1] += 2;
                }
								memcpy(&h->cabac, pcCabac_TempBest,  sizeof(xCabac));
								memcpy(pcCabac_CurrBest, &h->cabac,  sizeof(xCabac)); 
					}
        }
    }
}

void processSaoCu(X265_t *h, int idxX, int idxY, int saoType, int yCbCr)
{
    int x, y;
    UInt8* rec;
    int  stride;
    int  lcuWidth  = h->ucMaxCUWidth;
    int  lcuHeight = h->ucMaxCUWidth;
    Int lpelx     = idxX * h->ucMaxCUWidth;//x64 modify, convert to Int
    Int tpely     = idxY * h->ucMaxCUWidth;//x64 modify, convert to Int
    Int rpelx;//x64 modify, convert to Int
    Int bpely;//x64 modify, convert to Int
    int  signLeft;
    int  signRight;
    int  signDown;
    int  signDown1;
    int  signDown2;
    UInt edgeType;
    int picWidthTmp;
    int picHeightTmp;
    int startX;
    int startY;
    int endX;
    int endY;
    int isChroma = (yCbCr != 0) ? 1 : 0;
    int shift;
    int cuHeightTmp;
    UInt8 *tmpLSwap;
    UInt8 *tmpL;
    UInt8 *tmpU;
    UInt8 *clipTbl = NULL;
    int *offsetBo = NULL;
    int *tmp_swap;

    picWidthTmp  = h->iWidth  >> isChroma;
    picHeightTmp = h->iHeight >> isChroma;
    lcuWidth     = lcuWidth    >> isChroma;
    lcuHeight    = lcuHeight   >> isChroma;
    lpelx        = lpelx      >> isChroma;
    tpely        = tpely      >> isChroma;
    rpelx        = lpelx + lcuWidth;
    bpely        = tpely + lcuHeight;
    rpelx        = rpelx > picWidthTmp  ? picWidthTmp  : rpelx;
    bpely        = bpely > picHeightTmp ? picHeightTmp : bpely;
    lcuWidth     = rpelx - lpelx;
    lcuHeight    = bpely - tpely;

    if (yCbCr == 0)
    {
        rec  = h->pFrameRec->pucY + idxY * h->ucMaxCUWidth * h->pFrameRec->iStrideY + idxX * h->ucMaxCUWidth;
        stride = h->pFrameRec->iStrideY;
    }
    else if (yCbCr == 1)
    {
        rec  = h->pFrameRec->pucU + idxY * h->ucMaxCUWidth / 2 * h->pFrameRec->iStrideC + idxX * h->ucMaxCUWidth / 2;
        stride = h->pFrameRec->iStrideC;
    }
    else
    {
        rec  = h->pFrameRec->pucV + idxY * h->ucMaxCUWidth / 2 * h->pFrameRec->iStrideC + idxX * h->ucMaxCUWidth / 2;
        stride = h->pFrameRec->iStrideC;
    }

//   if (iSaoType!=SAO_BO_0 || iSaoType!=SAO_BO_1)
    {
        cuHeightTmp = (h->ucMaxCUWidth >> isChroma);
        shift = (h->ucMaxCUWidth >> isChroma) - 1;
        for (int i = 0; i < cuHeightTmp + 1; i++)
        {
            h->m_tmpL2[i] = rec[shift];
            rec += stride;
        }

        rec -= (stride * (cuHeightTmp + 1));

        tmpL = h->m_tmpL1;
        tmpU = &(h->m_tmpU1[yCbCr][lpelx]);
    }

    clipTbl = (yCbCr == 0) ? h->g_clipTable : h->g_chromaClipTable;
    offsetBo = (yCbCr == 0) ? h->m_offsetBo : h->m_chromaOffsetBo;

    switch (saoType)
    {
    case SAO_EO_0: // dir: -
    {
        startX = (lpelx == 0) ? 1 : 0;
        endX   = (rpelx == picWidthTmp) ? lcuWidth - 1 : lcuWidth;
        for (y = 0; y < lcuHeight; y++)
        {
            signLeft = xSign(rec[startX] - tmpL[y]);
            for (x = startX; x < endX; x++)
            {
                signRight =  xSign(rec[x] - rec[x + 1]);
                edgeType =  signRight + signLeft + 2;
                signLeft  = -signRight;

                rec[x] = clipTbl[rec[x] + h->m_offsetEo[edgeType]];
            }

            rec += stride;
        }

        break;
    }
    case SAO_EO_1: // dir: |
    {
        startY = (tpely == 0) ? 1 : 0;
        endY   = (bpely == picHeightTmp) ? lcuHeight - 1 : lcuHeight;
        if (tpely == 0)
        {
            rec += stride;
        }
        for (x = 0; x < lcuWidth; x++)
        {
            h->m_upBuff1[x] = xSign(rec[x] - tmpU[x]);
        }

        for (y = startY; y < endY; y++)
        {
            for (x = 0; x < lcuWidth; x++)
            {
                signDown  = xSign(rec[x] - rec[x + stride]);
                edgeType = signDown + h->m_upBuff1[x] + 2;
                h->m_upBuff1[x] = -signDown;

                rec[x] = clipTbl[rec[x] + h->m_offsetEo[edgeType]];
            }

            rec += stride;
        }

        break;
    }
    case SAO_EO_2: // dir: 135
    {
        startX = (lpelx == 0)            ? 1 : 0;
        endX   = (rpelx == picWidthTmp) ? lcuWidth - 1 : lcuWidth;

        startY = (tpely == 0) ?             1 : 0;
        endY   = (bpely == picHeightTmp) ? lcuHeight - 1 : lcuHeight;

        if (tpely == 0)
        {
            rec += stride;
        }

        for (x = startX; x < endX; x++)
        {
            h->m_upBuff1[x] = xSign(rec[x] - tmpU[x - 1]);
        }

        for (y = startY; y < endY; y++)
        {
            signDown2 = xSign(rec[stride + startX] - tmpL[y]);
            for (x = startX; x < endX; x++)
            {
                signDown1      =  xSign(rec[x] - rec[x + stride + 1]);
                edgeType      =  signDown1 + h->m_upBuff1[x] + 2;
                h->m_upBufft[x + 1] = -signDown1;
                rec[x] = clipTbl[rec[x] + h->m_offsetEo[edgeType]];
            }

            h->m_upBufft[startX] = signDown2;

            tmp_swap     = h->m_upBuff1;
            h->m_upBuff1 = h->m_upBufft;
            h->m_upBufft = tmp_swap;

            rec += stride;
        }

        break;
    }
    case SAO_EO_3: // dir: 45
    {
        startX = (lpelx == 0) ? 1 : 0;
        endX   = (rpelx == picWidthTmp) ? lcuWidth - 1 : lcuWidth;

        startY = (tpely == 0) ? 1 : 0;
        endY   = (bpely == picHeightTmp) ? lcuHeight - 1 : lcuHeight;

        if (startY == 1)
        {
            rec += stride;
        }

        for (x = startX - 1; x < endX; x++)
        {
            h->m_upBuff1[x] = xSign(rec[x] - tmpU[x + 1]);
        }

        for (y = startY; y < endY; y++)
        {
            x = startX;
            signDown1      =  xSign(rec[x] - tmpL[y + 1]);
            edgeType      =  signDown1 + h->m_upBuff1[x] + 2;
            h->m_upBuff1[x - 1] = -signDown1;
            rec[x] = clipTbl[rec[x] + h->m_offsetEo[edgeType]];
            for (x = startX + 1; x < endX; x++)
            {
                signDown1      =  xSign(rec[x] - rec[x + stride - 1]);
                edgeType      =  signDown1 + h->m_upBuff1[x] + 2;
                h->m_upBuff1[x - 1] = -signDown1;
                rec[x] = clipTbl[rec[x] + h->m_offsetEo[edgeType]];
            }

            h->m_upBuff1[endX - 1] = xSign(rec[endX - 1 + stride] - rec[endX]);

            rec += stride;
        }

        break;
    }
    case SAO_BO:
    {
        for (y = 0; y < lcuHeight; y++)
        {
            for (x = 0; x < lcuWidth; x++)
            {
                rec[x] = offsetBo[rec[x]];
            }

            rec += stride;
        }

        break;
    }
    default: break;
    }

//   if (iSaoType!=SAO_BO_0 || iSaoType!=SAO_BO_1)
    {
        tmpLSwap    = h->m_tmpL1;
        h->m_tmpL1  = h->m_tmpL2;
        h->m_tmpL2  = tmpLSwap;
    }
}

void processSaoUnitAll(X265_t *h, SaoLcuParam* saoLcuParam, int yCbCr)
{
    UInt8 *rec;
    int picWidthTmp;

    if (yCbCr == 0)
    {
        rec         = h->pFrameRec->pucY;
        picWidthTmp = h->iWidth;
    }
    else if (yCbCr == 1)
    {
        rec         = h->pFrameRec->pucU;
        picWidthTmp = h->iWidth >> 1;
    }
    else
    {
        rec         = h->pFrameRec->pucV;
        picWidthTmp = h->iWidth >> 1;
    }

//     if (idxY == 0)
        memcpy(h->m_tmpU1[yCbCr], rec, sizeof(UInt8) * picWidthTmp);

    int  i;
    UInt edgeType;
    UInt8* lumaTable = NULL;
    UInt8* clipTable = NULL;
    int* offsetBo = NULL;
    int  typeIdx;

    int offset[LUMA_GROUP_NUM + 1];
    int idxX, idxY;
    int addr;
	int frameWidthInCU = h->PicWidthInCUs;
	int frameHeightInCU = h->PicHeightInCUs;
    int stride;
    UInt8 *tmpUSwap;
    int sChroma = (yCbCr == 0) ? 0 : 1;
    int mergeLeftFlag;
    int saoBitIncrease = 0;//(yCbCr == 0) ? m_saoBitIncreaseY : m_saoBitIncreaseC;

    offsetBo = (yCbCr == 0) ? h->m_offsetBo : h->m_chromaOffsetBo;

    offset[0] = 0;
    for (idxY = 0; idxY < frameHeightInCU; idxY++)
    {
        addr = idxY * frameWidthInCU;
        rec  = getPicYuvAddr(h->pFrameRec, yCbCr, 0, idxY, h->ucMaxCUWidth);
        if (yCbCr == 0)
        {
            stride = h->pFrameRec->iStrideY;
            picWidthTmp = h->iWidth;
        }
        else
        {
            stride = h->pFrameRec->iStrideC;
            picWidthTmp = h->iWidth >> 1;
        }

        //     pRec += stride*(m_uiMaxCUHeight-1);
        for (i = 0; i < (h->ucMaxCUWidth >> sChroma) + 1; i++)
        {
            h->m_tmpL1[i] = rec[0];
            rec += stride;
        }

        rec -= (stride << 1);

        memcpy(h->m_tmpU2[yCbCr], rec, sizeof(UInt8) * picWidthTmp);

        for (idxX = 0; idxX < frameWidthInCU; idxX++)
        {
            addr = idxY * frameWidthInCU + idxX;

            typeIdx = saoLcuParam[addr].typeIdx;
            mergeLeftFlag = saoLcuParam[addr].mergeLeftFlag;

            if (typeIdx >= 0)
            {
                if (!mergeLeftFlag)
                {
                    if (typeIdx == SAO_BO)
                    {
                        for (i = 0; i < SAO_MAX_BO_CLASSES + 1; i++)
                        {
                            offset[i] = 0;
                        }

                        for (i = 0; i < saoLcuParam[addr].length; i++)
                        {
                            offset[(saoLcuParam[addr].subTypeIdx + i) % SAO_MAX_BO_CLASSES  + 1] = saoLcuParam[addr].offset[i] << saoBitIncrease;
                        }

                        lumaTable = (yCbCr == 0) ? h->g_lumaTableBo : h->g_chromaTableBo;
                        clipTable = (yCbCr == 0) ? h->g_clipTable : h->g_chromaClipTable;

                        for (i = 0; i < (1 << 8/*X265_DEPTH*/); i++)
                        {
                            offsetBo[i] = clipTable[i + offset[lumaTable[i]]];
                        }
                    }
                    if (typeIdx == SAO_EO_0 || typeIdx == SAO_EO_1 || typeIdx == SAO_EO_2 || typeIdx == SAO_EO_3)
                    {
                        for (i = 0; i < saoLcuParam[addr].length; i++)
                        {
                            offset[i + 1] = saoLcuParam[addr].offset[i] << saoBitIncrease;
                        }

                        for (edgeType = 0; edgeType < 6; edgeType++)
                        {
                            h->m_offsetEo[edgeType] = offset[g_eoTable[edgeType]];
                        }
                    }
                }
                processSaoCu(h, idxX, idxY, typeIdx, yCbCr);
            }
            else
            {
                if (idxX != (frameWidthInCU - 1))
                {
                    rec  = getPicYuvAddr(h->pFrameRec, yCbCr, idxX, idxY, h->ucMaxCUWidth);
                    if (yCbCr == 0) {
                        stride = h->pFrameRec->iStrideY;
                    }
                    else {
                        stride = h->pFrameRec->iStrideC;
                    }
                    int widthShift = h->ucMaxCUWidth >> sChroma;
                    for (i = 0; i < (h->ucMaxCUWidth >> sChroma) + 1; i++)
                    {
                        h->m_tmpL1[i] = rec[widthShift - 1];
                        rec += stride;
                    }
                }
            }
        }

        tmpUSwap          = h->m_tmpU1[yCbCr];
        h->m_tmpU1[yCbCr] = h->m_tmpU2[yCbCr];
        h->m_tmpU2[yCbCr] = tmpUSwap;
    }
}

static
void codeSaoMerge(UInt32 **pCmd, UInt uiCode)
{
    xEncodeBin_pCmd(pCmd, !!uiCode, OFF_SAO_MERGE_FLAG_CTX);
//     if (uiCode == 0) {
//         m_pcBinIf->encodeBin(0,  m_cSaoMergeSCModel.get(0, 0, 0));
//     }
//     else {
//         m_pcBinIf->encodeBin(1,  m_cSaoMergeSCModel.get(0, 0, 0));
//     }
}

static
void codeSaoTypeIdx(UInt32 **pCmd, UInt uiCode)
{

    xEncodeBin_pCmd(pCmd, !!uiCode, OFF_SAO_TYPE_IDX_CTX);
    if (uiCode) {
        xEncodeBinsEP_pCmd(pCmd, (uiCode <= 4 ? 1 : 0), 1);
    }
//     if (uiCode == 0)
//     {
//         m_pcBinIf->encodeBin(0, m_cSaoTypeIdxSCModel.get(0, 0, 0));
//     }
//     else
//     {
//         m_pcBinIf->encodeBin(1, m_cSaoTypeIdxSCModel.get(0, 0, 0));
//         m_pcBinIf->encodeBinEP(uiCode <= 4 ? 1 : 0);
//     }
}

static
void codeSAOSign(UInt32 **pCmd, UInt uiCode)
{
    xEncodeBinsEP_pCmd(pCmd, uiCode, 1);
//     m_pcBinIf->encodeBinEP(code);
}

static
void codeSaoMaxUvlc(UInt32 **pCmd, UInt code, UInt maxSymbol)
{
    if (maxSymbol == 0)
    {
        return;
    }

    bool bCodeLast = (maxSymbol > code);

    xEncodeBinsEP_pCmd(pCmd, !!code, 1);
    if (code) {
        assert(code < 16);
        UInt32 mask = (1 << (code-1)) - 1;
        xEncodeBinsEP_pCmd(pCmd, mask << bCodeLast, code - 1 + bCodeLast);
    }

//     if (code == 0) {
//         m_pcBinIf->encodeBinEP(0);
//     }
//     else {
//         m_pcBinIf->encodeBinEP(1);
//         for (i = 0; i < code - 1; i++)
//         {
//             m_pcBinIf->encodeBinEP(1);
//         }
// 
//         if (bCodeLast)
//         {
//             m_pcBinIf->encodeBinEP(0);
//         }
//     }
}

static
void codeSaoUflc(UInt32 **pCmd, UInt uiLength, UInt uiCode)
{
    xEncodeBinsEP_pCmd(pCmd, uiCode, uiLength);
//     m_pcBinIf->encodeBinsEP(uiCode, uiLength);
}

void encodeSaoOffset(UInt32 **pCmd, SaoLcuParam* saoLcuParam, UInt compIdx)
{
    UInt symbol;
    int i;

    symbol = saoLcuParam->typeIdx + 1;
    if (compIdx != 2)
    {
        codeSaoTypeIdx(pCmd, symbol);
    }
    if (symbol)
    {
        if (saoLcuParam->typeIdx < 4 && compIdx != 2)
        {
            saoLcuParam->subTypeIdx = saoLcuParam->typeIdx;
        }
        int offsetTh = 1 << (8 - 5);
        if (saoLcuParam->typeIdx == SAO_BO)
        {
            for (i = 0; i < saoLcuParam->length; i++)
            {
                UInt absOffset = ((saoLcuParam->offset[i] < 0) ? -saoLcuParam->offset[i] : saoLcuParam->offset[i]);
                codeSaoMaxUvlc(pCmd, absOffset, offsetTh - 1);
            }

            for (i = 0; i < saoLcuParam->length; i++)
            {
                if (saoLcuParam->offset[i] != 0)
                {
                    UInt sign = (saoLcuParam->offset[i] < 0) ? 1 : 0;
                    codeSAOSign(pCmd, sign);
                }
            }

            symbol = (UInt)(saoLcuParam->subTypeIdx);
            codeSaoUflc(pCmd, 5, symbol);
        }
        else if (saoLcuParam->typeIdx < 4)
        {
            codeSaoMaxUvlc(pCmd, saoLcuParam->offset[0], offsetTh - 1);
            codeSaoMaxUvlc(pCmd, saoLcuParam->offset[1], offsetTh - 1);
            codeSaoMaxUvlc(pCmd, -saoLcuParam->offset[2], offsetTh - 1);
            codeSaoMaxUvlc(pCmd, -saoLcuParam->offset[3], offsetTh - 1);
            if (compIdx != 2)
            {
                symbol = (UInt)(saoLcuParam->subTypeIdx);
                codeSaoUflc(pCmd, 2, symbol);
            }
        }
    }
}

#endif // ~SAO


// ***************************************************************************
// * Internal Functions
// ***************************************************************************
void xFramePadding( xFrame *pFrm, CUInt32 uiWidth, CUInt32 uiHeight, CUInt32 OFF)
{
	CUInt32 uiWidthY    = uiWidth;
	CUInt32 uiHeightY   = uiHeight;
	CUInt32 uiWidthC    = uiWidth/2;
	CUInt32 uiHeightC   = uiHeight/2;
	UInt    nStrideY    = pFrm->iStrideY;
	UInt    nStrideC    = pFrm->iStrideC;
	UInt8  *pucY        = pFrm->pucY;
	UInt8  *pucU        = pFrm->pucU;
	UInt8  *pucV        = pFrm->pucV;
	UInt8  *pucY0, *pucY1;
	UInt8  *pucU0, *pucU1;
	UInt8  *pucV0, *pucV1;
	int i;

	// Padding Left & Right
	pucY0 = pucY - (PAD_YSIZE - OFF);
	pucU0 = pucU - (PAD_CSIZE - OFF/2);
	pucV0 = pucV - (PAD_CSIZE - OFF/2);
	pucY1 = pucY + uiWidthY;
	pucU1 = pucU + uiWidthC;
	pucV1 = pucV + uiWidthC;
	for( i=0; i<(int)uiHeightY; i++ ) {
		memset( pucY0 + i * nStrideY, pucY[i * nStrideY + 0           ], (PAD_YSIZE - OFF) );
		memset( pucY1 + i * nStrideY, pucY[i * nStrideY + uiWidthY - 1], (PAD_YSIZE - OFF) );
	}
	for( i=0; i<(int)uiHeightC; i++ ) {
		memset( pucU0 + i * nStrideC, pucU[i * nStrideC + 0           ], (PAD_CSIZE - OFF/2) );
		memset( pucU1 + i * nStrideC, pucU[i * nStrideC + uiWidthC - 1], (PAD_CSIZE - OFF/2) );
		memset( pucV0 + i * nStrideC, pucV[i * nStrideC + 0           ], (PAD_CSIZE - OFF/2) );
		memset( pucV1 + i * nStrideC, pucV[i * nStrideC + uiWidthC - 1], (PAD_CSIZE - OFF/2) );
	}

	// Padding Top & Bottom
	pucY1 = pucY0 + nStrideY * (uiHeightY - 1);
	pucU1 = pucU0 + nStrideC * (uiHeightC - 1);
	pucV1 = pucV0 + nStrideC * (uiHeightC - 1);
	for( i=0; i<(Int)(PAD_YSIZE - OFF); i++ ) {//x64 modify
		memcpy( pucY0 - (i+1) * nStrideY, pucY0, nStrideY );
		memcpy( pucY1 + (i+1) * nStrideY, pucY1, nStrideY );
	}
	for( i=0; i<(Int)(PAD_CSIZE - OFF/2); i++ ) {//x64 modify
		memcpy( pucU0 - (i+1) * nStrideC, pucU0, nStrideC );
		memcpy( pucV0 - (i+1) * nStrideC, pucV0, nStrideC );
		memcpy( pucU1 + (i+1) * nStrideC, pucU1, nStrideC );
		memcpy( pucV1 + (i+1) * nStrideC, pucV1, nStrideC );
	}

#if 0
	{
		static FILE *fpo = NULL;
		if ( fpo == NULL ) {
			fpo = fopen("OXE.YUV", "wb");
		}
		if ( fpo != NULL ) {
			fwrite( pFrm->base, 1, nStrideY * (uiHeight+PAD_YSIZE*2) * 3 / 2, fpo );
			fflush(fpo);
		}
	}
#endif
}

UInt32 countNonZeroCoeffs( Int16 *psCoef, UInt nStride, UInt nSize )
{
    Int count = 0;
    UInt x, y;

    for( y=0; y<nSize; y++ ) {
        for( x=0; x<nSize; x++ ) {
            count += (psCoef[y*nStride+x] != 0);
        }
    }

    return count;
}

UInt getCoefScanIdx( UInt nWidth, UInt8 bIsIntra, UInt8 bIsLuma, UInt nMode )
{
    UInt uiCTXIdx;
    UInt nScanIdx;
    UInt nOff = bIsLuma ? 0 : 1;

    if( !bIsIntra ) {
        return SCAN_DIAG;
    }

    switch( nWidth ) {
    case  2: uiCTXIdx = 6; break;
    case  4: uiCTXIdx = 5; break;
    case  8: uiCTXIdx = 4; break;
    case 16: uiCTXIdx = 3; break;
    case 32: uiCTXIdx = 2; break;
    case 64: uiCTXIdx = 1; break;
    default: uiCTXIdx = 0; break;
    }

    nScanIdx = SCAN_DIAG;
    if( uiCTXIdx > 3+nOff && uiCTXIdx < 6+nOff ) {
        //if multiple scans supported for PU size
        nScanIdx = abs((Int) nMode - VER_IDX) < 5 ? SCAN_HOR : (abs((Int)nMode - HOR_IDX) < 5 ? SCAN_VER : SCAN_DIAG);
    }

    return nScanIdx;
}

/** Pattern decision for context derivation process of significant_coeff_flag
 * \param sigCoeffGroupFlag pointer to prior coded significant coeff group
 * \param nPosXCG column of current coefficient group
 * \param nPosYCG row of current coefficient group
 * \param nSize width/height of the block
 * \returns pattern for current coefficient group
 */
static
Int calcPatternSigCtx( CUInt8* sigCoeffGroupFlag, UInt nPosXCG, UInt nPosYCG, UInt nSize )
{
    if( nSize == 4 )
        return -1;

    UInt sigRight = 0;
    UInt sigLower = 0;

    nSize >>= 2;
    if( nPosXCG < nSize - 1 )
    {
        sigRight = (sigCoeffGroupFlag[ nPosYCG * nSize + nPosXCG + 1 ] != 0);
    }
    if (nPosYCG < nSize - 1 )
    {
        sigLower = (sigCoeffGroupFlag[ (nPosYCG  + 1 ) * nSize + nPosXCG ] != 0);
    }
    return sigRight + (sigLower<<1);
}

/** Context derivation process of coeff_abs_significant_flag
 * \param patternSigCtx pattern for current coefficient group
 * \param posX column of current scan position
 * \param posY row of current scan position
 * \param blockType log2 value of block size if square block, or 4 otherwise
 * \param nSize width/height of the block
 * \param bIsLuma texture type (TEXT_LUMA...)
 * \returns ctxInc for current scan position
 */
static
Int getSigCtxInc(Int patternSigCtx, UInt scanIdx, Int posX, Int posY, Int log2BlockSize, UInt8 bIsLuma)//x64 modify, remove unused parameter
{
    //CInt width  = nSize; //x64 modify, these two parameter are not used
    //CInt height = nSize;
    CUInt nStride = (MAX_CU_SIZE >> (bIsLuma ? 0 : 1));

    CInt ctxIndMap[16] = {
        0, 1, 4, 5,
        2, 3, 4, 5,
        6, 6, 8, 8,
        7, 7, 8, 8
    };

    if( posX + posY == 0 ) {
        return 0;
    }

    if ( log2BlockSize == 2 ) {
        return ctxIndMap[ 4 * posY + posX ];
    }

    Int offset = (log2BlockSize == 3) ? (scanIdx==SCAN_DIAG ? 9 : 15) : (bIsLuma ? 21 : 12);
    Int posXinSubset = posX-((posX>>2)<<2);
    Int posYinSubset = posY-((posY>>2)<<2);
    Int cnt = 0;
    if( patternSigCtx == 0 ) {
        cnt = posXinSubset+posYinSubset<=2 ? (posXinSubset+posYinSubset==0 ? 2 : 1) : 0;
    }
    else if( patternSigCtx == 1 ) {
        cnt = posYinSubset<=1 ? (posYinSubset==0 ? 2 : 1) : 0;
    }
    else if( patternSigCtx == 2 ) {
        cnt = posXinSubset<=1 ? (posXinSubset==0 ? 2 : 1) : 0;
    }
    else {
        cnt = 2;
    }
    return (( bIsLuma && ((posX>>2) + (posY>>2)) > 0 ) ? 3 : 0) + offset + cnt;
}

static
UInt getSigCoeffGroupCtxInc(CUInt8 *uiSigCoeffGroupFlag, CUInt nCGPosX, CUInt nCGPosY, UInt nSize)//x64 modify, remove unused parameter
{
    UInt uiRight = 0;
    UInt uiLower = 0;
    UInt width = nSize;
    UInt height = nSize;

    width >>= 2;
    height >>= 2;
    if( nCGPosX < width - 1 ) {
        uiRight = (uiSigCoeffGroupFlag[ nCGPosY * width + nCGPosX + 1 ] != 0);
    }
    if( nCGPosY < height - 1 ) {
        uiLower = (uiSigCoeffGroupFlag[ (nCGPosY  + 1 ) * width + nCGPosX ] != 0);
    }
    return (uiRight || uiLower);
}

static
UInt32 codeLastSignificantXY( UInt32 *pCTUCmd, UInt nPosX, UInt nPosY, UInt nSize, UInt8 bIsLuma, UInt nScanIdx )
{
	CUInt32 *pCmd0 = pCTUCmd;
    CUInt   nLog2Size = xLog2( nSize - 1 );

    // swap
    if( nScanIdx == SCAN_VER ) {
        UInt tmp = nPosY;
        nPosY = nPosX;
        nPosX = tmp;
    }

    UInt nCtxLast;
    UInt nCtxX = OFF_LAST_X_CTX + (bIsLuma ? 0 : NUM_LAST_FLAG_XY_CTX);
    UInt nCtxY = OFF_LAST_Y_CTX + (bIsLuma ? 0 : NUM_LAST_FLAG_XY_CTX);
    UInt uiGroupIdxX    = xg_uiGroupIdx[ nPosX ];
    UInt uiGroupIdxY    = xg_uiGroupIdx[ nPosY ];

    Int blkSizeOffsetXY = bIsLuma ? ((nLog2Size-2)*3 + ((nLog2Size-1)>>2)) : 0;
    Int shiftXY         = bIsLuma ? ((nLog2Size+1)>>2) : nLog2Size-2;

    // posX
    for( nCtxLast = 0; nCtxLast < uiGroupIdxX; nCtxLast++ ) {
        xEncodeBin_pCmd(&pCTUCmd,  1, nCtxX + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }
    if( uiGroupIdxX < xg_uiGroupIdx[nSize-1]) {
        xEncodeBin_pCmd(&pCTUCmd,  0, nCtxX + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }

    // posY
    for( nCtxLast = 0; nCtxLast < uiGroupIdxY; nCtxLast++ ) {
        xEncodeBin_pCmd(&pCTUCmd,  1, nCtxY + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }
    if( uiGroupIdxY < xg_uiGroupIdx[ nSize - 1 ]) {
        xEncodeBin_pCmd(&pCTUCmd,  0, nCtxY + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }

    if( uiGroupIdxX > 3 ) {
        UInt nCount = ( uiGroupIdxX - 2 ) >> 1;
        nPosX       = nPosX - xg_uiMinInGroup[ uiGroupIdxX ];
        xEncodeBinsEP_pCmd(& pCTUCmd,  nPosX, nCount );
    }
    if( uiGroupIdxY > 3 ) {
        UInt nCount = ( uiGroupIdxY - 2 ) >> 1;
        nPosY       = nPosY - xg_uiMinInGroup[ uiGroupIdxY ];
        xEncodeBinsEP_pCmd(& pCTUCmd,  nPosY, nCount );
    }
    return(pCTUCmd - pCmd0);
}

static
UInt32 xWriteEpExGolomb( UInt32 *pCTUCmd, UInt uiSymbol, UInt uiCount )
{
	CUInt32 *pCmd0 = pCTUCmd;
   
    UInt bins = 0;
    Int numBins = 0;

    while( uiSymbol >= (UInt)(1<<uiCount) ) {
        bins = 2 * bins + 1;
        numBins++;
        uiSymbol -= 1 << uiCount;
        uiCount  ++;
    }
    bins = 2 * bins + 0;
    numBins++;


    bins = (bins << uiCount) | uiSymbol;
    numBins += uiCount;

    assert( numBins <= 32 );
    xEncodeBinsEP_pCmd(& pCTUCmd,  bins, numBins );
    return(pCTUCmd - pCmd0);
}

/** Coding of coeff_abs_level_minus3
 * \param uiSymbol value of coeff_abs_level_minus3
 * \param ruiGoRiceParam reference to Rice parameter
 * \returns Void
 */
static
UInt32 xWriteCoefRemainExGolomb( UInt32 *pCTUCmd, UInt nSymbol, UInt rParam )
{
	CUInt32 *pCmd0        = pCTUCmd;

    Int codeNumber  = (Int)nSymbol;
    UInt nLength;

    if (codeNumber < (COEF_REMAIN_BIN_REDUCTION << rParam)) {
        nLength = codeNumber>>rParam;
        xEncodeBinsEP_pCmd(& pCTUCmd,  (1<<(nLength+1))-2 , nLength+1);
        // OPT_ME: use mask to replace '%'
        if ( rParam )
            xEncodeBinsEP_pCmd(& pCTUCmd,  (codeNumber%(1<<rParam)),rParam);
    }
    else {
        nLength = rParam;
        codeNumber  = codeNumber - ( COEF_REMAIN_BIN_REDUCTION << rParam);
        while (codeNumber >= (1 << nLength)) {
            codeNumber -= (1 << nLength);
            nLength++;
        }
        xEncodeBinsEP_pCmd(& pCTUCmd,  (1<<(COEF_REMAIN_BIN_REDUCTION+nLength+1-rParam))-2,COEF_REMAIN_BIN_REDUCTION+nLength+1-rParam );
        if ( nLength )
            xEncodeBinsEP_pCmd(& pCTUCmd,  codeNumber, nLength);
    }
    return(pCTUCmd - pCmd0);
}

static
UInt32 xWriteUnaryMaxSymbol( UInt32 *pCTUCmd, UInt uiSymbol, UInt nCtx, Int iOffset, UInt uiMaxSymbol )
{
	CUInt32 *pCmd0 = pCTUCmd;

	if (uiMaxSymbol == 0) {
		return(pCTUCmd - pCmd0);
	}
	xEncodeBin_pCmd(&pCTUCmd,  (uiSymbol ? 1 : 0), nCtx );

	if ( uiSymbol == 0 ) {
		return(pCTUCmd - pCmd0);
	}

	UInt bCodeLast = ( uiMaxSymbol > uiSymbol );

	while( --uiSymbol ) {
		xEncodeBin_pCmd(&pCTUCmd,  1, nCtx + iOffset );
	}

	if( bCodeLast ) {
		xEncodeBin_pCmd(&pCTUCmd,  0, nCtx + iOffset );
	}

	return(pCTUCmd - pCmd0);
}

static
UInt32 xEncodeCoeffNxN( UInt32 *pCTUCmd, Int16 *psCoef, UInt nSize, UInt8 bIsLuma, UInt nMode, UInt nIsInter )
{
	CUInt32 *pCmd0        = pCTUCmd;
   CUInt    nStride      = (MAX_CU_SIZE >> (bIsLuma ? 0 : 1));
    UInt    nLog2Size    = xLog2( nSize - 1 );
    UInt32  uiNumSig     = countNonZeroCoeffs( psCoef, nStride, nSize );
    UInt    nScanIdx     = getCoefScanIdx( nSize, !(nIsInter), bIsLuma, nMode );
   CUInt16 *scan         = NULL;
   CUInt16 *scanCG       = NULL;
   CUInt    uiShift      = MLS_CG_SIZE >> 1;
   CUInt    uiNumBlkSide = nSize >> uiShift;
   CInt     blockType    = nLog2Size;
    UInt8 uiSigCoeffGroupFlag[MLS_GRP_NUM];
    Int idx;

    assert( nLog2Size <= 5 );
    scan   = xg_ausScanIdx[ nScanIdx ][ nLog2Size - 1 ];
    scanCG = xg_ausScanIdx[ nScanIdx ][ nLog2Size < 3 ? 0 : 1 ]; // only worked when nLog2Size is 4
    if( nLog2Size == 3 ) {
      scanCG = xg_sigLastScan8x8[ nScanIdx ];
    }
    else if( nLog2Size == 5 ) {
      scanCG = xg_sigLastScanCG32x32;
    }

    memset( uiSigCoeffGroupFlag, 0, sizeof(uiSigCoeffGroupFlag) );

    // Find position of last coefficient
    Int scanPosLast = -1;
    Int iRealPos = -1;
    Int posLast;
    do {
        posLast = scan[ ++scanPosLast ];

        // get L1 sig map
        UInt nPosY    = posLast >> nLog2Size;
        UInt nPosX    = posLast - ( nPosY << nLog2Size );
        UInt nBlkIdx  = uiNumBlkSide * (nPosY >> uiShift) + (nPosX >> uiShift);

        iRealPos = nPosY * nStride + nPosX;

        if( psCoef[iRealPos] ) {
            uiSigCoeffGroupFlag[nBlkIdx] = 1;
        }

        uiNumSig -= ( psCoef[iRealPos] != 0 );
    } while( uiNumSig > 0 );

    // Code position of last coefficient
    UInt posLastY = posLast >> nLog2Size;
    UInt posLastX = posLast - ( posLastY << nLog2Size );
    pCTUCmd += codeLastSignificantXY( pCTUCmd, posLastX, posLastY, nSize, bIsLuma, nScanIdx );

    //===== code significance flag =====
    UInt nBaseCoeffGroupCtx = OFF_SIG_CG_FLAG_CTX + (bIsLuma ? 0 : NUM_SIG_CG_FLAG_CTX);
    UInt nBaseCtx = OFF_SIG_FLAG_CTX + (bIsLuma ? 0 : NUM_SIG_FLAG_CTX_LUMA);

    CInt  iLastScanSet      = scanPosLast >> LOG2_SCAN_SET_SIZE;
    UInt c1                 = 1;
    UInt uiGoRiceParam      = 0;
    Int  iScanPosSig        = scanPosLast;
    Int  iSubSet;

    for( iSubSet = iLastScanSet; iSubSet >= 0; iSubSet-- ) {
        Int numNonZero = 0;
        Int  iSubPos     = iSubSet << LOG2_SCAN_SET_SIZE;
        uiGoRiceParam    = 0;
        Int16 absCoeff[16];
        UInt32 coeffSigns = 0;

        if( iScanPosSig == scanPosLast ) {
            absCoeff[0] = abs( psCoef[iRealPos] );
            coeffSigns  = ( psCoef[iRealPos] < 0 );
            numNonZero  = 1;
            iScanPosSig--;
        }

        // encode significant_coeffgroup_flag
        Int iCGBlkPos = scanCG[ iSubSet ];
        Int iCGPosY   = iCGBlkPos / uiNumBlkSide;
        Int iCGPosX   = iCGBlkPos - (iCGPosY * uiNumBlkSide);
        if( iSubSet == iLastScanSet || iSubSet == 0) {
            uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
        }
        else {
            UInt nSigCoeffGroup   = (uiSigCoeffGroupFlag[ iCGBlkPos ] != 0);
            UInt nCtxSig  = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, nSize );
            xEncodeBin_pCmd(&pCTUCmd,  nSigCoeffGroup, nBaseCoeffGroupCtx + nCtxSig );
        }

        // encode significant_coeff_flag
        if( uiSigCoeffGroupFlag[ iCGBlkPos ] ) {
            Int patternSigCtx = calcPatternSigCtx( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, nSize );
            UInt nBlkPos, nPosY, nPosX, nSig, nCtxSig;
            UInt nRealBlkPos;
            for( ; iScanPosSig >= iSubPos; iScanPosSig-- ) {
                nBlkPos     = scan[ iScanPosSig ];
                nPosY       = nBlkPos >> nLog2Size;
                nPosX       = nBlkPos - ( nPosY << nLog2Size );
                nRealBlkPos = nPosY * nStride + nPosX;
                nSig        = (psCoef[ nRealBlkPos ] != 0);
                if( (iScanPosSig != iSubPos) || iSubSet == 0 || numNonZero ) {
                    nCtxSig  = getSigCtxInc( patternSigCtx, nScanIdx, nPosX, nPosY, nLog2Size, bIsLuma );
                    xEncodeBin_pCmd(&pCTUCmd,  nSig, nBaseCtx + nCtxSig );
                }
                if( nSig ) {
                    absCoeff[numNonZero] = abs( psCoef[nRealBlkPos] );
                    coeffSigns = (coeffSigns << 1) + ( psCoef[nRealBlkPos] < 0 );
                    numNonZero++;
                }
            }
        }
        else {
            iScanPosSig = iSubPos - 1;
        }

        if( numNonZero > 0 ) {
            UInt uiCtxSet = (iSubSet > 0 && bIsLuma) ? 2 : 0;

            if( c1 == 0 ) {
                uiCtxSet++;
            }

            c1 = 1;
            UInt nBaseCtxMod = OFF_ONE_FLAG_CTX + 4 * uiCtxSet + ( bIsLuma ? 0 : NUM_ONE_FLAG_CTX_LUMA);

            Int numC1Flag = MIN(numNonZero, C1FLAG_NUMBER);
            Int firstC2FlagIdx = 16;
            for( idx = 0; idx < numC1Flag; idx++ ) {
                UInt uiSymbol = absCoeff[ idx ] > 1;
                xEncodeBin_pCmd(&pCTUCmd,  uiSymbol, nBaseCtxMod + c1 );
                if( uiSymbol ) {
                    c1 = 0;
                    firstC2FlagIdx = MIN(firstC2FlagIdx, idx);
                }
                else if( c1 != 0 ) {
                    c1 = MIN(c1+1, 3);
                }
            }

            if( c1 == 0 ) {
                nBaseCtxMod = OFF_ABS_FLAG_CTX + uiCtxSet + (bIsLuma ? 0 : NUM_ABS_FLAG_CTX_LUMA);
                if( firstC2FlagIdx != 16 ) {
                    UInt symbol = absCoeff[ firstC2FlagIdx ] > 2;
                    xEncodeBin_pCmd(&pCTUCmd,  symbol, nBaseCtxMod + 0 );
                }
            }

            xEncodeBinsEP_pCmd(& pCTUCmd,  coeffSigns, numNonZero );

            Int iFirstCoeff2 = 1;
            if( c1 == 0 || numNonZero > C1FLAG_NUMBER ) {
                for( idx = 0; idx < numNonZero; idx++ ) {
                    Int baseLevel = (idx < C1FLAG_NUMBER) ? (2 + iFirstCoeff2 ) : 1;

                    if( absCoeff[ idx ] >= baseLevel ) {
                        pCTUCmd += xWriteCoefRemainExGolomb( pCTUCmd, absCoeff[ idx ] - baseLevel, uiGoRiceParam );
                        if(absCoeff[idx] > 3*(1<<uiGoRiceParam))
                            uiGoRiceParam = MIN(uiGoRiceParam+ 1, 4);
                    }
                    if( absCoeff[ idx ] >= 2 ) {
                        iFirstCoeff2 = 0;
                    }
                }
            }
        }
    }
    return(pCTUCmd - pCmd0);
}


// ***************************************************************************
// * Cache Manage Functions
// ***************************************************************************
void xEncCahceInitLine( xCache *pCache )
{
	int i;

	memset( pCache->aucPredY, MODE_INVALID, sizeof(pCache->aucPredY) );
	memset( pCache->aucPredU, MODE_INVALID, sizeof(pCache->aucPredU) );
	memset( pCache->aucPredV, MODE_INVALID, sizeof(pCache->aucPredV) );

	// Clear Mode of Left
	for( i=0; i<MAX_PU_XY+1; i++ ) {
		pCache->aucModeY[i*(MAX_PU_XY+2)] = MODE_INVALID;
	}
}

void xEncCacheLoadCU( X265_t *h, xCache *pCache, UInt uiCUX, UInt uiCUY )
{
	xFrame  *pFrame     = h->pFrameCur;
	CInt   iMaxCUWidth   = h->ucMaxCUWidth;//x64 modify, convert to int
	//CInt32 iWidth    = h->iWidth;//x64 modify, convert to int
	CInt  iStrideY = pFrame->iStrideY;//x64 modify, convert to int
	CInt  iStrideC = pFrame->iStrideC;//x64 modify, convert to int
	CInt32 iOffsetY = (iStrideY * uiCUY * iMaxCUWidth   + uiCUX * iMaxCUWidth  );//x64 modify, convert to int
	CInt32 iOffsetC = (iStrideC * uiCUY * iMaxCUWidth/2 + uiCUX * iMaxCUWidth/2);//x64 modify, convert to int
	UInt8 *pucSY0 = pFrame->pucY + iOffsetY + 0*iStrideY;
	UInt8 *pucSY1 = pFrame->pucY + iOffsetY + 1*iStrideY;
	UInt8 *pucSU  = pFrame->pucU + iOffsetC;
	UInt8 *pucSV  = pFrame->pucV + iOffsetC;
	UInt8 *pucDY0 = pCache->aucPixY + 0*MAX_CU_SIZE;//x64 advice: use MAX_CU_SIZE or iMaxCUWidth in this function, remove one because they are redundant
	UInt8 *pucDY1 = pCache->aucPixY + 1*MAX_CU_SIZE;
	UInt8 *pucDU  = pCache->aucPixU;
	UInt8 *pucDV  = pCache->aucPixV;
	Int y;

	for( y=0; y < h->ucMaxCUWidth/2; y++ ) {
		memcpy( pucDY0, pucSY0, iMaxCUWidth     );
		memcpy( pucDY1, pucSY1, iMaxCUWidth     );
		memcpy( pucDU,  pucSU,  iMaxCUWidth / 2 );
		memcpy( pucDV,  pucSV,  iMaxCUWidth / 2 );
		pucSY0 += iStrideY * 2;
		pucSY1 += iStrideY * 2;
		pucSU  += iStrideC;
		pucSV  += iStrideC;
		pucDY0 += MAX_CU_SIZE * 2;
		pucDY1 += MAX_CU_SIZE * 2;
		pucDU  += MAX_CU_SIZE / 2;
		pucDV  += MAX_CU_SIZE / 2;
	}
}

void xEncCacheStoreCU( X265_t *h, xCache *pCache, UInt uiCUX, UInt uiCUY )
{
	xFrame  *pFrame     = h->pFrameRec;
	CUInt32 uiStrideY   = pFrame->iStrideY;
	CUInt32 uiStrideC   = pFrame->iStrideC;
	CUInt   nCUWidth   = h->ucMaxCUWidth;
	CUInt32 uiWidthY   = pFrame->iStrideY;
	CUInt32 uiOffsetY  = (uiStrideY * uiCUY * nCUWidth   + uiCUX * nCUWidth  );
	CUInt32 uiOffsetC  = (uiStrideC * uiCUY * nCUWidth/2 + uiCUX * nCUWidth/2);
	UInt8 *pucSY0 = pCache->aucRecY + 16 + 0*(MAX_CU_SIZE+16);
	UInt8 *pucSY1 = pCache->aucRecY + 16 + 1*(MAX_CU_SIZE+16);
	UInt8 *pucSU  = pCache->aucRecU + 8;
	UInt8 *pucSV  = pCache->aucRecV + 8;
	UInt8 *pucDY0 = pFrame->pucY + uiOffsetY + 0*uiStrideY;
	UInt8 *pucDY1 = pFrame->pucY + uiOffsetY + 1*uiStrideY;
	UInt8 *pucDU  = pFrame->pucU + uiOffsetC;
	UInt8 *pucDV  = pFrame->pucV + uiOffsetC;
	Int y;

	for( y=0; y < h->ucMaxCUWidth/2; y++ ) {
		memcpy( pucDY0, pucSY0, nCUWidth     );
		memcpy( pucDY1, pucSY1, nCUWidth     );
		memcpy( pucDU,  pucSU,  nCUWidth / 2 );
		memcpy( pucDV,  pucSV,  nCUWidth / 2 );
		pucSY0 += (MAX_CU_SIZE  +16) * 2;
		pucSY1 += (MAX_CU_SIZE  +16) * 2;
		pucSU  += (MAX_CU_SIZE/2+8);
		pucSV  += (MAX_CU_SIZE/2+8);
		pucDY0 += uiStrideY*2;
		pucDY1 += uiStrideY*2;
		pucDU  += uiStrideC;
		pucDV  += uiStrideC;
	}
}

void xEncCacheUpdate(
	X265_t *h,
	xCache *pCache,
	Int    iSize,    //x64 modify, change type from UInt to Int
	UInt32  uiOffset,
	CUInt32 idxTiles
	)
{
	UInt8 *paucModeY        =  pCache->aucModeY + (MAX_PU_XY+2) + 1;
	UInt8 *paucPredY        =  pCache->aucPredY;
	UInt8 *pauclPixTopY     =  h->aucTopPixY[idxTiles] + 1 + uiOffset;
	UInt8 *pauclPixTopU     =  h->aucTopPixU[idxTiles] + 1 + uiOffset/2;
	UInt8 *pauclPixTopV     =  h->aucTopPixV[idxTiles] + 1 + uiOffset/2;
	xMV   *pasTopMV         =  h->asTopMV[idxTiles] + 1 + uiOffset / MIN_MV_SIZE;
	xMV   *paslMV           =  pCache->asMV + (MAX_MV_XY+2) + 1;

	UInt8 *paucModeYInfo1     =  pCache->aucModeYInfo1 + (MAX_PU_XY+1) + 1;
	UInt8 *pauclTopModeYInfo1 =  h->aucTopModeYInfo1[idxTiles] + uiOffset / MIN_CU_SIZE;

	UInt8 *paucRecY         =  pCache->aucRecY + 16;
	UInt8 *paucRecU         =  pCache->aucRecU + 8;
	UInt8 *paucRecV         =  pCache->aucRecV + 8;

	Int y;//x64 modify, change type from UInt to Int

	// Update Mode Split Info
	memcpy( pauclTopModeYInfo1, paucModeYInfo1 + (iSize/MIN_CU_SIZE - 1) * (MAX_PU_XY+1), MAX_PU_XY );

	// Update Mode Infos
	for( y=0; y<MAX_PU_XY; y++ ) {
		paucModeY[y*(MAX_PU_XY+2)-1] = paucModeY[y*(MAX_PU_XY+2) + (iSize/MIN_CU_SIZE - 1)];
		paucModeYInfo1[y*(MAX_PU_XY+1)-1] = paucModeYInfo1[y*(MAX_PU_XY+1) + (iSize/MIN_CU_SIZE - 1)];
		memset( &paucModeY[y*(MAX_PU_XY+2)], MODE_INVALID, MAX_PU_XY );
	}

	// Update MV Infos
	memcpy( pasTopMV, &paslMV[(iSize/MIN_MV_SIZE-1) * (MAX_MV_XY+2)], sizeof(xMV)*MAX_MV_XY );
	for( y=0; y<MAX_MV_XY; y++ ) {
		paslMV[y*(MAX_MV_XY+2)-1] = paslMV[y*(MAX_MV_XY+2) + (iSize/MIN_MV_SIZE - 1)];
	}

	// Update Luma Pred Pixel
	memcpy( pauclPixTopY, &paucRecY[(iSize-1) * (MAX_CU_SIZE+16)], MAX_CU_SIZE );
	for( y=0; y<MAX_CU_SIZE; y++ ) {
		paucRecY[y*(MAX_CU_SIZE+16)-1] = paucRecY[y*(MAX_CU_SIZE+16) + (iSize - 1)];
	}

	// Update Chroma Pred Pixel
	memcpy( pauclPixTopU, &paucRecU[(iSize/2-1) * (MAX_CU_SIZE/2+8)], MAX_CU_SIZE/2 );
	memcpy( pauclPixTopV, &paucRecV[(iSize/2-1) * (MAX_CU_SIZE/2+8)], MAX_CU_SIZE/2 );
	for( y=0; y<MAX_CU_SIZE/2; y++ ) {
		paucRecU[y*(MAX_CU_SIZE/2+8)-1] = paucRecU[y*(MAX_CU_SIZE/2+8) + (iSize/2 - 1)];
		paucRecV[y*(MAX_CU_SIZE/2+8)-1] = paucRecV[y*(MAX_CU_SIZE/2+8) + (iSize/2 - 1)];
	}
}


// ***************************************************************************
// * Inter Prediction Functions 
// ***************************************************************************
Int xFillMvpCand(
	xMV        *psMVP,
	CUInt8     *pucModeY,
	const xMV  *psMV,
	CUInt       nSizeX,
	CUInt       nSizeY,
	CUInt32     ePUMode,
	CUInt32		nPUIdx
	)
{
	int  isAvailableB1 = (ePUMode == SIZE_2NxN)&&(nPUIdx==1);
	int  isAvailableA1 = (ePUMode == SIZE_Nx2N)&&(nPUIdx==1);

	xMV  mvZero   = {0, 0};
	Int iMvSizeX  = nSizeX / MIN_MV_SIZE;//x64 modify, change type from UInt to Int
	Int iMvSizeY  = nSizeY / MIN_MV_SIZE;//x64 modify, change type from UInt to Int
	Int iBlkSizeX = nSizeX / MIN_CU_SIZE;//x64 modify, change type from UInt to Int
	Int iBlkSizeY = nSizeY / MIN_CU_SIZE;//x64 modify, change type from UInt to Int
	UInt bBL      = ( pucModeY[ -1 + (Int)(iBlkSizeY  )*(MAX_PU_XY+2) ] != MODE_INVALID );//x64 modify
	UInt bL       = ( pucModeY[ -1 + (Int)(iBlkSizeY-1)*(MAX_PU_XY+2) ] != MODE_INVALID );//x64 modify
	UInt bTR      = ( pucModeY[      (Int)(iBlkSizeX  )-(Int)(MAX_PU_XY+2) ] != MODE_INVALID );//x64 modify
	UInt bT       = ( pucModeY[      (Int)(iBlkSizeX-1)-(Int)(MAX_PU_XY+2) ] != MODE_INVALID );//x64 modify
	UInt bTL      = ( pucModeY[ -1                -(Int)(MAX_PU_XY+2) ] != MODE_INVALID );//x64 modify
	UInt bGrpL    = (bBL || bL);
	UInt bGrpT    = (bTR || bT || bTL);
	xMV  mvBL     = psMV[ -1 + (Int)(iMvSizeY  )*(MAX_MV_XY+2) ];//x64 modify
	xMV  mvL      = psMV[ -1 + (Int)(iMvSizeY-1)*(MAX_MV_XY+2) ];//x64 modify
	xMV  mvTR     = psMV[      (Int)(iMvSizeX  )-(Int)(MAX_MV_XY+2) ];//x64 modify
	xMV  mvT      = psMV[      (Int)(iMvSizeX-1)-(Int)(MAX_MV_XY+2) ];//x64 modify
	xMV  mvTL     = psMV[ -1               -(Int)(MAX_MV_XY+2) ];//x64 modify
	UInt nCount   = 0;

	// Fill AMVP
	psMVP[0] = mvZero;

	// Below Left & Left
	if ( bGrpL ) {
		psMVP[nCount++] = bBL ? mvBL : mvL;
	}

	// Above Right & Right & Above Left
	if ( bGrpT ) {
		psMVP[nCount++] = bTR ? mvTR : bT ? mvT : mvTL;
	}

	if ( bGrpL && bGrpT ) {
		if ( (psMVP[0].x == psMVP[1].x) && (psMVP[0].y == psMVP[1].y) ) {
			psMVP[1] = mvZero;
		}
	}
	else {
		assert( nCount < AMVP_MAX_NUM_CANDS );
	}
	if ( nCount < AMVP_MAX_NUM_CANDS ) {
		psMVP[1] = mvZero;
	}

	// Fill Merge MVP
	nCount = 0;

	// Left MV
	if (!isAvailableA1)
		if ( bL ) {
			psMVP[MRG_CANDS_START + (nCount++)] = mvL;
		}

		// Above
		if (!isAvailableB1)
			if ( bT && (!bL ||isAvailableA1 || (mvL.x != mvT.x || mvL.y != mvT.y)) ) {
				psMVP[MRG_CANDS_START + (nCount++)] = mvT;
			}

			// Above Right
			if ( bTR && (!bT ||isAvailableB1|| (mvT.x != mvTR.x || mvT.y != mvTR.y)) ) {
				psMVP[MRG_CANDS_START + (nCount++)] = mvTR;
			}

			// Left Bottom
			if ( bBL && (!bL ||isAvailableA1|| (mvL.x != mvBL.x || mvL.y != mvBL.y)) ) {
				psMVP[MRG_CANDS_START + (nCount++)] = mvBL;
			}

			// Above Left
			if ( nCount < 4 ) {
				if ( bTL && (!bL ||isAvailableA1|| (mvL.x != mvTL.x || mvL.y != mvTL.y))
					&& (!bT ||isAvailableB1|| (mvT.x != mvTL.x || mvT.y != mvTL.y)) ) {
						psMVP[MRG_CANDS_START + (nCount++)] = mvTL;
				}
				else {
					//org: psMVP[MRG_CANDS_START + (nCount++)] = mvZero;--by chen
					//below is updated to avoid duplicated Zero MV --by wxw
					if (nCount == 0)
						psMVP[MRG_CANDS_START + (nCount++)] = mvZero;
					else
					{
						UInt bZeroMVUsed = 0;
						for (Int i=0; i<(Int)nCount; i++)//x64
						{
							bZeroMVUsed += (*(UInt32*)(psMVP+MRG_CANDS_START + i) == 0);
						}
						if (bZeroMVUsed == 0)
							psMVP[MRG_CANDS_START + (nCount++)] = mvZero;
					}
				}
			}		
			return nCount;
}

// RDO from JM
#define  LAMBDA_ACCURACY_BITS			5
#define  LAMBDA_FACTOR(lambda)        ((int)((double)(1 << LAMBDA_ACCURACY_BITS) * lambda + 0.5))

xMV xMotionSearch(
	CUInt8     *pucCurY,
	CUInt8     *pucRefY,
	Int16      *psTmp,
	CUInt32     nRefStrideY,
	CUInt       nSizeX,
	CUInt       nSizeY,
	UInt32     *uiBestRDCostY,
	xMV		   *sMVP,
	//CUInt	    nMVP,//x64 modify, remove unused parameter
	Int 	   *pMVPIdx,	
	int 		bZeroMV,
	xMV			mv_min_ipel,
	xMV			mv_max_ipel,
	double		dLambda_ssd,
	UInt		*pSad_best
#if MVD_COST_BY_TABLE
	, UInt16 * mv_cost_table
#endif
	)
{
	UInt32	uiLambda = ( UInt32)sqrt(dLambda_ssd);
	Int		  nMVPIdx = -1;
	CUInt32	nSize = (nSizeX>nSizeY)?nSizeX:nSizeY;
	CInt8	  dia4[4][2] = { {0, -1}, {0, 1},   {-1, 0}, {1, 0}};
	CInt8	  dia9[9][2] = { { 0, 0}, { 0, -1}, { 0, 1}, {-1, 0}, { 1, 0}, {-1, -1},	{-1, 1}, { 1, -1}, { 1, 1}};//,
	CUInt	  nSadIdx = 2*xLog2(nSizeX-1) + xLog2(nSizeY-1) - 9;  //there will be bug if 4x8/8x4 block size is used!!!--fixME wxw
	xMV		  bmv = {0, 0}, tmv;
	Int		  bmx = 0, bmy = 0;
	Int		  bestK=-1;
	UInt	  bChanged;
	UInt8	  tmp[MAX_CU_SIZE*MAX_CU_SIZE];
	Int		  tX, tY;
	UInt32	tSad, bSad = INT_MAX;
	UInt32  tRDCost, bRDCost = INT_MAX;
	Int		  nMEPass = 1;
	CUInt8 *paucRefY = pucRefY;   //if no Macro, paucRefY = pucRefY

	*pSad_best = INT_MAX;
	*uiBestRDCostY = INT_MAX;

	if( (abs(sMVP[0].x-sMVP[1].x)>>2) + (abs(sMVP[0].y - sMVP[1].y)>>2) > 4) {
		nMEPass = 2;
	}

	//check the better MVP as the start point
	for (Int i=0; i<nMEPass; i++) {
		tmv = sMVP[i];
		xPredInterLumaBlk( paucRefY, tmp, nRefStrideY, nSizeX, nSizeY, tmv, psTmp );
		tSad = xSad_AMP[nSadIdx]( pucCurY, MAX_CU_SIZE, tmp, MAX_CU_SIZE ) ;
		if(tSad < bSad)	{
			bSad = tSad;
			nMVPIdx = i;
		}
	}

	Int		iMVPIdxBits = nMVPIdx==0?1:1;  //reference xGetMvpIdxBits in multi-core xHEVC
	UInt	uiMVPRateCost;
#if MVD_COST_BY_TABLE
	UInt16	*m_cost_mvx = mv_cost_table - sMVP[nMVPIdx].x;
	UInt16	*m_cost_mvy = mv_cost_table - sMVP[nMVPIdx].y;
#endif

	if(!bZeroMV){
#if  MVD_COST_BY_TABLE
		uiMVPRateCost = m_cost_mvx[0] + m_cost_mvy[0];
#else
		int iMvBits = xGetComponentBits(sMVP[nMVPIdx].x - 0) + xGetComponentBits(sMVP[nMVPIdx].y - 0) ;
		uiMVPRateCost = uiLambda * iMvBits; //ii==0 is the MVP rate cost
#endif
		tSad = xSad_AMP[nSadIdx]( pucCurY, MAX_CU_SIZE, pucRefY, nRefStrideY ) ;

		tRDCost = tSad + uiMVPRateCost ;
		if ( tRDCost < bRDCost ) {
			bRDCost = tRDCost;
			bmx  = 0;
			bmy  = 0;
		}
	}

	//begin to ME
	{
		Int omx = sMVP[nMVPIdx].x>>2; 
		Int omy = sMVP[nMVPIdx].y>>2; 

		//test integer point
#if  MVD_COST_BY_TABLE
		uiMVPRateCost = m_cost_mvx[omx*4] + m_cost_mvy[omy*4];
#else
		int iMvBits = xGetComponentBits(sMVP[nMVPIdx].x - omx*4) + xGetComponentBits(sMVP[nMVPIdx].y - omy*4) ;
		uiMVPRateCost = uiLambda * iMvBits; //ii==0 is the MVP rate cost
#endif
		tSad = xSad_AMP[nSadIdx]( pucCurY, MAX_CU_SIZE, pucRefY + omy * (int)nRefStrideY + omx, nRefStrideY ) ;//x64 modify

		tRDCost = tSad + uiMVPRateCost ;
		if ( tRDCost < bRDCost ) {
			bRDCost = tRDCost;
			bmx  = omx<<2; bmy  = omy<<2;
		}
		// Integer Search
		for(Int i=0; i<ME_RANGE; i++ ) { 
			bChanged = FALSE;
			for(Int k=0; k<(Int)ASIZE(dia4); k++ ) {//x64
				tX = omx + dia4[k][0];	
				tY = omy + dia4[k][1];
#if  MV_CLIP
				tX = Clip3(mv_min_ipel.x, mv_max_ipel.x, tX); 
				tY = Clip3(mv_min_ipel.y, mv_max_ipel.y, tY); 
#endif

#if  MVD_COST_BY_TABLE
				uiMVPRateCost = m_cost_mvx[tX*4] + m_cost_mvy[tY*4];
#else
				int iMvBits = xGetComponentBits(sMVP[nMVPIdx].x - tX*4) + xGetComponentBits(sMVP[nMVPIdx].y - tY*4) ;
				uiMVPRateCost = uiLambda * iMvBits; //ii==0 is the MVP rate cost
#endif
				tSad = xSad_AMP[nSadIdx]( pucCurY, MAX_CU_SIZE, pucRefY + tY * (int)nRefStrideY + tX, nRefStrideY ) ;//x64 modify, convert nRefStrideY to int, otherwise will cause severe error

				tRDCost = tSad + uiMVPRateCost;

				if ( tRDCost < bRDCost ) {
					bChanged = TRUE; 
					bRDCost = tRDCost; 
					bestK = k; 
					bmx  = tX<<2; bmy  = tY<<2;
				}
			}
			if ( !bChanged )
				break;
			omx += dia4[bestK][0]; omy += dia4[bestK][1];
			if (( bRDCost < 800 ) )
				break;
		}

#if MV_CLIP
		xMV mv_min = { 4*( mv_min_ipel.x - 4 ), 4*( mv_min_ipel.y - 4 )};
		xMV mv_max = { 4*( mv_max_ipel.x - 4 ), 4*( mv_max_ipel.y - 4 )};
		bmx = Clip3(mv_min.x, mv_max.y, bmx);
		bmy = Clip3(mv_min.y, mv_max.y, bmy);
#endif

		//begin fraction pixels search
		bRDCost = INT_MAX;
		Int step = 2;
		for(Int i=0; i<2; i++ ) {
			bChanged = FALSE;

			for(Int k=0; k<(Int)ASIZE(dia9); k++ ) {//x64
				tmv.x = bmx + dia9[k][0] * step;
				tmv.y = bmy + dia9[k][1] * step;

				xPredInterLumaBlk( paucRefY, tmp, nRefStrideY, nSizeX, nSizeY, tmv, psTmp );

#if  MVD_COST_BY_TABLE
				uiMVPRateCost = m_cost_mvx[tmv.x] + m_cost_mvy[tmv.y];
#else
				Int iMvBits = xGetComponentBits(sMVP[nMVPIdx].x - tmv.x) + xGetComponentBits(sMVP[nMVPIdx].y - tmv.y) ;   //--DEBUG --lzy
				uiMVPRateCost = uiLambda * iMvBits; //ii==0 is the MVP rate cost
#endif

#if SATD_FOR_FRAC_ME
				tSad = xSatd_AMP[nSadIdx]( pucCurY, MAX_CU_SIZE, tmp, MAX_CU_SIZE ) ;
#else
				tSad = xSad_AMP[nSadIdx]( pucCurY, MAX_CU_SIZE, tmp, MAX_CU_SIZE ) ;
#endif

				tRDCost = tSad + uiMVPRateCost ;
				if ( tRDCost < bRDCost ) {
					bChanged = TRUE;
					bRDCost = tRDCost;
					bestK = k;
				}
			}
			if ( bChanged ) {
				bmx += dia9[bestK][0] * step;
				bmy += dia9[bestK][1] * step;
			}
			step >>= 1;
		}
	}

	//set the return values
	*uiBestRDCostY = bRDCost;
	*pSad_best = bSad;
	*pMVPIdx = nMVPIdx;
	bmv.x = bmx;
	bmv.y = bmy;
	return bmv;
}

UInt32 xSubDctQuantRec(UInt8* puclPix, UInt8* puclPred, UInt8 *puclRec, Int16 *pslCoef, CUInt nSize, Int16* psTmp[2], CUInt isChroma, CInt nQP, UInt nBestModeY,xSliceType eSType )
{
	xSubDct( psTmp[0], puclPix, (MAX_CU_SIZE >> isChroma),	puclPred, (MAX_CU_SIZE >> isChroma),	psTmp[0], psTmp[1],	nSize, nBestModeY );
	UInt32 uiSum = xQuant(pslCoef , psTmp[0], (MAX_CU_SIZE >> isChroma), nQP, nSize, nSize, eSType );
	
	if( uiSum ) {
		xDeQuant( psTmp[0], pslCoef, (MAX_CU_SIZE >> isChroma), nQP, nSize, nSize );
    xIDctAdd(puclRec , ((MAX_CU_SIZE+16) >> isChroma), psTmp[0], (MAX_CU_SIZE >> isChroma),	puclPred,	psTmp[1], psTmp[0],	nSize, nBestModeY );
	}
	else {
		for(Int i=0; i<(int)nSize; i++ ) {
			memcpy( puclRec + i*((MAX_CU_SIZE+16) >> isChroma), puclPred + i*(MAX_CU_SIZE >> isChroma), nSize );
		}
	}

	return uiSum;
}

UInt32 xPreEncRec(UInt32  *pCTUCmd,
					UInt8	*puclPixY, 
					UInt8	*puclPredY, 
					UInt8	*puclRecY, 
					UInt	 nSizeY, 
					UInt	 nQP, 
					UInt8	*pucModeY, 
					short	*pslCoefY, 
					short	*psTmp[2],
					int		*piCbfY,
					UInt	 nBestModeY,
					xSliceType eSType,
					xRdoParam *rdoParam
)
{  
	rdoParam->nCoeffBits = 0;
	UInt32 *pCmd0 = pCTUCmd;

	//encoding PredMode, PartSize, PredInfo... symbols.
	xEncodeBin_pCmd( &pCTUCmd, 0, OFF_PRED_MODE_CTX ); //PredMode
	xEncodeBin_pCmd( &pCTUCmd, 1, OFF_PART_SIZE_CTX );

	UInt uiSum = 0;

	if (nSizeY == 64){
		*piCbfY = 0;
		UInt nBlock = nSizeY / MIN_CU_SIZE;
		for(int j=0; j<2; j++) {
			for(int i=0; i<2; i++){
				UInt32	nModeYOffset   = j*(MAX_PU_XY+2)*nBlock/2	 + i*nBlock/2;
				UInt    nPredOffsetY   = j*nSizeY/2*MAX_CU_SIZE		 + i*nSizeY/2;
				UInt    nRecOffsetY    = j*nSizeY/2*(MAX_CU_SIZE+16) + i*nSizeY/2;
				UInt    nCbfY = xSubDctQuantRec(puclPixY+nPredOffsetY,  puclPredY+nPredOffsetY,  puclRecY+nRecOffsetY,  pslCoefY+nPredOffsetY,  nSizeY/2, psTmp, FALSE, nQP, nBestModeY, eSType );
				int iSub32x32Idx = j*2+i;
				*piCbfY |= ((nCbfY==0?0:1)<<iSub32x32Idx);
				uiSum   += nCbfY;
				xEncodeBin_pCmd( &pCTUCmd, !!(nCbfY), OFF_QT_CBF_CTX + 0*NUM_QT_CBF_CTX + 0  );
				if (nCbfY){
					pCTUCmd += xEncodeCoeffNxN( pCTUCmd, pslCoefY+nPredOffsetY, nSizeY/2, 1, *(pucModeY + nModeYOffset), 1 );
				}
			}
		}
		xEncodeBin_pCmd( &pCTUCmd, !!(uiSum), OFF_QT_ROOT_CBF_CTX ); 
	}
	else{
		UInt nCbfY = xSubDctQuantRec(puclPixY, puclPredY, puclRecY, pslCoefY, nSizeY, psTmp, FALSE, nQP, nBestModeY, eSType );
		*piCbfY = nCbfY==0?0:1;
		uiSum = nCbfY;
		xEncodeBin_pCmd( &pCTUCmd, !!(nCbfY), OFF_QT_CBF_CTX + 0*NUM_QT_CBF_CTX + 0  );
		if(nCbfY){
			pCTUCmd += xEncodeCoeffNxN( pCTUCmd, pslCoefY, nSizeY, 1, *(pucModeY), 1 );
		}
	}

	return xGetWrittenCmdRate( pCmd0, pCTUCmd - pCmd0, rdoParam);
}

UInt32 xEncDecideCU_Inter(
	X265_t *h,
	UInt32 *pCTUCmd,
	UInt8  *pucPixY,
	CUInt8 *pucRefY,
	UInt8  *pucPredY,
	UInt8  *pucRecY,
	Int16  *psCoefY,
	xMV    *pasMV,
	xMV    *pasMVD,
	UInt8  *paucMVPIdx,
	UInt8  *pucModeY,
	UInt16 *pusModeYInfo0,
	UInt8  *pucModeYInfo1,
	Int16  *ppsTmp[4],
	UInt8  *paucPredTmp[MAX_CU_DEPTH],
	UInt8  *paucRecYTmp[MAX_CU_DEPTH],
	Int16  *pasCoefYTmp[MAX_CU_DEPTH],
	Int	   *paiCbfYTmp[MAX_CU_DEPTH],
	CInt32 iSize,//x64 modify, change type from CUInt32 to CInt32
	CUInt   nQP,
	double  dLambda,
	CInt32 iRefStrideY,//x64 modify, convert to CInt
	CInt   off_y,//x64 modify, convert to CInt
	CInt   off_x,//x64 modify, convert to CInt
	xMV		mv_min_ipel,
	xMV		mv_max_ipel
	,UInt	uiDepth
	,xCabac *ppcCabac[MAX_CU_DEPTH+1]
	,xRdoParam *rdoParam
#if MVD_COST_BY_TABLE
	,UInt16 *mv_cost_table
#endif
	,Int yCtu
	,Int xCtu
	)
{
	UInt32 *pCTUCmd0 = pCTUCmd;
	UInt8   ucSKIPState;

	xCabac *pcCabac_CurrBak = ppcCabac[MAX_CU_DEPTH ];  
	memcpy(pcCabac_CurrBak, ppcCabac[uiDepth], sizeof(xCabac));

	Int   iLog2Size = xLog2( iSize - 1 );//64 modify, convert to Int
	Int    iMEBlock  = iSize / MIN_MV_SIZE;//64 modify, convert to Int
	Int    iBlock    = iSize / MIN_CU_SIZE;//64 modify, convert to Int

	double  dSubRDCost[4];

	UInt8  *pucPredDstY =  paucPredTmp[iLog2Size-2]; // -2 means there is only nLog2Size-2 CU depth for Inter CUs
	UInt8  *pucRecYDst  =  paucRecYTmp[iLog2Size - 2]; // -2 means there is only nLog2Size-2 CU depth for Inter CUs
	Int16  *psCoefYDst  =  pasCoefYTmp[iLog2Size - 2]; // -2 means there is only nLog2Size-2 CU depth for Inter CUs
	Int	   *piCbfYDst   =  paiCbfYTmp[iLog2Size - 2];
	Int		  iCbfY_tmp;

	UInt8   ePUMode = SIZE_2Nx2N;

	xMV		  bmv, bmv_tmp;
	Int		  nMVPIdx, nMVPIdx_tmp;

	Int 	  nMergeIdx = 0, nMergeIdx_tmp=0;
	Int     bZeroMV = 0;
	Int     bSKIP = FALSE;
	Int 	  bCbfY = FALSE;

	xMV     sMVP[AMVP_MAX_NUM_CANDS + MRG_MAX_NUM_CANDS];
	Int		  nMVP = xFillMvpCand( sMVP, pucModeY, pasMV, iSize, iSize, 0, 0);

	UInt8   *pucPredY_tmp = (UInt8 *)ppsTmp[1];
	UInt8   *pucRecY_tmp  = (UInt8 *)(ppsTmp[1]+MAX_CU_SIZE*MAX_CU_SIZE/2);
	Int16   *psCoefY_tmp = ppsTmp[2]; 
	Int16   *psTmp[2] = {ppsTmp[0],ppsTmp[3]};

	//MV_CLIP
	xMV mv_min = { 4*( mv_min_ipel.x - 4 ), 4*( mv_min_ipel.y - 4 )};
	xMV mv_max = { 4*( mv_max_ipel.x - 4 ), 4*( mv_max_ipel.y - 4 )};
	xMV tMv;

	double  dBestRDCostY = DOUBLE_MAX;
	double	tRDCost, tRDCost_skip;
	UInt32  tBits,  tSsd;
	UInt32  uiBestSSD, uiBestBits;

	UInt    bSkippedLeft    = !!( (pucModeYInfo1[-1] & FLAG_MASK_SKIP) && (pucModeY[-1+(iSize/MIN_CU_SIZE-1)*(MAX_PU_XY+2)] != MODE_INVALID) );
	UInt    bSkippedAbove   = !!( (pucModeYInfo1[-1*(MAX_PU_XY+1)] & FLAG_MASK_SKIP) && (pucModeY[-1*(MAX_PU_XY+2)+(iSize/MIN_CU_SIZE-1)] != MODE_INVALID) );
		
	//Fast algorithm by lzy, implemented by YannaZhao, init here to avoid warning
	UInt uiCeoffBits = 0;//x64 modify, avoid warning
	UInt uiMvdRate = 0;//x64 modify, avoid warning
	UInt uiMergeRate = 0;//x64 modify, avoid warning
#if !FAST_MD
	//loop over all merge candidates
	for(Int nMrgMvIdx=0; nMrgMvIdx<nMVP; nMrgMvIdx++){
		memcpy(rdoParam->pCabacRdo,  pcCabac_CurrBak, sizeof(xCabac));
		tMv.x  = Clip3(mv_min.x, mv_max.x, sMVP[nMrgMvIdx+MRG_CANDS_START].x);		
		tMv.y  = Clip3(mv_min.y, mv_max.y, sMVP[nMrgMvIdx+MRG_CANDS_START].y);
			
		if(*(UInt32*)(&sMVP[nMrgMvIdx+MRG_CANDS_START].x) != *(UInt32*)(&tMv.x)){
			continue;
		}
			
		xPredInterLumaBlk( pucRefY, pucPredY_tmp, iRefStrideY, iSize, iSize, tMv, psTmp[0] );

		//sub+DCT+Q+iQ+iDCT+REC + Entropy, return true bits
		UInt uiCoeffBits = xPreEncRec(	pCTUCmd,
										pucPixY, 
										pucPredY_tmp, 
										pucRecY_tmp, 
										iSize, 
										nQP, 
										pucModeY, 
										psCoefY_tmp, 
										psTmp,
										&iCbfY_tmp,
										MODE_INVALID,
										SLICE_P, 
										rdoParam
									);
		UInt32 *pCmd0 = pCTUCmd;
		xEncodeBin_pCmd( &pCTUCmd, 1, OFF_MERGE_FLAG_EXT_CTX );//!!(nMrgMvIdx+1)
		xEncodeBin_pCmd( &pCTUCmd, !!nMrgMvIdx, OFF_MERGE_IDX_EXT_CTX );
		if ( nMrgMvIdx ) {
			xEncodeBinsEP_pCmd( &pCTUCmd, ( 1 << nMrgMvIdx ) - 2, nMrgMvIdx );
		}
		int iMrgBits = xGetWrittenCmdRate(pCmd0, pCTUCmd-pCmd0, rdoParam);

#if SSD_USE_TRUE_ASM
		tSsd = xSsdN[iLog2Size-2](pucPixY, MAX_CU_SIZE, pucRecY_tmp,  MAX_CU_SIZE+16);
		UInt32 tSsd_skip = xSsdN[iLog2Size-2]( pucPixY, MAX_CU_SIZE, pucPredY_tmp, MAX_CU_SIZE ) ;
#else
		tSsd			 = xSsd( pucPixY, MAX_CU_SIZE, pucRecY_tmp,  MAX_CU_SIZE+16, iSize ) ;
		UInt32 tSsd_skip = xSsd( pucPixY, MAX_CU_SIZE, pucPredY_tmp, MAX_CU_SIZE, iSize ) ;
#endif
		tRDCost		  = tSsd  + dLambda * (uiCoeffBits + iMrgBits);// + iNoSkipBits);

		UInt32 iSkipBits = 0;
		pCmd0 = pCTUCmd;
		ucSKIPState = rdoParam->pCabacRdo->contextModels[OFF_SKIP_FLAG_CTX + bSkippedLeft + bSkippedAbove];
		xEncodeBin_pCmd( &pCTUCmd, 1, OFF_SKIP_FLAG_CTX + bSkippedLeft + bSkippedAbove);
		tRDCost_skip  = tSsd_skip + dLambda * (iMrgBits + iSkipBits);

		tBits = uiCoeffBits + iMrgBits;
		int  tSKIP = 0;
		if( tRDCost_skip < tRDCost){ //SKIP is selected
			tSKIP = 1;
			tSsd = tSsd_skip;
			tBits = iMrgBits;
			tRDCost = tRDCost_skip;
			memcpy(rdoParam->pCabacRdo,  pcCabac_CurrBak, sizeof(xCabac));
			xEncodeBin_BitEstimate( 1, OFF_SKIP_FLAG_CTX + bSkippedLeft + bSkippedAbove, rdoParam);
		}
		else{
			rdoParam->pCabacRdo->contextModels[OFF_SKIP_FLAG_CTX + bSkippedLeft + bSkippedAbove] = ucSKIPState;
		}

		if((sMVP[nMrgMvIdx+MRG_CANDS_START].x == 0)&&(sMVP[nMrgMvIdx+MRG_CANDS_START].y==0)){
			bZeroMV = 1; //to escape zero MV repeat in ME
		}
			
		if(tRDCost < dBestRDCostY){
			dBestRDCostY = tRDCost;
			nMergeIdx = nMrgMvIdx+1;
			uiBestBits = tBits;
			uiBestSSD = tSsd;
			bSKIP = tSKIP;
			memcpy(ppcCabac[uiDepth], rdoParam->pCabacRdo, sizeof(xCabac));  //saves the best context for current depth into ppcCabac[uiDepth]
			
			bmv.x = tMv.x; bmv.y = tMv.y; 
			for( int i=0; i<iSize; i++ ){
				memcpy( pucPredDstY+i*iSize, pucPredY_tmp+i*MAX_CU_SIZE, iSize);
			}
			for( int i=0; i<iSize; i++ ) {
				memcpy( pucRecYDst+i*iSize, pucRecY_tmp+i*(MAX_CU_SIZE+16), iSize);
			}
			for( int i=0; i<iSize; i++ ) {
				memcpy( psCoefYDst+i*iSize, psCoefY_tmp+i*MAX_CU_SIZE, iSize*sizeof(Int16));
			}
				
			*piCbfYDst = iCbfY_tmp;
			if ( bSKIP){
				memcpy( pucRecYDst, pucPredDstY, iSize*iSize);
				memset( psCoefYDst, 0, iSize*iSize*sizeof(Int16));
				*piCbfYDst = 0;
			}
		}
	}  //end for(Int nMrgMvIdx=0; nMrgMvIdx<nMVP; nMrgMvIdx++)
#else
	UInt32 uiCurrent_Prediction_SSD ;
	UInt32 uiBest_Prediction_SSD = UINT_MAX ;
	Int iBest_MrgMvIdx = -1 ;

	//loop over all merge candidates
	for(Int nMrgMvIdx=0; nMrgMvIdx<nMVP; nMrgMvIdx++)	{
		tMv.x  = Clip3(mv_min.x, mv_max.x, sMVP[nMrgMvIdx+MRG_CANDS_START].x);		
		tMv.y  = Clip3(mv_min.y, mv_max.y, sMVP[nMrgMvIdx+MRG_CANDS_START].y);
		if(*(UInt32*)(&sMVP[nMrgMvIdx+MRG_CANDS_START].x) != *(UInt32*)(&tMv.x)) continue;

		xPredInterLumaBlk( pucRefY, h->nPredYTmp[yCtu][nMrgMvIdx], iRefStrideY, iSize, iSize, tMv, psTmp[0] ) ;

#if SSD_USE_TRUE_ASM
		uiCurrent_Prediction_SSD = xSsdN[iLog2Size-2]( pucPixY, MAX_CU_SIZE, h->nPredYTmp[yCtu][nMrgMvIdx], MAX_CU_SIZE) ;
#else
		uiCurrent_Prediction_SSD = xSsd( pucPixY, MAX_CU_SIZE, h->nPredYTmp[yCtu][nMrgMvIdx], MAX_CU_SIZE, iSize) ;
#endif

		if (uiCurrent_Prediction_SSD < uiBest_Prediction_SSD) {
			uiBest_Prediction_SSD = uiCurrent_Prediction_SSD ;
			iBest_MrgMvIdx = nMrgMvIdx ;
		}
	}  //end for(Int nMrgMvIdx=0; nMrgMvIdx<nMVP; nMrgMvIdx++)

	if (iBest_MrgMvIdx >= 0) {
		tMv.x = sMVP[iBest_MrgMvIdx + MRG_CANDS_START].x ;
		tMv.y = sMVP[iBest_MrgMvIdx + MRG_CANDS_START].y ;

		memcpy(rdoParam->pCabacRdo,  pcCabac_CurrBak, sizeof(xCabac));

		UInt uiCoeffBits = xPreEncRec(	pCTUCmd, 
			pucPixY, 
			h->nPredYTmp[yCtu][iBest_MrgMvIdx],
			pucRecY_tmp, 
			iSize, 
			nQP, 
			pucModeY, 
			psCoefY_tmp, 
			psTmp,
			&iCbfY_tmp,
			MODE_INVALID,
			SLICE_P,
			rdoParam
			);

		UInt32 *pCmd0 = pCTUCmd;
		xEncodeBin_pCmd( &pCTUCmd, 1, OFF_MERGE_FLAG_EXT_CTX );//!!(nMrgMvIdx+1)
		xEncodeBin_pCmd( &pCTUCmd, !!iBest_MrgMvIdx, OFF_MERGE_IDX_EXT_CTX );
		if ( iBest_MrgMvIdx ) {
			xEncodeBinsEP_pCmd( &pCTUCmd, ( 1 << iBest_MrgMvIdx ) - 2, iBest_MrgMvIdx );
		}
		int iMrgBits = xGetWrittenCmdRate(pCmd0, pCTUCmd-pCmd0, rdoParam);
		
#if SSD_USE_TRUE_ASM
		tSsd			 = xSsdN[iLog2Size-2]( pucPixY, MAX_CU_SIZE, pucRecY_tmp,  MAX_CU_SIZE+16 ) ;
		UInt32 tSsd_skip = xSsdN[iLog2Size-2]( pucPixY, MAX_CU_SIZE, h->nPredYTmp[yCtu][iBest_MrgMvIdx], MAX_CU_SIZE ) ;
#else
		tSsd			 = xSsd( pucPixY, MAX_CU_SIZE, pucRecY_tmp,  MAX_CU_SIZE+16, iSize ) ;
		UInt32 tSsd_skip = xSsd( pucPixY, MAX_CU_SIZE, h->nPredYTmp[yCtu][iBest_MrgMvIdx], MAX_CU_SIZE, iSize ) ;
#endif

		tRDCost		  = tSsd	  + dLambda * (uiCoeffBits + iMrgBits);// + iNoSkipBits);
		tRDCost_skip  = tSsd_skip + dLambda * (iMrgBits);// + iSkipBits);
		tBits = uiCoeffBits + iMrgBits;
		int  tSKIP = 0;
		if( tRDCost_skip < tRDCost)	{
			tSKIP = 1;
			tSsd = tSsd_skip;
			tBits = iMrgBits;
			tRDCost = tRDCost_skip;
			memcpy(rdoParam->pCabacRdo,  pcCabac_CurrBak, sizeof(xCabac)); 
		}
	
		xEncodeBin_pCmd( &pCTUCmd, tSKIP, OFF_SKIP_FLAG_CTX + bSkippedLeft + bSkippedAbove);
		if((sMVP[iBest_MrgMvIdx+MRG_CANDS_START].x == 0)&&(sMVP[iBest_MrgMvIdx+MRG_CANDS_START].y==0)) bZeroMV = 1; //to escape zero MV repeat in ME

		if(tRDCost < dBestRDCostY) {
			dBestRDCostY = tRDCost;
			nMergeIdx = iBest_MrgMvIdx+1;

			uiBestBits = tBits;
			uiBestSSD = tSsd;
			bSKIP = tSKIP;
			memcpy(ppcCabac[uiDepth], rdoParam->pCabacRdo, sizeof(xCabac));//saves the best context for current depth into ppcCabac[uiDepth]
			bmv.x = tMv.x; bmv.y = tMv.y; 
			
			for( int i=0; i<iSize; i++ ) {
				memcpy( pucPredDstY+i*iSize, h->nPredYTmp[yCtu][iBest_MrgMvIdx]+i*MAX_CU_SIZE, iSize);
			}
			for( int i=0; i<iSize; i++ ) {
				memcpy( pucRecYDst+i*iSize, pucRecY_tmp+i*(MAX_CU_SIZE+16), iSize);
			}
			for( int i=0; i<iSize; i++ ) {
				memcpy( psCoefYDst+i*iSize, psCoefY_tmp+i*MAX_CU_SIZE, iSize*sizeof(Int16));
			}
			*piCbfYDst = iCbfY_tmp;
			if ( bSKIP)	{
				memcpy( pucRecYDst, pucPredDstY, iSize*iSize);
				memset( psCoefYDst, 0, iSize*iSize*sizeof(Int16));
				*piCbfYDst = 0;
			}
		}

		if (FAST_MD) {
			if ( bSKIP || (iCbfY_tmp == 0) ){
				goto Fast_MD_SubBranch_Mark ;
			}
		}
	}
#endif

	UInt tSad, tRDCost_sad;
	tMv = xMotionSearch( pucPixY, pucRefY, psTmp[0], iRefStrideY, iSize, iSize, &tRDCost_sad, 
		sMVP, &nMVPIdx, bZeroMV, mv_min_ipel, mv_max_ipel, dLambda, &tSad
																		#if MVD_COST_BY_TABLE
																				, mv_cost_table
																		#endif
																				);
	xPredInterLumaBlk( pucRefY, pucPredY_tmp, iRefStrideY, iSize, iSize, tMv, psTmp[0] );

	memcpy(rdoParam->pCabacRdo,  pcCabac_CurrBak, sizeof(xCabac));  //load the best Ctx for the current depth before merge detection encoding

	uiCeoffBits = xPreEncRec(  pCTUCmd, //x64 modify
								pucPixY, 
								pucPredY_tmp, 
								pucRecY_tmp, 
								iSize, 
								nQP, 
								pucModeY, 
								psCoefY_tmp, 
								psTmp,
								&iCbfY_tmp,
								MODE_INVALID,
								SLICE_P,
								rdoParam
							);
#if SSD_USE_TRUE_ASM
	tSsd = xSsdN[iLog2Size-2]( pucPixY, MAX_CU_SIZE, pucRecY_tmp, MAX_CU_SIZE+16 ) ;
#else
	tSsd = xSsd( pucPixY, MAX_CU_SIZE, pucRecY_tmp, MAX_CU_SIZE+16, iSize ) ;
#endif
	//is here tSsd_skip should calculated for cbfY fast?
	////////////////////////////////////////////////////////////////////////// 
	
	//merge detection
	Int idx;//x64 modify
	for( idx=0; idx<nMVP; idx++ ) {
		if ( tMv.x == sMVP[MRG_CANDS_START+idx].x && tMv.y == sMVP[MRG_CANDS_START+idx].y ){
			break;
		}
	}
	nMergeIdx_tmp = (idx != nMVP ? idx+1 : 0); 

	uiMvdRate = 0, uiMergeRate = 0;//x64 modify
	if(nMergeIdx_tmp != 0){ // != 0 means Merge mode
		uiMergeRate = (idx==MRG_MAX_NUM_CANDS - 1)?idx:(idx+1);
	}
	else{
		UInt8 ucMVPIdx = 0;//initialize to zero, it was non-initialized before my modification, YananZhao, 2013-11-14
		if( (abs(sMVP[0].x-sMVP[1].x) + abs(sMVP[0].y - sMVP[1].y)) > 0){   
			UInt diff0 = abs(sMVP[0].x - tMv.x) + abs(sMVP[0].y - tMv.y);
			UInt diff1 = abs(sMVP[1].x - tMv.x) + abs(sMVP[1].y - tMv.y) + 1;
				ucMVPIdx = diff0 <= diff1 ? 0 : 1;
		}

		xMV mvd;
		mvd.x = sMVP[ucMVPIdx].x - tMv.x; mvd.y = sMVP[ucMVPIdx].y - tMv.y;
		UInt32 * pCmd0 = pCTUCmd;
		pCTUCmd += xWriteMVD( pCTUCmd, mvd );
		pCTUCmd += xWriteUnaryMaxSymbol( pCTUCmd, ucMVPIdx, OFF_MVP_IDX_CTX, 1, AMVP_MAX_NUM_CANDS-1 );
		uiMvdRate = xGetWrittenCmdRate(pCmd0, pCTUCmd - pCmd0, rdoParam); 
	}

	tBits   = uiMvdRate + uiMergeRate + uiCeoffBits;
	tRDCost = tSsd + dLambda*tBits;

	if(tRDCost < dBestRDCostY){   //MV by ME is selected
		dBestRDCostY = tRDCost;
		uiBestBits = tBits;
		uiBestSSD  = tSsd;

		nMergeIdx = nMergeIdx_tmp;
		bSKIP = 0;   //skip is not selected
		bmv = tMv;

		xEncodeBin_BitEstimate( bSKIP, OFF_SKIP_FLAG_CTX + bSkippedLeft + bSkippedAbove, rdoParam);
		memcpy(ppcCabac[uiDepth], rdoParam->pCabacRdo, sizeof(xCabac)); 

		for( int i=0; i<iSize; i++ ) {
			memcpy( pucPredDstY+i*iSize, pucPredY_tmp+i*MAX_CU_SIZE, iSize);
		}
		for( int i=0; i<iSize; i++ ) {
			memcpy( pucRecYDst+i*iSize, pucRecY_tmp+i*(MAX_CU_SIZE+16), iSize);
		}
		for( int i=0; i<iSize; i++ ) {
			memcpy( psCoefYDst+i*iSize, psCoefY_tmp+i*MAX_CU_SIZE, iSize*sizeof(Int16));
		}
		*piCbfYDst = iCbfY_tmp;
	}


#if FAST_MD
Fast_MD_SubBranch_Mark:
#endif

	UInt bReplace = ( iSize == MIN_MV_SIZE ? TRUE : FALSE ) || (bSKIP) ;
	if ( (iSize > MIN_MV_SIZE) && (!bSKIP)){

#if FAST_MD
		if (uiDepth != 0)	{
			double dTotalCostCU        = h->dAverageRdCosts[yCtu][xCtu][uiDepth] ;
			double iTotalCountCU       = h->nRdCounts[yCtu][xCtu][uiDepth] ;
			double dTotalCostNeighbor  = 0.0;
			Int    iTotalCountNeighbor = 0;
			double dAverageCost        = -1.0;

			if (yCtu > 0)	{//Top --- FIX ME: Code needs to be improved here in order to support Tiles, ZhengyiLuo, YananZhao, 2013-11-19
				dTotalCostNeighbor += h->dAverageRdCosts[yCtu-1][xCtu][uiDepth];
				iTotalCountNeighbor += h->nRdCounts[yCtu-1][xCtu][uiDepth];
			}
			if ( (yCtu > 0) && (xCtu > 0) )	{//Left Top --- FIX ME: Code needs to be improved here in order to support Tiles
				dTotalCostNeighbor += h->dAverageRdCosts[yCtu-1][xCtu-1][uiDepth];
				iTotalCountNeighbor += h->nRdCounts[yCtu-1][xCtu-1][uiDepth];
			}
			if ( (yCtu > 0) && (xCtu < (Int)(h->PicWidthInCUs - 1)) ) {//Top Right --- FIX ME: Code needs to be improved here in order to support Tiles //x64 modify
				dTotalCostNeighbor += h->dAverageRdCosts[yCtu-1][xCtu+1][uiDepth];
				iTotalCountNeighbor += h->nRdCounts[yCtu-1][xCtu+1][uiDepth];
			}
			if (xCtu > 0)	{//Left --- FIX ME: Code needs to be improved here in order to support Tiles
				dTotalCostNeighbor += h->dAverageRdCosts[yCtu][xCtu-1][uiDepth];
				iTotalCountNeighbor += h->nRdCounts[yCtu][xCtu-1][uiDepth];
			}

			if (iTotalCountCU + iTotalCountNeighbor){
				dAverageCost = (0.6 * dTotalCostCU + 0.4 * dTotalCostNeighbor) / (0.6 * iTotalCountCU + 0.4 * iTotalCountNeighbor);
			}

			if ( (dBestRDCostY < dAverageCost) && (dAverageCost > 0) && (uiDepth != 0) ) {
				bReplace = TRUE ;
				goto Fast_MD_Replace_Mark ;
			}
		}
#endif

		UInt nextDepth = uiDepth + 1;
		UInt nSubSize  = (iSize>>1);
		UInt nSubBlock = (iBlock>>1);
		UInt nSubMEBlock = (iMEBlock>>1);
		UInt uiPartUnitIdx;

		for( uiPartUnitIdx=0; uiPartUnitIdx<4; uiPartUnitIdx++ ){
			UInt sdx = (uiPartUnitIdx &  1);
			UInt sdy = (uiPartUnitIdx >> 1);
			dSubRDCost[uiPartUnitIdx] = xEncDecideCU_Inter( h, pCTUCmd0,
				pucPixY         + sdy * nSubSize  *  MAX_CU_SIZE    + sdx * nSubSize,
				pucRefY         + sdy * nSubSize  *  iRefStrideY    + sdx * nSubSize,
				pucPredY        + sdy * nSubSize  *  MAX_CU_SIZE    + sdx * nSubSize,
				pucRecY 			  + sdy * nSubSize  * (MAX_CU_SIZE+16)+ sdx * nSubSize,
				psCoefY 			  + sdy * nSubSize  *  MAX_CU_SIZE    + sdx * nSubSize,
				pasMV           + sdy * nSubMEBlock*(MAX_MV_XY+2)   + sdx * nSubMEBlock,
				pasMVD          + sdy * nSubMEBlock*(MAX_MV_XY+0)   + sdx * nSubMEBlock,
				paucMVPIdx      + sdy * nSubMEBlock*(MAX_MV_XY+0)   + sdx * nSubMEBlock,
				pucModeY        + sdy * nSubBlock * (MAX_PU_XY+2)   + sdx * nSubBlock,
				pusModeYInfo0   + sdy * nSubBlock * (MAX_PU_XY+0)   + sdx * nSubBlock,
				pucModeYInfo1   + sdy * nSubBlock * (MAX_PU_XY+1)   + sdx * nSubBlock,
				ppsTmp,
				paucPredTmp,
				paucRecYTmp,
				pasCoefYTmp,
				paiCbfYTmp,
				nSubSize,
				nQP,
				dLambda,
				iRefStrideY,
				off_y + sdy * iSize/2,
				off_x + sdx * iSize/2,
				mv_min_ipel,
				mv_max_ipel
				,nextDepth
				,ppcCabac
				,rdoParam
#if MVD_COST_BY_TABLE
				, mv_cost_table
#endif
				,yCtu
				,xCtu
				);

#if FAST_MD
			h->dSumOfRdCosts[yCtu][xCtu][nextDepth] += dSubRDCost[uiPartUnitIdx] ;
			h->nRdCounts[yCtu][xCtu][nextDepth]     += 1 ;
			h->dAverageRdCosts[yCtu][xCtu][nextDepth] = h->dSumOfRdCosts[yCtu][xCtu][nextDepth] / h->nRdCounts[yCtu][xCtu][nextDepth];
#endif
		}

		tRDCost = dSubRDCost[0] + dSubRDCost[1] + dSubRDCost[2] + dSubRDCost[3] ;

#if FAST_MD
		if (uiDepth == 0)	{
			h->dSumOfRdCosts[yCtu][xCtu][0] += dBestRDCostY ;
			h->nRdCounts[yCtu][xCtu][0]     += 1 ;
			h->dAverageRdCosts[yCtu][xCtu][0] = h->dSumOfRdCosts[yCtu][xCtu][0] / h->nRdCounts[yCtu][xCtu][0];
		}
#endif

		if ( tRDCost >= dBestRDCostY ) {
			bReplace = TRUE;
			for(Int jj=uiDepth+1; jj<MAX_CU_DEPTH; jj++){
				memcpy(ppcCabac[jj], ppcCabac[uiDepth], sizeof(xCabac)); 
			}
		}
		else{		
			dBestRDCostY = tRDCost;  
			memcpy(ppcCabac[uiDepth], ppcCabac[uiDepth+1], sizeof(xCabac)); 
		}
	}   // end of next depth

#if FAST_MD
Fast_MD_Replace_Mark:
#endif

	if(bReplace){ 
		assert(ePUMode == 0);
		for(int i=0; i<iSize; i++ ) {
			memcpy( pucPredY+i*MAX_CU_SIZE, pucPredDstY+i*iSize, iSize);
			memcpy( pucRecY +i*(MAX_CU_SIZE+16), pucRecYDst+i*iSize, iSize);
			memcpy( psCoefY +i*MAX_CU_SIZE,		 psCoefYDst+i*iSize, iSize*sizeof(Int16));
		}
		UInt8 ucMVPIdx = 0;
		if( (abs(sMVP[0].x-sMVP[1].x) + abs(sMVP[0].y - sMVP[1].y)) > 0){
			UInt diff0 = abs(sMVP[0].x - bmv.x) + abs(sMVP[0].y - bmv.y);
			UInt diff1 = abs(sMVP[1].x - bmv.x) + abs(sMVP[1].y - bmv.y) + 1;
			ucMVPIdx = diff0 <= diff1 ? 0 : 1;
		}
		for(int i=0; i<(Int)iMEBlock; i++ ) {//x64 modify
			for(int j=0; j<(Int)iMEBlock; j++ ) {//x64 modify
				pasMV[i*(MAX_MV_XY+2)+j] = bmv;
				paucMVPIdx[i*(MAX_MV_XY+0)+j] = ucMVPIdx;
				pasMVD[i*(MAX_MV_XY+0)+j].x = bmv.x - sMVP[ucMVPIdx].x;
				pasMVD[i*(MAX_MV_XY+0)+j].y = bmv.y - sMVP[ucMVPIdx].y;
			}
		}

		for(int i=0; i<(Int)iBlock; i++ ) {//x64 modify
			for(int j=0; j<(Int)iBlock; j++ ) {//x64 modify
				pucModeY[i*(MAX_PU_XY+2) + j] = MODE_VALID;
				pusModeYInfo0[i*MAX_PU_XY + j] = FLAG_MASK_INTER | nMergeIdx|(ePUMode << FLAG_OFF_PUPART);
				int iCbfY = *piCbfYDst;
				if (iSize == 64){
					int iSub32x32Idx = (i>>3)*2+(j>>3);
					int bCbfY = (iCbfY>>iSub32x32Idx) & 1;
					pucModeYInfo1[i*(MAX_PU_XY+1) + j] = ((iLog2Size-2) << FLAG_OFF_SIZE)
						|((iSize == 64)<<B64x64_FLAG_OFF)
						|( off_y>0 ? (1<<VALID_T) : 0) 
						|( off_x>0 ? (1<<VALID_L) : 0)
						|( bCbfY << FLAG_OFF_CBFY)
						|  bSKIP ;
				}
				else{
					int bCbfY = iCbfY & 1;
					pucModeYInfo1[i*(MAX_PU_XY+1) + j] = ((iLog2Size-2) << FLAG_OFF_SIZE)
						|((iSize == 64)<<B64x64_FLAG_OFF)
						|( off_y>0 ? (1<<VALID_T) : 0) 
						|( off_x>0 ? (1<<VALID_L) : 0)
						|( bCbfY << FLAG_OFF_CBFY)
						|  bSKIP ;
				}
			}
		}
	}

	return dBestRDCostY;
}

void xEncInterCTU(
	UInt8  *pucPixY,
	UInt8  *pucPredY,
	UInt8  *pucRecY,
	UInt8  *pucPixU,
	CUInt8 *pucRefU,
	UInt8  *pucPredU,
	UInt8  *pucRecU,
	UInt8  *pucPixV,
	CUInt8 *pucRefV,
	UInt8  *pucPredV,
	UInt8  *pucRecV,
	xMV    *pasMV,
	//xMV    *pasMVD,//x64 modify, remove unused parameter
	//UInt8  *paucMVPIdx,//x64 modify, remove unused parameter
	UInt16 *pusModeYInfo0,
	UInt8  *pucModeYInfo1,
	Int16  *psCoefY,
	Int16  *psCoefU,
	Int16  *psCoefV,
	Int16  *psTmp[3],
	//UInt8  *pucPredTmp[4],//x64 modify, remove unused parameter
	UInt    nQP,
	//double  dLambda,    //x64 modify, remove unused parameter
	Int    iRefStrideY//x64 modify, convert to Int
	)
{
	UInt    nIdx = 0;
	Int    nRefStrideC = iRefStrideY/2;//x64 modify, convert from UInt to Int
	UInt32  uiSum[3], nCbfC, nCbfY;
	UInt    bSkipped = FALSE;
	CInt    nQPC        = xg_aucChromaScale[nQP];

	do {
		Int     dx = getInterleaveBits( nIdx      ); 
		Int     dy = getInterleaveBits( nIdx >> 1 );

		UInt16 *puslModeYInfo0  = &pusModeYInfo0[dy*(MAX_PU_XY    ) + dx];
		UInt8  *puclModeYInfo1  = &pucModeYInfo1[dy*(MAX_PU_XY + 1) + dx];
		UInt    nPredOffsetY   = (dy*4*MAX_CU_SIZE   + dx*4);
		UInt    nPredOffsetUV  = (dy*2*MAX_CU_SIZE/2 + dx*2);
		UInt    nRecOffsetY    = (dy*4*(MAX_CU_SIZE+16)  + dx*4);
		UInt    nRecOffsetUV   = (dy*2*(MAX_CU_SIZE/2+8) + dx*2);

		Int16  *pslCoefY  = psCoefY + nPredOffsetY;
		Int16  *pslCoefU  = psCoefU + nPredOffsetUV;
		Int16  *pslCoefV  = psCoefV + nPredOffsetUV;

		UInt8  *puclPixY  = pucPixY + nPredOffsetY;
		UInt8  *puclPixU  = pucPixU + nPredOffsetUV;
		UInt8  *puclPixV  = pucPixV + nPredOffsetUV;

		UInt8  *puclPredY = pucPredY + nPredOffsetY;
		UInt8  *puclPredU = pucPredU + nPredOffsetUV;
		UInt8  *puclPredV = pucPredV + nPredOffsetUV;

		UInt8  *puclRecY  = pucRecY + nRecOffsetY;
		UInt8  *puclRecU  = pucRecU + nRecOffsetUV;
		UInt8  *puclRecV  = pucRecV + nRecOffsetUV;

		CUInt8  *puclRefU  = pucRefU + (dy*2*nRefStrideC + dx*2);
		CUInt8  *puclRefV  = pucRefV + (dy*2*nRefStrideC + dx*2);

		UInt8   ucInfo          = puclModeYInfo1[0];
		UInt    nSplit          = (ucInfo >> FLAG_OFF_SIZE );
		int     b64x64_Flag     = (puclModeYInfo1[0] >> B64x64_FLAG_OFF)&1&(pusModeYInfo0[0]>>7);//only for inter CTU 
		if (b64x64_Flag == 1){
			nSplit = 4;
		}
		UInt    bSKIP           = (puclModeYInfo1[0])&1&(pusModeYInfo0[0]>>7);
		UInt    nLog2SizeY      = (2 + nSplit);
		UInt    nSizeY          = ( 1 << nLog2SizeY );
		UInt    nSizeC          = ( nSizeY >> 1 );
		UInt    nBlock          = ( nSizeY / MIN_CU_SIZE );
		UInt    nMEBlock        = ( nSizeY / MIN_MV_SIZE );
		UInt    ePUMode			=   (puslModeYInfo0[0] >> FLAG_OFF_PUPART) & 3;  //--WXW
		UInt    nMergeIdx       =   (puslModeYInfo0[0] & FLAG_MASK_MRG);
		
		if(!bSKIP){
			if (ePUMode == SIZE_2Nx2N){
				xMV  bmv        =      pasMV[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*(MAX_MV_XY+2) + dx/(MIN_MV_SIZE/MIN_CU_SIZE)];
				xPredInterChromaBlk( puclRefU, puclPredU, nRefStrideC, nSizeC, nSizeC, bmv, psTmp[0] );
				xPredInterChromaBlk( puclRefV, puclPredV, nRefStrideC, nSizeC, nSizeC, bmv, psTmp[0] );
				
				if (b64x64_Flag == 1){   //split one 64x64 CU into 4 32x32 TUs
					UInt32  nCbf_Local = 0;
					for(int j=0; j<2; j++) {
						for(int i=0; i<2; i++){
							UInt    nPredOffsetY   = (j*nSizeY/2*MAX_CU_SIZE		+ i*nSizeY/2);
							UInt    nPredOffsetUV  = (j*nSizeC/2*MAX_CU_SIZE/2		+ i*nSizeC/2);
							UInt    nRecOffsetY    = (j*nSizeY/2*(MAX_CU_SIZE+16)   + i*nSizeY/2);
							UInt    nRecOffsetUV   = (j*nSizeC/2*(MAX_CU_SIZE/2+8)	+ i*nSizeC/2);
							UInt    nInfo0Offset   = (j*nBlock/2*MAX_PU_XY			+ i*nBlock/2);
							UInt    nInfo1Offset   = (j*nBlock/2*(MAX_PU_XY+1)      + i*nBlock/2);

							uiSum[1] = xSubDctQuantRec(puclPixU+nPredOffsetUV, puclPredU+nPredOffsetUV, puclRecU+nRecOffsetUV, pslCoefU+nPredOffsetUV, nSizeC/2, psTmp, TRUE,  nQPC, MODE_INVALID, SLICE_P);
							uiSum[2] = xSubDctQuantRec(puclPixV+nPredOffsetUV, puclPredV+nPredOffsetUV, puclRecV+nRecOffsetUV, pslCoefV+nPredOffsetUV, nSizeC/2, psTmp, TRUE,  nQPC, MODE_INVALID, SLICE_P);
							nCbfY = (puclModeYInfo1[nInfo1Offset]>>FLAG_OFF_CBFY) & 1;
							nCbfC = ( (uiSum[2] ? 2 : 0) | (uiSum[1] ? 1 : 0) );
							nCbf_Local |= (nCbfY | nCbfC);

							for(int ii=0; ii<(Int)nBlock/2; ii++) {//x64 modify
								for(int jj=0; jj<(Int)nBlock/2; jj++) {//x64 modify
									puslModeYInfo0[ii*MAX_PU_XY + jj + nInfo0Offset]	 = puslModeYInfo0[nInfo0Offset] | (nCbfC << FLAG_OFF_CBFC);
								}
							}
						}
					}
				
					bSkipped = ( !!nMergeIdx && (nCbf_Local == 0) ); 
					if(bSkipped) {
						for(int ii=0; ii<(Int)nBlock; ii++) {//x64 modify
							for(int jj=0; jj<(Int)nBlock; jj++) {//x64 modify
								puclModeYInfo1[ii*(MAX_PU_XY+1) + jj] |= FLAG_MASK_SKIP;
							}
						}
					}
				}
        else {
					uiSum[1] = xSubDctQuantRec(puclPixU, puclPredU, puclRecU, pslCoefU, nSizeC, psTmp, TRUE,  nQPC, MODE_INVALID, SLICE_P);
					uiSum[2] = xSubDctQuantRec(puclPixV, puclPredV, puclRecV, pslCoefV, nSizeC, psTmp, TRUE,  nQPC, MODE_INVALID, SLICE_P);
					nCbfY = ( puclModeYInfo1[0] >> FLAG_OFF_CBFY) & 1;
					nCbfC = ( (uiSum[2] ? 2 : 0) | (uiSum[1] ? 1 : 0) );
					bSkipped = ( !!nMergeIdx && ((nCbfY | nCbfC) == 0) );  //--WXW
					
					for(int ii=0; ii<(Int)nBlock; ii++) {//x64 modify
						for(int jj=0; jj<(Int)nBlock; jj++) {//x64 modify
							puslModeYInfo0[ii*MAX_PU_XY + jj] = puslModeYInfo0[0]|((FLAG_MASK_INTER | (nCbfC << FLAG_OFF_CBFC)));
							puclModeYInfo1[ii*(MAX_PU_XY+1) + jj] = puclModeYInfo1[0] | (bSkipped ? FLAG_MASK_SKIP : 0) ;

						}
					}
				}
			}//if (ePUMode == SIZE_2Nx2N)
		}//if(!bSkip)
		else{
			xMV bmv = pasMV[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*(MAX_MV_XY+2) + dx/(MIN_MV_SIZE/MIN_CU_SIZE)];
			xPredInterChromaBlk( puclRefU, puclPredU, nRefStrideC, nSizeC, nSizeC, bmv, psTmp[0] );
			xPredInterChromaBlk( puclRefV, puclPredV, nRefStrideC, nSizeC, nSizeC, bmv, psTmp[0] );
			for(Int i=0; i<(Int)nSizeC; i++)	{//x64 modify
				memcpy(puclRecU + i*(MAX_CU_SIZE + 16)/2, puclPredU + i*MAX_CU_SIZE/2, nSizeC);
				memset(pslCoefU + i*MAX_CU_SIZE/2, 0, nSizeC*sizeof(Int16));
			}
			for(Int i=0; i<(Int)nSizeC; i++)	{//x64 modify
				memcpy(puclRecV + i*(MAX_CU_SIZE + 16)/2, puclPredV + i*MAX_CU_SIZE/2, nSizeC);
				memset(pslCoefV + i*MAX_CU_SIZE/2, 0, nSizeC*sizeof(Int16));
			}
		}
		nIdx += (nSizeY * nSizeY / 16);
	} while( nIdx != (MAX_PU_XY * MAX_PU_XY) );  // TODO: Can't Support max CUSize other than MAX_CU_SIZE
}

#if MV_CLIP
static void xCuMvClip( X265_t *h, CInt x, CInt y, CInt iCUWidth, CInt iCUHeight )//x64 modify, change type to Int: x, y, nCUWidth, nCUHeight
{
	Int i_fmv_range = MAX_IMV_VAL; 
	
	/* Calculate max allowed MV range */
#define CLIP_FMV(xx) Clip3(-i_fmv_range, i_fmv_range,  xx)
	h->mv_min_ipel.x = CLIP_FMV( -MAX_CU_SIZE*x - (PAD_YSIZE - 16) );  //HERE (- 4) is for boundary interpolate; (-6) can detection mv only at the org point
	h->mv_max_ipel.x = CLIP_FMV(  MAX_CU_SIZE*( iCUWidth - x ) + (PAD_YSIZE - MAX_CU_SIZE) - 16 );

	h->mv_min.x = 4*( h->mv_min_ipel.x - 4 );
	h->mv_max.x = 4*( h->mv_max_ipel.x + 4 );
	if( x == 0) {
		h->mv_min_ipel.y = CLIP_FMV( -MAX_CU_SIZE*y  - (PAD_YSIZE - 16)  );
		h->mv_max_ipel.y = CLIP_FMV(  MAX_CU_SIZE*( iCUHeight - y ) + (PAD_YSIZE - MAX_CU_SIZE) - 16 );
		h->mv_min.y = 4*( h->mv_min_ipel.y - 4 );
		h->mv_max.y = 4*( h->mv_max_ipel.y + 4 );
	}
#undef CLIP_FMV
}
#endif

#if MVD_COST_BY_TABLE
#define BC_MAX_MV 0x8000

void CalculateLogs(float * s_bitsizes)
{
	s_bitsizes[0] = 0.718f;
	float log2_2 = 2.0f / log(2.0f);  // 2 x 1/log(2)
	for (int i = 1; i <= BC_MAX_MV; i++){
		s_bitsizes[i] = log((float)(i + 1)) * log2_2 + 1.718f;
	}
}

void initMvdCostTable(double lambda, UInt16 *mvd_cost_table)//x64 modify, remove unused parameter
{
	float s_bitsizes[BC_MAX_MV + 1];
	CalculateLogs(s_bitsizes);
	// estimate same cost for negative and positive MVD
	for (int i = 0; i < BC_MAX_MV; i++) {
		mvd_cost_table[i]  = 
		mvd_cost_table[-i] = (UInt16)MIN(s_bitsizes[i] * lambda + 0.5f, (1 << 16) - 1);
	}
}
#endif


// ***************************************************************************
// * Intra Prediction Functions
// ***************************************************************************
void xPaddingRef( UInt8 *pucRef, CUInt8 ucValids, CInt32 nBlkOffset[6] )
{
    Int    i, n;//x64
    UInt8   ucPadding;
    CInt nValid = 5;

    // Padding from Right to Left
    for( n=0; n<nValid; n++ ) {
        if( ucValids & (1 << n) )
            break;
    }
    ucPadding = pucRef[nBlkOffset[n]];
    for( i=0; i<nBlkOffset[n]; i++ ) {
        pucRef[i] = ucPadding;
    }

    // Padding from Left to Right
    for( ; n<nValid; n++ ) {
        if( !(ucValids & (1 << n)) ) {
            assert( n > 0 );
            CInt nBlkAddr = nBlkOffset[n];//x64
            CInt nBlkSize = nBlkOffset[n + 1] - nBlkOffset[n];//x64
            ucPadding = pucRef[nBlkAddr - 1];
            for( i=0; i<nBlkSize; i++ ) {
                pucRef[nBlkAddr + i] = ucPadding;
            }
        }
    }
}

void xFilterRef( UInt8 *pucDst, UInt8 *pucSrc, CInt32 iSize, CUInt bStrongFilter )//x64 modify, convert to Int
{
    int i;
    UInt8   ucBL = pucSrc[0];
    UInt8   ucTL = pucSrc[2*iSize];
    UInt8   ucTR = pucSrc[4*iSize];
    UInt32  threshold = 1 << (8 - 5);
    UInt    bilinearLeft = abs(ucBL + ucTL - 2*pucSrc[  iSize]) < (Int)threshold;//x64
    UInt    bilinearTop  = abs(ucTL + ucTR - 2*pucSrc[3*iSize]) < (Int)threshold;//x64

    if ( bStrongFilter && (iSize == 32) && (bilinearLeft && bilinearTop) ) {
        CInt nShift = xLog2( iSize - 1 ) + 1;
        pucDst[0      ] = pucSrc[0];
        pucDst[2*iSize] = pucSrc[2*iSize];
        pucDst[4*iSize] = pucSrc[4*iSize];

        for (i = 1; i < 2*(Int)iSize; i++) {//x64 modify, convert to Int
            pucDst[          i] = ((2*iSize-i)*ucBL + i*ucTL + iSize) >> nShift;
            pucDst[2*iSize + i] = ((2*iSize-i)*ucTL + i*ucTR + iSize) >> nShift;
        }
    }
    else {
        // Filter with [1 2 1]
        pucDst[0      ] = pucSrc[0];
        pucDst[4*iSize] = pucSrc[4*iSize];
        for( i=1; i<4*(int)iSize; i++ ) {
            pucDst[i] = (pucSrc[i - 1] + 2 * pucSrc[i] + pucSrc[i + 1] + 2) >> 2;
        }
    }
}

static UInt32 xWriteIntraDirLumaAngGroup(UInt32 *pCmd, UInt nGroup, UInt nPredIdx[4]);

double xEncDecideCU_Intra(
	UInt32  *pCTUCmd,
	xCabac *ppcCabac[MAX_CU_DEPTH+1],
	UInt8  *paucRecYTmp[MAX_CU_DEPTH],
	Int16  *pasCoefYTmp[MAX_CU_DEPTH],
	Int32  *paiCbfYTmp[MAX_CU_DEPTH],
	UInt    uiDepth,
	xRdoParam *rdoParam,
    UInt8  *pucPixY,
    UInt8  *pucRecY,
    UInt8  *pucRefTopY,
    UInt8  *pucModeY,
    UInt16 *pusModeYInfo0,
    UInt8  *pucModeYInfo1,
    UInt8  *pucPredY,
    Int16  *psCoefY,
    Int16  *ppsTmp[4],
    UInt8  *paucPredTmp[MAX_CU_DEPTH],
    Int32   iSize,//x64 modify, change type to Int
    UInt    dy,
    UInt    nQP,
   const double   dLambda,
   CUInt    bStrongIntraSmoothing
)
{
	double  dSqrtLambda = sqrt(dLambda) ;  
	UInt    nLog2Size = xLog2( iSize - 1 );

	UInt32 *pCTUCmd0 = pCTUCmd;
	memcpy(rdoParam->pCabacRdo, ppcCabac[uiDepth], sizeof(xCabac)); 
	
	UInt8   *pucPredY_tmp = (UInt8 *)ppsTmp[1];
	UInt8   *pucRecY_tmp  = (UInt8 *)(ppsTmp[1]+MAX_CU_SIZE*MAX_CU_SIZE/2);
	Int16   *psCoefY_tmp = ppsTmp[2];  //psTmp[0] for temp use
	Int16   *psTmp[2] = {ppsTmp[0],ppsTmp[3]};
	Int32   iCbfY_tmp;

	UInt8  *pucRecYDst =  paucRecYTmp[nLog2Size - 2]; 
	Int16  *psCoefYDst =  pasCoefYTmp[nLog2Size - 2]; 
	int	   *piCbfYDst  =  paiCbfYTmp[nLog2Size - 2];

  Int    iBlock = iSize / MIN_CU_SIZE;//x64 modify, change type from UInt to Int
  CInt32  iBlkOffsetY[6] = {0, iSize,  2*iSize,  2*iSize +1, 3*iSize +1, 4*iSize +1};//x64 modify, convert to int
  double  dBestRDCostY = DOUBLE_MAX ;   
  UInt    nBestModeY = 0;
  UInt    nMode;
 	Int32	  iPredIdx=0;
  UInt8   aucRef[2][4 * MAX_CU_SIZE + 1];
  UInt8   ucValids = 0;
  UInt8   aucMode[5];
  UInt8   aucMostModeY[3];
  //Int32   i; //x64 modify, declare only when need it
  UInt32  uiSumY;

  double  dRDCost = DOUBLE_MAX, dSubRDCost[4] ;  
  UInt8  *pucDstY[2] = { paucPredTmp[nLog2Size-2], paucPredTmp[nLog2Size-2]+iSize*iSize };
  UInt    nTmpIdx = 0;

	if(iSize < 64) {
		aucMode[VALID_LB] = pucModeY[iBlock*(MAX_PU_XY+2) - 1     ];//x64 modify, force to Int
		aucMode[VALID_L ] = pucModeY[                     - 1     ];
		aucMode[VALID_LT] = pucModeY[-1    *(MAX_PU_XY+2) - 1     ];//x64 modify, force to Int
		aucMode[VALID_T ] = pucModeY[-1    *(MAX_PU_XY+2)         ];//x64 modify, force to Int
		aucMode[VALID_TR] = pucModeY[-1    *(MAX_PU_XY+2) + iBlock];//x64 modify, force to Int
		ucValids |= (aucMode[VALID_LB] != MODE_INVALID) ? (1 << VALID_LB) : 0;
		ucValids |= (aucMode[VALID_L ] != MODE_INVALID) ? (1 << VALID_L ) : 0;
		ucValids |= (aucMode[VALID_LT] != MODE_INVALID) ? (1 << VALID_LT) : 0;
		ucValids |= (aucMode[VALID_T ] != MODE_INVALID) ? (1 << VALID_T ) : 0;
		ucValids |= (aucMode[VALID_TR] != MODE_INVALID) ? (1 << VALID_TR) : 0;

		// Most Mode
		UInt8 ucLeftMode = ucValids & (1 << VALID_L)       ? aucMode[VALID_L] : DC_IDX;
		UInt8 ucTopMode  = ucValids & (1 << VALID_T) && dy ? aucMode[VALID_T] : DC_IDX;

		if( ucLeftMode == ucTopMode ) {
			if( ucLeftMode > 1 ) {
				// angular modes
				aucMostModeY[0] = ucLeftMode;
				aucMostModeY[1] = ((ucLeftMode + 29) % 32) + 2;
				aucMostModeY[2] = ((ucLeftMode -  1) % 32) + 2;
			}
			else {
				// non angular modes
				aucMostModeY[0] = PLANAR_IDX;
				aucMostModeY[1] = DC_IDX;
				aucMostModeY[2] = VER_IDX;
			}
		}
		else {
			aucMostModeY[0] = ucLeftMode;
			aucMostModeY[1] = ucTopMode;
			if( ucLeftMode && ucTopMode ){
				aucMostModeY[2] = PLANAR_IDX;
			}
			else{
				aucMostModeY[2] = ( ucLeftMode + ucTopMode ) < 2 ? VER_IDX : DC_IDX;
			}
		}

		// Generator Reference Samples
		if( ucValids == 0 ) {
			memset( aucRef, 0x80, sizeof(aucRef) );
		}
		else {
			// Copy Left Bottom and Left
			for(Int i=0; i<iSize*2; i++ ) {//x64 modify
				aucRef[0][i] = pucRecY[-1+(iSize*2-1-i)*(Int)(MAX_CU_SIZE+16)];//x64 modify, force to Int
			}
			// Copy Top and Top Right
			memcpy( &aucRef[0][iSize*2], (dy == 0 ? pucRefTopY-1 : &pucRecY[-1*(Int)(MAX_CU_SIZE+16)-1] ), 2*iSize+1 );//x64 modify, force to Int
			// Padding
			xPaddingRef( aucRef[0], ucValids, iBlkOffsetY );
			xFilterRef( aucRef[1], aucRef[0], iSize, bStrongIntraSmoothing );
		}
		
		double dSATDCost = 0.0; 
		double dBestSATDCost = DOUBLE_MAX;
		for( nMode=0; nMode<35; nMode++ ) {
			UInt    bFilter = xg_aucIntraFilterType[nLog2Size-2][nMode];
			UInt8  *pucRefY = aucRef[bFilter];
			xEncIntraPred( pucDstY[nTmpIdx], pucRefY, iSize, nMode, iSize, (iSize != 32) );

			if( nMode == aucMostModeY[0] ){
				dSATDCost = 1 * dSqrtLambda;
			}
			else if( nMode == aucMostModeY[1] || nMode == aucMostModeY[2] ){
				dSATDCost = 2 * dSqrtLambda;
			}
			else{
				dSATDCost = 3 * dSqrtLambda;
			}

			UInt32 ui32Ret = xSatdN[nLog2Size-2]( pucPixY, MAX_CU_SIZE, pucDstY[nTmpIdx], iSize );
			dSATDCost += (double)(ui32Ret) ;

			if( dSATDCost < dBestSATDCost ) {
				dBestSATDCost = dSATDCost ;
				nBestModeY = nMode;
				nTmpIdx   ^= 1;
			}
		}

		// Map Mode into index
		if( nBestModeY == aucMostModeY[0] ){
			iPredIdx = 0;
		}
		else if( nBestModeY == aucMostModeY[1] ){
			iPredIdx = 1;
		}
		else if( nBestModeY == aucMostModeY[2] ){
			iPredIdx = 2;
		}
		else {
			// Sort MostModeY
			UInt8 tmp;
	#define SwapMode(a, b) \
		tmp = aucMostModeY[(a)]; \
		aucMostModeY[(a)] = aucMostModeY[(b)]; \
			aucMostModeY[(b)] = tmp

			if( aucMostModeY[0] > aucMostModeY[1] ) {
				SwapMode(0, 1);
			}
			if( aucMostModeY[0] > aucMostModeY[2] ) {
				SwapMode(0, 2);
			}
			if( aucMostModeY[1] > aucMostModeY[2] ) {
				SwapMode(1, 2);
			}
	#undef SwapMode
			iPredIdx = nBestModeY;
			iPredIdx -= (iPredIdx > aucMostModeY[2]);
			iPredIdx -= (iPredIdx > aucMostModeY[1]);
			iPredIdx -= (iPredIdx > aucMostModeY[0]);
			iPredIdx += 3;
		}

		nTmpIdx ^= 1;
		for(Int i=0; i<iSize; i++ ) {//x64 modify
			memcpy( pucPredY_tmp + i*MAX_CU_SIZE, pucDstY[nTmpIdx] + i*iSize, iSize );
		}

		UInt uiCoeffBits = xPreEncRec(	pCTUCmd,
										pucPixY, 
										pucPredY_tmp, 
										pucRecY_tmp, 
										iSize, 
										nQP, 
										pucModeY, 
										psCoefY_tmp, 
										psTmp,
										&iCbfY_tmp,
										nBestModeY,
										SLICE_I,
										rdoParam
									);

		for( int i=0; i<iSize; i++ ){
			memcpy( pucRecYDst+i*iSize, pucRecY_tmp+i*(MAX_CU_SIZE+16), iSize);
		}
		for( int i=0; i<iSize; i++ ){
			memcpy( psCoefYDst+i*iSize, psCoefY_tmp+i*MAX_CU_SIZE, iSize*sizeof(Int16));
		}
		*piCbfYDst = iCbfY_tmp;

#if SSD_USE_TRUE_ASM
		Int iSsd	= xSsdN[nLog2Size-2]( pucPixY, MAX_CU_SIZE, pucRecY_tmp,  MAX_CU_SIZE+16 ) ;//x64 modify, convert to Int
#else
		Int iSsd	= xSsd( pucPixY, MAX_CU_SIZE, pucRecY_tmp,  MAX_CU_SIZE+16, iSize ) ;//x64 modify, convert to Int
#endif

 	  //encodes syntax except Coeff
		UInt32 *pCmd0 = pCTUCmd;
		UInt nSplit = (nLog2Size - 2);
		if (  nSplit <= 1 ) {
			xEncodeBin_pCmd(&pCTUCmd, (nSplit != 0 ? 1 : 0), OFF_PART_SIZE_CTX );
		}

		UInt nPredIdx[4] = {iPredIdx};
		pCTUCmd += xWriteIntraDirLumaAngGroup( pCTUCmd, 1, nPredIdx );
		UInt uiModesBits = xGetWrittenCmdRate(pCmd0, pCTUCmd-pCmd0, rdoParam);

		dBestRDCostY     = iSsd + dLambda * (double)(uiCoeffBits + uiModesBits);//dBestSATDCost;//
		memcpy(ppcCabac[uiDepth], rdoParam->pCabacRdo, sizeof(xCabac));  //save one depth context
	}

	// Check and Split
  UInt bReplace = ( iSize == MIN_CU_SIZE ? TRUE : FALSE );
  if ( (iSize > MIN_CU_SIZE) && (dBestRDCostY >= 0) ) {   
		Int iSubSize  = (iSize>>1);//x64 modify, convert to int
		Int iSubBlock = (iBlock>>1);//x64 modify, convert to int
		//UInt uiPartUnitIdx;//x64 modify, convert to int
		for(Int iPartUnitIdx=0; iPartUnitIdx<4; iPartUnitIdx++ ) {
			Int sdx = (iPartUnitIdx &  1);//x64 modify, convert to int
			Int sdy = (iPartUnitIdx >> 1);//x64 modify, convert to int
			dSubRDCost[iPartUnitIdx] = xEncDecideCU_Intra( 
				pCTUCmd0,
				ppcCabac,
				paucRecYTmp,
				pasCoefYTmp,
				paiCbfYTmp,
				uiDepth + 1,
				rdoParam,
				pucPixY       + sdy * iSubSize  * MAX_CU_SIZE    + sdx * iSubSize,
				pucRecY       + sdy * iSubSize  *(MAX_CU_SIZE+16)+ sdx * iSubSize,
				pucRefTopY    + sdy * iSubSize  * MAX_CU_SIZE    + sdx * iSubSize,
				pucModeY      + sdy * iSubBlock *(MAX_PU_XY+2)   + sdx * iSubBlock,
				pusModeYInfo0 + sdy * iSubBlock * MAX_PU_XY      + sdx * iSubBlock,
				pucModeYInfo1 + sdy * iSubBlock *(MAX_PU_XY+1)   + sdx * iSubBlock,
				pucPredY      + sdy * iSubSize  * MAX_CU_SIZE    + sdx * iSubSize,
				psCoefY       + sdy * iSubSize  * MAX_CU_SIZE    + sdx * iSubSize,
				ppsTmp,
				paucPredTmp,
				iSubSize,
				dy + sdy * iSubSize,
				nQP,
				dLambda,  
				bStrongIntraSmoothing );
		}
		dRDCost = dSubRDCost[0] + dSubRDCost[1] + dSubRDCost[2] + dSubRDCost[3] + 4;

		if ( dRDCost >= dBestRDCostY ) { 
			bReplace = TRUE;
			for(Int jj=uiDepth+1; jj<MAX_CU_DEPTH; jj++){
				memcpy(ppcCabac[jj], ppcCabac[uiDepth], sizeof(xCabac)); 
			}
		}
		else {
			memcpy( ppcCabac[uiDepth], ppcCabac[uiDepth+1], sizeof(xCabac)); 
		}
  }

  if ( bReplace ) {
		assert(iSize<64);
   
		UInt    bFilter = xg_aucIntraFilterType[nLog2Size-2][nBestModeY];
    UInt8  *pucRefY = aucRef[bFilter];
    nTmpIdx ^= 1;
    for(Int i=0; i<iSize; i++ ) {//x64 modify
        memcpy( pucPredY + i*MAX_CU_SIZE, pucDstY[nTmpIdx] + i*iSize, iSize );
    }

		for(Int i=0; i<iSize; i++)	{//x64 modify
			memcpy( pucRecY +i*(MAX_CU_SIZE+16), pucRecYDst+i*iSize, iSize);
			memcpy( psCoefY +i*MAX_CU_SIZE,		 psCoefYDst+i*iSize, iSize*sizeof(Int16));
		}
			
		// Update best info
		pusModeYInfo0[0]  = iPredIdx;
		assert( iPredIdx < NUM_INTRA_MODE+3 );
		pucModeYInfo1[0]  = ((ucValids << FLAG_OFF_VALIDS) & 0x3f)  |  (*piCbfYDst != 0 ? FLAG_MASK_CBFY : 0x00)	 | ((nLog2Size-2) << FLAG_OFF_SIZE);

		for(Int i=0; i<iBlock; i++ ) {//x64 modify
				memset( &pucModeY[i*(MAX_PU_XY+2)], nBestModeY, iBlock );
				memset( &pucModeYInfo1[i*(MAX_PU_XY+1)], pucModeYInfo1[0], iBlock );
		}
		for (int j=0; j<iBlock; j++){
			for ( int i=0; i<iBlock; i++)	{
				pucModeYInfo1[j*(MAX_PU_XY+1) + i] = pucModeYInfo1[0];
			}
		}
  }
  else {
    assert( iSize > 4 && iSize <= MAX_CU_SIZE );
  }

	dBestRDCostY = MIN(dBestRDCostY, dRDCost) ;

  return dBestRDCostY; 
}

// Get Interleave Bits
// Input:  [7 6 5 4 3 2 1 0]
// Output: [        6 4 2 0]
UInt getInterleaveBits( UInt8 x )
{
    UInt r = 0;
#if 1
    r = ( ((x * 0x01010101) & 0x40100401) * 0x8040201 ) >> 27;
#else
    r |= ((x   ) & 1);
    r |= ((x>>1) & 2);
    r |= ((x>>2) & 4);
    r |= ((x>>3) & 8);
#endif
    return r;
}

void xEncodeChromaCU(
    UInt8  *pucPixU,
    UInt8  *pucPixV,
    UInt8  *pucRecY,
    UInt8  *pucRecU,
    UInt8  *pucRecV,
    UInt8  *pucPredU,
    UInt8  *pucPredV,
    UInt8  *pucRefTopY,
    UInt8  *pucRefTopU,
    UInt8  *pucRefTopV,
    UInt8  *pucModeY,
    UInt16 *pucModeYInfo0,
    UInt8  *pucModeYInfo1,
    Int16  *psCoefU,
    Int16  *psCoefV,
    Int16  *psTmp[3],
    UInt8  *pucPredTmp,
    UInt    nQPC
   //, double  dLambda  //x64 modify, remove unused parameter 
)
{
    UInt    nIdx = 0;
    UInt    nMostModeC[NUM_CHROMA_MODE];

    // GetAllowedChromaMode
    nMostModeC[0] = PLANAR_IDX;
    nMostModeC[1] = VER_IDX;
    nMostModeC[2] = HOR_IDX;
    nMostModeC[3] = DC_IDX;

    do {
         Int    dx = getInterleaveBits( nIdx      ); // Position to Luma
         Int    dy = getInterleaveBits( nIdx >> 1 );
        UInt    nBestModeY = pucModeY[ (MAX_PU_XY+2) * dy + dx ];
         Int    iSizeY = ( 1 << (2 + (pucModeYInfo1[(MAX_PU_XY+1) * dy + dx] >> FLAG_OFF_SIZE)) );//x64 modify, convert to Int
         Int    iSize  = (iSizeY == 4 ? 4 : (iSizeY >> 1));                                       //x64 modify, convert to Int
        UInt8   ucValids = (pucModeYInfo1[ (MAX_PU_XY+1) * dy + dx ] >> FLAG_OFF_VALIDS) & 0x1F;
        UInt    nLog2Size = xLog2( iSize - 1 );
        UInt8  *pucDstU[2] = {pucPredTmp, pucPredTmp + MAX_CU_SIZE*MAX_CU_SIZE/2};
        UInt8  *pucDstV[2] = {pucDstU[0] + MAX_CU_SIZE/2, pucDstU[1] + MAX_CU_SIZE/2};
        UInt    nPredOffset = (dy*2*MAX_CU_SIZE/2 + dx*2);
        UInt    nRecOffsetC = (dy*2*(MAX_CU_SIZE/2+8) + dx*2);
        UInt8  *puclPixU = pucPixU + nPredOffset;
        UInt8  *puclPixV = pucPixV + nPredOffset;
        UInt8  *puclPredU = pucPredU + nPredOffset;
        UInt8  *puclPredV = pucPredV + nPredOffset;
        UInt8  *puclRecU = pucRecU + nRecOffsetC;
        UInt8  *puclRecV = pucRecV + nRecOffsetC;
        UInt8  *puclRecY = pucRecY + ( dy*4*(MAX_CU_SIZE+16) + dx*4 );
        UInt8  *puclTopY = (dy == 0) ? (pucRefTopY + dx * 4) : (puclRecY - (MAX_CU_SIZE+16));
        UInt8   aucRef[2][4 * MAX_CU_SIZE/2 + 1];   // 0:Left, 1:Top
        UInt8  *paucRefU = aucRef[0];
        UInt8  *paucRefV = aucRef[1];
       CInt32   nBlkOffsetC[6] = {0, iSize,  2*iSize,  2*iSize +1, 3*iSize +1, 4*iSize +1};
        UInt    nTmpIdx = 0;
        UInt    nMode, realModeC, nBestModeC=0, nCbfC;
        UInt32  uiBestSadC, uiSumU, uiSumV;
        UInt32 uiSumSad;
        UInt32 uiSad[2];
        Int     i;

        // Fixup Valids
        if ( iSizeY == 4 ) {
            // Clear LeftBottom and TopRight
            ucValids &= ~0x11;
            // Use LeftBottom from (+1,0)
            ucValids |= (pucModeYInfo1[ (MAX_PU_XY+1) * (dy+1) + dx     ] >> FLAG_OFF_VALIDS) & 0x01;
            // Use TopRight from (0,+1)
            ucValids |= (pucModeYInfo1[ (MAX_PU_XY+1) *  dy    + dx + 1 ] >> FLAG_OFF_VALIDS) & 0x10;
        }

        // Generator Reference Samples
        UInt bL = !!(ucValids & (1 << VALID_L));
        UInt bT = !!(ucValids & (1 << VALID_T));
        if( ucValids == 0 ) {
            memset( aucRef,    0x80, sizeof(aucRef   ) );
        }
        else {
            // Copy Left Bottom and Left
            for( i=0; i<(int)iSize*2; i++ ) {
                paucRefU[i] = puclRecU[-1+(iSize*2-1-i)*(Int)(MAX_CU_SIZE/2+8)];//x64 modify, convert to Int
                paucRefV[i] = puclRecV[-1+(iSize*2-1-i)*(Int)(MAX_CU_SIZE/2+8)];//x64 modify, convert to Int
            }
            // Copy Top and Top Right
            memcpy( &paucRefU[iSize*2], (dy == 0 ? pucRefTopU+2*dx-1 : &puclRecU[-1*(Int)(MAX_CU_SIZE/2+8)-1] ), 2*iSize+1 );//x64 modify, convert to Int
            memcpy( &paucRefV[iSize*2], (dy == 0 ? pucRefTopV+2*dx-1 : &puclRecV[-1*(Int)(MAX_CU_SIZE/2+8)-1] ), 2*iSize+1 );//x64 modify, convert to Int
            // Padding
            xPaddingRef( paucRefU, ucValids, nBlkOffsetC );
            xPaddingRef( paucRefV, ucValids, nBlkOffsetC );
        }

        // GetAllowedChromaMode
        nMostModeC[CHROMA_MODE_DM] = nBestModeY;

        uiBestSadC = MAX_SAD;
        for( nMode=0; nMode<=CHROMA_MODE_DM; nMode++ ) {
            realModeC = nMostModeC[nMode];

            if ( (nMode < 4) && (realModeC == nBestModeY) )
                realModeC = 34;

            xEncIntraPred(
                pucDstU[nTmpIdx],
                paucRefU,
                MAX_CU_SIZE,
                realModeC,
                iSize,
                FALSE
            );
            xEncIntraPred(
                pucDstV[nTmpIdx],
                paucRefV,
                MAX_CU_SIZE,
                realModeC,
                iSize,
                FALSE
            );

            uiSad[0] = xSatdN[nLog2Size-2]( puclPixU, MAX_CU_SIZE/2, pucDstU[nTmpIdx], MAX_CU_SIZE );
            uiSad[1] = xSatdN[nLog2Size-2]( puclPixV, MAX_CU_SIZE/2, pucDstV[nTmpIdx], MAX_CU_SIZE );

            uiSumSad = uiSad[0] + uiSad[1];
            if( uiSumSad < uiBestSadC ) {
                uiBestSadC = uiSumSad;
                nBestModeC = nMode;
                nTmpIdx   ^= 1;
            }
        }
        realModeC = nMostModeC[nBestModeC];
        if ( (nBestModeC < 4) && (realModeC == nBestModeY) )
            realModeC = 34;

        // Copy Best Cb/Cr Pred Pixel into Cache
        nTmpIdx ^= 1;
        for( i=0; i<(int)iSize; i++ ) {
            memcpy( puclPredU + i*MAX_CU_SIZE/2, pucDstU[nTmpIdx] + i*MAX_CU_SIZE, iSize );
            memcpy( puclPredV + i*MAX_CU_SIZE/2, pucDstV[nTmpIdx] + i*MAX_CU_SIZE, iSize );
        }
        xSubDct( psTmp[0],
                 puclPixU, MAX_CU_SIZE/2,
                 puclPredU, MAX_CU_SIZE/2,
                 psTmp[0], psTmp[1],
                 iSize, MODE_INVALID );
        uiSumU = xQuant( psCoefU + nPredOffset, psTmp[0], MAX_CU_SIZE/2, nQPC, iSize, iSize, SLICE_I );
        if( uiSumU ) {
            xDeQuant( psTmp[0], psCoefU + nPredOffset, MAX_CU_SIZE/2, nQPC, iSize, iSize );
            xIDctAdd( puclRecU, MAX_CU_SIZE/2+8,
                      psTmp[0], MAX_CU_SIZE/2,
                      puclPredU,
                      psTmp[1], psTmp[0],
                      iSize, MODE_INVALID );
        }
        else {
            for( i=0; i<(int)iSize; i++ ) {
                memcpy( &puclRecU[i*(MAX_CU_SIZE/2+8)], &puclPredU[i*MAX_CU_SIZE/2], iSize );
            }
        }
        xSubDct( psTmp[0],
                 puclPixV, MAX_CU_SIZE/2,
                 puclPredV, MAX_CU_SIZE/2,
                 psTmp[0], psTmp[1],
                 iSize, MODE_INVALID );
        uiSumV = xQuant( psCoefV + nPredOffset, psTmp[0], MAX_CU_SIZE/2, nQPC, iSize, iSize, SLICE_I );
        if( uiSumV ) {
            xDeQuant( psTmp[0], psCoefV + nPredOffset, MAX_CU_SIZE/2, nQPC, iSize, iSize );
            xIDctAdd( puclRecV, MAX_CU_SIZE/2+8,
                      psTmp[0], MAX_CU_SIZE/2,
                      puclPredV,
                      psTmp[1], psTmp[0],
                      iSize, MODE_INVALID );
        }
        else {
            for( i=0; i<(int)iSize; i++ ) {
                memcpy( &puclRecV[i*(MAX_CU_SIZE/2+8)], &puclPredV[i*MAX_CU_SIZE/2], iSize );
            }
        }
        nCbfC = ( (uiSumV ? 2 : 0) | (uiSumU ? 1 : 0) );
        pucModeYInfo0[dy * MAX_PU_XY + dx] |= (nBestModeC << FLAG_OFF_MODEC) | (nCbfC << FLAG_OFF_CBFC);

        if (iSizeY == 4)
            iSizeY <<= 1;
        nIdx += (iSizeY * iSizeY / 16);
    } while( nIdx != (MAX_PU_XY * MAX_PU_XY) );  // TODO: Can't Support max CUSize other than MAX_CU_SIZE
}


// ***************************************************************************
// * Entropy Coding Functions
// ***************************************************************************
static
UInt getCtxQtCbf( UInt nSize, UInt bLuma )
{
    // CHECK: Is me right?
    if( bLuma ) {
        return (nSize != 4 ? 1 : 0);
    }
    else {
        // We don't care nSize when Chroma
        //return (nSize == 4 ? 1 : 0);
        return 0;
    }
}

static
	UInt32 xWriteIntraDirLumaAngGroup(
	UInt32     *pCTUCmd,
	UInt        nGroup,
	UInt        nPredIdx[4]
)
{
	UInt32 *pCmd0 = pCTUCmd;
	UInt i;
	for( i=0; i<nGroup; i++ ) {
		xEncodeBin_pCmd(&pCTUCmd, (nPredIdx[i] < 3), OFF_INTRA_PRED_CTX );
	}
	for( i=0; i<nGroup; i++ ) {
		UInt nIdx = nPredIdx[i];
		if( nIdx < 3 ) {
			// 0 -> 0 
			// 1 -> 10
			// 2 -> 11
			xEncodeBinsEP_pCmd(&pCTUCmd, nIdx + (nIdx!=0), 1+(nIdx!=0) );
		}
		else {
			assert( nIdx-3 < NUM_INTRA_MODE );
			xEncodeBinsEP_pCmd(&pCTUCmd, nIdx-3, 5 );
		}
	}
	return(pCTUCmd - pCmd0);
}

static
	UInt32 xWriteIntraDirModeChroma(
	UInt32     *pCmd,
	UInt        nModeC
	)
{
	UInt32 *pCmd0 = pCmd;
	xEncodeBin_pCmd(&pCmd,  (nModeC != CHROMA_MODE_DM), OFF_CHROMA_PRED_CTX );

	if ( nModeC != CHROMA_MODE_DM ) {
		// Non DM_CHROMA_IDX
		xEncodeBinsEP_pCmd(&pCmd,  nModeC, 2 );
	}
	return(pCmd - pCmd0);
}

static
	UInt32 xWriteMVD(
	UInt32     *pCTUCmd,
	xMV         mvd
	)
{
	UInt32 *pCmd0 = pCTUCmd;
	CInt iHor = mvd.x;
	CInt iVer = mvd.y;
	CUInt bHorAbsGr0 = iHor != 0;
	CUInt bVerAbsGr0 = iVer != 0;
	CUInt uiHorAbs   = abs(iHor);
	CUInt uiVerAbs   = abs(iVer);

	xEncodeBin_pCmd(&pCTUCmd, (bHorAbsGr0 ? 1 : 0), OFF_MVD_CTX );
	xEncodeBin_pCmd(&pCTUCmd, (bVerAbsGr0 ? 1 : 0), OFF_MVD_CTX );

	if( bHorAbsGr0 ) {
		xEncodeBin_pCmd(&pCTUCmd, (uiHorAbs > 1 ? 1 : 0), OFF_MVD_CTX + 1 );
	}

	if( bVerAbsGr0 ) {
		xEncodeBin_pCmd(&pCTUCmd,  (uiVerAbs > 1 ? 1 : 0), OFF_MVD_CTX + 1 );
	}

	if( bHorAbsGr0 ) {
		if( uiHorAbs > 1 ) {
			pCTUCmd += xWriteEpExGolomb( pCTUCmd, uiHorAbs-2, 1 );
		}

		xEncodeBinsEP_pCmd(& pCTUCmd,  (iHor < 0 ? 1 : 0), 1 );
	}

	if( bVerAbsGr0 ) {
		if( uiVerAbs > 1 ) {
			pCTUCmd += xWriteEpExGolomb( pCTUCmd, uiVerAbs-2, 1 );
		}

		xEncodeBinsEP_pCmd(& pCTUCmd,  (iVer < 0 ? 1 : 0), 1 );
	}
	return(pCTUCmd - pCmd0);
}

static
	UInt32 xWriteMergeIdx(
	UInt32     *pCmd,
	UInt        nMergeIdx
	)
{
	UInt32 *pCmd0 = pCmd;

	//  Idx  Bins
	//  0    0
	//  1    1 0
	//  2    1 10
	//  3    1 110
	//  4    1 1110
	xEncodeBin_pCmd(&pCmd, !!nMergeIdx, OFF_MERGE_IDX_EXT_CTX );
	if ( nMergeIdx ) {
		xEncodeBinsEP_pCmd(&pCmd, ( 1 << nMergeIdx ) - 2, nMergeIdx );
	}

	return(pCmd - pCmd0);
}

UInt32 xEncWriteCU(
	const xSliceType eSliceType,
	UInt32      *pCmd,
	UInt8       *pucModeY,
	UInt16      *pusModeYInfo0,
	UInt8       *pucModeYInfo1,
	xMV         *pasMVD,
	UInt8       *paucMVPIdx,
	Int16       *psCoefY,
	Int16       *psCoefU,
	Int16       *psCoefV,
	CUInt32     rx,
	CUInt32     ry
#if		CU_TYPE_STAT
	, X265_t *h
#endif
	,xMV		*pasMV
//#if DEBLOCK //x64 modify, remove #if DEBLOCK
	,CUInt16		nWidth
//#endif
	)
{
	CUInt32  *pCmd0 = pCmd;
	UInt    nIdx = 0;
	CUInt8   nMostModeC[NUM_CHROMA_MODE] = {PLANAR_IDX, VER_IDX, HOR_IDX, DC_IDX, DM_CHROMA_IDX};

	do {
		UInt    dx              = getInterleaveBits( nIdx      ); // Position to Luma
		UInt    dy              = getInterleaveBits( nIdx >> 1 );
		UInt8  *puclModeY       = &pucModeY     [dy*(MAX_PU_XY + 2) + dx];
		UInt16 *puslModeYInfo0  = &pusModeYInfo0[dy*(MAX_PU_XY    ) + dx];
		UInt8  *puclModeYInfo1  = &pucModeYInfo1[dy*(MAX_PU_XY + 1) + dx];
		Int16  *pslCoefY        = &psCoefY[dy*4*(MAX_CU_SIZE  ) + dx*4];
		Int16  *pslCoefU        = &psCoefU[dy*2*(MAX_CU_SIZE/2) + dx*2];
		Int16  *pslCoefV        = &psCoefV[dy*2*(MAX_CU_SIZE/2) + dx*2];
		UInt8   ucInfo          = puclModeYInfo1[0];
		UInt8   ucValids        = (ucInfo >> FLAG_OFF_VALIDS) & 0x1F;
		UInt    nSplit          = (ucInfo >> FLAG_OFF_SIZE );
		UInt8   nSplitLeft      = (ucValids & (1 << VALID_L)) ? (puclModeYInfo1[-1            ] >> FLAG_OFF_SIZE) : 7;
		UInt8   nSplitTop       = (ucValids & (1 << VALID_T)) ? (puclModeYInfo1[-(MAX_PU_XY+1)] >> FLAG_OFF_SIZE) : 7;

		int     b64x64_Flag     = (puclModeYInfo1[0] >> B64x64_FLAG_OFF)&1&(pusModeYInfo0[0]>>7);
		if (b64x64_Flag == 1){
			nSplit = 4;
		}
		
		int    b64x64_Flag_Left = (puclModeYInfo1[-1] >> B64x64_FLAG_OFF)&1&(pusModeYInfo0[0]>>7);
		if (b64x64_Flag_Left == 1){
			nSplitLeft = 4;
		}
		
		int     b64x64_Flag_Top = (puclModeYInfo1[-(MAX_PU_XY+1)] >> B64x64_FLAG_OFF)&1&(pusModeYInfo0[0]>>7);
		if (b64x64_Flag_Top == 1){
			nSplitTop = 4;
		}

		Int    iLog2SizeY      = (2 + nSplit);             //x64 modify, change type from UInt to Int
		Int    iSizeY          = ( 1 << iLog2SizeY );      //x64 modify, change type from UInt to Int
		Int    iSizeC          = ( iSizeY >> 1 );          //x64 modify, change type from UInt to Int
		Int    iBlock          = ( iSizeY / MIN_CU_SIZE ); //x64 modify, change type from UInt to Int
		Int    iModeY          = puclModeY[0];             //x64 modify, change type from UInt to Int

		UInt    nModeYIdx       = (puslModeYInfo0[0] & FLAG_MASK_MODE);
		UInt    nModeIdxC       = (puslModeYInfo0[0] >> FLAG_OFF_MODEC) & 7;
		UInt    nCbfY           = !!(puclModeYInfo1[0] & FLAG_MASK_CBFY);
		UInt    nCbfU           = !!(puslModeYInfo0[0] & FLAG_MASK_CBFU);
		UInt    nCbfV           = !!(puslModeYInfo0[0] & FLAG_MASK_CBFV);
		UInt    nIsInter        = !!(puslModeYInfo0[0] & FLAG_MASK_INTER);
		UInt    nMergeIdx       =   (puslModeYInfo0[0] & FLAG_MASK_MRG);
		UInt    bSkipped        = !!(puclModeYInfo1[0] & FLAG_MASK_SKIP);
		UInt    ePUMode			=   (puslModeYInfo0[0] >> FLAG_OFF_PUPART) & 3;  //--WXW
		UInt    bSkippedLeft    = !!( (puclModeYInfo1[-1] & FLAG_MASK_SKIP) && (puclModeY[-1+(Int)(iSizeY/MIN_CU_SIZE-1)*(MAX_PU_XY+2)] != MODE_INVALID) );//x64 modify
		UInt    bSkippedAbove   = !!( (puclModeYInfo1[-(MAX_PU_XY+1)] & FLAG_MASK_SKIP) && (puclModeY[-(MAX_PU_XY+2)+(Int)(iSizeY/MIN_CU_SIZE-1)] != MODE_INVALID) );//x64 modify
		xMV     mvd             = pasMVD[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*MAX_MV_XY + dx/(MIN_MV_SIZE/MIN_CU_SIZE)];
		UInt8   ucMVPIdx        = paucMVPIdx[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*MAX_MV_XY + dx/(MIN_MV_SIZE/MIN_CU_SIZE)];

		UInt    nQtRootCbfC     = nCbfU | nCbfV;
		UInt    nQtRootCbf      = nCbfY | nQtRootCbfC;
		UInt    realModeC=0;
		UInt    uiCtx;

		xMV     my_bmv[2];
		UInt    nMEBlock        = ( iSizeY / MIN_MV_SIZE );
		UInt32  nMVOffset = nMEBlock/2*(ePUMode == SIZE_2NxN?(MAX_MV_XY+2):1);
		my_bmv[0] = pasMV[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*(MAX_MV_XY+2) + dx/(MIN_MV_SIZE/MIN_CU_SIZE)];
		my_bmv[1] = pasMV[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*(MAX_MV_XY+2) + dx/(MIN_MV_SIZE/MIN_CU_SIZE) + nMVOffset];

//#if DEBLOCK//x64 modify
		xMV		*pasMV_for_BS;
		if (DEBLOCK) {
			pasMV_for_BS = pasMV + (dy/(MIN_MV_SIZE/MIN_CU_SIZE)*(MAX_MV_XY+2) + dx/(MIN_MV_SIZE/MIN_CU_SIZE));
		}
//#endif

#if	CU_TYPE_STAT
		h->stat.iCuTypeCount[eSliceType][(iLog2SizeY - 1)*CU_TYPE_INTERVAL + 0]++;
#endif

		// ***** Write Mode Info *****
		if((nSplit <= 4)&& (nIdx == 0)){
			uiCtx = ( nSplitLeft < 4 ) + ( nSplitTop < 4 );
			assert( uiCtx < 3 );
			xEncodeBin_pCmd(&pCmd, !!(nSplit < 4), OFF_SPLIT_FLAG_CTX + uiCtx );
		}
		// Check Depth 0
		if ( (nSplit <= 3) && ((nIdx & 63) == 0) ) {
			uiCtx = ( nSplitLeft < 3 ) + ( nSplitTop < 3 );
			assert( uiCtx < 3 );
			xEncodeBin_pCmd(&pCmd,  !!(nSplit < 3), OFF_SPLIT_FLAG_CTX + uiCtx );
		}
		// Check Depth 1
		if ( (nSplit <= 2) && ((nIdx & 15) == 0) ) {
			uiCtx = ( nSplitLeft < 2 ) + ( nSplitTop < 2 );
			assert( uiCtx < 3 );
			xEncodeBin_pCmd(&pCmd, !!(nSplit < 2), OFF_SPLIT_FLAG_CTX + uiCtx );
		}

		if ( eSliceType != SLICE_I ) {
			// SkipFlag
			uiCtx = bSkippedLeft + bSkippedAbove;  // LeftSkip + AboveSkip
			xEncodeBin_pCmd(&pCmd, bSkipped, OFF_SKIP_FLAG_CTX + uiCtx );

			if ( bSkipped ) {
				pCmd += xWriteMergeIdx( pCmd, nMergeIdx-1 );
#if		CU_TYPE_STAT
				h->stat.iCuTypeCount[eSliceType][(iLog2SizeY - 1)*CU_TYPE_INTERVAL + CU_TYPE_SKIP_OFF]++;
#endif
				goto _do_next_pu;
			}

			// Inter PredMode
			xEncodeBin_pCmd(&pCmd,  0, OFF_PRED_MODE_CTX );
		}

		// Check Depth 2
		if ( ((nIdx & 3) == 0) ) {
			// Write only 8x8
			// Intra
			if ( eSliceType == SLICE_I ) {
				if ( nSplit <= 1 ) {
					// codePartSize
					xEncodeBin_pCmd(&pCmd, (nSplit != 0 ? 1 : 0), OFF_PART_SIZE_CTX );
				}

				// codePredInfo
				UInt nPredIdx[4] = { nModeYIdx };
				if ( nSplit == 0 ) {
					// SIZE_NxN is used by Luma4x4 only!
					nPredIdx[1] = puslModeYInfo0[0*MAX_PU_XY + 1] & FLAG_MASK_MODE;
					nPredIdx[2] = puslModeYInfo0[1*MAX_PU_XY + 0] & FLAG_MASK_MODE;
					nPredIdx[3] = puslModeYInfo0[1*MAX_PU_XY + 1] & FLAG_MASK_MODE;
				}
				pCmd += xWriteIntraDirLumaAngGroup( pCmd, (nSplit == 0 ? 4 : 1), nPredIdx );
				pCmd += xWriteIntraDirModeChroma( pCmd, (puslModeYInfo0[0] >> FLAG_OFF_MODEC) & 7 );
			}
			// Inter
			else {
				// codePartSize
				// SIZE_NxN  -> 1
				// SIZE_2NxN -> 01
				// SIZE_Nx2N -> 00
				if(ePUMode == SIZE_2Nx2N)
				{
					xEncodeBin_pCmd(&pCmd, 1, OFF_PART_SIZE_CTX );
					// MergeFlag
					xEncodeBin_pCmd(&pCmd, !!nMergeIdx, OFF_MERGE_FLAG_EXT_CTX );

					if ( nMergeIdx ) {
						pCmd += xWriteMergeIdx( pCmd, nMergeIdx-1 );
#if		CU_TYPE_STAT
						h->stat.iCuTypeCount[eSliceType][(iLog2SizeY - 1)*CU_TYPE_INTERVAL + CU_TYPE_MERGE_OFF]++;
#endif
					}
					else {
						// InterDirPU
						// RefFrmIdxPU
#if		CU_TYPE_STAT
						h->stat.iCuTypeCount[eSliceType][(iLog2SizeY - 1)*CU_TYPE_INTERVAL + CU_TYPE_FULL_OFF]++;
#endif

						// MvdPU
						pCmd += xWriteMVD( pCmd, mvd );

						// MVPIdxPU
						pCmd += xWriteUnaryMaxSymbol( pCmd, ucMVPIdx, OFF_MVP_IDX_CTX, 1, AMVP_MAX_NUM_CANDS-1 );
					}
					
					if(b64x64_Flag == 1) {
						UInt U_tmp=0;
						UInt V_tmp=0;
						UInt Y_tmp=0;
						for(int j=0; j<2; j++) {
							for(int i=0; i<2; i++) {
								UInt32		nModeYInfo0Offset = j*MAX_PU_XY    *iBlock/2   + i*iBlock/2;//MAX_PU_XY/2;
								UInt32		nModeYInfo1Offset = j*(MAX_PU_XY+1)*iBlock/2   + i*iBlock/2;//MAX_PU_XY/2;
								Y_tmp |= !!(puclModeYInfo1[nModeYInfo1Offset] & FLAG_MASK_CBFY);
								U_tmp |= !!(puslModeYInfo0[nModeYInfo0Offset] & FLAG_MASK_CBFU);
								V_tmp |= !!(puslModeYInfo0[nModeYInfo0Offset] & FLAG_MASK_CBFV);
							}
						}
						nCbfY = Y_tmp;
						nCbfU = U_tmp;
						nCbfV = V_tmp;

						nQtRootCbf = nCbfY | nCbfU | nCbfV;
						// Merge && SIZE_2Nx2N have not QT_ROOT_CBF. merge + nQtRootCbf = SKIP
						if ( !( nMergeIdx ) ){  // FIXME --wxw
							xEncodeBin_pCmd(&pCmd,  nQtRootCbf, OFF_QT_ROOT_CBF_CTX );
						}
#if		CU_TYPE_STAT
						if(nQtRootCbf == 0){
							h->stat.iCuTypeCount[eSliceType][(iLog2SizeY - 1)*CU_TYPE_INTERVAL + CU_TYPE_CBF_ZERO]++;
						}
#endif
					}else{
						if ( !( nMergeIdx ) ) {
							xEncodeBin_pCmd(&pCmd,  nQtRootCbf, OFF_QT_ROOT_CBF_CTX );
						}

						if(nQtRootCbf == 1)	{
							xEncodeBin_pCmd(&pCmd,  0, OFF_TRANS_SUBDIV_FLAG_CTX + 5 - iLog2SizeY );
						}
#if		CU_TYPE_STAT
						else{
							h->stat.iCuTypeCount[eSliceType][(iLog2SizeY - 1)*CU_TYPE_INTERVAL + CU_TYPE_CBF_ZERO]++;
						}
#endif
					}
				}
			}

			// ***** Write Transform Info *****
			if ( !nIsInter || ((nQtRootCbf && (ePUMode == SIZE_2Nx2N)) && (b64x64_Flag != 1) )) {
				// Write Cbf of Chroma
				xEncodeBin_pCmd(&pCmd,  nCbfU, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(iSizeY, FALSE) );
				xEncodeBin_pCmd(&pCmd,  nCbfV, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(iSizeY, FALSE) );
			}
		}

		// ***** Write Transform Info *****
		if(((ePUMode == SIZE_2Nx2N)&&(nIsInter )) || !nIsInter ){
			if(b64x64_Flag == 1){
				assert(nIdx == 0);
				if (nQtRootCbf == 0)
					goto _do_next_pu;

				xEncodeBin_pCmd(&pCmd,  nCbfU, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(iSizeY, FALSE) );
				xEncodeBin_pCmd(&pCmd,  nCbfV, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(iSizeY, FALSE) );
				UInt32  nCbfURoot = nCbfU;
				UInt32  nCbfVRoot = nCbfV;
				for(int j=0; j<2; j++) {
					for(int i=0; i<2; i++) {
						UInt32		nModeYOffset      = j*(MAX_PU_XY+2)*iBlock/2	+ i*iBlock/2;
						UInt32		nModeYInfo0Offset = j*MAX_PU_XY*iBlock/2		+ i*iBlock/2;
						UInt32		nModeYInfo1Offset = j*(MAX_PU_XY+1)*iBlock/2	+ i*iBlock/2;
						UInt32		nCoefYOffset      = j*MAX_CU_SIZE*iSizeY/2		+ i*iSizeY/2;
						UInt32		nCoefCOffset      = j*MAX_CU_SIZE/2*iSizeC/2	+ i*iSizeC/2;

						nCbfY           = !!(puclModeYInfo1[nModeYInfo1Offset] & FLAG_MASK_CBFY);
						nCbfU           = !!(puslModeYInfo0[nModeYInfo0Offset] & FLAG_MASK_CBFU);
						nCbfV           = !!(puslModeYInfo0[nModeYInfo0Offset] & FLAG_MASK_CBFV);
						if (nCbfURoot == 1)	{
							xEncodeBin_pCmd(&pCmd,  nCbfU, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + 1 + getCtxQtCbf(iSizeY/2, FALSE) );
						}
						if (nCbfVRoot == 1)	{
							xEncodeBin_pCmd(&pCmd,  nCbfV, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + 1  + getCtxQtCbf(iSizeY/2, FALSE) );
						}
						xEncodeBin_pCmd(&pCmd,  nCbfY, OFF_QT_CBF_CTX + 0*NUM_QT_CBF_CTX + 0  );

						if(nCbfY) {
							pCmd += xEncodeCoeffNxN( pCmd, pslCoefY+nCoefYOffset, iSizeY/2, TRUE, *(puclModeY+nModeYOffset),nIsInter);
						}

						if(nCbfU) {
							pCmd += xEncodeCoeffNxN( pCmd, pslCoefU+nCoefCOffset, iSizeC/2, FALSE, realModeC,nIsInter);
						}
						if(nCbfV) {
							pCmd += xEncodeCoeffNxN( pCmd, pslCoefV+nCoefCOffset, iSizeC/2, FALSE, realModeC,nIsInter);
						}
					}
				}
			}
			else {
				// ***** Write Transform Info *****
				if ( !nIsInter || (nIsInter && nQtRootCbfC) ) {
					xEncodeBin_pCmd(&pCmd,  nCbfY, OFF_QT_CBF_CTX + 0*NUM_QT_CBF_CTX + getCtxQtCbf(iSizeY, TRUE) );
				}
				if(nCbfY) {
					pCmd += xEncodeCoeffNxN( pCmd, pslCoefY, iSizeY, TRUE, iModeY,nIsInter);
				}
				if ( (nSplit != 0) || ((nSplit == 0) && ((nIdx & 3) == 3)) ) {
					// Last Luma4x4 will output Chroma, so I use LeftAbove info
					if ( (nSplit == 0) && ((nIdx & 3) == 3) ) {
						UInt ucInfoLT = puslModeYInfo0[-MAX_PU_XY-1];//x64 modify
						nModeIdxC = (ucInfoLT >> FLAG_OFF_MODEC) & 7;
						iModeY    = puclModeY[-(MAX_PU_XY+2)-1];//x64 modify
						nCbfU     = !!(ucInfoLT & FLAG_MASK_CBFU);
						nCbfV     = !!(ucInfoLT & FLAG_MASK_CBFV);
						pslCoefU -= 1*2*MAX_CU_SIZE/2 + 1*2;
						pslCoefV -= 1*2*MAX_CU_SIZE/2 + 1*2;
						iSizeC    = 4;
					}

					assert( nModeIdxC <= NUM_CHROMA_MODE-1 );
					if ( nModeIdxC == CHROMA_MODE_DM ) {
						realModeC = iModeY;
					}
					else {
						realModeC = nMostModeC[nModeIdxC];
						if ( realModeC == iModeY ) {
							realModeC = 34;
						}
					}

					if(nCbfU) {
						pCmd += xEncodeCoeffNxN( pCmd, pslCoefU, iSizeC, FALSE, realModeC,nIsInter );
					}
					if(nCbfV) {
						pCmd += xEncodeCoeffNxN( pCmd, pslCoefV, iSizeC, FALSE, realModeC,nIsInter );
					}
				}
			}
		}

_do_next_pu:;

//#if DEBLOCK //x64 modify
		if (DEBLOCK) {
			if( (b64x64_Flag == 1) && (nQtRootCbf != 0)) {
				for(int j=0; j<2; j++) {
					for(int i=0; i<2; i++) {
						UInt32		nModeYInfo1Offset = j*(MAX_PU_XY+1)*iBlock/2	+ i*iBlock/2;
						UInt32		nPasMVOffset	  =	j*(MAX_MV_XY+2)*iBlock/4	+ i*iBlock/4;
						//UInt32		nCoefYOffset      = j*MAX_CU_SIZE*nSizeY/2		+ i*nSizeY/2;
						//UInt32		nCoefCOffset      = j*MAX_CU_SIZE/2*nSizeC/2	+ i*nSizeC/2;

						UInt32 pos_x = rx + dx*4 + i*iSizeY/2;
						UInt32 pos_y = ry + dy*4 + j*iSizeY/2;
						ff_hevc_deblocking_boundary_strengths( 
							h->horizontal_bs, 
							h->vertical_bs,
							nWidth,
							pos_x, 
							pos_y, 
							iLog2SizeY-1,  
							1, 
							pasMV_for_BS + nPasMVOffset, 
							puclModeYInfo1 + nModeYInfo1Offset,
							eSliceType
							);
					}
				}

			}
			else {
				UInt32 pos_x = rx + dx*4;
				UInt32 pos_y = ry + dy*4;
				ff_hevc_deblocking_boundary_strengths( 
					h->horizontal_bs, 
					h->vertical_bs,
					nWidth,
					pos_x, 
					pos_y, 
					iLog2SizeY,  //only support TU_size == CU_size
					0, 
					pasMV_for_BS, 
					puclModeYInfo1,
					eSliceType
					);
			}
		}
//#endif
		
		nIdx += ( 1 << ( ((2+nSplit) << 1) - 4) );
	} while( nIdx != (MAX_PU_XY * MAX_PU_XY) );  // TODO: Can't Support max CUSize other than MAX_CU_SIZE

	return (pCmd - pCmd0);
}


// ***************************************************************************
// * Internal Debug Functions
// ***************************************************************************
#if defined(DEBUG_VIS)
static
void xDrawBox( UInt8 *pucPix, UInt8 ucVal, UInt nSize, UInt nWidth, UInt ePUMode )
{
    UInt i;

    for( i=0; i<nSize; i++ ) {
        pucPix[i*nWidth + 0] = ucVal;
        pucPix[i           ] = ucVal;
				if (ePUMode == SIZE_2NxN)
				{
					pucPix[i+nSize/2*nWidth] = ucVal;

				}
				if (ePUMode == SIZE_Nx2N)
				{
					pucPix[i*nWidth+nSize/2] = ucVal;
				}
   }
}

void xDrawVisCU(
	xFrame *frm_vis,
	UInt8  *pucPixY,
	UInt8  *pucPixU,
	UInt8  *pucPixV,
	UInt16 *pusModeYInfo0,
	UInt8  *pucModeYInfo1,
	UInt    nX,
	UInt    nY,
	UInt    nWidth
	)
{
    UInt    i;
    UInt    nIdx = 0;
    UInt8  *pucVisY = frm_vis->pucY + (nY*MAX_CU_SIZE)*nWidth   + (nX*MAX_CU_SIZE);
    UInt8  *pucVisU = frm_vis->pucU + (nY*MAX_CU_SIZE)*nWidth/4 + (nX*MAX_CU_SIZE)/2;
    UInt8  *pucVisV = frm_vis->pucV + (nY*MAX_CU_SIZE)*nWidth/4 + (nX*MAX_CU_SIZE)/2;

    // Copy Image into buffer
    for( i=0; i<MAX_CU_SIZE/2; i++ ) {
        memcpy( &pucVisY[(i*2+0)*nWidth  ], &pucPixY[(i*2+0)*MAX_CU_SIZE], MAX_CU_SIZE );
        memcpy( &pucVisY[(i*2+1)*nWidth  ], &pucPixY[(i*2+1)*MAX_CU_SIZE], MAX_CU_SIZE );
        memcpy( &pucVisU[i*nWidth/2], &pucPixU[i*MAX_CU_SIZE/2], MAX_CU_SIZE/2 );
        memcpy( &pucVisV[i*nWidth/2], &pucPixV[i*MAX_CU_SIZE/2], MAX_CU_SIZE/2 );
    }

    do {
        UInt    dx = getInterleaveBits( nIdx      ); // Position to Luma
        UInt    dy = getInterleaveBits( nIdx >> 1 );
        UInt    nSizeY = ( 1 << (2 + (pucModeYInfo1[(MAX_PU_XY+1) * dy + dx] >> FLAG_OFF_SIZE)) );
		int     b64x64_Flag     = ((pucModeYInfo1[0] >> B64x64_FLAG_OFF)&1&(pusModeYInfo0[0]>>7)) ;
		if (b64x64_Flag == 1)
			nSizeY = 64;
		UInt8   ucInfo          = pucModeYInfo1[0];
		UInt    nSplit          = (ucInfo >> FLAG_OFF_SIZE );


        UInt    nSizeC = ( nSizeY >> 1 );
		UInt    ePUMode		=   (pusModeYInfo0[0] >> FLAG_OFF_PUPART) & 3;  //--WXW
		UInt8  *pucY = pucVisY + dy*4*nWidth   + dx*4;
        UInt8  *pucU = pucVisU + dy*2*nWidth/2 + dx*2;
        UInt8  *pucV = pucVisV + dy*2*nWidth/2 + dx*2;

		if(nSizeY == 8)
			ePUMode = 0;
        xDrawBox( pucY, 0x00, nSizeY, nWidth, ePUMode );
		//xDrawBox( pucU, 0x80, nSizeC, nWidth/2 );
		//xDrawBox( pucV, 0x80, nSizeC, nWidth/2 );

        nIdx += (nSizeY * nSizeY / 16);
    } while( nIdx != (MAX_PU_XY * MAX_PU_XY) );  // TODO: Can't Support max CUSize other than MAX_CU_SIZE
}
#endif // DEBUG_VIS


// ***************************************************************************
// * Interface Functions
// ***************************************************************************
void xEncInit( X265_t *h )
{
	CUInt32 uiWidthPad  = h->iWidth  + PAD_YSIZE * 2;
	CUInt32 uiHeightPad = h->iHeight + PAD_YSIZE * 2;
	CUInt32 uiYSize     = uiWidthPad * uiHeightPad;
	CUInt32 uiCSize     = uiWidthPad * uiHeightPad >>2;
	CUInt32 uiPadOffY   = PAD_YSIZE * uiWidthPad   + PAD_YSIZE;
	CUInt32 uiPadOffC   = PAD_CSIZE * uiWidthPad/2 + PAD_CSIZE;

	int i;
	UInt8 *ptr;
	for(Int i=0; i < MAX_REF_NUM+1; i++ ) {
		ptr = (UInt8 *)MALLOC(uiYSize * 3 / 2);
		assert(ptr != NULL);
		h->refn[i].base = ptr;
		h->refn[i].pucY = (UInt8 *)ptr + uiPadOffY;
		h->refn[i].pucU = (UInt8 *)ptr + uiYSize + uiPadOffC;
		h->refn[i].pucV = (UInt8 *)ptr + uiYSize * 5 / 4 + uiPadOffC;
		h->refn[i].iStrideY = uiWidthPad;
		h->refn[i].iStrideC = uiWidthPad / 2;
	}

#if defined(DEBUG_VIS)
	ptr = (UInt8 *)MALLOC(uiYSize * 3 / 2);
	assert(ptr != NULL);
	h->frm_vis.base = NULL;
	h->frm_vis.pucY = (UInt8 *)ptr;
	h->frm_vis.pucU = (UInt8 *)ptr + uiYSize;
	h->frm_vis.pucV = (UInt8 *)ptr + uiYSize * 5 / 4;
#endif
	h->iPoc = -1;
	h->uiFrames = 0;
	h->PicWidthInCUs  = h->iWidth / h->ucMaxCUWidth;
	h->PicHeightInCUs = h->iHeight / h->ucMaxCUWidth;

#if		CU_TYPE_STAT
	for(i=0; i<X265_MAX_SLICE_TYPE; i++) {
		for(Int j=0; j<X265_MAX_CUTYPE;j++)	{
			h->stat.iCuTypeCount[i][j] = 0;
		}
	}
#endif

	xBuildNextStateTable(h->rdoParam); //FIX ME: initializing all rdoParams or just the first one? YananZhao, 2013-11-15

//#if DEBLOCK //x64 modify
	if (DEBLOCK) {
		h->sps = (SPS *)MALLOC(sizeof(SPS));
		assert(h->sps != NULL);
		h->sps->pic_width_in_luma_samples = h->iWidth;
		h->sps->pic_height_in_luma_samples = h->iHeight;
		h->sps->log2_min_coding_block_size = xLog2((h->ucMaxCUWidth-1)>>h->ucMaxCUDepth );
		h->sps->log2_ctb_size = xLog2(h->ucMaxCUWidth - 1);
		h->sps->pic_width_in_ctbs = h->PicWidthInCUs;
		h->sps->pic_height_in_ctbs = h->PicHeightInCUs;
		h->sps->pic_width_in_min_cbs = h->iWidth/(h->ucMaxCUWidth>>h->ucMaxCUDepth);
		h->sps->pic_height_in_min_cbs = h->iHeight/(h->ucMaxCUWidth>>h->ucMaxCUDepth);
		h->sps->qp_bd_offset = 0;//6 * (sps->bit_depth - 8);
		h->sps->log2_min_pu_size = h->sps->log2_min_coding_block_size - 1;

		h->deblock = (DBParams*)MALLOC(h->sps->pic_width_in_ctbs*h->sps->pic_height_in_ctbs * sizeof(DBParams));
		assert(h->deblock!=NULL);
		memset((Int8*)h->deblock, 0, h->sps->pic_width_in_ctbs*h->sps->pic_height_in_ctbs * sizeof(DBParams));

		h->vertical_bs   = (UInt8 *)MALLOC(h->iWidth/4 * h->iHeight/4 * sizeof(UInt8));
		assert(h->vertical_bs != NULL);
		memset(h->vertical_bs, 0, h->iWidth/4 * h->iHeight/4 * sizeof(UInt8));

		h->horizontal_bs = (UInt8 *)MALLOC(h->iWidth/4 * h->iHeight/4 * sizeof(UInt8));
		assert(h->horizontal_bs != NULL);
		memset(h->horizontal_bs, 0, h->iWidth/4 * h->iHeight/4 * sizeof(UInt8));

		h->qp_y_tab = (UInt8*)MALLOC(h->iWidth/h->ucMaxCUDepth * h->iHeight/h->ucMaxCUDepth);
		assert(h->qp_y_tab != NULL);
		//memset(h->qp_y_tab, h->iQP, h->usWidth/h->ucMaxCUDepth * h->usHeight/h->ucMaxCUDepth);

		h->bs_width = h->sps->pic_width_in_luma_samples >> 3;  //only for 8 boundary
		h->bs_height = h->sps->pic_height_in_luma_samples >> 3;  //only for 8 boundary
		h->pps_transquant_bypass_enable_flag = 0;
		h->pps_cb_qp_offset = 0;
		h->pps_cr_qp_offset = 0;
	}
//#endif

#if SAO
	//YananZhao, 2013-12-14, init pointers
	h->g_clipTable = NULL;
	h->g_chromaClipTable = NULL;

	h->m_upBuff1 = h->_upBuff1 + 1;
	h->m_upBuff2 = h->_upBuff2 + 1;
	h->m_upBufft = h->_upBufft + 1;
	h->numlcus = (float)(h->PicWidthInCUs * h->PicHeightInCUs);

	const UInt pixelRangeY = (1 << 8);
	const UInt boRangeShiftY = 8 - SAO_BO_BITS;
	const UInt boRangeShiftC = 8 - SAO_BO_BITS;

	for (Int k2 = 0; k2 < (Int)pixelRangeY; k2++) {
		h->g_lumaTableBo[k2]   = 1 + (k2 >> boRangeShiftY);
		h->g_chromaTableBo[k2] = 1 + (k2 >> boRangeShiftC);
	}

	UInt maxY  = (1 << 8) - 1;
	UInt minY  = 0;

	int rangeExt = maxY >> 1;

	for (i = 0; i < (Int)(minY + rangeExt); i++) {//x64 modify, convert to Int avoiding warnings
		h->g_clipTableBase[i] = minY;
	}

	for (i = (Int)(minY + rangeExt); i < (Int)(maxY +  rangeExt); i++) {//x64 modify, convert to Int avoiding warnings
		h->g_clipTableBase[i] = i - rangeExt;
	}

	for (i = (Int)(maxY + rangeExt); i < (Int)(maxY + 2 * rangeExt); i++) {//x64 modify, convert to Int avoiding warnings
		h->g_clipTableBase[i] = maxY;
	}

	h->g_clipTable = &(h->g_clipTableBase[rangeExt]);

	UInt maxC = (1 << 8) - 1;
	UInt minC = 0;

	int rangeExtC = maxC >> 1;

	for (i = 0; i < (Int)(minC + rangeExtC); i++) {//x64 modify, convert to Int avoiding warnings
		h->g_chromaClipTableBase[i] = minC;
	}

	for (i = (Int)(minC + rangeExtC); i < (Int)(maxC +  rangeExtC); i++) {//x64 modify, convert to Int avoiding warnings
		h->g_chromaClipTableBase[i] = i - rangeExtC;
	}

	for (i = (Int)(maxC + rangeExtC); i < (Int)(maxC + 2 * rangeExtC); i++) {//x64 modify, convert to Int avoiding warnings
		h->g_chromaClipTableBase[i] = maxC;
	}

	h->g_chromaClipTable = &(h->g_chromaClipTableBase[rangeExtC]);

	h->m_tmpL1 = (UInt8*)MALLOC((MAX_CU_SIZE + 1) * sizeof(UInt8));
	h->m_tmpL2 = (UInt8*)MALLOC((MAX_CU_SIZE + 1) * sizeof(UInt8));

	h->m_tmpU1[0] = (UInt8*)MALLOC(MAX_WIDTH * sizeof(UInt8));
	h->m_tmpU1[1] = (UInt8*)MALLOC(MAX_WIDTH * sizeof(UInt8));
	h->m_tmpU1[2] = (UInt8*)MALLOC(MAX_WIDTH * sizeof(UInt8));
	h->m_tmpU2[0] = (UInt8*)MALLOC(MAX_WIDTH * sizeof(UInt8));
	h->m_tmpU2[1] = (UInt8*)MALLOC(MAX_WIDTH * sizeof(UInt8));
	h->m_tmpU2[2] = (UInt8*)MALLOC(MAX_WIDTH * sizeof(UInt8));
#endif
}

void xEncFree( X265_t *h )
{
//#if DEBLOCK //x64 modify
	if (DEBLOCK) {
		assert( h->vertical_bs != NULL );
		FREE( h->vertical_bs );
		assert( h->horizontal_bs != NULL );
		FREE( h->horizontal_bs );
		assert( h->qp_y_tab != NULL );
		FREE( h->qp_y_tab );
		assert(h->sps != NULL);
		FREE(h->sps);
		assert(h->deblock!=NULL);
		FREE(h->deblock);
	}
//#endif

	for(Int i=0; i < MAX_REF_NUM+1; i++ ) {
		assert( h->refn[i].pucY != NULL );
		FREE( h->refn[i].base );
	}

	memset( h->refn, 0, sizeof(h->refn) );
#if defined(DEBUG_VIS)
	assert(h->frm_vis.pucY != NULL);
	FREE(h->frm_vis.pucY);
#endif

#if SAO
	FREE(h->m_tmpL1);
	FREE(h->m_tmpL2);
	FREE(h->m_tmpU1[0]);
	FREE(h->m_tmpU1[1]);
	FREE(h->m_tmpU1[2]);
	FREE(h->m_tmpU2[0]);
	FREE(h->m_tmpU2[1]);
	FREE(h->m_tmpU2[2]);
#endif
}

static
void xEncodeCUs( X265_t *h, CInt32 iCUWidth, CInt32 iCUHeight, CInt32 offX, CInt32 offY, CInt32 bsOff, CUInt32 idxTiles )//x64 modify, convert to CInt
{
	 CUInt        nMaxCuWidth = h->ucMaxCUWidth;
   CInt         nQP         = h->iQP;
   CInt         nQPC        = xg_aucChromaScale[nQP];
   CInt         iCUSize     = h->ucMaxCUWidth;//x64 modify, change type to CInt
   CUInt        nLog2CUSize = xLog2(iCUSize-1);
   CUInt32      bTiles      = h->bTiles;
   const xSliceType eSliceType = h->eSliceType;
   Int y=0;
#if (USE_ASM > ASM_NONE)
    __declspec(align(16))
#endif
    xCache cache;

//#if DEBLOCK//x64 modify
		if (DEBLOCK)	{
			memset( h->horizontal_bs, 0, h->iWidth/4 * h->iHeight/4 * sizeof(UInt8));
			memset( h->vertical_bs, 0, h->iWidth/4 * h->iHeight/4 * sizeof(UInt8));
		}
//#endif

	xCabacInit( h );
	xCabacReset( &h->cabac );
	memcpy(h->cabacRdoLast[0], &h->cabac, sizeof(xCabac));

#ifdef _OPENMP
    #pragma omp parallel default(none) private(cache) private(y) shared(h)  num_threads(h->nThreads) 
    #pragma omp for schedule(dynamic, 1) /*ordered*/ nowait
#endif
    for( y=0; y < (int)iCUHeight; y++ ) {
        UInt32      uiOffset    = 0;
        UInt8      *paucPixY    = cache.aucPixY;
        UInt8      *paucPixU    = cache.aucPixU;
        UInt8      *paucPixV    = cache.aucPixV;
        UInt8      *paucRecY    = cache.aucRecY + 16;
        UInt8      *paucRecU    = cache.aucRecU + 8;
        UInt8      *paucRecV    = cache.aucRecV + 8;
        Int16      *psCoefY     = cache.psCoefY;
        Int16      *psCoefU     = cache.psCoefU;
        Int16      *psCoefV     = cache.psCoefV;
        UInt8      *pucModeY                    = cache.aucModeY;
        UInt16     *pusModeYInfo0               = cache.ausModeYInfo0;
        UInt8      *pucModeYInfo1               = cache.aucModeYInfo1;
        xMV        *pasMV                       = cache.asMV;
        xMV        *pasMVD                      = cache.asMVD;
        UInt8      *paucMVPIdx                  = cache.aucMVPIdx;
        UInt8      *paucPredY                   = cache.aucPredY;
        UInt8      *paucPredU                   = cache.aucPredU;
        UInt8      *paucPredV                   = cache.aucPredV;
				UInt8      *paucPredTmp[MAX_CU_DEPTH]   = { cache.aucPredTmp4, cache.aucPredTmp8, cache.aucPredTmp16, cache.aucPredTmp32, cache.aucPredTmp64 };
				UInt8      *paucRecYTmp[MAX_CU_DEPTH]   = { cache.aucRecYTmp4, cache.aucRecYTmp8, cache.aucRecYTmp16, cache.aucRecYTmp32, cache.aucRecYTmp64 };
				Int16      *pasCoefYTmp[MAX_CU_DEPTH]   = { cache.asCoefYTmp4, cache.asCoefYTmp8, cache.asCoefYTmp16, cache.asCoefYTmp32, cache.asCoefYTmp64 };
				Int        *paiCbfYTmp[MAX_CU_DEPTH]    = { &(cache.iCbfYTmp4), &(cache.iCbfYTmp8), &(cache.iCbfYTmp16), &(cache.iCbfYTmp32), &(cache.iCbfYTmp64) };
				UInt8      *paucTopModeYInfo1           = h->aucTopModeYInfo1[idxTiles];
				Int16      *psTmp[4]                    = { cache.psTmp[0], cache.psTmp[1], cache.psTmp[2], cache.psTmp[3] }; 
        UInt8       ucSavedLT, ucSavedLTU, ucSavedLTV;
        xMV         savedMvLT = {0, 0};
        Int x;
				
				h->rdoParam[y].pCabacRdo = h->cabacRdo[y];//cabac for each CTU line
				h->rdoParam[y].pCabacRdoLast = h->cabacRdoLast[y];//last cabac state, used to init the next CTU RDO info
				xCabac * ppcCabacRdo[MAX_CU_DEPTH+1] = { &h->cabacRdo[y][0], &h->cabacRdo[y][1], &h->cabacRdo[y][2], &h->cabacRdo[y][3], &h->cabacRdo[y][4], &h->cabacRdo[y][5] }; 

#ifdef _OPENMP
        #ifndef WIN32
        while( h->cuaddr[bsOff+y] < 2 ) {
            SLEEP(1);
        }
        #else
        int dummy;
        if ( y ) {
            fifo_deq( &h->cuaddr[bsOff+y], &dummy );
            //printf("\n Dequeue[%2d]=%d\n", bsOff+y, dummy);
        }
        #endif
#endif
        ucSavedLT   = h->aucTopPixY[idxTiles][1];
        ucSavedLTU  = h->aucTopPixU[idxTiles][1];
        ucSavedLTV  = h->aucTopPixV[idxTiles][1];

        memset( pucModeY, MODE_INVALID, sizeof(cache.aucModeY) );

        // Top always valid when (y > 0)
        if ( y != 0 ) {
            memset( pucModeY, MODE_VALID, MAX_PU_XY+2 );
        }
        xEncCahceInitLine( &cache );
        for( x=0; x < (int)iCUWidth; x++ ) {
            UInt32 *pCmd0 = h->auiCabacCmd[bsOff+y][x];
            UInt32 *pCmd = pCmd0;
						UInt32 *pCTUCmd = h->auiCTUCmd[y];
						CUInt   bLastCU     = (y == iCUHeight - 1) && (x == iCUWidth - 1);//x64 modify, remove (int), they are int now
            UInt8  *pucTopPixY  = h->aucTopPixY[idxTiles] + 1 + uiOffset;
            UInt8  *pucTopPixU  = h->aucTopPixU[idxTiles] + 1 + uiOffset/2;
            UInt8  *pucTopPixV  = h->aucTopPixV[idxTiles] + 1 + uiOffset/2;
            xMV    *pasTopMV    = h->asTopMV[idxTiles]    + 1 + uiOffset / MIN_MV_SIZE;
            CInt32 iStrideY    = h->pFrameRef->iStrideY;//x64 modify, convert to int, considering to use int64_ptr instead of Int for strides
            CInt32 iStrideC    = h->pFrameRef->iStrideC;//x64 modify, convert to int
						CUInt8 *pucRefY     = h->pFrameRef->pucY + (offY+y) * iCUSize * iStrideY + uiOffset + offX*iCUSize;
						CUInt8 *pucRefU     = h->pFrameRef->pucU + (offY+y) * iCUSize/2 * iStrideC + uiOffset/2 + offX*iCUSize/2;
						CUInt8 *pucRefV     = h->pFrameRef->pucV + (offY+y) * iCUSize/2 * iStrideC + uiOffset/2 + offX*iCUSize/2;
						CInt32 iRefStrideY = h->pFrameRef->iStrideY;//x64 modify, convert to int
            CInt32 iRefStrideC = h->pFrameRef->iStrideC;//x64 modify, convert to int
            UInt8   tmp;
            xMV     tmpMV;

            // Stage 0: Init internal
#ifdef _OPENMP
            //printf( "\n----> Compressing CTU(%3d,%3d), using thread %d\n", (offY+y), (offX+x), omp_get_thread_num() );
#endif
            // Stage 1a: Load image to cache
            xEncCacheLoadCU( h, &cache, (offX+x), (offY+y) );
			
            // Stage 2a: Decide Intra

            // Set Top Right Valid Flag and Swap Pixel LT
            // LT-Y
            tmp = pucTopPixY[-1];
            pucTopPixY[-1] = ucSavedLT;
            ucSavedLT = tmp;
            // LT-U
            tmp = pucTopPixU[-1];
            pucTopPixU[-1] = ucSavedLTU;
            ucSavedLTU = tmp;
            // LT-V
            tmp = pucTopPixV[-1];
            pucTopPixV[-1] = ucSavedLTV;
            ucSavedLTV = tmp;
            // LT-MV
            tmpMV = pasTopMV[-1];
            pasTopMV[-1] = savedMvLT;
            savedMvLT    = tmpMV;

            pucModeY[0          ] = ( (y > 0) && (x > 0) ? MODE_VALID : MODE_INVALID );
            pucModeY[MAX_PU_XY+1] = ( (y > 0) && (x < iCUWidth-1) ? MODE_VALID : MODE_INVALID );//x64 modify, remove (int) cause' it is Int now
            memcpy( pucModeYInfo1+1, paucTopModeYInfo1 + uiOffset/MIN_CU_SIZE, MAX_PU_XY );

						xCabac *pCabacInit = (!y || x)? h->cabacRdoLast[y] : &h->cabacRdoSaved;
						for ( int ii=0; ii<MAX_CU_DEPTH; ii++){
							memcpy(ppcCabacRdo[ii], pCabacInit, sizeof(xCabac));
						}
	
						if ( eSliceType == SLICE_I ) {
							//double dLambda = 0.3 * 0.57 * pow(2.0, (nQP - 12) / 3.0) ;
							double dLambda = 0.57 * pow(2.0, (nQP - 12) / 3.0) ; 
                xEncDecideCU_Intra(
										pCTUCmd, 
										ppcCabacRdo,
										paucRecYTmp,
										pasCoefYTmp,
										paiCbfYTmp,
										0,
										&(h->rdoParam[y]),
                    paucPixY,
                    paucRecY,
                    pucTopPixY,
                    pucModeY + (MAX_PU_XY + 2) + 1,
                    pusModeYInfo0,
                    pucModeYInfo1 + (MAX_PU_XY + 1) + 1,
                    paucPredY,
                    psCoefY,
                    psTmp,
                    paucPredTmp,
                    nMaxCuWidth,
                    0,
                    nQP,
                    dLambda,                             //lambda
                    h->bStrongIntraSmoothing
                );

                // Stage 3: Encode Chroma
                xEncodeChromaCU(
                    paucPixU,
                    paucPixV,
                    paucRecY,
                    paucRecU,
                    paucRecV,
                    paucPredU,
                    paucPredV,
                    pucTopPixY,
                    pucTopPixU,
                    pucTopPixV,
                    pucModeY + (MAX_PU_XY + 2) + 1,
                    pusModeYInfo0,
                    pucModeYInfo1 + (MAX_PU_XY + 1) + 1,
                    psCoefU,
                    psCoefV,
                    psTmp,
                    paucPredTmp[3],
                    nQPC
                    //dLambda  
                );
      }
			else{ // P_SLICE
				memcpy( pasMV, pasTopMV-1, sizeof(xMV)*(MAX_MV_XY+2) );
#if MV_CLIP
				xMV mv_min_ipel = {0,0};
				xMV mv_max_ipel = {0,0};
				xCuMvClip(h, x, y, iCUWidth, iCUHeight);
				mv_min_ipel = h->mv_min_ipel;
				mv_max_ipel = h->mv_max_ipel;
#endif				                                                     
				double dLambda = 0.578 * pow(2.0, (nQP - 12) / 3.0);
								//0.4624*LAMBDA_SCALING_FACTOR*pow(2.0, (nQP - 12) / 3.0);  //ref from x265 multicoreware -- FOR RDO info debug

#if MVD_COST_BY_TABLE  //MV cost for ME process
				UInt16 mvd_cost_table[2 * BC_MAX_MV];
				UInt16 *p_mvd_cost_table = mvd_cost_table + BC_MAX_MV;
				initMvdCostTable( sqrt(dLambda), p_mvd_cost_table);
#endif

				xEncDecideCU_Inter(
					h,
					pCTUCmd,
					paucPixY,
					pucRefY,
					paucPredY,
					paucRecY,
					psCoefY,
					pasMV + (MAX_MV_XY + 2) + 1,
					pasMVD,
					paucMVPIdx,
					pucModeY + (MAX_PU_XY + 2) + 1,
					pusModeYInfo0,
					pucModeYInfo1 + (MAX_PU_XY + 1) + 1,
					psTmp,
					paucPredTmp,
					paucRecYTmp,
					pasCoefYTmp,
					paiCbfYTmp,
					nMaxCuWidth,
					nQP,
					dLambda,
					iRefStrideY,
					offY+y*(MAX_CU_SIZE),
					offX+x*(MAX_CU_SIZE), 
					mv_min_ipel,
					mv_max_ipel
					,0
					,ppcCabacRdo
					,&(h->rdoParam[y])
#if MVD_COST_BY_TABLE
					,p_mvd_cost_table
#endif
					,y
					,x
					);

				xEncInterCTU(
					paucPixY,
					paucPredY,
					paucRecY,
					paucPixU,
					pucRefU,
					paucPredU,
					paucRecU,
					paucPixV,
					pucRefV,
					paucPredV,
					paucRecV,
					pasMV + (MAX_MV_XY + 2) + 1,
					//pasMVD,
					//paucMVPIdx,
					pusModeYInfo0,
					pucModeYInfo1 + (MAX_PU_XY + 1) + 1,
					psCoefY,
					psCoefU,
					psCoefV,
					psTmp,
					//paucPredTmp,
					nQP,
					//dLambda,  
					iRefStrideY
					);
			}

			memcpy(h->cabacRdoLast[y], ppcCabacRdo[0], sizeof(xCabac)); //load the CTX after Inter MD RDO for the next CTU, the CTX for encoding the curr CTU is updated in Compress process
			if (x==1 && y!=iCUHeight-1){
				memcpy(&h->cabacRdoSaved, ppcCabacRdo[0], sizeof(xCabac));
			}

#ifdef _OPENMP
			//printf( "----> CTU(%3d,%3d) Compressing Done! \n", (offY+y), (offX+x));
#endif
			
			pucTopPixY[-1] = ucSavedLT;
      pucTopPixU[-1] = ucSavedLTU;
      pucTopPixV[-1] = ucSavedLTV;
      pasTopMV[-1]   = savedMvLT;

			pCmd += xEncWriteCU(
                            eSliceType,
                            pCmd,
                            pucModeY + (MAX_PU_XY + 2) + 1,
                            pusModeYInfo0,
                            pucModeYInfo1 + (MAX_PU_XY + 1) + 1,
                            pasMVD,
                            paucMVPIdx,
                            psCoefY,
                            psCoefU,
                            psCoefV,
                            offX+x*(MAX_CU_SIZE),
                            offY+y*(MAX_CU_SIZE)
#if		CU_TYPE_STAT
							, h
#endif
							,pasMV + (MAX_MV_XY + 2) + 1
//#if DEBLOCK //x64 modify
							,h->iWidth
//#endif
            );
			
			// FinishCU
#if USE_WPP_YANAN
						if ( x==(iCUWidth-1) ) {
							if (y!= (iCUHeight-1)){
								xEncodeTerminatingBit_pCmd( &pCmd,  0 ); 
							}
#else
						if ( (!bTiles || (idxTiles == 3) )  && bLastCU ) {
#endif
							xEncodeTerminatingBit_pCmd( &pCmd,  1 );
						}
						else {
							xEncodeTerminatingBit_pCmd( &pCmd,  0 );
							if ( bTiles && bLastCU ) {
									xEncodeTerminatingBit_pCmd( &pCmd,  1 );
							}
						}

            // Stage 5a: Update reconstruct
            xEncCacheStoreCU( h, &cache, (offX+x), (offY+y) );

            // Stage 5b: Update context
            ucSavedLT = pucTopPixY[iCUSize - 1];
            ucSavedLTU = pucTopPixU[iCUSize/2 - 1];
            ucSavedLTV = pucTopPixV[iCUSize/2 - 1];
            savedMvLT = pasTopMV[iCUSize/MIN_MV_SIZE - 1];
            xEncCacheUpdate(
                h,
                &cache,
                iCUSize,
                uiOffset,
                idxTiles
            );
            uiOffset += iCUSize;
           
						#ifdef _OPENMP
						//printf( "----> CTU(%3d,%3d) Encoding Done! \n", (offY+y), (offX+x));
            #ifndef WIN32
            if ( y != iCUHeight-1 )
                h->cuaddr[bsOff+y+1] = x;
            while( (h->cuaddr[bsOff+y] != iCUWidth-1) && ((h->cuaddr[bsOff+y] - (x+1)) < 2) ) {
                SLEEP(1);
            }
            #else
            if ( x >= 2 ) {
                //printf("Enqueue[%2d]=%d\n", bsOff+y+1, x);
                if ( y != iCUHeight-1 )
                    fifo_enq( &h->cuaddr[bsOff+y+1], x );
            }
            if ( y && (x != iCUWidth-1) ) {
                fifo_deq( &h->cuaddr[bsOff+y], &dummy );
                //printf("Dequeue[%2d]=%d\n", bsOff+y, dummy);
            }
            #endif
            #endif

            Int32 iCmdNum = (pCmd - pCmd0);
            h->auiCabacCmdNum[bsOff+y][x] = iCmdNum;
            #ifndef __GNUC__
            //assert( iCmdNum < MAX_CMDBUF_SIZE );
            #endif
        } // end of x
        
				#ifdef _OPENMP
        #ifdef WIN32
        if ( y != iCUHeight-1 ) {
            //printf("Enqueue[%2d]=%d\n", bsOff+y+1, -2);
            //printf("Enqueue[%2d]=%d\n", bsOff+y+1, -1);
            fifo_enq( &h->cuaddr[bsOff+y+1], 0 );
            fifo_enq( &h->cuaddr[bsOff+y+1], 0 );
        }
        #endif
        //omp_unset_lock(&h->bslck[bsOff+y]); //lzy_temp
        #endif
    } // end of y
}

static
void xCompressCU( X265_t *h, CInt32 iTilesT, CInt32 iTilesB )//x64 modify, convert to Int
{
  Int iTilesT_Temp = iTilesT;//x64 modify, avoid warning 
	Int iTilesB_Temp = iTilesB;//x64 modify, avoid warning 

	CUInt32      uiHeight    = h->iHeight;
   CUInt        nMaxCuWidth = h->ucMaxCUWidth;
   CInt        iCUWidth    = h->PicWidthInCUs;//x64 modify 
   CInt        iCUHeight   = h->PicHeightInCUs;//x64 modify
   CUInt32      bTiles      = h->bTiles;
    xCabac     *pCabac      = &h->cabac;
    xBitStream *pBS         = &h->bs;
    int x, y;

    xCabacInit( h );
    xCabacReset( &h->cabac );

#if USE_WPP_YANAN
	Int32 iNumEntropyPointOffsets = iCUHeight-1; //x64 modify, change type to Int
	Int32 iEntropyPointOffsets[128] = {0};//in unit of BYTE,//x64 modify, change type to Int
	Int   iSubstreamSizes     [128] = {0};//in unit of BIT,//x64 modify, change type to Int
	Int32  iBytes0[129] = {0};

	//save pBS state
	UInt8 *pucBitsSaved      = pBS->pucBits;
	UInt8 *pucBits0Saved     = pBS->pucBits0;
	Int    nCachedBitsSaved  = pBS->nCachedBits;
	UInt32 dwCacheSaved      = pBS->dwCache;
	//re-init pBS
  h->pOutBuf0[0] = 0xFF;
	h->pOutBuf0[1] = 0xFF;
	pBS->pucBits        = 
	pBS->pucBits0       = h->pOutBuf0+2;
	pBS->dwCache        = 0;
	pBS->nCachedBits    = 0;
	pBS->numPreventByte = 0;

	xCabac *pCabacSaved = &h->cabacSavedWPP;
#endif 
	
	for( y=0; y < (int)(bTiles ? 2 : 1) * iCUHeight; y++ ) {
        #ifdef _OPENMP
        //omp_set_lock(&h->bslck[y]); //lzy_temp
        #endif
        for( x=0; x < (int) iCUWidth; x++ ) {
            UInt32 *pCmd    = h->auiCabacCmd[y][x];
            Int32   iCmdNum = h->auiCabacCmdNum[y][x];
#if SAO
            UInt32 *pSaoCmd    = h->auiSaoCmd[y][x];
            Int32   iSaoCmdNum = h->auiSaoCmdNum[y][x];
#endif
            assert(iCmdNum < MAX_CMDBUF_SIZE);
						
#if USE_WPP_YANAN
			if (!x && y){ 
				xCabacFlush( pCabac, pBS );
				xWriteByteAlignment(pBS); 
				iBytes0[y] = xBitFlush(pBS);

				iEntropyPointOffsets[y-1] =  (iBytes0[y]) - (iBytes0[y-1]) - (pBS->numPreventByte);
				pBS->numPreventByte = 0;
				iSubstreamSizes		[y-1] = (iEntropyPointOffsets[y-1]<<3) + pBS->nCachedBits;
			}
						
			if (!x && y){
				pCabac->uiLow      = 0;
				pCabac->uiRange    = 510;
				pCabac->iBitsLeft  = 23;
				pCabac->ucCache		 = 0xFF;
				pCabac->uiNumBytes = 0;
				memcpy(pCabac->contextModels, pCabacSaved->contextModels, MAX_NUM_CTX_MOD*sizeof(UInt8));
			}
#endif

#if SAO
            xWriteCabacCmds( pCabac, pBS, pSaoCmd,	iSaoCmdNum );
#endif
            xWriteCabacCmds( pCabac, pBS, pCmd,		iCmdNum );

#if !SAO
            if ( bTiles ) {
                if ( (y == iTilesT-1) || (y == 2*iTilesT-1) || (y == 2*iTilesT+iTilesB-1) ) {
                    xCabacFlush( pCabac, pBS );
                    xWriteByteAlignment(pBS);
                    xBitFlush(pBS);
                    xCabacInit( h );
                    xCabacReset( &h->cabac );
                }
            }
#endif

#if USE_WPP_YANAN
			if (x==1 && y!=(iCUHeight-1)){
				memcpy(pCabacSaved->contextModels, pCabac->contextModels, MAX_NUM_CTX_MOD*sizeof(UInt8));
			}
#endif
     }//end of x
    }//end of y

	Int32 nWPPBytes0 = 0;//x64 modify, convert to Int, try to rename to iWPPBytes later
	Int32 nWPPBytes1 = 0;
#if USE_WPP_YANAN
	xCabacFlush( pCabac, pBS );
	xWriteByteAlignment(pBS); 
	iBytes0[iNumEntropyPointOffsets+1] = xBitFlush(pBS);
	iEntropyPointOffsets[iNumEntropyPointOffsets] =  iBytes0[iNumEntropyPointOffsets+1] - iBytes0[iNumEntropyPointOffsets] - (Int)(pBS->numPreventByte) ;//x64 modify, force type conversion pBS->numPreventByte
	pBS->numPreventByte = 0;
	iSubstreamSizes		[iNumEntropyPointOffsets] = (iEntropyPointOffsets[iNumEntropyPointOffsets]<<3) + pBS->nCachedBits;

	Int   numZeroSubstreamsAtEndOfSlice = 0;
	for (Int idx=iNumEntropyPointOffsets-2; idx>=0; idx--){
		if (!iSubstreamSizes[idx]){
			numZeroSubstreamsAtEndOfSlice++;
		}
		else{
			break;
		}
	}
	Int32 maxOffset = 0;//x64 modify
	for (Int idx=0; idx<(Int)iNumEntropyPointOffsets; idx++){
		if (iEntropyPointOffsets[idx] > maxOffset){
			maxOffset = iEntropyPointOffsets[idx];
		}
	}
	UInt offsetLenMinus1 = 0;
	while (maxOffset >= (1 << (offsetLenMinus1+1))){
		offsetLenMinus1++;
		assert(offsetLenMinus1+1<32);
	}

	UInt32 nTotalCtuBytes = xBitFlush(pBS); //NO.of BYTES written when finish all CUs

	//restore pBS state
	pBS->pucBits     = pucBitsSaved;
	pBS->pucBits0    = pucBits0Saved;
	pBS->nCachedBits = nCachedBitsSaved;
	pBS->dwCache     = dwCacheSaved;

	if (h->bUseRateCtrl) {
		nWPPBytes0 = xBitFlush(pBS);
	}
	xWriteTilesWPPEntryPoint_Yanan(h, iNumEntropyPointOffsets, offsetLenMinus1, iEntropyPointOffsets);
	if (h->bUseRateCtrl) {
		nWPPBytes1 = xBitFlush(pBS);
	}
	xBitFlush(pBS);

	//concatenating bins &update pBS info
	memcpy(pBS->pucBits, h->pOutBuf0+2, sizeof(UInt8)*(nTotalCtuBytes-0));
	pBS->pucBits     += (nTotalCtuBytes-0);
#endif
	if (h->bUseRateCtrl && USE_WPP_YANAN){
		h->rc.rcPic.m_picActualHeaderBits += ((nWPPBytes1-nWPPBytes0)<<3);
	}

    xCabacFlush( pCabac, pBS );
}

Int32 xEncodeFrame( X265_t *h, xFrame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize)
{
   CInt32      iWidth     = h->iWidth;//x64 modify, change type to Int
   CInt32      iHeight    = h->iHeight;//x64 modify, change type to Int
   CInt        iMaxCuWidth = h->ucMaxCUWidth;//x64 modify, change type to Int
   CInt        iCUHeight   = h->iCUHeight;//iHeight / iMaxCuWidth;//x64 modify, change type to Int
   CInt        iCUWidth    = h->iCUWidth;//iWidth  / iMaxCuWidth;//x64 modify, change type to Int
	 Int          actualTotalBits  = 0;//for rate control
	 Int          actualHeaderBits = 0;//for rate control
   CUInt32      bTiles      = h->bTiles;
   CUInt32      numTiles    = bTiles ? 4 : 1;
   CInt32       iTilesL     = (iCUWidth >> bTiles);
   CInt32       iTilesR     = (iCUWidth - iTilesL);
   CInt32       iTilesT     = (iCUHeight >> bTiles);//x64 modify, convert to Int
   CInt32       iTilesB     = (iCUHeight - iTilesT);
    xCabac     *pCabac      = &h->cabac;
    xBitStream *pBS         = &h->bs;
    Int i; //x64
   const xSliceType eSliceType = h->eSliceType;

    /// Copy to local
    h->pFrameCur = pFrame;
    h->pFrameRef = &h->refn[0];
		h->pFrameRec = &h->refn[MAX_REF_NUM];
    /// Initial local
    xCabacInit( h );
    xCabacReset( &h->cabac );
    memset( h->aucTopModeYInfo1, MODE_INVALID, sizeof(h->aucTopModeYInfo1) );
    h->iPoc++;
    xBitStreamInit( pBS, pucOutBuf, uiBufSize );

    // Padding Reference Frame
    if ( h->eSliceType != SLICE_I ) {
		xFramePadding( &h->refn[0], iWidth, iHeight, 0 );
    }

    // FIXME: Two IDR will be make wrong decode output, so I send them first frame only.
    if ( eSliceType == SLICE_I ) {
        h->iPoc = 0; //-wxw 
        /// Write VPS Header
        xPutBits32(pBS, 0x01000000);
        xPutBits(pBS, 0x4001, 16); // [0nnnnnn0 00000ttt ]
        xWriteVPS(h);
        xBitFlush(pBS);

        /// Write SPS Header
        xPutBits32(pBS, 0x01000000);
        xPutBits(pBS, 0x4201, 16); // [0nnnnnn0 00000ttt ]
        xWriteSPS(h);
        xBitFlush(pBS);

        /// Write PPS Header
        xPutBits32(pBS, 0x01000000);
        xPutBits(pBS, 0x4401, 16); // [0nnnnnn0 00000ttt ]
        xWritePPS(h);
        xBitFlush(pBS);
    }

    /// Write Silces Header
    // FIX_ME: the HM-6.1 can't decoder my IDR only stream,
    //         they want CRA, so I do this chunk
    if ( eSliceType == SLICE_I ) {
        xPutBits32(pBS, 0x26010000);
    }
    else {
        // CHECK_ME: modify it later, DONT direct access to memory
        *pBS->pucBits++ = 0x00;
        xPutBits32(pBS, 0x02010000);
    }
    xPutBits(pBS, 0x01, 8);
#if !SAO
		if(h->bUseRateCtrl){
			actualTotalBits = Int(pBS->pucBits-pBS->pucBits0)<<3;
		}
    xWriteSliceHeader(h);
		if(h->bUseRateCtrl){
			actualHeaderBits = (xBitFlush(pBS)<<3)-actualTotalBits;
		}
#endif

#ifdef _OPENMP
    memset(&h->cuaddr, 0, sizeof(h->cuaddr));
    memset(&h->bslck, 0, sizeof(h->bslck));
    for( i=0; i<(Int)(bTiles ? 2 : 1) * iCUHeight+1; i++ ) {//x64 modify
        omp_init_lock(&h->bslck[i]);
        omp_set_lock (&h->bslck[i]);
        #ifdef WIN32
        fifo_init( &h->cuaddr[i], iCUWidth );
        #endif
    }
    #ifndef WIN32
    h->cuaddr[0] = (iCUWidth-1);
    if ( bTiles ) {
        h->cuaddr[1*iTilesT        ] = (iCUWidth-1);
        h->cuaddr[2*iTilesT        ] = (iCUWidth-1);
        h->cuaddr[2*iTilesT+iTilesB] = (iCUWidth-1);
    }
    #endif
#endif

    #ifdef _OPENMP
    omp_set_nested(1);
    //#pragma omp parallel num_threads(2)
    #pragma omp sections
    #endif
    {
        #ifdef _OPENMP
        //if (omp_get_thread_num() == 0)
        #pragma omp section
        #endif
        {
            Int32 n;
            #ifdef _OPENMP
            #pragma omp parallel default(none) private(n) shared(h) num_threads(numTiles)
            #pragma omp for schedule(dynamic, 1) /*ordered*/ nowait
            #endif
            for( n=0; n<(Int)numTiles; n++ ) {//x64
                CInt32 dx = n &  1;
                CInt32 dy = n >> 1;
                xEncodeCUs(
                    h,
                    (!dx ? iTilesL : iTilesR),
                    (!dy ? iTilesT : iTilesB),
                    (!dx ? 0 : iTilesL),
                    (!dy ? 0 : iTilesT),
                    (n<3 ? n*iTilesT : 2*iTilesT+iTilesB),
                    n
                );
            }
        }
#if !SAO
        #ifdef _OPENMP
        //if (omp_get_thread_num() == 1)
        #pragma omp section
        #endif
        {
            xCompressCU( h, iTilesT, iTilesB );
        }
#endif
    }

#ifdef _OPENMP
		for( i=0; i<(Int)(bTiles ? 2 : 1) * iCUHeight+1; i++ ) {//x64 modify
			omp_destroy_lock(&h->bslck[i]);
#ifdef WIN32
			fifo_free( &h->cuaddr[i] );
#endif
		}
#endif

#if !SAO
    xWriteSliceEnd( h );
#endif

#if SAO
    //calcSaoStatsRowCus_BeforeDblk(h);
#endif

//#if DEBLOCK //x64 modify
	xDeblockFrame( 
		h, 
		iCUWidth, 
		iCUHeight, 
		0, //tile offset is 0 - not support tiles
		0, //tile offset is 0 - not support tiles
		0, //bsOff, 
		0 //idxTiles 
		);
//#endif

#if SAO
  SAOParam *saoParam = &h->sao_param;
 	h->sao_depth = (h->eSliceType == SLICE_I ? 1 : 1);

	//clock_t start = clock();
  rdoSaoUnitRowInit(h);
	rdoSaoUnitAll(h);
	//double dTime = (double)(clock()-start) / CLOCKS_PER_SEC;
	//printf("\n SAO Time: %.3f sec. \n", dTime);


	//--wxw
	xCabacInit( h ); //end of one frame, reset it 
	xCabacReset( &h->cabac );
    //saoParam->bSaoFlag[0] = 0;
    //saoParam->bSaoFlag[1] = 0;

		//clock_t start = clock();
    if (saoParam->bSaoFlag[0]){   //LUMA	
      processSaoUnitAll(h, saoParam->saoLcuParam[0], 0);
    }
    if (saoParam->bSaoFlag[1]){   //CHROMA
      processSaoUnitAll(h, saoParam->saoLcuParam[1], 1);
      processSaoUnitAll(h, saoParam->saoLcuParam[2], 2);
    }
		//double dTime = (double)(clock()-start) / CLOCKS_PER_SEC;
		//printf("\n SAO Time: %.3f sec. \n", dTime);

    memset(h->auiSaoCmdNum, 0, sizeof(h->auiSaoCmdNum));

    if (saoParam->bSaoFlag[0] || saoParam->bSaoFlag[1]) {
        int numCuInWidth    = h->PicWidthInCUs;
        int allowMergeLeft  = 1;
        int allowMergeUp    = 1;
        int rx, ry;
        int compIdx;

        for(ry=0; ry<(Int)h->PicHeightInCUs; ry++) {
            for(rx=0; rx<(Int)h->PicWidthInCUs; rx++) {
                int cuAddr          = ry * numCuInWidth + rx;
                int cuAddrInSlice   = cuAddr;
                UInt32 *pCmd0       = h->auiSaoCmd[ry][rx];
                UInt32 *pCmd        = pCmd0;

                allowMergeLeft = (rx > 0) && (cuAddrInSlice != 0);
                allowMergeUp = (ry > 0) && (cuAddrInSlice >= 0);

                int mergeLeft = saoParam->saoLcuParam[0][cuAddr].mergeLeftFlag;
                int mergeUp = saoParam->saoLcuParam[0][cuAddr].mergeUpFlag;
                if (allowMergeLeft) {
                    codeSaoMerge(&pCmd, mergeLeft);
                }
                else {
                    mergeLeft = 0;
                }
                if (mergeLeft == 0)
                {
                    if (allowMergeUp) {
                        codeSaoMerge(&pCmd, mergeUp);
                    }
                    else {
                        mergeUp = 0;
                    }
                    if (mergeUp == 0) {
                        for (compIdx = 0; compIdx < 3; compIdx++) {
                            if ((compIdx == 0 && saoParam->bSaoFlag[0]) || (compIdx > 0 && saoParam->bSaoFlag[1])) {
                                encodeSaoOffset(&pCmd, &saoParam->saoLcuParam[compIdx][cuAddr], compIdx);
                            }
                        }
                    }
                }
                h->auiSaoCmdNum[ry][rx] = pCmd - pCmd0;
            } // end of rx
        } // end of ry
    }

		if(h->bUseRateCtrl){
			actualTotalBits = Int(pBS->pucBits-pBS->pucBits0)<<3;
		}
		xWriteSliceHeader(h);
		if(h->bUseRateCtrl){
			actualHeaderBits = (xBitFlush(pBS)<<3)-actualTotalBits;
		}

    xCompressCU( h, iTilesT, iTilesB );
    xWriteSliceEnd( h );

    rdoSaoUnitRowEnd(h);
#endif

    #ifdef CHECK_SEI
    {
        UInt8 md5[3][16];
        MD5Context ctx;

        // Y
        MD5Init( &ctx );
        for( i=0; i<iHeight; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucY + i * h->pFrameRec->iStrideY, iWidth );
        }
        MD5Final( &ctx, md5[0] );

        // U
        MD5Init( &ctx );
        for( i=0; i<iHeight/2; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucU + i * h->pFrameRec->iStrideC, iWidth/2 );
        }
        MD5Final( &ctx, md5[1] );

        // V
        MD5Init( &ctx );
        for( i=0; i<iHeight/2; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucV + i * h->pFrameRec->iStrideC, iWidth/2 );
        }
        MD5Final( &ctx, md5[2] );

        /// Write SEI Header
        xBitFlush(pBS);
        xPutBits32(pBS, 0x50010000); // [0nnnnnn0 00000ttt ]
        xPutBits(pBS, 0x01, 8);
        xPutBits(pBS, 0x84, 8); // PICTURE_DIGEST
        xPutBits(pBS, 0x31, 8); // Payload length
        xPutBits(pBS, 0x00, 8); // Method = MD5
        for( i=0; i<3; i++ ) {
            int j;
            for( j=0; j<16; j++ ) {
                xPutBits(pBS, md5[i][j], 8);
            }
        }
        xWriteRBSPTrailingBits(pBS);
        xBitFlush(pBS);
    }
    #endif

    if ( h->bWriteRecFlag ) {
        static FILE *fpx=NULL;
        if ( fpx == NULL ) {
            fpx = fopen("OX.YUV", "wb");
        }
        assert( fpx != NULL );
        for( i=0; i<iHeight; i++ ) {
            fwrite(h->pFrameRec->pucY + i * h->pFrameRec->iStrideY, 1, iWidth, fpx);
        }
        for( i=0; i<iHeight/2; i++ ) {
            fwrite(h->pFrameRec->pucU + i * h->pFrameRec->iStrideC, 1, iWidth/2, fpx);
        }
        for( i=0; i<iHeight/2; i++ ) {
            fwrite(h->pFrameRec->pucV + i * h->pFrameRec->iStrideC, 1, iWidth/2, fpx);
        }
        fflush(fpx);
    }

		// Save Visual CU Split
		#ifdef DEBUG_VIS
		if ( h->bWriteVisCUFlag ) {
				static FILE *fpx=NULL;
				if ( fpx == NULL )
						fpx = fopen("VIS_CU.YUV", "wb");
				assert( fpx != NULL );
				fwrite(h->frm_vis.pucY, 1, iWidth*iHeight*3/2, fpx);
				fflush(fpx);
		}
		#endif

		// Update Reference Frame Pointer
		xFrame tmp = h->refn[MAX_REF_NUM];

		for( i=MAX_REF_NUM; i>0; i-- ) {
					h->refn[i] = h->refn[i-1];
    }
    h->refn[0] = tmp;

    /* 7.4.1.1
     * ... when the last byte of the RBSP data is equal to 0x00 (which can
     * only occur when the RBSP ends in a cabac_zero_word), a final byte equal
     * to 0x03 is appended to the end of the data.
     */

    h->uiFrames++;

    Int32 iLen = xBitFlush(pBS);
    if ( pucOutBuf[iLen-1] == 0 )
        pucOutBuf[iLen++] = 0x03;

		if (h->bUseRateCtrl){
			xRCPicUpdateAfterPic(h, actualHeaderBits, (iLen<<3));
			xRCSeqUpdateAfterPic(h, (iLen<<3));
			if (h->eSliceType!=SLICE_I){
				xRCGOPUpdateAfterPic(h, (iLen<<3) );
			}
			else{// for intra picture, the estimated bits are used to update the current status in the GOP
				xRCGOPUpdateAfterPic(h, h->rc.rcPic.m_estBits );
			}
		}

    return iLen;
}

Int32 xEncodeFrameAPI( X265_t *h, xFrame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize, int* nal_num, int* nal_len)
{
  (*nal_num) = 0 ; 
	
	CInt32      iWidth     = h->iWidth;
   CInt32      iHeight    = h->iHeight;
   CInt        iMaxCuWidth = h->ucMaxCUWidth;
	 CInt        iCUHeight   = h->iCUHeight;//iHeight / iMaxCuWidth;//x64 modify, change type to Int
	 CInt        iCUWidth    = h->iCUWidth;//iWidth  / iMaxCuWidth;//x64 modify, change type to Int
	 Int          actualTotalBits  = 0;//for rate control
	 Int          actualHeaderBits = 0;//for rate control
   CUInt32      bTiles      = h->bTiles;
   CUInt32      numTiles    = bTiles ? 4 : 1;
   CInt32       iTilesL     = (iCUWidth >> bTiles);
   CInt32       iTilesR     = (iCUWidth - iTilesL);
   CInt32       iTilesT     = (iCUHeight >> bTiles);
   CInt32       iTilesB     = (iCUHeight - iTilesT);
    xCabac     *pCabac      = &h->cabac;
    xBitStream *pBS         = &h->bs;
    Int i;
   const xSliceType eSliceType = h->eSliceType;

    /// Copy to local
    h->pFrameCur = pFrame;
    h->pFrameRef = &h->refn[0];
		h->pFrameRec = &h->refn[MAX_REF_NUM];
    /// Initial local
    xCabacInit( h );
    xCabacReset( &h->cabac );
    memset( h->aucTopModeYInfo1, MODE_INVALID, sizeof(h->aucTopModeYInfo1) );
    h->iPoc++;
    xBitStreamInit( pBS, pucOutBuf, uiBufSize );

    // Padding Reference Frame
    if ( h->eSliceType != SLICE_I ) {
			xFramePadding( &h->refn[0], iWidth, iHeight, 0 );
    }

		
    Int32 nLenPps = 0;
    if ( eSliceType == SLICE_I ) {
        h->iPoc = 0; //-wxw 
        /// Write VPS Header
        xPutBits32(pBS, 0x01000000);
        xPutBits(pBS, 0x4001, 16); // [0nnnnnn0 00000ttt ]
        xWriteVPS(h);
				Int32 nLenVPS = xBitFlush(pBS);
				nal_len[(*nal_num)++] = nLenVPS;//------------>nal_num

        /// Write SPS Header
        xPutBits32(pBS, 0x01000000);
        xPutBits(pBS, 0x4201, 16); // [0nnnnnn0 00000ttt ]
        xWriteSPS(h);
				Int32 nLenSPS = xBitFlush(pBS);
				nal_len[(*nal_num)++] = nLenSPS - nLenVPS;//------------>nal_num

        /// Write PPS Header
        xPutBits32(pBS, 0x01000000);
        xPutBits(pBS, 0x4401, 16); // [0nnnnnn0 00000ttt ]
        xWritePPS(h);
				nLenPps = xBitFlush(pBS);
				nal_len[(*nal_num)++] = nLenPps - nLenSPS;//------------>nal_num
    }

    /// Write Silces Header
    // FIX_ME: the HM-6.1 can't decoder my IDR only stream,
    //         they want CRA, so I do this chunk
    if ( eSliceType == SLICE_I ) {
        xPutBits32(pBS, 0x26010000);
    }
    else {
        // CHECK_ME: modify it later, DONT direct access to memory
        *pBS->pucBits++ = 0x00;
        xPutBits32(pBS, 0x02010000);
    }
    xPutBits(pBS, 0x01, 8);
#if !SAO
		if(h->bUseRateCtrl){
			actualTotalBits = Int(pBS->pucBits-pBS->pucBits0)<<3;
		}
    xWriteSliceHeader(h);
		if(h->bUseRateCtrl){
			actualHeaderBits = (xBitFlush(pBS)<<3)-actualTotalBits;
		}
#endif
		
		
#ifdef _OPENMP
    memset(&h->cuaddr, 0, sizeof(h->cuaddr));
    memset(&h->bslck, 0, sizeof(h->bslck));
    for( i=0; i<(Int)(bTiles ? 2 : 1) * iCUHeight+1; i++ ) {
        omp_init_lock(&h->bslck[i]);
        //lzy_temp omp_set_lock (&h->bslck[i]);
        #ifdef WIN32
        fifo_init( &h->cuaddr[i], iCUWidth );
        #endif
    }
    #ifndef WIN32
    h->cuaddr[0] = (iCUWidth-1);
    if ( bTiles ) {
        h->cuaddr[1*iTilesT        ] = (iCUWidth-1);
        h->cuaddr[2*iTilesT        ] = (iCUWidth-1);
        h->cuaddr[2*iTilesT+iTilesB] = (iCUWidth-1);
    }
    #endif
#endif
		
    #ifdef _OPENMP
    omp_set_nested(1);
    //#pragma omp parallel num_threads(2)
    #pragma omp sections
    #endif
		{
#ifdef _OPENMP
			//if (omp_get_thread_num() == 0)
#pragma omp section
#endif
			{
				Int32 n;
#ifdef _OPENMP
#pragma omp parallel default(none) private(n) shared(h) num_threads(numTiles)
#pragma omp for schedule(dynamic, 1) /*ordered*/ nowait
#endif
				for( n=0; n<(Int)numTiles; n++ ) {//x64
					CInt32 dx = n &  1;
					CInt32 dy = n >> 1;
					xEncodeCUs(
						h,
						(!dx ? iTilesL : iTilesR),
						(!dy ? iTilesT : iTilesB),
						(!dx ? 0 : iTilesL),
						(!dy ? 0 : iTilesT),
						(n<3 ? n*iTilesT : 2*iTilesT+iTilesB),
						n
						);
				}
			}
#if !SAO
#ifdef _OPENMP
			//if (omp_get_thread_num() == 1)
#pragma omp section
#endif
			{
				xCompressCU( h, iTilesT, iTilesB );
			}
#endif
		}
		
#ifdef _OPENMP
		for( i=0; i<(Int)(bTiles ? 2 : 1) * iCUHeight+1; i++ ) {
			omp_destroy_lock(&h->bslck[i]);
#ifdef WIN32
			fifo_free( &h->cuaddr[i] );
#endif
		}
#endif

#if !SAO
    xWriteSliceEnd( h );
#endif

#if SAO
    //calcSaoStatsRowCus_BeforeDblk(h);
#endif
		
  if (DEBLOCK) {
		xDeblockFrame( 
			h, 
			iCUWidth, 
			iCUHeight, 
			0, //tile offset is 0 - not support tiles
			0, //tile offset is 0 - not support tiles
			0, //bsOff, 
			0 //idxTiles 
			);
	}

#if SAO
		SAOParam *saoParam = &h->sao_param;
		h->sao_depth = (h->eSliceType == SLICE_I ? 1 : 1);

		rdoSaoUnitRowInit(h);
		rdoSaoUnitAll(h);

		//--wxw
		xCabacInit( h ); //end of one frame, reset it 
		xCabacReset( &h->cabac );
		//saoParam->bSaoFlag[0] = 0;
		//saoParam->bSaoFlag[1] = 0;

		if (saoParam->bSaoFlag[0]){   //LUMA	
			processSaoUnitAll(h, saoParam->saoLcuParam[0], 0);
		}
		if (saoParam->bSaoFlag[1]){   //CHROMA
			processSaoUnitAll(h, saoParam->saoLcuParam[1], 1);
			processSaoUnitAll(h, saoParam->saoLcuParam[2], 2);
		}

		memset(h->auiSaoCmdNum, 0, sizeof(h->auiSaoCmdNum));
		
		if (saoParam->bSaoFlag[0] || saoParam->bSaoFlag[1]) {
			int numCuInWidth    = h->PicWidthInCUs;
			int allowMergeLeft  = 1;
			int allowMergeUp    = 1;
			int rx, ry;
			int compIdx;

			for(ry=0; ry<(Int)h->PicHeightInCUs; ry++) {
				for(rx=0; rx<(Int)h->PicWidthInCUs; rx++) {
					int cuAddr          = ry * numCuInWidth + rx;
					int cuAddrInSlice   = cuAddr;
					UInt32 *pCmd0       = h->auiSaoCmd[ry][rx];
					UInt32 *pCmd        = pCmd0;

					allowMergeLeft = (rx > 0) && (cuAddrInSlice != 0);
					allowMergeUp = (ry > 0) && (cuAddrInSlice >= 0);

					int mergeLeft = saoParam->saoLcuParam[0][cuAddr].mergeLeftFlag;
					int mergeUp = saoParam->saoLcuParam[0][cuAddr].mergeUpFlag;
					if (allowMergeLeft) {
						codeSaoMerge(&pCmd, mergeLeft);
					}
					else {
						mergeLeft = 0;
					}
					if (mergeLeft == 0)
					{
						if (allowMergeUp) {
							codeSaoMerge(&pCmd, mergeUp);
						}
						else {
							mergeUp = 0;
						}
						if (mergeUp == 0) {
							for (compIdx = 0; compIdx < 3; compIdx++) {
								if ((compIdx == 0 && saoParam->bSaoFlag[0]) || (compIdx > 0 && saoParam->bSaoFlag[1])) {
									encodeSaoOffset(&pCmd, &saoParam->saoLcuParam[compIdx][cuAddr], compIdx);
								}
							}
						}
					}
					h->auiSaoCmdNum[ry][rx] = pCmd - pCmd0;
				} // end of rx
			} // end of ry
		}
		
		if(h->bUseRateCtrl){
			actualTotalBits = Int(pBS->pucBits-pBS->pucBits0)<<3;
		}
		xWriteSliceHeader(h);
		if(h->bUseRateCtrl){
			actualHeaderBits = (xBitFlush(pBS)<<3)-actualTotalBits;
		}

		xCompressCU( h, iTilesT, iTilesB );

		xWriteSliceEnd( h );

		rdoSaoUnitRowEnd(h);
#endif
		
    #ifdef CHECK_SEI
    {
        UInt8 md5[3][16];
        MD5Context ctx;

        // Y
        MD5Init( &ctx );
        for( i=0; i<iHeight; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucY + i * h->pFrameRec->iStrideY, iWidth );
        }
        MD5Final( &ctx, md5[0] );

        // U
        MD5Init( &ctx );
        for( i=0; i<iHeight/2; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucU + i * h->pFrameRec->iStrideC, iWidth/2 );
        }
        MD5Final( &ctx, md5[1] );

        // V
        MD5Init( &ctx );
        for( i=0; i<iHeight/2; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucV + i * h->pFrameRec->iStrideC, iWidth/2 );
        }
        MD5Final( &ctx, md5[2] );

        /// Write SEI Header
        xBitFlush(pBS);
        xPutBits32(pBS, 0x50010000); // [0nnnnnn0 00000ttt ]
        xPutBits(pBS, 0x01, 8);
        xPutBits(pBS, 0x84, 8); // PICTURE_DIGEST
        xPutBits(pBS, 0x31, 8); // Payload length
        xPutBits(pBS, 0x00, 8); // Method = MD5
        for( i=0; i<3; i++ ) {
            int j;
            for( j=0; j<16; j++ ) {
                xPutBits(pBS, md5[i][j], 8);
            }
        }
        xWriteRBSPTrailingBits(pBS);
        xBitFlush(pBS);
    }
    #endif

		// Update Reference Frame Pointer
		xFrame tmp = h->refn[MAX_REF_NUM];

		for( i=MAX_REF_NUM; i>0; i-- ) {
					h->refn[i] = h->refn[i-1];
    }
    h->refn[0] = tmp;

    /* 7.4.1.1
     * ... when the last byte of the RBSP data is equal to 0x00 (which can
     * only occur when the RBSP ends in a cabac_zero_word), a final byte equal
     * to 0x03 is appended to the end of the data.
     */

    h->uiFrames++;

    Int32 iLen = xBitFlush(pBS);
    if ( pucOutBuf[iLen-1] == 0 )
        pucOutBuf[iLen++] = 0x03;

		if (h->bUseRateCtrl){
			xRCPicUpdateAfterPic(h, actualHeaderBits, (iLen<<3));
			xRCSeqUpdateAfterPic(h, (iLen<<3));
			if (h->eSliceType!=SLICE_I){
				xRCGOPUpdateAfterPic(h, (iLen<<3) );
			}
			else{// for intra picture, the estimated bits are used to update the current status in the GOP
				xRCGOPUpdateAfterPic(h, h->rc.rcPic.m_estBits );
			}
		}

		nal_len[(*nal_num)++] = iLen - nLenPps;//------------>nal_num
    return iLen;
}