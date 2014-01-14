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

#include "x265.h"
#include <math.h>
#include "rateCtrl.h"

const Int		 g_RCInvalidQPValue = -999;
const Int		 g_RCSmoothWindowSize = 40;
const Int		 g_RCMaxPicListSize = 32;
const DOUBLE g_RCWeightPicTargetBitInGOP    = 0.9;
const DOUBLE g_RCWeightPicRargetBitInBuffer = 1.0 - g_RCWeightPicTargetBitInGOP;

static DOUBLE Clip3Double( DOUBLE minVal, DOUBLE maxVal, DOUBLE a )
{
	if ( a < minVal )
		a = minVal;
	else if ( a > maxVal )
		a = maxVal;
	return a;
}

DOUBLE xRCPicEstPicLambda( X265_t* h )
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	xRCGOP*   rcGOP     = &rc->rcGOP;
	xRCPic*   rcPic     = &rc->rcPic;
	
	Int    frameLevel    = (h->eSliceType==SLICE_I? 0:1);
	DOUBLE alpha         = rcSeq->m_picPara[frameLevel].m_alpha;
	DOUBLE beta          = rcSeq->m_picPara[frameLevel].m_beta;
	DOUBLE bpp           = (DOUBLE)rcPic->m_targetBits/(rcSeq->m_numberOfPixel);
	DOUBLE estLambda     = alpha * pow( bpp, beta );
	DOUBLE lastLevelLambda = -1.0;
	DOUBLE lastPicLambda   = -1.0;
	DOUBLE lastValidLambda = -1.0;

	lastLevelLambda = rcGOP->m_picLastLevelLambda[frameLevel];
	lastPicLambda   = rcGOP->m_picLastPicLambda;
	if ( lastPicLambda > 0.0 ){
		lastValidLambda = lastPicLambda;
	}

	if ( lastLevelLambda > 0.0 ){
		lastLevelLambda = Clip3Double( 0.1, 10000.0, lastLevelLambda );
		estLambda = Clip3Double( lastLevelLambda * pow( 2.0, -3.0/3.0 ), lastLevelLambda * pow( 2.0, 3.0/3.0 ), estLambda );
	}

	if ( lastPicLambda > 0.0 ){
		lastPicLambda = Clip3Double( 0.1, 2000.0, lastPicLambda );
		estLambda = Clip3Double( lastPicLambda * pow( 2.0, -10.0/3.0 ), lastPicLambda * pow( 2.0, 10.0/3.0 ), estLambda );
	}
	else if ( lastValidLambda > 0.0 ){
		lastValidLambda = Clip3Double( 0.1, 2000.0, lastValidLambda );
		estLambda = Clip3Double( lastValidLambda * pow(2.0, -10.0/3.0), lastValidLambda * pow(2.0, 10.0/3.0), estLambda );
	}
	else{
		estLambda = Clip3Double( 0.1, 10000.0, estLambda );
	}

	if ( estLambda < 0.1 ){
		estLambda = 0.1;
	}

	rcPic->m_estPicLambda = estLambda;

	//update lambda
	rcGOP->m_picLastLevelLambda[frameLevel] = estLambda;
	rcGOP->m_picLastPicLambda = estLambda;

	return estLambda;
}

Int  xRCPicEstPicQP( X265_t* h, DOUBLE lambda )
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	xRCGOP*   rcGOP     = &rc->rcGOP;
	xRCPic*   rcPic     = &rc->rcPic;
	
	Int QP = Int( 4.2005 * log( lambda ) + 13.7122 + 0.5 ); 

	Int lastLevelQP = g_RCInvalidQPValue;
	Int lastPicQP   = g_RCInvalidQPValue;
	Int lastValidQP = g_RCInvalidQPValue;

	Int frameLevel = (h->eSliceType==SLICE_I? 0:1);
	lastLevelQP = rcGOP->m_picLastLevelQP[frameLevel];
	lastPicQP   = rcGOP->m_picLastPicQP;
	if ( lastPicQP > 0.0 ){
		lastValidQP = lastPicQP;
	}

	if ( lastLevelQP > g_RCInvalidQPValue ){
		QP = Clip3( lastLevelQP - 3, lastLevelQP + 3, QP );
	}

	if( lastPicQP > g_RCInvalidQPValue ){
		QP = Clip3( lastPicQP - 10, lastPicQP + 10, QP );
	}
	else if( lastValidQP > g_RCInvalidQPValue ){
		QP = Clip3( lastValidQP - 10, lastValidQP + 10, QP );
	}

	//update QP
	rcGOP->m_picLastLevelQP[frameLevel] = QP;
	rcGOP->m_picLastPicQP = QP;

	return QP;
}

Int  xRCPicEstPicTargetBits(X265_t* h)
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	xRCGOP*   rcGOP     = &rc->rcGOP;
	
	Int currPicPosition   = rcGOP->m_picLeft;
	Int targetBits        = Int( rcGOP->m_bitsLeft / currPicPosition );

	if ( targetBits < 100 ){
		targetBits = 100;   // at least allocate 100 bits for one picture
	}

	if ( rcSeq->m_framesLeft > 16 ){
		targetBits = Int( g_RCWeightPicRargetBitInBuffer * targetBits + g_RCWeightPicTargetBitInGOP * rcGOP->m_avgTargetBitsPerPicInGOP);
	}

	return targetBits;
}

Int  xRCPicEstPicHeaderBits(X265_t* h)
{
	RateCtrl* rc          = &(h->rc);	
	xRCGOP*   rcGOP       = &rc->rcGOP;
	Int numPreviousPics   = 0;
	Int totalPreviousBits = 0;

	while (numPreviousPics<MAX_GOP_SIZE && rcGOP->m_picHeaderBits[numPreviousPics]!=0){
		totalPreviousBits += rcGOP->m_picHeaderBits[numPreviousPics++];
	}

	Int estHeaderBits = 0;
	if ( numPreviousPics > 0 ){
		estHeaderBits = totalPreviousBits / numPreviousPics;
	}

	return estHeaderBits;
}

void xRCPicUpdateAfterPic(X265_t* h, Int actualHeaderBits, Int actualTotalBits )
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	xRCGOP*   rcGOP     = &rc->rcGOP;
	xRCPic*   rcPic     = &rc->rcPic;
	
	DOUBLE avgQP     = h->iQP;
	DOUBLE avgLambda = rcPic->m_picLambda;

	rcPic->m_picActualHeaderBits = actualHeaderBits;
	rcPic->m_picActualBits       = actualTotalBits;
	if ( avgQP > 0.0 ){
		rcPic->m_picQP             = Int( avgQP + 0.5 );
	}
	else{
		rcPic->m_picQP             = g_RCInvalidQPValue;
	}
	rcPic->m_picLambda           = avgLambda;
	rcPic->m_totalMAD = 0;//total MAD

	Int frameLevel = (h->eSliceType==SLICE_I? 0:1);
	DOUBLE alpha         = rcSeq->m_picPara[frameLevel].m_alpha;
	DOUBLE beta          = rcSeq->m_picPara[frameLevel].m_beta;

	// update parameters
	DOUBLE picActualBpp  = ( DOUBLE )actualTotalBits/rcSeq->m_numberOfPixel;
	DOUBLE calLambda     = alpha * pow( picActualBpp, beta );
	DOUBLE inputLambda   = rcPic->m_picLambda;

	if ( inputLambda < 0.01 || calLambda < 0.01 || picActualBpp < 0.0001 ){
		alpha *= ( 1.0 - rcSeq->m_alphaUpdate / 2.0 );
		beta  *= ( 1.0 - rcSeq->m_betaUpdate  / 2.0 );

		alpha = Clip3Double( 0.05, 20.0, alpha );
		beta  = Clip3Double( -3.0, -0.1, beta  );
		
		rcSeq->m_picPara[frameLevel].m_alpha = alpha;
		rcSeq->m_picPara[frameLevel].m_beta  = beta;

		return;
	}

	calLambda = Clip3Double( inputLambda / 10.0, inputLambda * 10.0, calLambda );
	alpha += rcSeq->m_alphaUpdate * ( log( inputLambda ) - log( calLambda ) ) * alpha;
	double lnbpp = log( picActualBpp );
	lnbpp = Clip3Double( -5.0, 1.0, lnbpp );
	beta  += rcSeq->m_betaUpdate * ( log( inputLambda ) - log( calLambda ) ) * lnbpp;

	alpha = Clip3Double( 0.05, 20.0, alpha );
	beta  = Clip3Double( -3.0, -0.1, beta  );

	rcSeq->m_picPara[frameLevel].m_alpha = alpha;
	rcSeq->m_picPara[frameLevel].m_beta  = beta;
}

void xInitRCPic(X265_t* h)
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	xRCGOP*   rcGOP     = &rc->rcGOP;
	xRCPic*   rcPic     = &rc->rcPic;
	
	Int targetBits    = xRCPicEstPicTargetBits( h );
	Int estHeaderBits = xRCPicEstPicHeaderBits( h );

	if ( targetBits < estHeaderBits + 100 ){
		targetBits = estHeaderBits + 100;   // at least allocate 100 bits for picture data
	}

	rcPic->m_estPicLambda     = 100.0;
	rcPic->m_targetBits       = targetBits;
	rcPic->m_estHeaderBits    = estHeaderBits;
	rcPic->m_estBits          = targetBits;
	rcPic->m_bitsLeft         = targetBits - estHeaderBits;

	rcPic->m_picActualHeaderBits = 0;
	rcPic->m_totalMAD            = 0.0;
	rcPic->m_picActualBits       = 0;
	rcPic->m_picQP               = 0;
	rcPic->m_picLambda           = 0.0;

	//rcPic->m_lastPicture = //NULL;
}

void xRCGOPUpdateAfterPic(X265_t* h, Int bitsCost )
{
	RateCtrl* rc        = &(h->rc);	
	xRCGOP*   rcGOP     = &rc->rcGOP;
	
	rcGOP->m_bitsLeft -= bitsCost;
	rcGOP->m_picLeft--;
}

void xInitRCGOP(X265_t* h, Int numPic )
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	xRCGOP*   rcGOP     = &rc->rcGOP;
	xRCPic*   rcPic     = &rc->rcPic;

	//Estimate GOP TargetBits
	Int64 framesLeft				  = rcSeq->m_framesLeft;
	Int64 realInfluPic				= MIN( g_RCSmoothWindowSize, framesLeft );
	Int avgTargetBitsPerPic = (Int)( rcSeq->m_targetBits / rcSeq->m_totalFrames );
	Int curTargetBitsPerPic = (Int)( ( rcSeq->m_bitsLeft - avgTargetBitsPerPic * (framesLeft - realInfluPic) ) / realInfluPic );
	Int targetBits = curTargetBitsPerPic * numPic;//target bits for this GOP
	if ( targetBits < 200 ){
		targetBits = 200;   // at least allocate 200 bits for one GOP
	}

	rcGOP->m_avgTargetBitsPerPicInGOP = curTargetBitsPerPic;
	rcGOP->m_numPic						 = numPic;
	rcGOP->m_targetBits				 = targetBits;
	rcGOP->m_picLeft					 = numPic;
	rcGOP->m_bitsLeft					 = targetBits;

	rcGOP->m_picLastLevelLambda[0] = -1.0;
	rcGOP->m_picLastLevelLambda[1] = -1.0;
	rcGOP->m_picLastPicLambda      = -1.0;

	rcGOP->m_picLastLevelQP[0] = g_RCInvalidQPValue;
	rcGOP->m_picLastLevelQP[1] = g_RCInvalidQPValue;
	rcGOP->m_picLastPicQP      = g_RCInvalidQPValue;

	////init picture level rate control
	//Int sliceQP              = h->iQP;
	//Int frameLevel = ( (h->eSliceType)==SLICE_I? 0: 1);//0: I frame; 1: ref; 2: non-ref
	for (Int i=0; i<MAX_GOP_SIZE; i++){
		rcGOP->m_picHeaderBits[i] = 0;
	}
	//xInitRCPic( h );
	//
	//DOUBLE lambda = 0.0;
	//if ( frameLevel == 0 ){   // intra case, but use the model
	//	if ( h->nIntraPeriod != 1 ){   // do not refine allocated bits for all intra case
	//		Int64 bits = rcSeq->m_bitsLeft / rcSeq->m_framesLeft;
	//		
	//		//get refine bits for intra
	//		DOUBLE bpp = ( (DOUBLE)bits ) / h->usHeight/h->usWidth;
	//		bits *= ( bpp>0.2? 5: (bpp>0.1? 7:10) );
	//		if ( bits < 200 ){
	//			bits = 200;
	//		}

	//		rcPic->m_targetBits =  Int64(bits);
	//	}

	//	lambda  = xRCPicEstPicLambda( h );
	//	sliceQP = xRCPicEstPicQP( h, lambda );
	//}
	//else{    // normal case
	//	lambda  = xRCPicEstPicLambda( h );
	//	sliceQP = xRCPicEstPicQP( h, lambda );
	//}

	//sliceQP = Clip3( 0, MAX_QP, sliceQP );//only appropriate for 8bit YUV 

	////xRCResetSliceQP( sliceQP, lambda );
	//h->iQP = sliceQP;
	//rcPic->m_picQP = sliceQP;
	//rcPic->m_picLambda = lambda;
}

void xRCSeqUpdateAfterPic (X265_t* h, Int bits )
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	
	rcSeq->m_bitsLeft -= bits;
	rcSeq->m_framesLeft--;
}

void xInitRCSeq(X265_t* h, UInt64 totalFrames, UInt8 frameRate, UInt8 GOPSize, Int targetBitsPerSec)
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	UInt16    picWdith  = h->iWidth;
	UInt16    picHeight = h->iHeight;

	//step1. init RCSeq
	rcSeq->m_totalFrames         = totalFrames;
	rcSeq->m_targetRate          = targetBitsPerSec;
	rcSeq->m_frameRate           = frameRate;
	rcSeq->m_GOPSize             = GOPSize;
	rcSeq->m_picWidth            = picWdith;
	rcSeq->m_picHeight           = picHeight;
	rcSeq->m_numberOfLevel       = 2;					//intra picture; P (reference) pictures 
	rcSeq->m_numberOfLCU				 = h->iCUWidth * h->iCUHeight;
	rcSeq->m_framesLeft					 = totalFrames;

	Int		 numberOfPixel		 = picWdith * picHeight;
	Int64  targetBits				 = ((Int64 )totalFrames*targetBitsPerSec/frameRate);    //target bits for whole sequence
	DOUBLE seqTargetBpp      = ((DOUBLE)targetBitsPerSec /frameRate /numberOfPixel);//bpp for whole sequence
	rcSeq->m_numberOfPixel   = numberOfPixel;
	rcSeq->m_targetBits      = targetBits;
	rcSeq->m_seqTargetBpp    = seqTargetBpp;

	if ( seqTargetBpp < 0.03 ){
		rcSeq->m_alphaUpdate = 0.01;
		rcSeq->m_betaUpdate  = 0.005;
	}
	else if ( seqTargetBpp < 0.08 ){
		rcSeq->m_alphaUpdate = 0.05;
		rcSeq->m_betaUpdate  = 0.025;
	}
	else{
		rcSeq->m_alphaUpdate = 0.1;
		rcSeq->m_betaUpdate  = 0.05;
	}

	rcSeq->m_averageBits   = (Int)(targetBits / totalFrames);//average bits for one picture
	rcSeq->m_bitsLeft			 = targetBits;

	//step2. init bitsRatio[]: always  10 in low delay

	//step3. init GOPID2Level[]: always  1  in low delay

	//step4. initPicPrama 
	rcSeq->m_picPara[0].m_alpha = 3.2003;
	rcSeq->m_picPara[0].m_beta  = -1.367;
	rcSeq->m_picPara[1].m_alpha = 3.2003;
	rcSeq->m_picPara[1].m_beta  = -1.367;
}

//For DLL
void xInitRCSequence(X265_t* h,  Int targetBitsPerSec)
{
	RateCtrl* rc        = &(h->rc);	
	xRCSeq*   rcSeq     = &rc->rcSeq;
	UInt16    picWdith  = h->iWidth;
	UInt16    picHeight = h->iHeight;
	UInt64 totalFrames = (UInt64)1892160000*100;//1892160000: one year under 60fps;//946080000;//
	UInt8 frameRate = h->nFrameRate;
	UInt8 GOPSize  = h->nGOPSize;

	//step1. init RCSeq
	rcSeq->m_totalFrames         = totalFrames;
	rcSeq->m_targetRate          = targetBitsPerSec;
	rcSeq->m_frameRate           = frameRate;
	rcSeq->m_GOPSize             = GOPSize;
	rcSeq->m_picWidth            = picWdith;
	rcSeq->m_picHeight           = picHeight;
	rcSeq->m_numberOfLevel       = 2;					//intra picture; P (reference) pictures 
	rcSeq->m_numberOfLCU				 = h->iCUWidth * h->iCUHeight;
	rcSeq->m_framesLeft					 = totalFrames;

	Int		 numberOfPixel		 = picWdith * picHeight;
	Int64  targetBits				 = ((Int64 )totalFrames*targetBitsPerSec/frameRate);    //target bits for whole sequence
	DOUBLE seqTargetBpp      = ((DOUBLE)targetBitsPerSec /frameRate /numberOfPixel);//bpp for whole sequence
	rcSeq->m_numberOfPixel   = numberOfPixel;
	rcSeq->m_targetBits      = targetBits;
	rcSeq->m_seqTargetBpp    = seqTargetBpp;

	if ( seqTargetBpp < 0.03 ){
		rcSeq->m_alphaUpdate = 0.01;
		rcSeq->m_betaUpdate  = 0.005;
	}
	else if ( seqTargetBpp < 0.08 ){
		rcSeq->m_alphaUpdate = 0.05;
		rcSeq->m_betaUpdate  = 0.025;
	}
	else{
		rcSeq->m_alphaUpdate = 0.1;
		rcSeq->m_betaUpdate  = 0.05;
	}

	rcSeq->m_averageBits   = (Int)(targetBits / totalFrames);//average bits for one picture
	rcSeq->m_bitsLeft			 = targetBits;

	//step2. init bitsRatio[]: always  10 in low delay

	//step3. init GOPID2Level[]: always  1  in low delay

	//step4. initPicPrama 
	rcSeq->m_picPara[0].m_alpha = 3.2003;
	rcSeq->m_picPara[0].m_beta  = -1.367;
	rcSeq->m_picPara[1].m_alpha = 3.2003;
	rcSeq->m_picPara[1].m_beta  = -1.367;
}

