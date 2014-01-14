/*****************************************************************************
 * rateCtrl.h: 
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
#ifndef __RATE_CONTROL__
#define __RATE_CONTROL__

#include "x265.h"

#ifndef MAX_GOP_SIZE
#define MAX_GOP_SIZE 12
#endif

#ifndef MAX_QP
#define MAX_QP 51
#endif
#ifndef MIN_QP
#define MIN_QP 0
#endif

#ifndef DOUBLE
#define DOUBLE double
#endif

//USE RATE_CONTROL_LAMBDA_DOMAIN
struct xRCParameter
{
	DOUBLE m_alpha;
	DOUBLE m_beta;
};

struct xRCSeq
{
	Int64 m_totalFrames;
	Int m_targetRate;
	Int m_frameRate; 
	Int m_GOPSize;
	Int m_picWidth;
	Int m_picHeight;
	Int m_numberOfLevel;
	Int m_averageBits;

	Int m_numberOfPixel;
	Int64 m_targetBits;
	Int m_numberOfLCU;
	xRCParameter m_picPara[2];

	Int64 m_framesLeft;
	Int64 m_bitsLeft;
	DOUBLE m_seqTargetBpp;
	DOUBLE m_alphaUpdate;
	DOUBLE m_betaUpdate;
};

struct xRCGOP
{
	//xRCSeq* m_rcSeq;
	Int m_avgTargetBitsPerPicInGOP;//m_picTargetBitInGOP;//simplify to avgTargetBitsPerPic
	Int m_numPic;
	Int m_targetBits;
	Int m_picLeft;
	Int m_bitsLeft;

	//pic info
	DOUBLE m_picLastLevelLambda[2]; //frameLevel 0 & 1
	DOUBLE m_picLastPicLambda;
	Int		 m_picLastLevelQP[2];
	Int    m_picLastPicQP;

	//Int m_picFrameLevel[MAX_GOP_SIZE];
	Int m_picHeaderBits[MAX_GOP_SIZE];
	//Int m_picTotalMAD  [MAX_GOP_SIZE];
	//Int m_picActualBits[MAX_GOP_SIZE];
	//Int m_picQP        [MAX_GOP_SIZE];
	//Int m_picLambda    [MAX_GOP_SIZE];
};

struct xRCPic
{
	Int64 m_targetBits;
	Int m_estHeaderBits;
	Int m_estBits;//estimatedBits for this pic
	Int m_estPicQP;
	DOUBLE m_estPicLambda;

	Int m_bitsLeft;

	Int     m_picActualHeaderBits;    // only SH and potential APS
	DOUBLE  m_totalMAD;
	Int     m_picActualBits;          // the whole picture, including header
	Int     m_picQP;                  // in integer form
	DOUBLE  m_picLambda;
};

struct RateCtrl
{
	xRCSeq rcSeq;
	xRCGOP rcGOP;
	xRCPic rcPic;
	Int     m_RCQP;
};

#endif 
