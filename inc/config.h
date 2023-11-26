/*****************************************************************************
 * config.h: 
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Xiangwen Wang <wxw21st@163.com>, Min Chen <chenm003@163.com> 
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

////////////////////////////
// x265 start
////////////////////////////

#define STATIC_PSNR             0 // x64 modify, use if(STATIC_PSNR) instead of #if STATIC_PSNR
#define CU_TYPE_STAT						1  //statistic all CU info. such Skip, Merge Cbf

#define USE_WPP_YANAN						1 //WPP Syntax
#define SSD_USE_TRUE_ASM        0 //TURE ASM from x265-multicore and x264, not intrinsic, by YananZhao, 2013-11-17
#define SATD_USE_TRUE_ASM				0 //CAUTION: x264 SATD with sizes larger than 4x4 have different results compared with HM,
																	//this is because 16x16,32x32,64x64 in x264 are independently calculated, while in HM, they are added up by many 8x8s.
#define LOGO_WIDTH (192)
#define LOGO_HEIGHT (128)

#define FAST_MD                 1 //Fast inter algorithm by Zhengyi Luo, Tile is not supported yet, 2013-11-19

#define FRAME_ALIGNMENT         1 //padding frame width and height to be CTU-Size-Aligned if they are not aligned, YananZhao, always on now, 2013-12-17

#define SCENE_CHANGE_DETECTION  1 // scene change detection, can be changed through command line parameter, by Yanan Zhao, 2014-01-16
#define SCENE_CHANGE_INTERVAL   4 // number of frames between two IDRs for scene detection, avoid too frequently IDRs, by Yanan Zhao, 2014-01-16

#define MV_CLIP								  1 //clip MV range 
#define ME_RANGE							  32
#define MAX_IMV_VAL							64 //126, there is error when 128
#define PAD_YSIZE               (MAX_CU_SIZE + 16)
#define PAD_CSIZE               (PAD_YSIZE / 2)

#define SATD_FOR_FRAC_ME				0 //use SATD for Inter ME at fractional pixels
#define MVD_COST_BY_TABLE				0//lzy 1 //MVD cost is from a lookup table from MULTICORE

#define DOUBLE_MAX 1.7976931348623158e+308 
		
#define DELTA_QP_SLICE_I					0

//#define FULL_RDO							    1  // x64 modify, always set this to be on
//#define FULL_RDO_INTRA					1  // x64 modify, always set this to be on
 
#define DEBLOCK								    1  // some default at current 20131009 //x64 modify, use if(DEBLOCK) instead of #if DEBLOCK

#define SAO                                 1
//#define FULL_RDO_SAO												1 //x64 modify, remove all FULL_RDO_SAO macros as it cannot be turned off now, delete this macro later
#define SAO_MAX_DEPTH                       4
#define SAO_BO_BITS                         5
#define LUMA_GROUP_NUM                      (1 << SAO_BO_BITS)
#define MAX_NUM_SAO_OFFSETS                 4
#define MAX_NUM_SAO_CLASS                   33
#define SAO_ENCODING_RATE                   0.75
#define SAO_ENCODING_RATE_CHROMA            0.5


enum SAOTypeLen
{
    SAO_EO_LEN    = 4,
    SAO_BO_LEN    = 4,
    SAO_MAX_BO_CLASSES = 32
};

enum SAOType
{
    SAO_EO_0 = 0,
    SAO_EO_1,
    SAO_EO_2,
    SAO_EO_3,
    SAO_BO,
    MAX_NUM_SAO_TYPE
};

//parameterization
#define MAX_CU_DEPTH                        (5)
#define MAX_CU_SIZE                         (64)

#define MAX_REF_NUM                         (1)
#define MAX_WIDTH                           3840//lzy (1920)//(4096)
#define MAX_HEIGHT                          2160//lzy (MAX_WIDTH) // don't change, Tiles need rectangle
#define MIN_CU_SIZE                         ( 4)
#define MAX_WIDTH_CU                        (MAX_WIDTH  / MAX_CU_SIZE)
#define MAX_HEIGHT_CU                       (MAX_HEIGHT / MAX_CU_SIZE)
#define MAX_PU_XY                           (MAX_CU_SIZE / MIN_CU_SIZE)
#define MIN_MV_SIZE                         ( 8)
#define MAX_MV_XY                           (MAX_CU_SIZE / MIN_MV_SIZE)
#define MAX_PART_NUM                        (MAX_PU_XY * MAX_PU_XY)
#define MAX_THREADS                         (16)

#define NUM_INTRA_MODE                      (36)
#define NUM_CHROMA_MODE                     ( 5)    // total number of chroma modes
#define CHROMA_MODE_DM                      (NUM_CHROMA_MODE - 1)
#define PLANAR_IDX                          ( 0)
#define VER_IDX                             (26)    // index for intra VERTICAL   mode
#define HOR_IDX                             (10)    // index for intra HORIZONTAL mode
#define DC_IDX                              ( 1)
#define DM_CHROMA_IDX                       (36)    // chroma mode index for derived from luma intra mode
#define SHIFT_INV_1ST                       ( 7)    // Shift after first inverse transform stage
#define SHIFT_INV_2ND                       (12)    // Shift after second inverse transform stage
#define MAX_SAD                             ( 1 << 31 )

#define ASM_NONE                            (0)
#define ASM_MMX                             (1)
#define ASM_SSE2                            (2)
#define ASM_SSE4                            (3)
#define ASM_AVX2                            (4)

#define USE_ASM								ASM_SSE4 //ASM_NONE //
#ifndef USE_ASM
    #define USE_ASM                         ASM_NONE
#endif

#if USE_ASM
#define INTERPOLATE_USE_ASM					USE_ASM
#define INTERPOLATE_CHROMA_USE_ASM	USE_ASM
#define INTRA_PRED_USE_ASM					USE_ASM
#define IDCT_ADD_USE_ASM						USE_ASM
#define SUB_DCT_USE_ASM							USE_ASM
#define DEQUANT_USE_ASM							USE_ASM
#define QUANT_USE_ASM								USE_ASM
#define SAD_USE_ASM									USE_ASM
#define SATD_USE_ASM								USE_ASM
#endif


#define ALWAYS_INLINE                       __inline
////////////////////////////
// x265 end
////////////////////////////

////////////////////////////
// JCT-VC start
////////////////////////////
#define QUANT_IQUANT_SHIFT              20 // Q(QP%6) * IQ(QP%6) = 2^20
#define QUANT_SHIFT                     14 // Q(4) = 2^14
#define SCALE_BITS                      15 // Inherited from TMuC, pressumably for fractional bit estimates in RDOQ
#define MAX_TR_DYNAMIC_RANGE            15 // Maximum transform dynamic range (excluding sign bit)
#define COEF_REMAIN_BIN_REDUCTION        3 ///< indicates the level at which the VLC 
                                           ///< transitions from Golomb-Rice to TU+EG(k)
// AMVP: advanced motion vector prediction
#define AMVP_MAX_NUM_CANDS               2 ///< max number of final candidates
// MERGE
#define MRG_MAX_NUM_CANDS                5
#define MRG_CANDS_START                 (AMVP_MAX_NUM_CANDS)
// Interpolate
#define NTAPS_LUMA                       8 ///< Number of taps for luma
#define NTAPS_CHROMA                     4 ///< Number of taps for chroma

////////////////////////////
// JCT-VC end
////////////////////////////

////////////////////////////
// JCT-VC H start
////////////////////////////
////////////////////////////
// JCT-VC H end
////////////////////////////

////////////////////////////
// JCT-VC G start
////////////////////////////
#define MRG_MAX_NUM_CANDS               5  ///< MERGE
#define MLS_GRP_NUM                    64  ///< G644: Max number of coefficient groups, max(16, 64)
#define MLS_CG_SIZE                     4  ///< G644: Coefficient group size of 4x4
#define SCAN_SET_SIZE                  16
#define LOG2_SCAN_SET_SIZE              4
#define C1FLAG_NUMBER                   8  ///< maximum number of largerThan1 flag coded in one chunk :  16 in HM5
#define C2FLAG_NUMBER                   1  ///< maximum number of largerThan2 flag coded in one chunk:  16 in HM5
////////////////////////////
// JCT-VC G end
////////////////////////////


/////////////////////////////////
// AHG SLICES defines section start
/////////////////////////////////

/////////////////////////////////
// AHG SLICES defines section end
/////////////////////////////////
#define DEPENDENT_SLICES                    1 ///< I0229:
#define BYTE_ALIGNMENT                      1 ///< I0330: Add byte_alignment() procedure to end of slice header
#define RESTRICT_INTRA_BOUNDARY_SMOOTHING   1 ///< K0380, K0186 
#define LINEBUF_CLEANUP                     1 ///< K0101
#define REMOVE_ENTROPY_SLICES               1 ///< K0288:
#define MAX_VPS_NUM_HRD_PARAMETERS                1
#define MAX_VPS_NUM_HRD_PARAMETERS_ALLOWED_PLUS1  1024
#define MAX_VPS_NUH_RESERVED_ZERO_LAYER_ID_PLUS1  1
#define FIX712                              1 ///< Fix for issue #712: CABAC init tables
#define MOVE_VPS_TEMPORAL_ID_NESTING_FLAG           1  ///< K0137: Move vps_temporal_id_nesting_flag

typedef unsigned char       UInt8;
typedef   signed char        Int8;
typedef unsigned short      UInt16;
typedef          short       Int16;

typedef UInt8 pixel;//x64 modify

#ifndef ARCH_X64
typedef unsigned int        UInt32;
typedef          int         Int32;
#else
typedef unsigned long       UInt32;
typedef          long        Int32;
#endif

#ifdef _MSC_VER
typedef unsigned __int64    UInt64;
typedef          __int64     Int64;
#else
typedef unsigned long long  UInt64;
typedef          long long   Int64;
#endif

// Fast integer depends on platform
typedef Int32    Int;
typedef UInt32  UInt;

typedef const  Int8      CInt8;
typedef const UInt8     CUInt8;
typedef const  Int16     CInt16;
typedef const UInt16    CUInt16;
typedef const  Int32     CInt32;
typedef const UInt32    CUInt32;
typedef const  Int64     CInt64;
typedef const UInt64    CUInt64;
typedef const  Int       CInt;
typedef const UInt      CUInt;

//#define FALSE   (0) //x64 modify, 
//#define TRUE    (1) //x64 mofify

#endif /* __CONFIG_H__ */
