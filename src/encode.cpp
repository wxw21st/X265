/*****************************************************************************
 * encode.cpp: Main for encode
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
#ifdef CHECK_SEI
#include "md5.h"
#endif

// ***************************************************************************
// * Internal Function Declares
// ***************************************************************************
void xEncCahceInitLine( xCache *pCache );
void xEncCacheLoadCU( X265_t *h, xCache *pCache, UInt uiCUX, UInt uiCUY );
void xEncCacheStoreCU( X265_t *h, xCache *pCache, UInt uiCUX, UInt uiCUY );
void xEncCacheUpdate(
    X265_t *h,
    xCache *pCache,
    UInt    nSize,
    UInt32  uiOffset
);
UInt32 xEncDecideCU(
    UInt8  *pucPixY,
    UInt8  *pucRecY,
    UInt8  *pucRefTopY,
    UInt8  *pucModeY,
    UInt16 *pusModeYInfo0,
    UInt8  *pucModeYInfo1,
    UInt8  *pucPredY,
    Int16  *psCoefY,
    Int16  *psTmp[2],
    UInt8  *paucPredTmp[MAX_CU_DEPTH],
     Int32  nSize,
    UInt32  dy,
    UInt32  nQP,
   CUInt32  lambda,
   CUInt32  bStrongIntraSmoothing
);
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
    Int16  *psTmp[2],
    UInt8  *pucPredTmp,
    UInt    nQPC,
    UInt    lambda
);
void xEncWriteCU(
    const xSliceType eSliceType,
    UInt8       *pucModeY,
    UInt16      *pucModeYInfo0,
    UInt8       *pucModeYInfo1,
    xMV         *pasMVD,
    UInt8       *paucMVPIdx,
    Int16       *psCoefY,
    Int16       *psCoefU,
    Int16       *psCoefV
);
#ifdef DEBUG_VIS
void xDrawVisCU(
    xFrame  *frm_vis,
    UInt8   *pucPixY,
    UInt8   *pucPixU,
    UInt8   *pucPixV,
    UInt8   *pucModeYInfo1,
    UInt     nX,
    UInt     nY,
    UInt     nWidth
);
#endif
Int xFillMvpCand(
    xMV        *psMVP,
    CUInt8     *pucModeY,
    const xMV  *psMV,
    CUInt       nSize
);
xMV xMotionSearch(
    CUInt8     *pucCurY,
    CUInt8     *pucRefY,
     UInt8     *pucPredY,
      Int16    *psTmp,
    CUInt32     nRefStrideY,
    CUInt       nSize
);
void xPredInterLumaBlk(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
      xMV       bmv,
      Int16    *psTmp
);
void xPredInterChromaBlk(
    CUInt8     *pucRefC,
     UInt8     *pucPredC,
    CUInt32     nRefStrideC,
    CUInt       nSizeC,
      xMV       bmv,
      Int16    *psTmp
);


// ***************************************************************************
// * Interface Functions
// ***************************************************************************
void xEncInit( X265_t *h )
{
    CUInt32 uiWidthPad  = h->usWidth  + PAD_YSIZE * 2;
    CUInt32 uiHeightPad = h->usHeight + PAD_YSIZE * 2;
    CUInt32 uiYSize     = uiWidthPad * uiHeightPad;
    CUInt32 uiPadOffY   = PAD_YSIZE * uiWidthPad   + PAD_YSIZE;
    CUInt32 uiPadOffC   = PAD_CSIZE * uiWidthPad/2 + PAD_CSIZE;

    int i;
    UInt8 *ptr;

    for( i=0; i < MAX_REF_NUM+1; i++ ) {
        ptr = (UInt8 *)MALLOC(uiYSize * 3 / 2);
        assert(ptr != NULL);
        h->refn[i].base = ptr;
        h->refn[i].pucY = (UInt8 *)ptr + uiPadOffY;
        h->refn[i].pucU = (UInt8 *)ptr + uiYSize + uiPadOffC;
        h->refn[i].pucV = (UInt8 *)ptr + uiYSize * 5 / 4 + uiPadOffC;
        h->refn[i].nStrideY = uiWidthPad;
        h->refn[i].nStrideC = uiWidthPad / 2;
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
}

void xEncFree( X265_t *h )
{
    int i;
    for( i=0; i < MAX_REF_NUM+1; i++ ) {
        assert( h->refn[i].pucY != NULL );
        FREE( h->refn[i].base );
    }
    memset( h->refn, 0, sizeof(h->refn) );

    #if defined(DEBUG_VIS)
    assert(h->frm_vis.pucY != NULL);
    FREE(h->frm_vis.pucY);
    #endif
}

// ***************************************************************************
// * Internal Functions
// ***************************************************************************
void xFramePadding( xFrame *pFrm, CUInt32 uiWidth, CUInt32 uiHeight )
{
    CUInt32 uiWidthY    = uiWidth;
    CUInt32 uiHeightY   = uiHeight;
    CUInt32 uiWidthC    = uiWidth/2;
    CUInt32 uiHeightC   = uiHeight/2;
    UInt    nStrideY    = pFrm->nStrideY;
    UInt    nStrideC    = pFrm->nStrideC;
    UInt8  *pucY        = pFrm->pucY;
    UInt8  *pucU        = pFrm->pucU;
    UInt8  *pucV        = pFrm->pucV;
    UInt8  *pucY0, *pucY1;
    UInt8  *pucU0, *pucU1;
    UInt8  *pucV0, *pucV1;
    int i;

    // Padding Left & Right
    pucY0 = pucY - PAD_YSIZE;
    pucU0 = pucU - PAD_CSIZE;
    pucV0 = pucV - PAD_CSIZE;
    pucY1 = pucY + uiWidthY;
    pucU1 = pucU + uiWidthC;
    pucV1 = pucV + uiWidthC;
    for( i=0; i<(int)uiHeightY; i++ ) {
        memset( pucY0 + i * nStrideY, pucY[i * nStrideY + 0           ], PAD_YSIZE );
        memset( pucY1 + i * nStrideY, pucY[i * nStrideY + uiWidthY - 1], PAD_YSIZE );
    }
    for( i=0; i<(int)uiHeightC; i++ ) {
        memset( pucU0 + i * nStrideC, pucU[i * nStrideC + 0           ], PAD_CSIZE );
        memset( pucU1 + i * nStrideC, pucU[i * nStrideC + uiWidthC - 1], PAD_CSIZE );
        memset( pucV0 + i * nStrideC, pucV[i * nStrideC + 0           ], PAD_CSIZE );
        memset( pucV1 + i * nStrideC, pucV[i * nStrideC + uiWidthC - 1], PAD_CSIZE );
    }

    // Padding Top & Bottom
    pucY1 = pucY0 + nStrideY * (uiHeightY - 1);
    pucU1 = pucU0 + nStrideC * (uiHeightC - 1);
    pucV1 = pucV0 + nStrideC * (uiHeightC - 1);
    for( i=0; i<PAD_YSIZE; i++ ) {
        memcpy( pucY0 - (i+1) * nStrideY, pucY0, nStrideY );
        memcpy( pucY1 + (i+1) * nStrideY, pucY1, nStrideY );
    }
    for( i=0; i<PAD_CSIZE; i++ ) {
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

static xCabac *g_pCabac = NULL;
static xBitStream *g_pBS = NULL;

extern void xCabacEncodeBin( xCabac *pCabac, xBitStream *pBS, UInt binValue, UInt nCtxState );
extern void xCabacEncodeBinEP( xCabac *pCabac, xBitStream *pBS, UInt binValue );
extern void xCabacEncodeBinsEP( xCabac *pCabac, xBitStream *pBS, UInt binValues, Int numBins );
extern void xCabacEncodeTerminatingBit( xCabac *pCabac, xBitStream *pBS, UInt binValue );
extern void testAndWriteOut( xCabac *pCabac, xBitStream *pBS );

#define xEncodeBin(binValue, nCtxState) \
    xCabacEncodeBin( g_pCabac, g_pBS, (binValue), (nCtxState) ); \
    testAndWriteOut( g_pCabac, g_pBS );

#define xEncodeBinsEP(binValue, numBins) \
    xCabacEncodeBinsEP( g_pCabac, g_pBS, (binValue), (numBins) ); \
    testAndWriteOut( g_pCabac, g_pBS );

#define xEncodeTerminatingBit(bLastCU) \
    xCabacEncodeTerminatingBit( g_pCabac, g_pBS, (bLastCU) ); \
    testAndWriteOut( g_pCabac, g_pBS );

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
Int getSigCtxInc(Int patternSigCtx, UInt scanIdx, Int posX, Int posY, Int log2BlockSize, Int nSize, UInt8 bIsLuma)
{
    CInt width  = nSize;
    CInt height = nSize;
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
UInt getSigCoeffGroupCtxInc(CUInt8 *uiSigCoeffGroupFlag, CUInt nCGPosX, CUInt nCGPosY, CUInt scanIdx, UInt nSize)
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
void codeLastSignificantXY( UInt nPosX, UInt nPosY, UInt nSize, UInt8 bIsLuma, UInt nScanIdx )
{
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
        xEncodeBin( 1, nCtxX + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }
    if( uiGroupIdxX < xg_uiGroupIdx[nSize-1]) {
        xEncodeBin( 0, nCtxX + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }

    // posY
    for( nCtxLast = 0; nCtxLast < uiGroupIdxY; nCtxLast++ ) {
        xEncodeBin( 1, nCtxY + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }
    if( uiGroupIdxY < xg_uiGroupIdx[ nSize - 1 ]) {
        xEncodeBin( 0, nCtxY + (blkSizeOffsetXY + (nCtxLast >>shiftXY)) );
    }

    if( uiGroupIdxX > 3 ) {
        UInt nCount = ( uiGroupIdxX - 2 ) >> 1;
        nPosX       = nPosX - xg_uiMinInGroup[ uiGroupIdxX ];
        xEncodeBinsEP( nPosX, nCount );
    }
    if( uiGroupIdxY > 3 ) {
        UInt nCount = ( uiGroupIdxY - 2 ) >> 1;
        nPosY       = nPosY - xg_uiMinInGroup[ uiGroupIdxY ];
        xEncodeBinsEP( nPosY, nCount );
    }
}

static
void xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
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
    xEncodeBinsEP( bins, numBins );
}

/** Coding of coeff_abs_level_minus3
 * \param uiSymbol value of coeff_abs_level_minus3
 * \param ruiGoRiceParam reference to Rice parameter
 * \returns Void
 */
static
void xWriteCoefRemainExGolomb( UInt nSymbol, UInt rParam )
{
    Int codeNumber  = (Int)nSymbol;
    UInt nLength;

    if (codeNumber < (COEF_REMAIN_BIN_REDUCTION << rParam)) {
        nLength = codeNumber>>rParam;
        xEncodeBinsEP( (1<<(nLength+1))-2 , nLength+1);
        // OPT_ME: use mask to replace '%'
        if ( rParam )
            xEncodeBinsEP( (codeNumber%(1<<rParam)),rParam);
    }
    else {
        nLength = rParam;
        codeNumber  = codeNumber - ( COEF_REMAIN_BIN_REDUCTION << rParam);
        while (codeNumber >= (1 << nLength)) {
            codeNumber -= (1 << nLength);
            nLength++;
        }
        xEncodeBinsEP( (1<<(COEF_REMAIN_BIN_REDUCTION+nLength+1-rParam))-2,COEF_REMAIN_BIN_REDUCTION+nLength+1-rParam );
        if ( nLength )
            xEncodeBinsEP( codeNumber, nLength);
    }
}

static
void xWriteUnaryMaxSymbol( UInt uiSymbol, UInt nCtx, Int iOffset, UInt uiMaxSymbol )
{
    if (uiMaxSymbol == 0) {
        return;
    }
  
    xEncodeBin( (uiSymbol ? 1 : 0), nCtx );
  
    if ( uiSymbol == 0 ) {
        return;
    }
  
    UInt bCodeLast = ( uiMaxSymbol > uiSymbol );
  
    while( --uiSymbol ) {
        xEncodeBin( 1, nCtx + iOffset );
    }

    if( bCodeLast ) {
        xEncodeBin( 0, nCtx + iOffset );
    }
}


static
void xEncodeCoeffNxN( Int16 *psCoef, UInt nSize, UInt8 bIsLuma, UInt nMode )
{
   CUInt    nStride      = (MAX_CU_SIZE >> (bIsLuma ? 0 : 1));
    UInt    nLog2Size    = xLog2( nSize - 1 );
    UInt32  uiNumSig     = countNonZeroCoeffs( psCoef, nStride, nSize );
    UInt    nScanIdx     = getCoefScanIdx( nSize, TRUE, bIsLuma, nMode );
   CUInt16 *scan         = NULL;
   CUInt16 *scanCG       = NULL;
   CUInt    uiShift      = MLS_CG_SIZE >> 1;
   CUInt    uiNumBlkSide = nSize >> uiShift;
   CInt     blockType    = nLog2Size;
    UInt8 uiSigCoeffGroupFlag[MLS_GRP_NUM];
    Int idx;

    // CHECK_ME: I think the size of 64x64 can't be here, but the HM say that 128x128 can be here?
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
    codeLastSignificantXY( posLastX, posLastY, nSize, bIsLuma, nScanIdx );

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
            UInt nCtxSig  = getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, nScanIdx, nSize );
            xEncodeBin( nSigCoeffGroup, nBaseCoeffGroupCtx + nCtxSig );
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
                    nCtxSig  = getSigCtxInc( patternSigCtx, nScanIdx, nPosX, nPosY, nLog2Size, nSize, bIsLuma );
                    xEncodeBin( nSig, nBaseCtx + nCtxSig );
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
                xEncodeBin( uiSymbol, nBaseCtxMod + c1 );
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
                    xEncodeBin( symbol, nBaseCtxMod + 0 );
                }
            }

            xEncodeBinsEP( coeffSigns, numNonZero );

            Int iFirstCoeff2 = 1;
            if( c1 == 0 || numNonZero > C1FLAG_NUMBER ) {
                for( idx = 0; idx < numNonZero; idx++ ) {
                    Int baseLevel = (idx < C1FLAG_NUMBER) ? (2 + iFirstCoeff2 ) : 1;

                    if( absCoeff[ idx ] >= baseLevel ) {
                        xWriteCoefRemainExGolomb( absCoeff[ idx ] - baseLevel, uiGoRiceParam );
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
}

static
void xEncodeCUs( X265_t *h )
{
   CUInt32      uiWidth     = h->usWidth;
   CUInt32      uiHeight    = h->usHeight;
   CUInt        nMaxCuWidth = h->ucMaxCUWidth;
   CUInt        nMaxCUDepth = h->ucMaxCUDepth;
   CInt         nQP         = h->iQP;
   CInt         nQPC        = xg_aucChromaScale[nQP];
   CInt32       lambda      = nQP;
   CUInt        nCUHeight   = uiHeight / nMaxCuWidth;
   CUInt        nCUWidth    = uiWidth  / nMaxCuWidth;
   CUInt        nCUSize     = h->ucMaxCUWidth;
   CUInt        nLog2CUSize = xLog2(nCUSize-1);
   const xSliceType eSliceType = h->eSliceType;
    int y;
    int i;
#if (USE_ASM > ASM_NONE)
    __declspec(align(16))
#endif
    xCache cache;

    /// Encode loop
    for( y=0; y < (int)nCUHeight; y++ ) {
        UInt32      uiOffset    = 0;
        UInt8      *paucPixY    = cache.aucPixY;
        UInt8      *paucPixU    = cache.aucPixU;
        UInt8      *paucPixV    = cache.aucPixV;
        UInt8      *paucRecY    = cache.aucRecY + 2;
        UInt8      *paucRecU    = cache.aucRecU + 1;
        UInt8      *paucRecV    = cache.aucRecV + 1;
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
        UInt8      *paucPredTmp[MAX_CU_DEPTH]   = { cache.aucPredTmp4, cache.aucPredTmp8, cache.aucPredTmp16, cache.aucPredTmp32 };
        UInt8      *paucTopModeYInfo1           = h->aucTopModeYInfo1;
        Int16      *psTmp[2]                    = { cache.psTmp[0], cache.psTmp[1] };
        UInt8       ucSavedLT, ucSavedLTU, ucSavedLTV;
        xMV         savedMvLT = {0, 0};
        Int x;

        ucSavedLT   = h->aucTopPixY[1];
        ucSavedLTU  = h->aucTopPixU[1];
        ucSavedLTV  = h->aucTopPixV[1];

        memset( pucModeY, MODE_INVALID, sizeof(cache.aucModeY) );

        // Top always valid when (y > 0)
        if ( y != 0 ) {
            memset( pucModeY, MODE_VALID, MAX_PU_XY+2 );
        }
        xEncCahceInitLine( &cache );
        for( x=0; x < (int)nCUWidth; x++ ) {
            CUInt   bLastCU     = (y == (int)nCUHeight - 1) && (x == (int)nCUWidth - 1);
            UInt8  *pucTopPixY  = h->aucTopPixY + 1 + uiOffset;
            UInt8  *pucTopPixU  = h->aucTopPixU + 1 + uiOffset/2;
            UInt8  *pucTopPixV  = h->aucTopPixV + 1 + uiOffset/2;
            xMV    *pasTopMV    = h->asTopMV    + 1 + uiOffset / MIN_MV_SIZE;
            CUInt32 nStrideY    = h->pFrameRef->nStrideY;
            CUInt32 nStrideC    = h->pFrameRef->nStrideC;
            CUInt8 *pucRefY     = h->pFrameRef->pucY + y * nCUSize * nStrideY + uiOffset;
            CUInt8 *pucRefU     = h->pFrameRef->pucU + y * nCUSize/2 * nStrideC + uiOffset/2;
            CUInt8 *pucRefV     = h->pFrameRef->pucV + y * nCUSize/2 * nStrideC + uiOffset/2;
            CUInt32 nRefStrideY = h->pFrameRef->nStrideY;
            CUInt32 nRefStrideC = h->pFrameRef->nStrideC;
            UInt8   tmp;
            xMV     tmpMV;

            // Stage 0: Init internal

            // Stage 1a: Load image to cache
            xEncCacheLoadCU( h, &cache, x, y );

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
            pucModeY[MAX_PU_XY+1] = ( (y > 0) && (x < (int)nCUWidth-1) ? MODE_VALID : MODE_INVALID );
            // Copy Above Line CUSize Info
            memcpy( pucModeYInfo1+1, paucTopModeYInfo1 + uiOffset/MIN_CU_SIZE, MAX_PU_XY );
            if ( eSliceType == SLICE_I ) {
                xEncDecideCU(
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
                    lambda,
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
                    nQPC,
                    lambda
                );
            }
            // P_SLICE
            else {
                CUInt nSize  = nMaxCuWidth/2;
                CUInt nSizeC = nSize/2;
                UInt32 uiSum[3];
                UInt nCbfY, nCbfC;
                xMV sMVP[AMVP_MAX_NUM_CANDS + MRG_MAX_NUM_CANDS];
                UInt nMergeIdx;
                UInt bSkipped = FALSE;

                // Copy Above MV Infos
                memcpy( pasMV, pasTopMV-1, sizeof(xMV)*(MAX_MV_XY+2) );

                int blk;
                for( blk=0; blk<4; blk++ ) {
                    int off_x = blk &  1;
                    int off_y = blk >> 1;
                    Int nMVP = xFillMvpCand(
                                    sMVP,
                                    pucModeY + (off_y*nSize/MIN_CU_SIZE+1)*(MAX_PU_XY + 2) + 1 + off_x*nSize/MIN_CU_SIZE,
                                    pasMV    + (off_y*nSize/MIN_MV_SIZE+1)*(MAX_MV_XY + 2) + 1 + off_x*nSize/MIN_MV_SIZE,
                                    nSize
                               );
                    // ME
                    xMV bmv = xMotionSearch(
                                    paucPixY  + off_y*nSize*MAX_CU_SIZE + off_x*nSize,
                                    pucRefY   + off_y*nSize*nRefStrideY + off_x*nSize,
                                    paucPredY + off_y*nSize*MAX_CU_SIZE + off_x*nSize,
                                    psTmp[0],
                                    nRefStrideY,
                                    nSize
                              );

                    // MC
                    xPredInterChromaBlk(
                        pucRefU   + off_y*nSizeC*nRefStrideC   + off_x*nSizeC,
                        paucPredU + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC,
                        nRefStrideC,
                        nSizeC,
                        bmv,
                        psTmp[0]
                    );
                    xPredInterChromaBlk(
                        pucRefV   + off_y*nSizeC*nRefStrideC   + off_x*nSizeC,
                        paucPredV + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC,
                        nRefStrideC,
                        nSizeC,
                        bmv,
                        psTmp[0]
                    );

                    // Encode
                    // Y
                    xSubDct( psTmp[0],
                        paucPixY  + off_y*nSize*MAX_CU_SIZE + off_x*nSize, MAX_CU_SIZE,
                        paucPredY + off_y*nSize*MAX_CU_SIZE + off_x*nSize, MAX_CU_SIZE,
                        psTmp[0], psTmp[1],
                        nSize, MODE_INVALID );
                    uiSum[0] = xQuant( psCoefY + off_y*nSize*MAX_CU_SIZE + off_x*nSize, psTmp[0], MAX_CU_SIZE, nQP, nSize, nSize, SLICE_P );
                    if( uiSum[0] ) {
                        xDeQuant( psTmp[0], psCoefY + off_y*nSize*MAX_CU_SIZE + off_x*nSize, MAX_CU_SIZE, nQP, nSize, nSize, SLICE_P );
                        xIDctAdd( paucRecY + off_y*nSize*(MAX_CU_SIZE+2) + off_x*nSize, MAX_CU_SIZE+2,
                            psTmp[0], MAX_CU_SIZE,
                            paucPredY + off_y*nSize*MAX_CU_SIZE + off_x*nSize,
                            psTmp[1], psTmp[0],
                            nSize, MODE_INVALID );
                    }
                    else {
                        for( i=0; i<(int)nSize; i++ ) {
                            memcpy( &paucRecY[(off_y*nSize+i)*(MAX_CU_SIZE+2)+off_x*nSize], &paucPredY[(off_y*nSize+i)*MAX_CU_SIZE+off_x*nSize], nSize );
                        }
                    }
                    // U
                    xSubDct( psTmp[0],
                        paucPixU  + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, MAX_CU_SIZE/2,
                        paucPredU + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, MAX_CU_SIZE/2,
                        psTmp[0], psTmp[1],
                        nSizeC, MODE_INVALID );
                    uiSum[1] = xQuant( psCoefU + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, psTmp[0], MAX_CU_SIZE/2, nQPC, nSizeC, nSizeC, SLICE_P );
                    if( uiSum[1] ) {
                        xDeQuant( psTmp[0], psCoefU + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, MAX_CU_SIZE/2, nQPC, nSizeC, nSizeC, SLICE_P );
                        xIDctAdd( paucRecU + off_y*nSizeC*(MAX_CU_SIZE/2+1) + off_x*nSizeC, MAX_CU_SIZE/2+1,
                            psTmp[0], MAX_CU_SIZE/2,
                            paucPredU + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC,
                            psTmp[1], psTmp[0],
                            nSizeC, MODE_INVALID );
                    }
                    else {
                        for( i=0; i<(int)nSizeC; i++ ) {
                            memcpy( &paucRecU[(off_y*nSizeC+i)*(MAX_CU_SIZE/2+1)+off_x*nSizeC], &paucPredU[(off_y*nSizeC+i)*MAX_CU_SIZE/2+off_x*nSizeC], nSizeC );
                        }
                    }
                    // V
                    xSubDct( psTmp[0],
                        paucPixV  + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, MAX_CU_SIZE/2,
                        paucPredV + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, MAX_CU_SIZE/2,
                        psTmp[0], psTmp[1],
                        nSizeC, MODE_INVALID );
                    uiSum[2] = xQuant( psCoefV + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, psTmp[0], MAX_CU_SIZE/2, nQPC, nSizeC, nSizeC, SLICE_P );
                    if( 1||uiSum[2] ) {
                        xDeQuant( psTmp[0], psCoefV + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC, MAX_CU_SIZE/2, nQPC, nSizeC, nSizeC, SLICE_P );
                        xIDctAdd( paucRecV + off_y*nSizeC*(MAX_CU_SIZE/2+1) + off_x*nSizeC, MAX_CU_SIZE/2+1,
                            psTmp[0], MAX_CU_SIZE/2,
                            paucPredV + off_y*nSizeC*MAX_CU_SIZE/2 + off_x*nSizeC,
                            psTmp[1], psTmp[0],
                            nSizeC, MODE_INVALID );
                    }
                    else {
                        for( i=0; i<(int)nSizeC; i++ ) {
                            memcpy( &paucRecV[(off_y*nSizeC+i)*(MAX_CU_SIZE/2+1)+off_x*nSizeC], &paucPredV[(off_y*nSizeC+i)*MAX_CU_SIZE/2+off_x*nSizeC], nSizeC );
                        }
                    }
                    nCbfY = (uiSum[0] ? 1 : 0);
                    nCbfC = ( (uiSum[2] ? 2 : 0) | (uiSum[1] ? 1 : 0) );

                    // Decide MVP Index
                    UInt diff0 = abs(sMVP[0].x - bmv.x) + abs(sMVP[0].y - bmv.y);
                    UInt diff1 = abs(sMVP[1].x - bmv.x) + abs(sMVP[1].y - bmv.y) + 1;
                    UInt8 ucMVPIdx = diff0 <= diff1 ? 0 : 1;

                    // Save MVD Info
                    paucMVPIdx[off_x*nSize/MIN_MV_SIZE + off_y*nSize/MIN_MV_SIZE*MAX_MV_XY] = ucMVPIdx;
                    pasMVD[off_x*nSize/MIN_MV_SIZE + off_y*nSize/MIN_MV_SIZE*MAX_MV_XY].x = bmv.x - sMVP[ucMVPIdx].x;
                    pasMVD[off_x*nSize/MIN_MV_SIZE + off_y*nSize/MIN_MV_SIZE*MAX_MV_XY].y = bmv.y - sMVP[ucMVPIdx].y;

                    nMergeIdx = 0;

                    // Check Skipped
                    bSkipped = 0;

                    // Update MV Info
                    xMV *P = pasMV + 1 + off_x*nSize/MIN_MV_SIZE + (MAX_MV_XY+2)*(off_y*nSize/MIN_MV_SIZE+1);
                    for( i=0; i<nSize/MIN_MV_SIZE; i++ ) {
                        int j;
                        for( j=0; j<nSize/MIN_MV_SIZE; j++ ) {
                            P[i*(MAX_MV_XY+2)+j] = bmv;
                        }
                    }
                    // Update Mode Info
                    for( i=0; i<nSize/MIN_CU_SIZE; i++ ) {
                        int j;
                        for( j=0; j<nSize/MIN_CU_SIZE; j++ ) {
                            pucModeY[(i+off_y*nSize/MIN_CU_SIZE)*(MAX_PU_XY+2)+j+1+(MAX_PU_XY+2)+off_x*nSize/MIN_CU_SIZE] = MODE_VALID;
                            pusModeYInfo0[(off_y*nSize/MIN_CU_SIZE+i)*MAX_PU_XY + j + off_x*nSize/MIN_CU_SIZE] = FLAG_MASK_INTER | (nCbfC << FLAG_OFF_CBFC) | nMergeIdx;
                            pucModeYInfo1[(i+1+off_y*nSize/MIN_CU_SIZE)*(MAX_PU_XY + 1) + 1 + j + off_x*nSize/MIN_CU_SIZE] = 0x80 | (nCbfY ? FLAG_MASK_CBFY : 0) | (bSkipped ? FLAG_MASK_SKIP : 0) | (y+off_y>0 ? (1<<VALID_T) : 0) | (x+off_x>0 ? (1<<VALID_L) : 0);
                        }
                    }
                } // end of blk
            } // end of P_SLICE

            pucTopPixY[-1] = ucSavedLT;
            pucTopPixU[-1] = ucSavedLTU;
            pucTopPixV[-1] = ucSavedLTV;
            pasTopMV[-1]   = savedMvLT;

            // Stage 4: Write CU
            xEncWriteCU(
                eSliceType,
                pucModeY + (MAX_PU_XY + 2) + 1,
                pusModeYInfo0,
                pucModeYInfo1 + (MAX_PU_XY + 1) + 1,
                pasMVD,
                paucMVPIdx,
                psCoefY,
                psCoefU,
                psCoefV
            );
            // FinishCU
            xEncodeTerminatingBit( bLastCU );

            // Stage Debug: Write Visual Info
            #if defined(DEBUG_VIS)
            if ( h->bWriteVisCUFlag ) {
                xDrawVisCU(
                    &h->frm_vis,
                    paucPixY, paucPixU, paucPixV,
                    pucModeYInfo1 + (MAX_PU_XY + 1) + 1,
                    x, y,
                    uiWidth
                );
            }
            #endif

            // Stage 5a: Update reconstruct
            xEncCacheStoreCU( h, &cache, x, y );

            // Stage 5b: Update context
            ucSavedLT = pucTopPixY[nCUSize - 1];
            ucSavedLTU = pucTopPixU[nCUSize/2 - 1];
            ucSavedLTV = pucTopPixV[nCUSize/2 - 1];
            savedMvLT = pasTopMV[nCUSize/MIN_MV_SIZE - 1];
            xEncCacheUpdate(
                h,
                &cache,
                nCUSize,
                uiOffset
            );
            uiOffset += nCUSize;
        }
    }
    xCabacFlush( g_pCabac, g_pBS );
}

Int32 xEncodeFrame( X265_t *h, xFrame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize )
{
   CUInt32      uiWidth     = h->usWidth;
   CUInt32      uiHeight    = h->usHeight;
   CUInt        nMaxCuWidth = h->ucMaxCUWidth;
   CUInt        nCUHeight   = uiHeight / nMaxCuWidth;
   CUInt        nCUWidth    = uiWidth  / nMaxCuWidth;
    xCabac     *pCabac      = &h->cabac;
    xBitStream *pBS         = &h->bs;
    UInt i;
   const xSliceType eSliceType = h->eSliceType;

    /// Copy to local
    h->pFrameCur = pFrame;
    h->pFrameRec = &h->refn[MAX_REF_NUM];
    h->pFrameRef = &h->refn[0];

    /// Initial local
    xCabacInit( h );
    xCabacReset( &h->cabac );
    g_pCabac = pCabac;
    g_pBS = pBS;
    memset( h->aucTopModeYInfo1, MODE_INVALID, sizeof(h->aucTopModeYInfo1) );
    h->iPoc++;
    xBitStreamInit( pBS, pucOutBuf, uiBufSize );

    // Padding Reference Frame
    if ( h->eSliceType != SLICE_I ) {
        xFramePadding( &h->refn[0], uiWidth, uiHeight );
    }

    // FIXME: Two IDR will be make wrong decode output, so I send them first frame only.
    if ( eSliceType == SLICE_I && h->iPoc == 0 ) {
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
        xPutBits32(pBS, (h->iPoc == 0 ? 0x26010000 : 0x2A010000));
    }
    else {
        xPutBits32(pBS, 0x02010000);
    }
    xPutBits(pBS, 0x01, 8);
    xWriteSliceHeader(h);

    xEncodeCUs( h );
    xWriteSliceEnd( h );

    #ifdef CHECK_SEI
    {
        UInt8 md5[3][16];
        MD5Context ctx;

        // Y
        MD5Init( &ctx );
        for( i=0; i<uiHeight; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucY + i * h->pFrameRec->nStrideY, uiWidth );
        }
        MD5Final( &ctx, md5[0] );

        // U
        MD5Init( &ctx );
        for( i=0; i<uiHeight/2; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucU + i * h->pFrameRec->nStrideC, uiWidth/2 );
        }
        MD5Final( &ctx, md5[1] );

        // V
        MD5Init( &ctx );
        for( i=0; i<uiHeight/2; i++ ) {
            MD5Update( &ctx, h->pFrameRec->pucV + i * h->pFrameRec->nStrideC, uiWidth/2 );
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

    // Save Restruct
    if ( h->bWriteRecFlag ) {
        CUInt32 uiWidthPad  = uiWidth  + 2 * PAD_YSIZE;
        CUInt32 uiHeightPad = uiHeight + 2 * PAD_YSIZE;
        static FILE *fpx=NULL;
        if ( fpx == NULL ) {
            fpx = fopen("OX.YUV", "wb");
            printf("Create OX.YUV with %dx%d\n", uiWidthPad, uiHeightPad);
        }
        assert( fpx != NULL );
        fwrite(h->pFrameRec->base, 1, uiWidthPad*uiHeightPad*3/2, fpx);
        fflush(fpx);
        //fclose(fpx);
    }

    // Save Visual CU Split
    #ifdef DEBUG_VIS
    if ( h->bWriteVisCUFlag ) {
        static FILE *fpx=NULL;
        if ( fpx == NULL )
            fpx = fopen("VIS_CU.YUV", "wb");
        assert( fpx != NULL );
        fwrite(h->frm_vis.pucY, 1, uiWidth*uiHeight*3/2, fpx);
        fflush(fpx);
        //fclose(fpx);
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
    return iLen;
}

// ***************************************************************************
// * Internal Functions
// ***************************************************************************

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
    CUInt   nCUWidth   = h->ucMaxCUWidth;
    CUInt32 uiWidth    = h->usWidth;
    CUInt  nStrideY = pFrame->nStrideY;
    CUInt  nStrideC = pFrame->nStrideC;
    CUInt32 uiOffsetY = (nStrideY * uiCUY * nCUWidth   + uiCUX * nCUWidth  );
    CUInt32 uiOffsetC = (nStrideC * uiCUY * nCUWidth/2 + uiCUX * nCUWidth/2);
    UInt8 *pucSY0 = pFrame->pucY + uiOffsetY + 0*nStrideY;
    UInt8 *pucSY1 = pFrame->pucY + uiOffsetY + 1*nStrideY;
    UInt8 *pucSU  = pFrame->pucU + uiOffsetC;
    UInt8 *pucSV  = pFrame->pucV + uiOffsetC;
    UInt8 *pucDY0 = pCache->aucPixY + 0*MAX_CU_SIZE;
    UInt8 *pucDY1 = pCache->aucPixY + 1*MAX_CU_SIZE;
    UInt8 *pucDU  = pCache->aucPixU;
    UInt8 *pucDV  = pCache->aucPixV;
    Int y;

    for( y=0; y < h->ucMaxCUWidth/2; y++ ) {
        memcpy( pucDY0, pucSY0, nCUWidth     );
        memcpy( pucDY1, pucSY1, nCUWidth     );
        memcpy( pucDU,  pucSU,  nCUWidth / 2 );
        memcpy( pucDV,  pucSV,  nCUWidth / 2 );
        pucSY0 += nStrideY * 2;
        pucSY1 += nStrideY * 2;
        pucSU  += nStrideC;
        pucSV  += nStrideC;
        pucDY0 += MAX_CU_SIZE * 2;
        pucDY1 += MAX_CU_SIZE * 2;
        pucDU  += MAX_CU_SIZE / 2;
        pucDV  += MAX_CU_SIZE / 2;
    }
}

void xEncCacheStoreCU( X265_t *h, xCache *pCache, UInt uiCUX, UInt uiCUY )
{
    xFrame  *pFrame     = h->pFrameRec;
    CUInt32 uiStrideY   = pFrame->nStrideY;
    CUInt32 uiStrideC   = pFrame->nStrideC;
    CUInt   nCUWidth   = h->ucMaxCUWidth;
    CUInt32 uiWidthY   = pFrame->nStrideY;
    CUInt32 uiOffsetY  = (uiStrideY * uiCUY * nCUWidth   + uiCUX * nCUWidth  );
    CUInt32 uiOffsetC  = (uiStrideC * uiCUY * nCUWidth/2 + uiCUX * nCUWidth/2);
    UInt8 *pucSY0 = pCache->aucRecY + 2 + 0*(MAX_CU_SIZE+2);
    UInt8 *pucSY1 = pCache->aucRecY + 2 + 1*(MAX_CU_SIZE+2);
    UInt8 *pucSU  = pCache->aucRecU + 1;
    UInt8 *pucSV  = pCache->aucRecV + 1;
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
        pucSY0 += (MAX_CU_SIZE  +2) * 2;
        pucSY1 += (MAX_CU_SIZE  +2) * 2;
        pucSU  += (MAX_CU_SIZE/2+1);
        pucSV  += (MAX_CU_SIZE/2+1);
        pucDY0 += uiStrideY*2;
        pucDY1 += uiStrideY*2;
        pucDU  += uiStrideC;
        pucDV  += uiStrideC;
    }
}

void xEncCacheUpdate(
    X265_t *h,
    xCache *pCache,
    UInt    nSize,
    UInt32  uiOffset
)
{
    UInt8 *paucModeY        =  pCache->aucModeY + (MAX_PU_XY+2) + 1;
    UInt8 *paucPredY        =  pCache->aucPredY;
    UInt8 *pauclPixTopY     =  h->aucTopPixY + 1 + uiOffset;
    UInt8 *pauclPixTopU     =  h->aucTopPixU + 1 + uiOffset/2;
    UInt8 *pauclPixTopV     =  h->aucTopPixV + 1 + uiOffset/2;
    xMV   *pasTopMV         =  h->asTopMV + 1 + uiOffset / MIN_MV_SIZE;
    xMV   *paslMV           =  pCache->asMV + (MAX_MV_XY+2) + 1;

    UInt8 *paucModeYInfo1     =  pCache->aucModeYInfo1 + (MAX_PU_XY+1) + 1;
    UInt8 *pauclTopModeYInfo1 =  h->aucTopModeYInfo1 + uiOffset / MIN_CU_SIZE;

    UInt8 *paucRecY         =  pCache->aucRecY + 2;
    UInt8 *paucRecU         =  pCache->aucRecU + 1;
    UInt8 *paucRecV         =  pCache->aucRecV + 1;

    UInt y;

    // Update Mode Split Info
    memcpy( pauclTopModeYInfo1, paucModeYInfo1 + (nSize/MIN_CU_SIZE - 1) * (MAX_PU_XY+1), MAX_PU_XY );

    // Update Mode Infos
    for( y=0; y<MAX_PU_XY; y++ ) {
        paucModeY[y*(MAX_PU_XY+2)-1] = paucModeY[y*(MAX_PU_XY+2) + (nSize/MIN_CU_SIZE - 1)];
        paucModeYInfo1[y*(MAX_PU_XY+1)-1] = paucModeYInfo1[y*(MAX_PU_XY+1) + (nSize/MIN_CU_SIZE - 1)];
        memset( &paucModeY[y*(MAX_PU_XY+2)], MODE_INVALID, MAX_PU_XY );
    }

    // Update MV Infos
    memcpy( pasTopMV, &paslMV[(nSize/MIN_MV_SIZE-1) * (MAX_MV_XY+2)], sizeof(xMV)*MAX_MV_XY );
    for( y=0; y<MAX_MV_XY; y++ ) {
        paslMV[y*(MAX_MV_XY+2)-1] = paslMV[y*(MAX_MV_XY+2) + (nSize/MIN_MV_SIZE - 1)];
    }

    // Update Luma Pred Pixel
    memcpy( pauclPixTopY, &paucRecY[(nSize-1) * (MAX_CU_SIZE+2)], MAX_CU_SIZE );
    for( y=0; y<MAX_CU_SIZE; y++ ) {
        paucRecY[y*(MAX_CU_SIZE+2)-2] = paucRecY[y*(MAX_CU_SIZE+2) + (nSize - 2)];
        paucRecY[y*(MAX_CU_SIZE+2)-1] = paucRecY[y*(MAX_CU_SIZE+2) + (nSize - 1)];
    }

    // Update Chroma Pred Pixel
    memcpy( pauclPixTopU, &paucRecU[(nSize/2-1) * (MAX_CU_SIZE/2+1)], MAX_CU_SIZE/2 );
    memcpy( pauclPixTopV, &paucRecV[(nSize/2-1) * (MAX_CU_SIZE/2+1)], MAX_CU_SIZE/2 );
    for( y=0; y<MAX_CU_SIZE/2; y++ ) {
        paucRecU[y*(MAX_CU_SIZE/2+1)-1] = paucRecU[y*(MAX_CU_SIZE/2+1) + (nSize/2 - 1)];
        paucRecV[y*(MAX_CU_SIZE/2+1)-1] = paucRecV[y*(MAX_CU_SIZE/2+1) + (nSize/2 - 1)];
    }
}

// ***************************************************************************
// * IntraPred Functions
// ***************************************************************************
void xPaddingRef( UInt8 *pucRef, CUInt8 ucValids, CInt32 nBlkOffset[6] )
{
    UInt    i, n;
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
            CUInt nBlkAddr = nBlkOffset[n];
            CUInt nBlkSize = nBlkOffset[n + 1] - nBlkOffset[n];
            ucPadding = pucRef[nBlkAddr - 1];
            for( i=0; i<nBlkSize; i++ ) {
                pucRef[nBlkAddr + i] = ucPadding;
            }
        }
    }
}

void xFilterRef( UInt8 *pucDst, UInt8 *pucSrc, CUInt32 nSize, CUInt bStrongFilter )
{
    UInt    i, n;

    UInt8   ucBL = pucSrc[0];
    UInt8   ucTL = pucSrc[2*nSize];
    UInt8   ucTR = pucSrc[4*nSize];
    UInt32  threshold = 1 << (8 - 5);
    UInt    bilinearLeft = abs(ucBL + ucTL - 2*pucSrc[  nSize]) < threshold;
    UInt    bilinearTop  = abs(ucTL + ucTR - 2*pucSrc[3*nSize]) < threshold;

    if ( bStrongFilter && (nSize == 32) && (bilinearLeft && bilinearTop) ) {
        CInt nShift = xLog2( nSize - 1 ) + 1;
        pucDst[0      ] = pucSrc[0];
        pucDst[2*nSize] = pucSrc[2*nSize];
        pucDst[4*nSize] = pucSrc[4*nSize];

        for (i = 1; i < 2*nSize; i++) {
            pucDst[          i] = ((2*nSize-i)*ucBL + i*ucTL + nSize) >> nShift;
            pucDst[2*nSize + i] = ((2*nSize-i)*ucTL + i*ucTR + nSize) >> nShift;
        }
    }
    else {
        // Filter with [1 2 1]
        pucDst[0      ] = pucSrc[0];
        pucDst[4*nSize] = pucSrc[4*nSize];
        for( i=1; i<4*(int)nSize; i++ ) {
            pucDst[i] = (pucSrc[i - 1] + 2 * pucSrc[i] + pucSrc[i + 1] + 2) >> 2;
        }
    }
}

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

#if (USE_ASM >= ASM_SSE2)
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

#if (USE_ASM >= ASM_SSE2)
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
    UInt8 *restrict P = pucDst;

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
        #pragma unroll(32)
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

#if (USE_ASM >= ASM_SSE2)
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
        xPredIntraPlanar(
            pucDstY,
            pucRefY,
            nStride,
            nSize
        );
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
#if (USE_ASM >= ASM_SSE2)
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


UInt32 xEncDecideCU(
    UInt8  *pucPixY,
    UInt8  *pucRecY,
    UInt8  *pucRefTopY,
    UInt8  *pucModeY,
    UInt16 *pusModeYInfo0,
    UInt8  *pucModeYInfo1,
    UInt8  *pucPredY,
    Int16  *psCoefY,
    Int16  *psTmp[2],
    UInt8  *paucPredTmp[MAX_CU_DEPTH],
     Int32  nSize,
    UInt    dy,
    UInt    nQP,
   CUInt    lambda,
   CUInt    bStrongIntraSmoothing
)
{
    UInt    nLog2Size = xLog2( nSize - 1 );
    UInt    nBlock = nSize / MIN_CU_SIZE;
    CInt32  nBlkOffsetY[6] = {0, nSize,  2*nSize,  2*nSize +1, 3*nSize +1, 4*nSize +1};
    UInt32  uiBestSadY;
    UInt    nBestModeY;
    UInt    nMode;
    UInt8   aucRef[2][4 * MAX_CU_SIZE + 1];
    UInt8   ucValids = 0;
    UInt8   aucMode[5];
    UInt8   aucMostModeY[3];
    Int     i;
    UInt32  uiSumY;
    UInt32  uiSad, uiSubSad[4];
    UInt8  *pucDstY[2] = { paucPredTmp[nLog2Size-2], paucPredTmp[nLog2Size-2]+nSize*nSize };
    UInt    nTmpIdx = 0;

    aucMode[VALID_LB] = pucModeY[nBlock*(MAX_PU_XY+2) - 1     ];
    aucMode[VALID_L ] = pucModeY[                     - 1     ];
    aucMode[VALID_LT] = pucModeY[-1    *(MAX_PU_XY+2) - 1     ];
    aucMode[VALID_T ] = pucModeY[-1    *(MAX_PU_XY+2)         ];
    aucMode[VALID_TR] = pucModeY[-1    *(MAX_PU_XY+2) + nBlock];
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
        if( ucLeftMode && ucTopMode )
            aucMostModeY[2] = PLANAR_IDX;
        else
            aucMostModeY[2] = ( ucLeftMode + ucTopMode ) < 2 ? VER_IDX : DC_IDX;
    }

    // Generator Reference Samples
    if( ucValids == 0 ) {
        memset( aucRef, 0x80, sizeof(aucRef) );
    }
    else {
        // Copy Left Bottom and Left
        for( i=0; i<(int)nSize*2; i++ ) {
            aucRef[0][i] = pucRecY[-1+(nSize*2-1-i)*(MAX_CU_SIZE+2)];
        }
        // Copy Top and Top Right
        memcpy( &aucRef[0][nSize*2], (dy == 0 ? pucRefTopY-1 : &pucRecY[-1*(MAX_CU_SIZE+2)-1] ), 2*nSize+1 );
        // Padding
        xPaddingRef( aucRef[0], ucValids, nBlkOffsetY );
        xFilterRef( aucRef[1], aucRef[0], nSize, bStrongIntraSmoothing );
    }

    // Pred and Decide
    uiBestSadY = MAX_SAD;
    nBestModeY = 0;
    for( nMode=0; nMode<35; nMode++ ) {
        UInt    bFilter = xg_aucIntraFilterType[nLog2Size-2][nMode];
        UInt8  *pucRefY = aucRef[bFilter];

        xEncIntraPred( pucDstY[nTmpIdx], pucRefY, nSize, nMode, nSize, (nSize != 32) );

        if( nMode == aucMostModeY[0] )
            uiSad = 1 * lambda;
        else if( nMode == aucMostModeY[1] || nMode == aucMostModeY[2] )
            uiSad = 2 * lambda;
        else
            uiSad = 3 * lambda;
        uiSad += xSadN[nLog2Size-2]( pucPixY, MAX_CU_SIZE,
                                     pucDstY[nTmpIdx], nSize );

        if( uiSad < uiBestSadY ) {
            uiBestSadY = uiSad;
            nBestModeY = nMode;
            nTmpIdx   ^= 1;
        }
    }

    // Map Mode into index
    Int iPredIdx;
    if( nBestModeY == aucMostModeY[0] )
        iPredIdx = 0;
    else if( nBestModeY == aucMostModeY[1] )
        iPredIdx = 1;
    else if( nBestModeY == aucMostModeY[2] )
        iPredIdx = 2;
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

    // Check and Split
    UInt bReplace = ( nSize == MIN_CU_SIZE ? TRUE : FALSE );
    if ( (nSize > MIN_CU_SIZE) && (uiBestSadY > 0) ) {
        UInt nSubSize  = (nSize>>1);
        UInt nSubBlock = (nBlock>>1);
        UInt uiPartUnitIdx;
        for( uiPartUnitIdx=0; uiPartUnitIdx<4; uiPartUnitIdx++ ) {
            UInt sdx = (uiPartUnitIdx &  1);
            UInt sdy = (uiPartUnitIdx >> 1);
            uiSubSad[uiPartUnitIdx] = xEncDecideCU( pucPixY       + sdy * nSubSize  * MAX_CU_SIZE    + sdx * nSubSize,
                                                    pucRecY       + sdy * nSubSize  *(MAX_CU_SIZE+2) + sdx * nSubSize,
                                                    pucRefTopY    + sdy * nSubSize  * MAX_CU_SIZE    + sdx * nSubSize,
                                                    pucModeY      + sdy * nSubBlock *(MAX_PU_XY+2)   + sdx * nSubBlock,
                                                    pusModeYInfo0 + sdy * nSubBlock * MAX_PU_XY      + sdx * nSubBlock,
                                                    pucModeYInfo1 + sdy * nSubBlock *(MAX_PU_XY+1)   + sdx * nSubBlock,
                                                    pucPredY      + sdy * nSubSize  * MAX_CU_SIZE    + sdx * nSubSize,
                                                    psCoefY       + sdy * nSubSize  * MAX_CU_SIZE    + sdx * nSubSize,
                                                    psTmp,
                                                    paucPredTmp,
                                                    nSubSize,
                                                    dy + sdy * nSubSize,
                                                    nQP,
                                                    lambda,
                                                    bStrongIntraSmoothing );
        }
        uiSad = uiSubSad[0] + uiSubSad[1] + uiSubSad[2] + uiSubSad[3] + 4;

        // Check if split is bad.
        //uiSad = MAX_SAD;
        if ( uiSad >= uiBestSadY ) {
            bReplace = TRUE;
        }
    }
    if ( bReplace ) {
        // Copy Pred data into Global buffer
        UInt    bFilter = xg_aucIntraFilterType[nLog2Size-2][nBestModeY];
        UInt8  *pucRefY = aucRef[bFilter];

        nTmpIdx ^= 1;
        for( i=0; i<(int)nSize; i++ ) {
            memcpy( pucPredY + i*MAX_CU_SIZE, pucDstY[nTmpIdx] + i*nSize, nSize );
        }

        xSubDct( psTmp[0],
                 pucPixY, MAX_CU_SIZE,
                 pucPredY, MAX_CU_SIZE,
                 psTmp[0], psTmp[1],
                 nSize, nBestModeY );
        uiSumY = xQuant( psCoefY, psTmp[0], MAX_CU_SIZE, nQP, nSize, nSize, SLICE_I );
        if( uiSumY ) {
            xDeQuant( psTmp[0], psCoefY, MAX_CU_SIZE, nQP, nSize, nSize, SLICE_I );
            xIDctAdd( pucRecY, MAX_CU_SIZE+2,
                      psTmp[0], MAX_CU_SIZE,
                      pucPredY,
                      psTmp[1], psTmp[0],
                      nSize, nBestModeY );
        }
        else {
            for( i=0; i<(int)nSize; i++ ) {
                memcpy( &pucRecY[i*(MAX_CU_SIZE+2)], &pucPredY[i*MAX_CU_SIZE], nSize );
            }
        }

        // Update best info
        //printf("(%2dx%2d): nBestModeY=%2d\n", nSize, nSize, nBestModeY);
        pusModeYInfo0[0]  = iPredIdx;
        assert( iPredIdx < NUM_INTRA_MODE+3 );
        pucModeYInfo1[0]  = (ucValids      << FLAG_OFF_VALIDS)
                          | (uiSumY != 0 ? FLAG_MASK_CBFY : 0x00)
                          | ((nLog2Size-2) << FLAG_OFF_SIZE);
        for( i=0; i<(int)nBlock; i++ ) {
            memset( &pucModeY[i*(MAX_PU_XY+2)], nBestModeY, nBlock );
            memset( &pucModeYInfo1[i*(MAX_PU_XY+1)], pucModeYInfo1[0], nBlock );
        }
    }
    else {
        assert( nSize > 4 && nSize <= 32 );
    }
    uiBestSadY = MIN(uiBestSadY, uiSad);

    return uiBestSadY;
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
    Int16  *psTmp[2],
    UInt8  *pucPredTmp,
    UInt    nQPC,
    UInt    lambda
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
         Int    nSizeY = ( 1 << (2 + (pucModeYInfo1[(MAX_PU_XY+1) * dy + dx] >> FLAG_OFF_SIZE)) );
         Int    nSize  = (nSizeY == 4 ? 4 : (nSizeY >> 1));
        UInt8   ucValids = (pucModeYInfo1[ (MAX_PU_XY+1) * dy + dx ] >> FLAG_OFF_VALIDS) & 0x1F;
        UInt    nLog2Size = xLog2( nSize - 1 );
        UInt8  *pucDstU[2] = {pucPredTmp, pucPredTmp + MAX_CU_SIZE*MAX_CU_SIZE/2};
        UInt8  *pucDstV[2] = {pucDstU[0] + MAX_CU_SIZE/2, pucDstU[1] + MAX_CU_SIZE/2};
        UInt    nPredOffset = (dy*2*MAX_CU_SIZE/2 + dx*2);
        UInt    nRecOffset = (dy*2*(MAX_CU_SIZE/2+1) + dx*2);
        UInt8  *puclPixU = pucPixU + nPredOffset;
        UInt8  *puclPixV = pucPixV + nPredOffset;
        UInt8  *puclPredU = pucPredU + nPredOffset;
        UInt8  *puclPredV = pucPredV + nPredOffset;
        UInt8  *puclRecU = pucRecU + nRecOffset;
        UInt8  *puclRecV = pucRecV + nRecOffset;
        UInt8  *puclRecY = pucRecY + ( dy*4*(MAX_CU_SIZE+2) + dx*4 );
        UInt8  *puclTopY = (dy == 0) ? (pucRefTopY + dx * 4) : (puclRecY - (MAX_CU_SIZE+2));
        UInt8   aucRef[2][4 * MAX_CU_SIZE/2 + 1];   // 0:Left, 1:Top
        UInt8  *paucRefU = aucRef[0];
        UInt8  *paucRefV = aucRef[1];
       CInt32   nBlkOffsetC[6] = {0, nSize,  2*nSize,  2*nSize +1, 3*nSize +1, 4*nSize +1};
        UInt    nTmpIdx = 0;
        UInt    nMode, realModeC, nBestModeC, nCbfC;
        UInt32  uiBestSadC, uiSumU, uiSumV;
        UInt32 uiSumSad;
        UInt32 uiSad[2];
        Int     i;

        // Fixup Valids
        if ( nSizeY == 4 ) {
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
            for( i=0; i<(int)nSize*2; i++ ) {
                paucRefU[i] = puclRecU[-1+(nSize*2-1-i)*(MAX_CU_SIZE/2+1)];
                paucRefV[i] = puclRecV[-1+(nSize*2-1-i)*(MAX_CU_SIZE/2+1)];
            }
            // Copy Top and Top Right
            memcpy( &paucRefU[nSize*2], (dy == 0 ? pucRefTopU+2*dx-1 : &puclRecU[-1*(MAX_CU_SIZE/2+1)-1] ), 2*nSize+1 );
            memcpy( &paucRefV[nSize*2], (dy == 0 ? pucRefTopV+2*dx-1 : &puclRecV[-1*(MAX_CU_SIZE/2+1)-1] ), 2*nSize+1 );
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
                nSize,
                FALSE
            );
            xEncIntraPred(
                pucDstV[nTmpIdx],
                paucRefV,
                MAX_CU_SIZE,
                realModeC,
                nSize,
                FALSE
            );

            uiSad[0] = xSadN[nLog2Size-2]( pucPixU, MAX_CU_SIZE/2,
                                           pucDstU[nTmpIdx], MAX_CU_SIZE );

            uiSad[1] = xSadN[nLog2Size-2]( pucPixV, MAX_CU_SIZE/2,
                                           pucDstV[nTmpIdx], MAX_CU_SIZE );

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
        for( i=0; i<(int)nSize; i++ ) {
            memcpy( puclPredU + i*MAX_CU_SIZE/2, pucDstU[nTmpIdx] + i*MAX_CU_SIZE, nSize );
            memcpy( puclPredV + i*MAX_CU_SIZE/2, pucDstV[nTmpIdx] + i*MAX_CU_SIZE, nSize );
        }
        xSubDct( psTmp[0],
                 puclPixU, MAX_CU_SIZE/2,
                 puclPredU, MAX_CU_SIZE/2,
                 psTmp[0], psTmp[1],
                 nSize, MODE_INVALID );
        uiSumU = xQuant( psCoefU + nPredOffset, psTmp[0], MAX_CU_SIZE/2, nQPC, nSize, nSize, SLICE_I );
        if( uiSumU ) {
            xDeQuant( psTmp[0], psCoefU + nPredOffset, MAX_CU_SIZE/2, nQPC, nSize, nSize, SLICE_I );
            xIDctAdd( puclRecU, MAX_CU_SIZE/2+1,
                      psTmp[0], MAX_CU_SIZE/2,
                      puclPredU,
                      psTmp[1], psTmp[0],
                      nSize, MODE_INVALID );
        }
        else {
            for( i=0; i<(int)nSize; i++ ) {
                memcpy( &puclRecU[i*(MAX_CU_SIZE/2+1)], &puclPredU[i*MAX_CU_SIZE/2], nSize );
            }
        }
        xSubDct( psTmp[0],
                 puclPixV, MAX_CU_SIZE/2,
                 puclPredV, MAX_CU_SIZE/2,
                 psTmp[0], psTmp[1],
                 nSize, MODE_INVALID );
        uiSumV = xQuant( psCoefV + nPredOffset, psTmp[0], MAX_CU_SIZE/2, nQPC, nSize, nSize, SLICE_I );
        if( uiSumV ) {
            xDeQuant( psTmp[0], psCoefV + nPredOffset, MAX_CU_SIZE/2, nQPC, nSize, nSize, SLICE_I );
            xIDctAdd( puclRecV, MAX_CU_SIZE/2+1,
                      psTmp[0], MAX_CU_SIZE/2,
                      puclPredV,
                      psTmp[1], psTmp[0],
                      nSize, MODE_INVALID );
        }
        else {
            for( i=0; i<(int)nSize; i++ ) {
                memcpy( &puclRecV[i*(MAX_CU_SIZE/2+1)], &puclPredV[i*MAX_CU_SIZE/2], nSize );
            }
        }
        nCbfC = ( (uiSumV ? 2 : 0) | (uiSumU ? 1 : 0) );
        pucModeYInfo0[dy * MAX_PU_XY + dx] |= (nBestModeC << FLAG_OFF_MODEC) | (nCbfC << FLAG_OFF_CBFC);

        if (nSizeY == 4)
            nSizeY <<= 1;
        nIdx += (nSizeY * nSizeY / 16);
    } while( nIdx != (MAX_PU_XY * MAX_PU_XY) );  // TODO: Can't Support max CUSize other than MAX_CU_SIZE
}


static
void xWriteIntraDirLumaAngGroup(
    UInt        nGroup,
    UInt        nPredIdx[4]
)
{
    UInt i;
    for( i=0; i<nGroup; i++ ) {
        xEncodeBin( (nPredIdx[i] < 3), OFF_INTRA_PRED_CTX );
    }
    for( i=0; i<nGroup; i++ ) {
        UInt nIdx = nPredIdx[i];
        if( nIdx < 3 ) {
            // 0 -> 0
            // 1 -> 10
            // 2 -> 11
            xEncodeBinsEP( nIdx + (nIdx!=0), 1+(nIdx!=0) );
        }
        else {
            assert( nIdx-3 < NUM_INTRA_MODE );
            xEncodeBinsEP( nIdx-3, 5 );
        }
    }
}

static
void xWriteIntraDirModeChroma(
    UInt        nModeC
)
{
    xEncodeBin( (nModeC != CHROMA_MODE_DM), OFF_CHROMA_PRED_CTX );

    if ( nModeC != CHROMA_MODE_DM ) {
        // Non DM_CHROMA_IDX
        xEncodeBinsEP( nModeC, 2 );
    }
}

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
void xWriteMVD(
    xMV         mvd
)
{
    CInt iHor = mvd.x;
    CInt iVer = mvd.y;
    CUInt bHorAbsGr0 = iHor != 0;
    CUInt bVerAbsGr0 = iVer != 0;
    CUInt uiHorAbs   = abs(iHor);
    CUInt uiVerAbs   = abs(iVer);

    xEncodeBin( (bHorAbsGr0 ? 1 : 0), OFF_MVD_CTX );
    xEncodeBin( (bVerAbsGr0 ? 1 : 0), OFF_MVD_CTX );

    if( bHorAbsGr0 ) {
        xEncodeBin( (uiHorAbs > 1 ? 1 : 0), OFF_MVD_CTX + 1 );
    }

    if( bVerAbsGr0 ) {
        xEncodeBin( (uiVerAbs > 1 ? 1 : 0), OFF_MVD_CTX + 1 );
    }

    if( bHorAbsGr0 ) {
        if( uiHorAbs > 1 ) {
            xWriteEpExGolomb( uiHorAbs-2, 1 );
        }

        xEncodeBinsEP( (iHor < 0 ? 1 : 0), 1 );
    }

    if( bVerAbsGr0 ) {
        if( uiVerAbs > 1 ) {
            xWriteEpExGolomb( uiVerAbs-2, 1 );
        }

        xEncodeBinsEP( (iVer < 0 ? 1 : 0), 1 );
    }
}

static
void xWriteMergeIdx(
    UInt        nMergeIdx
)
{
    UInt i;

    xEncodeBin( !!nMergeIdx, OFF_MERGE_IDX_EXT_CTX );
    if ( nMergeIdx ) {
        xEncodeBinsEP( ( 1 << nMergeIdx ) - 2, nMergeIdx );
    }
}

void xEncWriteCU(
    const xSliceType eSliceType,
    UInt8       *pucModeY,
    UInt16      *pucModeYInfo0,
    UInt8       *pucModeYInfo1,
    xMV         *pasMVD,
    UInt8       *paucMVPIdx,
    Int16       *psCoefY,
    Int16       *psCoefU,
    Int16       *psCoefV
)
{
    UInt    nIdx = 0;
   CUInt8   nMostModeC[NUM_CHROMA_MODE] = {PLANAR_IDX, VER_IDX, HOR_IDX, DC_IDX, DM_CHROMA_IDX};

    do {
        UInt    dx              = getInterleaveBits( nIdx      ); // Position to Luma
        UInt    dy              = getInterleaveBits( nIdx >> 1 );
        UInt8  *puclModeY       = &pucModeY     [dy*(MAX_PU_XY + 2) + dx];
        UInt16 *puclModeYInfo0  = &pucModeYInfo0[dy*(MAX_PU_XY    ) + dx];
        UInt8  *puclModeYInfo1  = &pucModeYInfo1[dy*(MAX_PU_XY + 1) + dx];
        Int16  *pslCoefY        = &psCoefY[dy*4*(MAX_CU_SIZE  ) + dx*4];
        Int16  *pslCoefU        = &psCoefU[dy*2*(MAX_CU_SIZE/2) + dx*2];
        Int16  *pslCoefV        = &psCoefV[dy*2*(MAX_CU_SIZE/2) + dx*2];
        UInt8   ucInfo          = puclModeYInfo1[0];
        UInt8   ucValids        = (ucInfo >> FLAG_OFF_VALIDS) & 0x1F;
        UInt    nSplit          = (ucInfo >> FLAG_OFF_SIZE );
        UInt8   nSplitLeft      = (ucValids & (1 << VALID_L)) ? (puclModeYInfo1[-1            ] >> FLAG_OFF_SIZE) : 7;
        UInt8   nSplitTop       = (ucValids & (1 << VALID_T)) ? (puclModeYInfo1[-(MAX_PU_XY+1)] >> FLAG_OFF_SIZE) : 7;
        UInt    nLog2SizeY      = (2 + nSplit);
        UInt    nSizeY          = ( 1 << nLog2SizeY );
        UInt    nSizeC          = ( nSizeY >> 1 );
        UInt    nBlock          = ( nSizeY / MIN_CU_SIZE );
        UInt    nModeY          = puclModeY[0];
        UInt    nModeYIdx       = (puclModeYInfo0[0] & FLAG_MASK_MODE);
        UInt    nModeIdxC       = (puclModeYInfo0[0] >> FLAG_OFF_MODEC) & 7;
        UInt    nCbfY           = !!(puclModeYInfo1[0] & FLAG_MASK_CBFY);
        UInt    nCbfU           = !!(puclModeYInfo0[0] & FLAG_MASK_CBFU);
        UInt    nCbfV           = !!(puclModeYInfo0[0] & FLAG_MASK_CBFV);
        UInt    nIsInter        = !!(puclModeYInfo0[0] & FLAG_MASK_INTER);
        UInt    nMergeIdx       =   (puclModeYInfo0[0] & FLAG_MASK_MRG);
        UInt    bSkipped        = !!(puclModeYInfo1[0] & FLAG_MASK_SKIP);
        UInt    bSkippedLeft    = !!( (puclModeYInfo1[-1+(nSizeY/MAX_PU_XY-1)*(MAX_PU_XY+1)] & FLAG_MASK_SKIP) && (puclModeY[-1+(nSizeY/MAX_PU_XY-1)*(MAX_PU_XY+2)] != MODE_INVALID) );
        UInt    bSkippedAbove   = !!( (puclModeYInfo1[-1*(MAX_PU_XY+1)+(nSizeY/MAX_PU_XY-1)] & FLAG_MASK_SKIP) && (puclModeY[-1*(MAX_PU_XY+2)+(nSizeY/MAX_PU_XY-1)] != MODE_INVALID) );
        xMV     mvd             = pasMVD[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*MAX_MV_XY + dx/(MIN_MV_SIZE/MIN_CU_SIZE)];
        UInt8   ucMVPIdx        = paucMVPIdx[dy/(MIN_MV_SIZE/MIN_CU_SIZE)*MAX_MV_XY + dx/(MIN_MV_SIZE/MIN_CU_SIZE)];
        UInt    nQtRootCbfC     = nCbfU | nCbfV;
        UInt    nQtRootCbf      = nCbfY | nQtRootCbfC;
        UInt    realModeC;
        UInt    uiCtx;

        // ***** Write Mode Info *****
        // Check Depth 0
        if ( (nSplit <= 3) && (nIdx == 0) ) {
            uiCtx = ( nSplitLeft < 3 ) + ( nSplitTop < 3 );

            assert( uiCtx < 3 );
            xEncodeBin( !!(nSplit < 3), OFF_SPLIT_FLAG_CTX + uiCtx );
        }
        // Check Depth 1
        if ( (nSplit <= 2) && ((nIdx & 15) == 0) ) {
            uiCtx = ( nSplitLeft < 2 ) + ( nSplitTop < 2 );

            assert( uiCtx < 3 );
            xEncodeBin( !!(nSplit < 2), OFF_SPLIT_FLAG_CTX + uiCtx );
        }

        if ( eSliceType != SLICE_I ) {
            // SkipFlag
            uiCtx = bSkippedLeft + bSkippedAbove;  // LeftSkip + AboveSkip
            xEncodeBin( bSkipped, OFF_SKIP_FLAG_CTX + uiCtx );

            if ( bSkipped ) {
                xWriteMergeIdx( nMergeIdx-1 );
                goto _do_next_pu;
            }

            // Inter PredMode
            xEncodeBin( 0, OFF_PRED_MODE_CTX );
        }

        // Check Depth 2
        if ( ((nIdx & 3) == 0) ) {
            // Write only 8x8
            // Intra
            if ( eSliceType == SLICE_I ) {
                if ( nSplit <= 1 ) {
                    // codePartSize
                    xEncodeBin( (nSplit != 0 ? 1 : 0), OFF_PART_SIZE_CTX );
                }

                // codePredInfo
                UInt nPredIdx[4] = { nModeYIdx };
                if ( nSplit == 0 ) {
                    // SIZE_NxN is used by Luma4x4 only!
                    nPredIdx[1] = puclModeYInfo0[0*MAX_PU_XY + 1] & FLAG_MASK_MODE;
                    nPredIdx[2] = puclModeYInfo0[1*MAX_PU_XY + 0] & FLAG_MASK_MODE;
                    nPredIdx[3] = puclModeYInfo0[1*MAX_PU_XY + 1] & FLAG_MASK_MODE;
                }
                xWriteIntraDirLumaAngGroup( (nSplit == 0 ? 4 : 1), nPredIdx );
                xWriteIntraDirModeChroma( (puclModeYInfo0[0] >> FLAG_OFF_MODEC) & 7 );
            }
            // Inter
            else {
                // codePartSize
                // SIZE_NxN  -> 1
                // SIZE_2NxN -> 01
                // SIZE_Nx2N -> 00
                xEncodeBin( 1, OFF_PART_SIZE_CTX );

                // MergeFlag
                xEncodeBin( !!nMergeIdx, OFF_MERGE_FLAG_EXT_CTX );

                if ( nMergeIdx ) {
                    xWriteMergeIdx( nMergeIdx-1 );
                }
                else {
                    // InterDirPU
                    // RefFrmIdxPU

                    // MvdPU
                    xWriteMVD( mvd );

                    // MVPIdxPU
                    xWriteUnaryMaxSymbol( ucMVPIdx, OFF_MVP_IDX_CTX, 1, AMVP_MAX_NUM_CANDS-1 );
                }

                // Merge && SIZE_2Nx2N have not QT_ROOT_CBF
                if ( !( nMergeIdx && 1 ) ) {
                    xEncodeBin( nQtRootCbf, OFF_QT_ROOT_CBF_CTX );
                }
            }

            // ***** Write Transform Info *****
            if ( !nIsInter || nQtRootCbf ) {
                // Write Cbf of Chroma
                xEncodeBin( nCbfU, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(nSizeY, FALSE) );
                xEncodeBin( nCbfV, OFF_QT_CBF_CTX + 1*NUM_QT_CBF_CTX + getCtxQtCbf(nSizeY, FALSE) );
            }
        }

        // ***** Write Transform Info *****
        if ( !nIsInter || (nIsInter && nQtRootCbfC) ) {
            // Write Cbf of Luma
            xEncodeBin( nCbfY, OFF_QT_CBF_CTX + 0*NUM_QT_CBF_CTX + getCtxQtCbf(nSizeY, TRUE) );
        }

        // Coeff
        if(nCbfY) {
            xEncodeCoeffNxN( pslCoefY, nSizeY, TRUE, nModeY );
        }
        if ( (nSplit != 0) || ((nSplit == 0) && ((nIdx & 3) == 3)) ) {

            // Last Luma4x4 will output Chroma, so I use LeftAbove info
            if ( (nSplit == 0) && ((nIdx & 3) == 3) ) {
                UInt ucInfoLT = puclModeYInfo0[-1*MAX_PU_XY-1];
                nModeIdxC = (ucInfoLT >> FLAG_OFF_MODEC) & 7;
                nModeY    = puclModeY[-1*(MAX_PU_XY+2)-1];
                nCbfU     = !!(ucInfoLT & FLAG_MASK_CBFU);
                nCbfV     = !!(ucInfoLT & FLAG_MASK_CBFV);
                pslCoefU -= 1*2*MAX_CU_SIZE/2 + 1*2;
                pslCoefV -= 1*2*MAX_CU_SIZE/2 + 1*2;
                nSizeC    = 4;
            }

            assert( nModeIdxC <= NUM_CHROMA_MODE-1 );
            if ( nModeIdxC == CHROMA_MODE_DM ) {
                realModeC = nModeY;
            }
            else {
                realModeC = nMostModeC[nModeIdxC];
                if ( realModeC == nModeY ) {
                    realModeC = 34;
                }
            }

            if(nCbfU) {
                xEncodeCoeffNxN( pslCoefU, nSizeC, FALSE, realModeC );
            }
            if(nCbfV) {
                xEncodeCoeffNxN( pslCoefV, nSizeC, FALSE, realModeC );
            }
        }

_do_next_pu:;
        // Update position index
        nIdx += ( 1 << ( ((2+nSplit) << 1) - 4) );
    } while( nIdx != (MAX_PU_XY * MAX_PU_XY) );  // TODO: Can't Support max CUSize other than MAX_CU_SIZE
}


// ***************************************************************************
// * Internal Debug Functions
// ***************************************************************************
#if defined(DEBUG_VIS)
static
void xDrawBox( UInt8 *pucPix, UInt8 ucVal, UInt nSize, UInt nWidth )
{
    UInt i;

    for( i=0; i<nSize; i++ ) {
        pucPix[i*nWidth + 0      ] = ucVal;
        pucPix[i*nWidth + nSize-1] = ucVal;
        pucPix[i                 ] = ucVal;
        pucPix[i+nWidth*(nSize-1)] = ucVal;
    }
}

void xDrawVisCU(
    xFrame *frm_vis,
    UInt8  *pucPixY,
    UInt8  *pucPixU,
    UInt8  *pucPixV,
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
        UInt    nSizeC = ( nSizeY >> 1 );
        UInt8  *pucY = pucVisY + dy*4*nWidth   + dx*4;
        UInt8  *pucU = pucVisU + dy*2*nWidth/2 + dx*2;
        UInt8  *pucV = pucVisV + dy*2*nWidth/2 + dx*2;

        xDrawBox( pucY, 0xFF, nSizeY, nWidth   );
        xDrawBox( pucU, 0x80, nSizeC, nWidth/2 );
        xDrawBox( pucV, 0x80, nSizeC, nWidth/2 );

        nIdx += (nSizeY * nSizeY / 16);
    } while( nIdx != (MAX_PU_XY * MAX_PU_XY) );  // TODO: Can't Support max CUSize other than MAX_CU_SIZE
}
#endif // DEBUG_VIS

Int xFillMvpCand(
    xMV        *psMVP,
    CUInt8     *pucModeY,
    const xMV  *psMV,
    CUInt       nSize
)
{
    xMV  mvZero   = {0, 0};
    UInt nMvSize  = nSize / MIN_MV_SIZE;
    UInt nBlkSize = nSize / MIN_CU_SIZE;
    UInt bBL      = ( pucModeY[ -1 + (nBlkSize  )*(MAX_PU_XY+2) ] != MODE_INVALID );
    UInt bL       = ( pucModeY[ -1 + (nBlkSize-1)*(MAX_PU_XY+2) ] != MODE_INVALID );
    UInt bTR      = ( pucModeY[      (nBlkSize  )-(MAX_PU_XY+2) ] != MODE_INVALID );
    UInt bT       = ( pucModeY[      (nBlkSize-1)-(MAX_PU_XY+2) ] != MODE_INVALID );
    UInt bTL      = ( pucModeY[ -1               -(MAX_PU_XY+2) ] != MODE_INVALID );
    UInt bGrpL    = (bBL || bL);
    UInt bGrpT    = (bTR || bT || bTL);
    xMV  mvBL     = psMV[ -1 + (nMvSize  )*(MAX_MV_XY+2) ];
    xMV  mvL      = psMV[ -1 + (nMvSize-1)*(MAX_MV_XY+2) ];
    xMV  mvTR     = psMV[      (nMvSize  )-(MAX_MV_XY+2) ];
    xMV  mvT      = psMV[      (nMvSize-1)-(MAX_MV_XY+2) ];
    xMV  mvTL     = psMV[ -1              -(MAX_MV_XY+2) ];
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
    if ( bL ) {
        psMVP[MRG_CANDS_START + (nCount++)] = mvL;
    }

    // Above
    if ( bT && (!bL || (mvL.x != mvT.x || mvL.y != mvT.y)) ) {
        psMVP[MRG_CANDS_START + (nCount++)] = mvT;
    }

    // Above Right
    if ( bTR && (!bT || (mvT.x != mvTR.x || mvT.y != mvTR.y)) ) {
        psMVP[MRG_CANDS_START + (nCount++)] = mvTR;
    }

    // Left Bottom
    if ( bBL && (!bL || (mvL.x != mvBL.x || mvL.y != mvBL.y)) ) {
        psMVP[MRG_CANDS_START + (nCount++)] = mvBL;
    }

    // Above Left
    if ( nCount < 4 ) {
        if ( bTL && (!bL || (mvL.x != mvTL.x || mvL.y != mvTL.y))
                 && (!bT || (mvT.x != mvTL.x || mvT.y != mvTL.y)) ) {
            psMVP[MRG_CANDS_START + (nCount++)] = mvTL;
        }
        else {
            psMVP[MRG_CANDS_START + (nCount++)] = mvZero;
        }
    }

    return nCount;
}

// ***************************************************************************
// * MotionSearch Functions
// ***************************************************************************
xMV xMotionSearch(
    CUInt8     *pucCurY,
    CUInt8     *pucRefY,
     UInt8     *pucPredY,
      Int16    *psTmp,
    CUInt32     nRefStrideY,
    CUInt       nSize
)
{
    CInt8 dir4[4][2] = {
        {-1, 0}, {0, -1}, {1, 0}, {0, 1}
    };
    CUInt nLog2Size = xLog2(nSize - 1);
    xMV bmv = {0, 0}, tmv;
    Int bmx = 0, bmy = 0;
    UInt32 bSad;
    int i, k, step;
    Int bestK;
    UInt bChanged;
    UInt8 tmp[2][MAX_CU_SIZE*MAX_CU_SIZE];
    UInt32 tmp_idx = 0;

    // Center
    bSad = xSadN[nLog2Size-2]( pucCurY, MAX_CU_SIZE, pucRefY, nRefStrideY );

    // Integer Search
    for( i=0; i<16; i++ ) {
        bChanged = FALSE;
        for( k=0; k<ASIZE(dir4); k++ ) {
            Int tX = bmx + dir4[k][0];
            Int tY = bmy + dir4[k][1];

            UInt32 tSad = xSadN[nLog2Size-2]( pucCurY, MAX_CU_SIZE, pucRefY + tY * nRefStrideY + tX, nRefStrideY );
            if ( tSad < bSad ) {
                bChanged = TRUE;
                bSad = tSad;
                bestK = k;
            }
        }
        if ( !bChanged )
            break;
        bmx += dir4[bestK][0];
        bmy += dir4[bestK][1];

        if ( bSad < 800 )
            break;
    }

    bmx <<= 2;
    bmy <<= 2;

    // Half & Quarter Search
    step = 2;
    for( i=0; i<2; i++ ) {
        bChanged = FALSE;
        for( k=0; k<ASIZE(dir4); k++ ) {
            Int tX = bmx + dir4[k][0] * step;
            Int tY = bmy + dir4[k][1] * step;

            tmv.x = tX;
            tmv.y = tY;
            xPredInterLumaBlk( pucRefY, tmp[tmp_idx], nRefStrideY, nSize, tmv, psTmp );
            UInt32 tSad = xSadN[nLog2Size-2]( pucCurY, MAX_CU_SIZE, tmp[tmp_idx], MAX_CU_SIZE );
            if ( tSad < bSad ) {
                bChanged = TRUE;
                bSad = tSad;
                bestK = k;
                tmp_idx ^= 1;
            }
        }
        if ( bChanged ) {
            bmx += dir4[bestK][0] * step;
            bmy += dir4[bestK][1] * step;
        }
        step >>= 1;
    }

    // MC
    Int32  dx = bmx >> 2;
    Int32  dy = bmy >> 2;
    UInt32 xyType = ((bmx|bmy)&3);

    CUInt8 *P =  xyType ? tmp[tmp_idx^1] : pucRefY+dy * nRefStrideY + dx;
    UInt32 nStride = xyType ? MAX_CU_SIZE : nRefStrideY;
    for( i=0; i<nSize; i++ ) {
        memcpy( pucPredY+i*MAX_CU_SIZE, P+i*nStride, nSize);
    }

    bmv.x = bmx;
    bmv.y = bmy;
    return bmv;
}

// ***************************************************************************
// * Internal Interpolate Functions
// ***************************************************************************
static
void xInterpolate8H(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
    CUInt       nFrac
)
{
    //assert( nFrac != 0 );
    int i, j;

    for( i=0; i<(int)nSize; i++ ) {
        CUInt8 *P = pucRef + i * nRefStride - (NTAPS_LUMA/2-1);
         UInt8 *Q = pucPred + i * MAX_CU_SIZE;

        for( j=0; j<(int)nSize; j++ ) {
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

static
void xInterpolate8V(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
    CUInt       nFrac
)
{
    //assert( nFrac != 0 );
    int i, j;

    for( i=0; i<(int)nSize; i++ ) {
        CUInt8 *P = pucRef  + i;
         UInt8 *Q = pucPred + i;

        for( j=0; j<(int)nSize; j++ ) {
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

static
void xInterpolate8HV(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
    CUInt       xFrac,
    CUInt       yFrac,
      Int16    *psTmp
)
{
    // TODO: Check the buffer size of psTmp
    //assert( nFrac != 0 );
    int i, j;

    // InterpolateH
    for( i=0; i<(int)nSize+NTAPS_LUMA-1; i++ ) {
        CUInt8 *P = pucRef + (i-(NTAPS_LUMA/2-1)) * nRefStride - (NTAPS_LUMA/2-1);

        for( j=0; j<(int)nSize; j++ ) {
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
    for( i=0; i<(int)nSize; i++ ) {
        CInt16 *P = psTmp + ((NTAPS_LUMA/2-1) * MAX_CU_SIZE) + i;
         UInt8 *Q = pucPred + i;

        for( j=0; j<(int)nSize; j++ ) {
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

static
void xInterpolate4H(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
    CUInt       nFrac
)
{
    //assert( nFrac != 0 );
    int i, j;

    for( i=0; i<(int)nSize; i++ ) {
        CUInt8 *P = pucRef + i * nRefStride - (NTAPS_CHROMA/2-1);
         UInt8 *Q = pucPred + i * MAX_CU_SIZE/2;

         for( j=0; j<(int)nSize; j++ ) {
            Int16 iSum;
            iSum  = P[0+j] * xg_chromaFilter[nFrac][0];
            iSum += P[1+j] * xg_chromaFilter[nFrac][1];
            iSum += P[2+j] * xg_chromaFilter[nFrac][2];
            iSum += P[3+j] * xg_chromaFilter[nFrac][3];
            Q[j] = Clip3( 0, 255, (iSum + 32) >> 6);
        }
    }
}

static
void xInterpolate4V(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
    CUInt       nFrac
)
{
    //assert( nFrac != 0 );
    int i, j;

    for( i=0; i<(int)nSize; i++ ) {
        CUInt8 *P = pucRef  + i;
         UInt8 *Q = pucPred + i;

         for( j=0; j<(int)nSize; j++ ) {
            Int16 iSum;
            iSum  = P[(j-1) * nRefStride] * xg_chromaFilter[nFrac][0];
            iSum += P[(j-0) * nRefStride] * xg_chromaFilter[nFrac][1];
            iSum += P[(j+1) * nRefStride] * xg_chromaFilter[nFrac][2];
            iSum += P[(j+2) * nRefStride] * xg_chromaFilter[nFrac][3];
            Q[j*MAX_CU_SIZE/2] = Clip3( 0, 255, (iSum + 32) >> 6);
        }
    }
}

static
void xInterpolate4HV(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
    CUInt       xFrac,
    CUInt       yFrac,
      Int16    *psTmp
)
{
    // TODO: Check the buffer size of psTmp
    //assert( nFrac != 0 );
    int i, j;

    // InterpolateH
    for( i=0; i<(int)nSize+NTAPS_CHROMA-1; i++ ) {
        CUInt8 *P = pucRef + (i-(NTAPS_CHROMA/2-1)) * nRefStride - (NTAPS_CHROMA/2-1);

        for( j=0; j<(int)nSize; j++ ) {
            Int16 iSum;
            iSum  = P[0+j] * xg_chromaFilter[xFrac][0];
            iSum += P[1+j] * xg_chromaFilter[xFrac][1];
            iSum += P[2+j] * xg_chromaFilter[xFrac][2];
            iSum += P[3+j] * xg_chromaFilter[xFrac][3];
            psTmp[i*MAX_CU_SIZE/2+j] = (iSum - 8192);
        }
    }

    // InterpolateV
    for( i=0; i<(int)nSize; i++ ) {
        CInt16 *P = psTmp + ((NTAPS_CHROMA/2-1) * MAX_CU_SIZE/2) + i;
         UInt8 *Q = pucPred + i;

        for( j=0; j<(int)nSize; j++ ) {
            Int32 iSum;
            iSum  = P[(j-1) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][0];
            iSum += P[(j+0) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][1];
            iSum += P[(j+1) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][2];
            iSum += P[(j+2) * MAX_CU_SIZE/2] * xg_chromaFilter[yFrac][3];
            Q[j*MAX_CU_SIZE/2] = Clip3( 0, 255, (iSum + 8192*64+2048) >> 12);
        }
    }
}

// ***************************************************************************
// * Interpolate Functions
// ***************************************************************************
void xPredInterLumaBlk(
    CUInt8     *pucRef,
     UInt8     *pucPred,
    CUInt32     nRefStride,
    CUInt       nSize,
    xMV         bmv,
      Int16    *psTmp
)
{
    Int  dx = bmv.x >> 2;
    Int  dy = bmv.y >> 2;
    UInt xFrac = bmv.x & 3;
    UInt yFrac = bmv.y & 3;
    UInt xyType = (yFrac != 0 ? 2 : 0) + (xFrac != 0 ? 1 : 0);
    int i;

    pucRef += dy * nRefStride + dx;
    switch( xyType ) {
    case 0:
        for( i=0; i<(int)nSize; i++ ) {
            memcpy( pucPred + i * MAX_CU_SIZE, pucRef + i * nRefStride, nSize );
        }
        break;
    case 1:
        xInterpolate8H( pucRef, pucPred, nRefStride, nSize, xFrac );
        break;
    case 2:
        xInterpolate8V( pucRef, pucPred, nRefStride, nSize, yFrac );
        break;
    /*case 3*/default:
        xInterpolate8HV( pucRef, pucPred, nRefStride, nSize, xFrac, yFrac, psTmp );
        break;
    }
}

void xPredInterChromaBlk(
    CUInt8     *pucRefC,
     UInt8     *pucPredC,
    CUInt32     nRefStrideC,
    CUInt       nSizeC,
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

    pucRefC += dy * nRefStrideC + dx;
    switch( xyType ) {
    case 0:
        for( i=0; i<(int)nSizeC; i++ ) {
            memcpy( pucPredC + i * MAX_CU_SIZE/2, pucRefC + i * nRefStrideC, nSizeC );
        }
        break;
    case 1:
        xInterpolate4H ( pucRefC, pucPredC, nRefStrideC, nSizeC, xFrac );
        break;
    case 2:
        xInterpolate4V ( pucRefC, pucPredC, nRefStrideC, nSizeC, yFrac );
        break;
    /*case 3*/default:
        xInterpolate4HV( pucRefC, pucPredC, nRefStrideC, nSizeC, xFrac, yFrac, psTmp );
        break;
    }
}
