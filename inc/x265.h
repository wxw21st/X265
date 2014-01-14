/*****************************************************************************
 * x265.h: Define struct and macro
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Min Chen <chenm003@163.com>, Xiangwen Wang <wxw21st@163.com>
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
#include "rateCtrl.h"

#ifdef STATIC_PSNR
#ifdef _MSC_VER
typedef unsigned __int64    UInt64;
typedef          __int64     Int64;
typedef          __int64    int64_t;
//x64 modify, comment out disabled warnings, pay intensive attention to all warnings
//#pragma warning(disable:4018)   // '<' signed/unsigned mismatch
//#pragma warning(disable:4389)   //  '==' : signed/unsigned mismatch
//#pragma warning(disable:4068)   // unknown pragma
//#pragma warning(disable:4324)   // structure was padded due to __declspec(align())

#else
typedef unsigned long long  UInt64;
typedef          long long   Int64;
#endif
#define SLICE_TYPE_I SLICE_I
#define SLICE_TYPE_P SLICE_P
#define SLICE_TYPE_B SLICE_B
#endif

#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef WIN32
#include "fifo.h"
#endif

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
    SLICE_I = 2,
	X265_MAX_SLICE_TYPE = 3
} xSliceType;

#define CU_TYPE_INTERVAL	5
#define CU_TYPE_OFF			0
#define CU_TYPE_MERGE_OFF	1
#define CU_TYPE_SKIP_OFF	2
#define CU_TYPE_CBF_ZERO	3
#define CU_TYPE_FULL_OFF	4
enum cu_class_e
{
	I_4x4           = 0,
	I_8x8           = 1,
	I_16x16         = 2,
	I_32x32         = 3,

	P_8x8           = CU_TYPE_INTERVAL,
	P_8x8_merge     = CU_TYPE_INTERVAL + CU_TYPE_MERGE_OFF,
	P_8x8_skip      = CU_TYPE_INTERVAL + CU_TYPE_SKIP_OFF,
	P_8x8_cbf       = CU_TYPE_INTERVAL + CU_TYPE_CBF_ZERO,
	P_8x8_full       = CU_TYPE_INTERVAL + CU_TYPE_FULL_OFF,

	P_16x16         = 8,
	P_16x16_merge   = 9,
	P_16x16_skip    = 10,
	P_16x16_cbf     = 11,

	P_32x32         = 12,
	P_32x32_merge   = 13,
	P_32x32_skip    = 14,
	P_32x32_cbf     = 15,

	P_64x64         = 16,
	P_64x64_merge   = 17,
	P_64x64_skip    = 18,
	P_64x64_cbf     = 19,

	I_PCM           = 22,

	P_L0            = 23,

	P_SKIP          = 25,

	B_DIRECT        = 26,
	B_SKIP          = 27,

	X265_MAX_CUTYPE = 60
};


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
	Int     iStrideY;//x64 modify, convert to int
	Int     iStrideC;//x64 modify, convert to int
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
    UInt8   aucRecY[(MAX_CU_SIZE  +16) * MAX_CU_SIZE   ];  
    UInt8   aucRecU[(MAX_CU_SIZE/2+8) * (MAX_CU_SIZE/2)];
    UInt8   aucRecV[(MAX_CU_SIZE/2+8) * (MAX_CU_SIZE/2)];

    // IntraDecide buffer
    UInt8   aucPredY[MAX_CU_SIZE*MAX_CU_SIZE  ];
    UInt8   aucPredU[MAX_CU_SIZE*MAX_CU_SIZE/4];
    UInt8   aucPredV[MAX_CU_SIZE*MAX_CU_SIZE/4];

    UInt8   aucModeY[(MAX_PU_XY + 2) * (MAX_PU_XY + 2)    ];
#define FLAG_MASK_MODE  (0x003F)
#define FLAG_MASK_MRG   (0x003F)
#define FLAG_OFF_MODEC  (    13)
#define FLAG_OFF_PUPART (    11)

#define FLAG_OFF_REALC  (    10)
#define FLAG_OFF_CBFC   (     8)
#define FLAG_MASK_INTER (0x0080)
#define FLAG_MASK_CBFU  (0x0100)
#define FLAG_MASK_CBFV  (0x0200)
    UInt16  ausModeYInfo0[MAX_PU_XY * MAX_PU_XY    ]; // bit15-13:modeC, bit9:cbfV, bit8:cbfU, bit7:Intra/Inter, bit5-0:Mapped Mode
#define FLAG_MASK_SIZE  (0xC0)
#define FLAG_OFF_SIZE   (   6)
#define FLAG_MASK_CBFY  (0x20)
#define FLAG_OFF_CBFY   (   5)
#define FLAG_OFF_VALIDS (   0)
#define FLAG_MASK_SKIP  (0x01)
#define B64x64_FLAG_OFF	(	4)
    UInt8   aucModeYInfo1[(MAX_PU_XY+1) * (MAX_PU_XY+1)    ]; // bit7-6:Size, bit5:cbfY, bit4-0:Valids

//#if (USE_ASM > ASM_NONE)
//    __declspec(align(16))
//#endif
		//x64 modify, set prediction buffers to be 16-byte aligned
	__declspec(align(16)) UInt8   aucPredTmp4[4*4*3];   // Internal buffers, pointer pucBuf to these
	__declspec(align(16)) UInt8   aucPredTmp8[8*8*3];   // *3 for PU partition, when MC, 0: 2Nx2N, 1: 2NxN, 2: Nx2N
	__declspec(align(16)) UInt8   aucPredTmp16[16*16*3];
	__declspec(align(16)) UInt8   aucPredTmp32[32*32*3];
	__declspec(align(16)) UInt8   aucPredTmp64[64*64*3];

	UInt8   aucRecYTmp4[4*4];   // Internal buffers, pointer pucBuf to these
	UInt8   aucRecYTmp8[8*8];   // 
	UInt8   aucRecYTmp16[16*16];
	UInt8   aucRecYTmp32[32*32];
	UInt8   aucRecYTmp64[64*64];

	Int16	asCoefYTmp4[4*4];   // Internal buffers, pointer pucBuf to these
	Int16   asCoefYTmp8[8*8];   // 
	Int16   asCoefYTmp16[16*16];
	Int16   asCoefYTmp32[32*32];
	Int16   asCoefYTmp64[64*64];

	Int		iCbfYTmp4;
	Int		iCbfYTmp8;
	Int		iCbfYTmp16;
	Int		iCbfYTmp32;
	Int		iCbfYTmp64;

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
    Int16   psTmp[4][(MAX_CU_SIZE+16)*MAX_CU_SIZE];
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
#define OFF_SKIP_FLAG_CTX           ( OFF_SPLIT_FLAG_CTX        +   NUM_SPLIT_FLAG_CTX      ) //0+3=3
#define OFF_MERGE_FLAG_EXT_CTX      ( OFF_SKIP_FLAG_CTX         +   NUM_SKIP_FLAG_CTX   )     //3+3=6
#define OFF_MERGE_IDX_EXT_CTX       ( OFF_MERGE_FLAG_EXT_CTX    +   NUM_MERGE_FLAG_EXT_CTX  ) //6+1=7
#define OFF_PART_SIZE_CTX           ( OFF_MERGE_IDX_EXT_CTX     +   NUM_MERGE_IDX_EXT_CTX   ) //7+1=8
#define OFF_CU_AMP_CTX              ( OFF_PART_SIZE_CTX         +   NUM_PART_SIZE_CTX       ) //8+4=12
#define OFF_PRED_MODE_CTX           ( OFF_CU_AMP_CTX            +   NUM_CU_AMP_CTX          ) //12+1=13
#define OFF_INTRA_PRED_CTX          ( OFF_PRED_MODE_CTX         +   NUM_PRED_MODE_CTX       ) //13+1=14
#define OFF_CHROMA_PRED_CTX         ( OFF_INTRA_PRED_CTX        +   NUM_ADI_CTX             ) //14+1=15
#define OFF_INTER_DIR_CTX           ( OFF_CHROMA_PRED_CTX       +   NUM_CHROMA_PRED_CTX     ) //15+2=17
#define OFF_MVD_CTX                 ( OFF_INTER_DIR_CTX         +   NUM_INTER_DIR_CTX       ) //17+5=22 //--4
#define OFF_REF_PIC_CTX             ( OFF_MVD_CTX               +   NUM_MV_RES_CTX          ) //22+2=24
#define OFF_DELTA_QP_CTX            ( OFF_REF_PIC_CTX           +   NUM_REF_NO_CTX          ) //24+2=26
#define OFF_QT_CBF_CTX              ( OFF_DELTA_QP_CTX          +   NUM_DELTA_QP_CTX        ) //26+3=29
#define OFF_QT_ROOT_CBF_CTX         ( OFF_QT_CBF_CTX            + 2*NUM_QT_CBF_CTX          ) //29+10=39
#define OFF_SIG_CG_FLAG_CTX         ( OFF_QT_ROOT_CBF_CTX       +   NUM_QT_ROOT_CBF_CTX     ) //39+1 =40
#define OFF_SIG_FLAG_CTX            ( OFF_SIG_CG_FLAG_CTX       + 2*NUM_SIG_CG_FLAG_CTX     ) //40+4 =44
#define OFF_LAST_X_CTX              ( OFF_SIG_FLAG_CTX          +   NUM_SIG_FLAG_CTX        ) //44+42=86
#define OFF_LAST_Y_CTX              ( OFF_LAST_X_CTX            + 2*NUM_LAST_FLAG_XY_CTX    ) //86+30=116
#define OFF_ONE_FLAG_CTX            ( OFF_LAST_Y_CTX            + 2*NUM_LAST_FLAG_XY_CTX    ) //116+30=146
#define OFF_ABS_FLAG_CTX            ( OFF_ONE_FLAG_CTX          +   NUM_ONE_FLAG_CTX        ) //146+24=170
#define OFF_MVP_IDX_CTX             ( OFF_ABS_FLAG_CTX          +   NUM_ABS_FLAG_CTX        ) //170+6 =176
#define OFF_TRANS_SUBDIV_FLAG_CTX   ( OFF_MVP_IDX_CTX           +   NUM_MVP_IDX_CTX         ) //176+2 =178
#define OFF_TS_FLAG_CTX             ( OFF_TRANS_SUBDIV_FLAG_CTX +   NUM_TRANS_SUBDIV_FLAG_CTX)//178+3 =181
#define OFF_SAO_MERGE_FLAG_CTX      ( OFF_TS_FLAG_CTX           +   NUM_TRANSFORMSKIP_FLAG_CTX)//181+1 =182
#define OFF_SAO_TYPE_IDX_CTX        ( OFF_SAO_MERGE_FLAG_CTX    +   NUM_SAO_MERGE_FLAG_CTX)   //182+1 =183

} xCabac;

//#if DEBLOCK //x64 modify
typedef struct SPS{
	int pic_width_in_luma_samples;
	int pic_height_in_luma_samples;
	int log2_min_coding_block_size; ///< log2_min_coding_block_size_minus3 + 3
	int log2_ctb_size; ///< Log2CtbSize
	int pic_width_in_ctbs; ///< PicWidthInCtbs
	int pic_height_in_ctbs; ///< PicHeightInCtbs
	int pic_width_in_min_cbs;
	int pic_height_in_min_cbs;
	int log2_min_pu_size;
	int qp_bd_offset;
}SPS;

typedef struct DBParams {
	UInt8	disable;
	int		beta_offset;
	int		tc_offset;
} DBParams;
//#endif

#ifdef SAO
typedef struct _SaoLcuParam {
    int     mergeUpFlag;
    int     mergeLeftFlag;
    int     typeIdx;
    int     subTypeIdx;                ///< indicates EO class or BO band position
    int     offset[4];
//     int     partIdx;
//     int     partIdxTmp;
    int     length;
} SaoLcuParam;

typedef struct _SAOParam {
    int             bSaoFlag[2];
    SaoLcuParam     saoLcuParam[3][MAX_WIDTH_CU * MAX_HEIGHT_CU];
} SAOParam;
#endif

typedef struct xRdoParam {
	UInt    nCoeffBits;
	xCabac *pCabacRdo;
	xCabac *pCabacRdoLast;
	UInt8   nextState[128][2];
	UInt64  fracBits;
} xRdoParam;

/// main handle
typedef struct X265_t {
    // Local
    xBitStream  bs;
    xCabac      cabac;

#if USE_WPP_YANAN
		xCabac      cabacSavedWPP;
#endif

//following parameters support parallel RDO processing, by YananZhao, 2013-11-14
		xCabac    cabacRdoSaved;                                //cabac RDO state for next CTU row initialization (WPP)
		xRdoParam rdoParam[MAX_HEIGHT_CU];                      //rdo parameter for each CTU row
		xCabac    cabacRdo[MAX_HEIGHT_CU][MAX_CU_DEPTH + 1];    //cabac in RDO for each CTU row
		xCabac    cabacRdoLast[MAX_HEIGHT_CU][MAX_CU_DEPTH + 1];//save the cabac state for the right neighbor CTU

#if FAST_MD
		double    dSumOfRdCosts[MAX_HEIGHT_CU][MAX_WIDTH_CU][MAX_CU_DEPTH];
		Int       nRdCounts[MAX_HEIGHT_CU][MAX_WIDTH_CU][MAX_CU_DEPTH];
		double    dAverageRdCosts[MAX_HEIGHT_CU][MAX_WIDTH_CU][MAX_CU_DEPTH];

		__declspec(align(16))
		UInt8     nPredYTmp[MAX_HEIGHT_CU][MRG_MAX_NUM_CANDS][MAX_CU_SIZE * MAX_CU_SIZE] ;//x64 modify, add align
#endif

		xSliceType  eSliceType;
    #if defined(DEBUG_VIS)
    xFrame      frm_vis;
    #endif
    xFrame      *pFrameCur;
		xFrame      *pFrameRec;
		xFrame      refn[MAX_REF_NUM+1];
		xFrame      *pFrameRef;

		Int32       iPoc;
    UInt32      uiFrames;
    Int32       iQP;

    // Thread Shared
    UInt8       aucTopPixY[4][MAX_WIDTH     + 1]; //wxw
    UInt8       aucTopPixU[4][MAX_WIDTH / 2 + 1];
    UInt8       aucTopPixV[4][MAX_WIDTH / 2 + 1];
    UInt8       aucTopModeYInfo1[4][(MAX_WIDTH / MIN_CU_SIZE)];
    xMV         asTopMV[4][(MAX_WIDTH / MIN_MV_SIZE)+1];

    // TODO: Optimize buffers.
#define FLAG_MASK_TYPE  (0x00008000)
#define FLAG_TYPE_BIN   (0x00008000)
#define FLAG_TYPE_EPS   (0x00002000)
#define FLAG_TYPE_TRM   (0x00004000)
#define FLAG_OFF_VAL    (        16)
#define FLAG_MASK_VAL   (0xFFFF0000)
#define FLAG_OFF_CTX    (         0)
#define FLAG_MASK_CTX   (0x000000FF)
#define MAX_CMDBUF_SIZE (4 * 4 * 1024)  // CHECK_ME: reduce  4* --wxw
    UInt32      auiCabacCmd[MAX_HEIGHT_CU][MAX_WIDTH_CU][MAX_CMDBUF_SIZE];
    UInt32      auiCabacCmdNum[MAX_HEIGHT_CU][MAX_WIDTH_CU];
		UInt32		  auiCTUCmd[MAX_HEIGHT_CU][MAX_CMDBUF_SIZE];  //this buffer is for RDO process of one CTU, just for bitrate calculate. 

#if SAO
    UInt32      auiSaoCmd[MAX_HEIGHT_CU][MAX_WIDTH_CU][MAX_CMDBUF_SIZE];
    UInt32      auiSaoCmdNum[MAX_HEIGHT_CU][MAX_WIDTH_CU];
#endif

#ifdef _OPENMP
  #ifndef WIN32
    UInt32      cuaddr[2*MAX_HEIGHT_CU+1];
  #else
    fifo_t      cuaddr[2*MAX_HEIGHT_CU+1];
  #endif
    omp_lock_t  bslck[2*MAX_HEIGHT_CU+1];
#endif
    // ~Thread Shared

    // Interface
    // Profile
    UInt8   ucProfileSpace;
    UInt8   ucProfileIdc;
    UInt8   ucLevelIdc;

    // Params
    Int16   iWidth; //x64 modify, change type to Int
    Int16   iHeight;//x64 modify, change type to Int
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
    UInt8   bTiles;

		// Pre-Processing
		UInt8 buf1[MAX_WIDTH * MAX_HEIGHT * 3 / 2];
		Int bFrameAlignment;
		Int iWidthOrg;
		Int iHeightOrg;
		Int bUseSceneChangeDetection;
		Int iSceneChangeIntervalCurr;


		Int8   iCUWidth;    //pic width in unit of CTU size //x64 modify, change to Int
		Int8   iCUHeight;   //pic height in unit of CTU size //x64 modify

		//Rate control
		UInt64  nFramesToBeEncoded;//for debug only
		Int     nIntraPeriod;
		UInt8   nFrameRate;
		UInt8   nGOPSize;
		UInt8   bUseRateCtrl;
		Int     targetKbps;
		RateCtrl rc;

    // Debug
    UInt8   bWriteRecFlag;
    UInt8   bWriteVisCUFlag;

    // Internal
    UInt32  PicWidthInCUs;
    UInt32  PicHeightInCUs;

	struct
	{
		/* per slice info */
		int     i_slice_count[5];
		Int64   i_slice_size[5];
		/* */
		Int64   i_sqe_global[5];
		float   f_psnr_average[5];
		float   f_psnr_mean_y[5];
		float   f_psnr_mean_u[5];
		float   f_psnr_mean_v[5];

		Int64   iCuTypeCount[3][X265_MAX_CUTYPE];
		//int i_mb_count[19];
	} stat;

	xMV 	mv_min_ipel;
	xMV		mv_max_ipel;
	xMV		mv_min;
	xMV		mv_max;

//#if DEBLOCK //x64 modify
	SPS    *sps;
	DBParams* deblock;
	UInt8	*vertical_bs;
	UInt8	*horizontal_bs;
	//bool	is_pcm[][];
	Int		bs_width;
	Int		bs_height;
	//bool	transquant_bypass_enable_flag;
	UInt8	*qp_y_tab;
	Int		pps_cb_qp_offset;
	Int		pps_cr_qp_offset;
	bool	pps_transquant_bypass_enable_flag;
//#endif

#if SAO
    int     _upBuff1[MAX_WIDTH + 2];
    int     _upBuff2[MAX_WIDTH + 2];
    int     _upBufft[MAX_WIDTH + 2];
    int     *m_upBuff1;
    int     *m_upBuff2;
    int     *m_upBufft;
    SAOParam sao_param;
    UInt32  numNoSao[2];
    int     sao_depth;
    float   numlcus;
    float   m_depthSaoRate[2][2];   // [2][4] if there have B-SLICE
    Int64   m_offsetOrgPreDblk[MAX_WIDTH_CU * MAX_HEIGHT_CU][3][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS]; //[LCU][YCbCr][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS];
    Int64   m_countPreDblk[MAX_WIDTH_CU * MAX_HEIGHT_CU][3][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS];    //[LCU][YCbCr][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS];
    Int64   m_offsetOrg[3][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS];
    Int64   m_countOrg[3][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS];
    Int64   m_offset[3][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS];
    Int64   m_count[3][MAX_NUM_SAO_TYPE][MAX_NUM_SAO_CLASS];
    double  m_cost[3][MAX_NUM_SAO_TYPE];
    UInt8  *m_tmpU1[3];
    UInt8  *m_tmpU2[3];
    UInt8  *m_tmpL1;
    UInt8  *m_tmpL2;
    int     m_offsetBo[2 * ((1 << 8) - 1)];
    int     m_chromaOffsetBo[2 * ((1 << 8) - 1)];
    int     m_offsetEo[LUMA_GROUP_NUM];

		UInt8 g_lumaTableBo[(1 << 8)];
		UInt8 g_chromaTableBo[(1 << 8)];
		UInt8 g_clipTableBase[2 * ((1 << 8) - 1)];
		UInt8 g_chromaClipTableBase[2 * ((1 << 8) - 1)];
		UInt8 *g_clipTable;// = NULL;
		UInt8 *g_chromaClipTable;// = NULL;
#endif

#if USE_WPP_YANAN
	UInt8* pOutBuf0;
	UInt8* pOutBuf1;
#endif

} X265_t;
// ***************************************************************************
// * bitstream.cpp
// ***************************************************************************
void xWriteSPS( X265_t *h );
void xWritePPS( X265_t *h );
void xWriteVPS( X265_t *h );
void xWriteSliceHeader( X265_t *h );
#if USE_WPP_YANAN
void xWriteTilesWPPEntryPoint_Yanan( X265_t *h, Int32 nNumEntropyPointOffsets, Int offsetLenMinus1, Int32 *nEntropyPointOffsets );//x64 modify, change type to Int
#endif
void xWriteSliceEnd( X265_t *h );
Int32 xPutRBSP(UInt8 *pucDst, UInt8 *pucSrc, UInt32 uiLength);
void xCabacInit( X265_t *h );
void xCabacReset( xCabac *pCabac );
void xCabacFlush( xCabac *pCabac, xBitStream *pBS );
UInt xCabacGetNumWrittenBits( xCabac *pCabac, xBitStream *pBS );
void xWriteCabacCmds( xCabac *pCabac, xBitStream *pBS, UInt32 *pCmd, Int32 iCmdNum );

typedef void (*tEncodeBin)(UInt32 **pCmd, UInt32 binValue, UInt32 nCtxState);
typedef void (*tEncodeBinsEP)(UInt32 **pCmd, UInt32 binValue, UInt32 numBins);
typedef void (*tEncodeTerminatingBit)(UInt32 **pCmd, UInt32 bLastCU);

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
// * RateCtrl.cpp
// ***************************************************************************
void xInitRCSeq(X265_t* h, UInt64 totalFrames, UInt8 nFrameRate, UInt8 GOPSize, Int targetKbps);
void xInitRCSequence(X265_t* h, Int targetKbps);
void xInitRCGOP(X265_t* h, Int numPic );
void xInitRCPic(X265_t* h);
void xRCPicUpdateAfterPic(X265_t* h, Int actualHeaderBits, Int actualTotalBits );
void xRCGOPUpdateAfterPic(X265_t* h, Int bitsCost );
void xRCSeqUpdateAfterPic(X265_t* h, Int bitsCost );
Int  xRCPicEstPicQP( X265_t* h, double lambda );
double xRCPicEstPicLambda( X265_t* h );

// ***************************************************************************
// * Pixel.cpp
// ***************************************************************************
typedef UInt32 xSad( CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef );
extern xSad *xSadN[MAX_CU_DEPTH];
void xSubDct ( Int16 *pDst, UInt8 *pSrc, UInt nStride, UInt8 *pRef, UInt nStrideRef, Int16 *piTmp0, Int16 *piTmp1, Int iWidth, UInt nMode );
void xIDctAdd( UInt8 *pDst, UInt nStrideDst, Int16 *pSrc, UInt nStride, UInt8 *pRef, Int16 *piTmp0, Int16 *piTmp1, Int iWidth, UInt nMode );
UInt32 xQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight, xSliceType eSType );
void xDeQuant( Int16 *pDst, Int16 *pSrc, UInt nStride, UInt nQP, Int iWidth, Int iHeight );//x64 modify, remove xSliceType


#if SATD_USE_TRUE_ASM	
typedef Int32 xSatd ( 	pixel *pSrc, intptr_t nStrideSrc, pixel *pRef, intptr_t nStrideRef 	);
#else
typedef UInt32 xSatd ( 	CUInt8 *pSrc, CUInt32 nStrideSrc, CUInt8 *pRef, CUInt32 nStrideRef 	);
#endif
extern xSatd *xSatdN[MAX_CU_DEPTH];

typedef int xSSD(pixel*, intptr_t, pixel*, intptr_t);

typedef UInt32  xME_SAD ( CUInt8 *, CUInt, CUInt8 *, CUInt );
extern xME_SAD *xSad_AMP[10];
typedef UInt32  xME_SATD ( CUInt8 *, CUInt, CUInt8 *, CUInt );
extern xME_SATD *xSatd_AMP[10];

// ***************************************************************************
// * Pre-Processing Functions
// ***************************************************************************
void xFrameAligning(xFrame *frameAligned, Int iWidthAligned, Int iHeightAligned, xFrame *frame, Int iWidth, Int iHeight);
Int sceneChangeDetection(X265_t *h, UInt8 *pYCurr); //xFrame *frameCurr); //

void	visa_encode_static(X265_t *h,int i_slice_type,int i_frame_size, UInt8 *Src_y,UInt8 *Src_u,UInt8 *Src_v);
void  visa_close_static ( X265_t *h );


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
