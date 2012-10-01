/*****************************************************************************
 * x265.h: Define struct and macro
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

#ifndef __X265_H__
#define __X265_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "bitstream.h"
#include "utils.h"

#if (USE_ASM >= ASM_AVX2)
    #error Unsupport now!
#elif (USE_ASM >= ASM_SSE4)
    #include <nmmintrin.h>
#elif (USE_ASM >= ASM_SSE2)
    #include <xmmintrin.h>
#elif (USE_ASM >= ASM_MMX)
    #include <mmintrin.h>
#endif


/// supported slice type
typedef enum {
    SLICE_B = 0,
    SLICE_P = 1,
    SLICE_I = 2
} xSliceType;

/// reference list index
typedef enum {
    REF_PIC_LIST_X = -1,    ///< reference list invalid
    REF_PIC_LIST_0 =  0,    ///< reference list 0
    REF_PIC_LIST_1 =  1,    ///< reference list 1
    REF_PIC_LIST_C =  2,    ///< combined reference list for uni-prediction in B-Slices
} xRefPicList;

/// chroma formats (according to semantics of chroma_format_idc)
typedef enum {
    CHROMA_400  = 0,
    CHROMA_420  = 1,
    CHROMA_422  = 2,
    CHROMA_444  = 3
} eChromaFormat;

/// reference frame
typedef struct xFrame {
    UInt8   *pucY;
    UInt8   *pucU;
    UInt8   *pucV;
    UInt     nStrideY;
    UInt     nStrideC;
    UInt8   *base;
} xFrame;

typedef struct xMV {
    Int16   x;
    Int16   y;
} xMV;

/// cache for every processor
typedef enum {
    MODE_INVALID    = 255,
    MODE_VALID      = 0,
} eIntraMode;

/// supported partition shape
typedef enum {
    SIZE_2Nx2N,           ///< symmetric motion partition,  2Nx2N
    SIZE_2NxN,            ///< symmetric motion partition,  2Nx N
    SIZE_Nx2N,            ///< symmetric motion partition,   Nx2N
    SIZE_NxN,             ///< symmetric motion partition,   Nx N
    SIZE_2NxnU,           ///< asymmetric motion partition, 2Nx( N/2) + 2Nx(3N/2)
    SIZE_2NxnD,           ///< asymmetric motion partition, 2Nx(3N/2) + 2Nx( N/2)
    SIZE_nLx2N,           ///< asymmetric motion partition, ( N/2)x2N + (3N/2)x2N
    SIZE_nRx2N,           ///< asymmetric motion partition, (3N/2)x2N + ( N/2)x2N
    SIZE_NONE   = 15
} ePartSize;

/// coefficient scanning type used in ACS
typedef enum {
    SCAN_DIAG   = 0,    ///< up-right diagonal scan
    SCAN_HOR    = 1,    ///< horizontal first scan
    SCAN_VER    = 2,    ///< vertical first scan
} eScanType;

typedef enum {
    VALID_LB    = 0,    // Left Bottom
    VALID_L     = 1,    // Left
    VALID_LT    = 2,    // Left Top
    VALID_T     = 3,    // Top
    VALID_TR    = 4,    // Top Right
} eValidIdx;

typedef struct xCache {
    /// current
    UInt8   aucPixY[MAX_CU_SIZE * MAX_CU_SIZE    ];
    UInt8   aucPixU[MAX_CU_SIZE * MAX_CU_SIZE / 4];
    UInt8   aucPixV[MAX_CU_SIZE * MAX_CU_SIZE / 4];
    UInt8   aucRecY[(MAX_CU_SIZE  +2) * MAX_CU_SIZE    ];
    UInt8   aucRecU[(MAX_CU_SIZE/2+1) * (MAX_CU_SIZE/2)];
    UInt8   aucRecV[(MAX_CU_SIZE/2+1) * (MAX_CU_SIZE/2)];

    // IntraDecide buffer
    UInt8   aucPredY[MAX_CU_SIZE*MAX_CU_SIZE  ];
    UInt8   aucPredU[MAX_CU_SIZE*MAX_CU_SIZE/4];
    UInt8   aucPredV[MAX_CU_SIZE*MAX_CU_SIZE/4];
    UInt8   aucModeY[(MAX_PU_XY + 2) * (MAX_PU_XY + 2)    ];
#define FLAG_MASK_MODE  (0x003F)
#define FLAG_MASK_MRG   (0x003F)
#define FLAG_OFF_MODEC  (    13)
#define FLAG_OFF_REALC  (    10)
#define FLAG_OFF_CBFC   (     8)
#define FLAG_MASK_INTER (0x0080)
#define FLAG_MASK_CBFU  (0x0100)
#define FLAG_MASK_CBFV  (0x0200)
    UInt16  ausModeYInfo0[MAX_PU_XY * MAX_PU_XY    ]; // bit15-13:modeC, bit9:cbfV, bit8:cbfU, bit7:Intra/Inter, bit5-0:Mapped Mode
#define FLAG_MASK_SIZE  (0xC0)
#define FLAG_OFF_SIZE   (   6)
#define FLAG_MASK_CBFY  (0x20)
#define FLAG_OFF_VALIDS (   0)
#define FLAG_MASK_SKIP  (0x01)
    UInt8   aucModeYInfo1[(MAX_PU_XY+1) * (MAX_PU_XY+1)    ]; // bit7-6:Size, bit5:cbfY, bit4-0:Valids
#if (USE_ASM > ASM_NONE)
    __declspec(align(16))
#endif
    UInt8   aucPredTmp4[4*4*2];   // Internal buffers, pointer pucBuf to these
    UInt8   aucPredTmp8[8*8*2];
    UInt8   aucPredTmp16[16*16*2];
    UInt8   aucPredTmp32[32*32*2];

    // Inter buffer
    xMV     asMV[(MAX_MV_XY + 2) * (MAX_MV_XY + 2)];
    xMV     asMVD[MAX_MV_XY * MAX_MV_XY];
    UInt8   aucMVPIdx[MAX_MV_XY * MAX_MV_XY];

    /// Encode coeff buffer
    Int16   psCoefY[MAX_CU_SIZE*MAX_CU_SIZE];
    Int16   psCoefU[MAX_CU_SIZE*MAX_CU_SIZE/4];
    Int16   psCoefV[MAX_CU_SIZE*MAX_CU_SIZE/4];

    /// Temp buffer(share for interpolate)
    #if (USE_ASM > ASM_NONE)
    __m128i res0;
    #endif
    Int16   psTmp[2][MAX_CU_SIZE*MAX_CU_SIZE];
} xCache;

typedef struct {
    // Engine
    UInt32  uiLow;
    UInt32  uiRange;
    Int32   iBitsLeft;
    UInt8   ucCache;
    UInt32  uiNumBytes;

    // Context Model
    UInt8   contextModels[MAX_NUM_CTX_MOD];
#define OFF_SPLIT_FLAG_CTX          ( 0 )
#define OFF_SKIP_FLAG_CTX           ( OFF_SPLIT_FLAG_CTX        +   NUM_SPLIT_FLAG_CTX      )
#define OFF_MERGE_FLAG_EXT_CTX      ( OFF_SKIP_FLAG_CTX         +   NUM_SKIP_FLAG_CTX   )
#define OFF_MERGE_IDX_EXT_CTX       ( OFF_MERGE_FLAG_EXT_CTX    +   NUM_MERGE_FLAG_EXT_CTX  )
#define OFF_PART_SIZE_CTX           ( OFF_MERGE_IDX_EXT_CTX     +   NUM_MERGE_IDX_EXT_CTX   )
#define OFF_CU_AMP_CTX              ( OFF_PART_SIZE_CTX         +   NUM_PART_SIZE_CTX       )
#define OFF_PRED_MODE_CTX           ( OFF_CU_AMP_CTX            +   NUM_CU_AMP_CTX          )
#define OFF_INTRA_PRED_CTX          ( OFF_PRED_MODE_CTX         +   NUM_PRED_MODE_CTX       )
#define OFF_CHROMA_PRED_CTX         ( OFF_INTRA_PRED_CTX        +   NUM_ADI_CTX             )
#define OFF_INTER_DIR_CTX           ( OFF_CHROMA_PRED_CTX       +   NUM_CHROMA_PRED_CTX     )
#define OFF_MVD_CTX                 ( OFF_INTER_DIR_CTX         +   NUM_INTER_DIR_CTX       )
#define OFF_REF_PIC_CTX             ( OFF_MVD_CTX               +   NUM_MV_RES_CTX          )
#define OFF_DELTA_QP_CTX            ( OFF_REF_PIC_CTX           +   NUM_REF_NO_CTX          )
#define OFF_QT_CBF_CTX              ( OFF_DELTA_QP_CTX          +   NUM_DELTA_QP_CTX        )
#define OFF_QT_ROOT_CBF_CTX         ( OFF_QT_CBF_CTX            + 2*NUM_QT_CBF_CTX          )
#define OFF_SIG_CG_FLAG_CTX         ( OFF_QT_ROOT_CBF_CTX       +   NUM_QT_ROOT_CBF_CTX     )
#define OFF_SIG_FLAG_CTX            ( OFF_SIG_CG_FLAG_CTX       + 2*NUM_SIG_CG_FLAG_CTX     )
#define OFF_LAST_X_CTX              ( OFF_SIG_FLAG_CTX          +   NUM_SIG_FLAG_CTX        )
#define OFF_LAST_Y_CTX              ( OFF_LAST_X_CTX            + 2*NUM_LAST_FLAG_XY_CTX    )
#define OFF_ONE_FLAG_CTX            ( OFF_LAST_Y_CTX            + 2*NUM_LAST_FLAG_XY_CTX    )
#define OFF_ABS_FLAG_CTX            ( OFF_ONE_FLAG_CTX          +   NUM_ONE_FLAG_CTX        )
#define OFF_MVP_IDX_CTX             ( OFF_ABS_FLAG_CTX          +   NUM_ABS_FLAG_CTX        )
#define OFF_TRANS_SUBDIV_FLAG_CTX   ( OFF_MVP_IDX_CTX           +   NUM_MVP_IDX_CTX         )
#define OFF_TS_FLAG_CTX             ( OFF_TRANS_SUBDIV_FLAG_CTX +   NUM_TRANS_SUBDIV_FLAG_CTX)

} xCabac;

/// main handle
typedef struct X265_t {
    // Local
    xBitStream  bs;
    xCabac      cabac;
    xSliceType  eSliceType;
    xFrame      refn[MAX_REF_NUM+1];
    #if defined(DEBUG_VIS)
    xFrame      frm_vis;
    #endif
    xFrame      *pFrameCur;
    xFrame      *pFrameRec;
    xFrame      *pFrameRef;
    Int32       iPoc;
    UInt32      uiFrames;
    Int32       iQP;

    UInt8       aucTopPixY[MAX_WIDTH     + 1];
    UInt8       aucTopPixU[MAX_WIDTH / 2 + 1];
    UInt8       aucTopPixV[MAX_WIDTH / 2 + 1];
    UInt8       aucTopModeYInfo1[(MAX_WIDTH / MIN_CU_SIZE)];
    xMV         asTopMV[(MAX_WIDTH / MIN_MV_SIZE)+1];

    // Interface
    // Profile
    UInt8   ucProfileSpace;
    UInt8   ucProfileIdc;
    UInt8   ucLevelIdc;

    // Params
    UInt16  usWidth;
    UInt16  usHeight;
    UInt8   ucMaxCUWidth;
    UInt8   ucMaxCUDepth;
    UInt8   ucQuadtreeTULog2MinSize;
    UInt8   ucQuadtreeTULog2MaxSize;
    UInt8   ucQuadtreeTUMaxDepthInter;
    UInt8   ucQuadtreeTUMaxDepthIntra;
    UInt8   ucMaxNumRefFrames;
    UInt8   ucBitsForPOC;
    UInt8   ucMaxNumMergeCand;
    UInt8   ucTSIG;
    UInt8   nThreads;

    // Feature
    UInt8   bUseNewRefSetting;
    UInt8   bUseSAO;
    UInt8   bMRG;
    UInt8   bLoopFilterDisable;
    UInt8   bSignHideFlag;
    UInt8   bStrongIntraSmoothing;

    // Debug
    UInt8   bWriteRecFlag;
    UInt8   bWriteVisCUFlag;
} X265_t;


// ***************************************************************************
// * bitstream.cpp
// ***************************************************************************
void xWriteSPS( X265_t *h );
void xWritePPS( X265_t *h );
void xWriteVPS( X265_t *h );
void xWriteSliceHeader( X265_t *h );
void xWriteSliceEnd( X265_t *h );
Int32 xPutRBSP(UInt8 *pucDst, UInt8 *pucSrc, UInt32 uiLength);
void xCabacInit( X265_t *h );
void xCabacReset( xCabac *pCabac );
void xCabacFlush( xCabac *pCabac, xBitStream *pBS );
UInt xCabacGetNumWrittenBits( xCabac *pCabac, xBitStream *pBS );
void xWriteCabacCmds( xCabac *pCabac, xBitStream *pBS, UInt32 *pCmd, Int32 iCmdNum );


// ***************************************************************************
// * params.cpp
// ***************************************************************************
void xDefaultParams( X265_t *h );
int xCheckParams( X265_t *h );

// ***************************************************************************
// * Table.cpp
// ***************************************************************************
extern CUInt8   xg_aucIntraFilterType[5][NUM_INTRA_MODE-1];
extern CInt8    xg_aucIntraPredAngle[NUM_INTRA_MODE-1];
extern CInt16   xg_aucInvAngle[NUM_INTRA_MODE-1];
extern CInt8    xg_aiT4[4*4];
extern CInt8    xg_aiT8[8*8];
extern CInt8    xg_aiT16[16*16];
extern CInt8    xg_aiT32[32*32];
extern CInt16   xg_quantScales[6];
extern CUInt8   xg_invQuantScales[6];
extern CUInt8   xg_aucChromaScale[58];
extern CUInt8   xg_aucNextStateMPS[128];
extern CUInt8   xg_aucNextStateLPS[128];
extern CUInt8   xg_aucLPSTable[64][4];
extern CUInt8   xg_aucRenormTable[32];
extern CUInt16 *xg_ausScanIdx[3][5];
extern CUInt16  xg_sigLastScan8x8[3][4];
extern CUInt16  xg_sigLastScanCG32x32[64];
extern CUInt8   xg_uiMinInGroup[10];
extern CUInt8   xg_uiGroupIdx[32];
extern CUInt8   xg_auiGoRiceRange[5];
extern CUInt8   xg_auiGoRicePrefixLen[5];
extern CInt16   xg_lumaFilter[4][NTAPS_LUMA];
extern CInt16   xg_chromaFilter[8][NTAPS_CHROMA];

// ***************************************************************************
// * Encode.cpp
// ***************************************************************************
void xEncInit( X265_t *h );
void xEncFree( X265_t *h );
Int32 xEncodeFrame( X265_t *h, xFrame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize );

// ***************************************************************************
// * Pixel.cpp
// ***************************************************************************
typedef UInt32 xSad( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef );
extern xSad *xSadN[MAX_CU_DEPTH];
void xSubDct ( Int16 *pDst, UInt8 *pSrc, UInt nStride, UInt8 *pRef, UInt nStrideRef, Int16 *piTmp0, Int16 *piTmp1, Int iWidth, UInt nMode );
void xIDctAdd( UInt8 *pDst, UInt nStrideDst, Int16 *pSrc, UInt nStride, UInt8 *pRef, Int16 *piTmp0, Int16 *piTmp1, Int iWidth, UInt nMode );
UInt32 xQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, xSliceType eSType );
void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, xSliceType eSType );


// ***************************************************************************
// * Macro for build platform information
// ***************************************************************************
#ifdef __GNUC__
#define X265_COMPILEDBY "[GCC %d.%d.%d]", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
#ifdef __IA64__
#define X265_ONARCH     "[on 64-bit] "
#else
#define X265_ONARCH     "[on 32-bit] "
#endif
#endif

#ifdef __INTEL_COMPILER
#define X265_COMPILEDBY "[ICC %d]", __INTEL_COMPILER
#elif  _MSC_VER
#define X265_COMPILEDBY "[VS %d]", _MSC_VER
#endif

#ifndef X265_COMPILEDBY
#define X265_COMPILEDBY "[Unk-CXX]"
#endif

#ifdef _WIN32
#define X265_ONOS       "[Windows]"
#elif  __linux
#define X265_ONOS       "[Linux]"
#elif  __CYGWIN__
#define X265_ONOS       "[Cygwin]"
#elif __APPLE__
#define X265_ONOS       "[Mac OS X]"
#else
#define X265_ONOS       "[Unk-OS]"
#endif

#define X265_BITS       "[%d bit]", (sizeof(void*) == 8 ? 64 : 32) ///< used for checking 64-bit O/S

#endif /* __X265_H__ */
