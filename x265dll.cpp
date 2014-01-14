/*****************************************************************************
 * x265dll.cpp: x265 encoder APIs
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Yanan Zhao, Xiangwen Wang, Zhengyi Luo, Li Song
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

#define X265CODEC_API_DLL_ __declspec(dllexport)

#include ".\inc\x265.h"
#include ".\inc\version.h"
#include <time.h>
#include "x265dll.h"
#include ".\inc\md5.h"

#define MAX_OUTBUF_SIZE     (1 * 1024 * 1024 *4)

extern UInt8 buf[MAX_WIDTH * MAX_HEIGHT * 3 / 2];
//extern UInt8 buf1[MAX_WIDTH * MAX_HEIGHT * 3 / 2];//for frame aligning
extern UInt8 pucOutBuf0[ MAX_OUTBUF_SIZE];
extern UInt8 pucOutBuf1[ MAX_OUTBUF_SIZE];

extern UInt8 iLogoY[LOGO_WIDTH*LOGO_HEIGHT];
extern UInt8 iLogoU[LOGO_WIDTH/2 * LOGO_HEIGHT/2];
extern UInt8 iLogoV[LOGO_WIDTH/2 * LOGO_HEIGHT/2];

extern Int32 xEncodeFrameAPI( X265_t *h, xFrame *pFrame, UInt8 *pucOutBuf, UInt32 uiBufSize, int* nal_num, int* nal_len );

int x265_encoder_init(void **ppParam, int format, char* cfg )
{
	(void )format; //avoid warning, this parameter is not used currently

	X265_t *ph = (X265_t *)malloc(sizeof(X265_t));
	if (ph == NULL){
		printf("x265_t malloc failed!\n") ;
		exit(0);
	}
	memset(ph, 0, sizeof(X265_t));

	UInt32  bWriteRecFlag = FALSE;
	UInt32  bWriteVisCUFlag = FALSE;
	UInt32  bStrongIntraSmoothing = FALSE;
	UInt32  bTiles = FALSE;
	UInt32  nThreads = 1;
	Int bUseSceneChange = SCENE_CHANGE_DETECTION;


	//params for rate control
	UInt8   nFrameRate = 25;
	UInt8   nGOPSize   = 4;
	UInt8   bUseRateCtrl = FALSE;
	Int     targetKbps = 1000;

	xDefaultParams( ph );

	ph->ucMaxCUWidth      = 64; 
	ph->ucMaxCUDepth      =  4;
	ph->nThreads          = nThreads;
	ph->bWriteRecFlag     = bWriteRecFlag;
	ph->bWriteVisCUFlag   = bWriteVisCUFlag;
	ph->bStrongIntraSmoothing = bStrongIntraSmoothing;
	ph->eSliceType        = SLICE_I;
	ph->iQP               = 32;
	ph->bTiles            = bTiles;

	ph->nFrameRate	 = nFrameRate;
	ph->nGOPSize     = nGOPSize;
	ph->bUseRateCtrl = bUseRateCtrl;
	ph->targetKbps   = targetKbps;

	ph->pOutBuf0        = (UInt8 *)malloc(MAX_OUTBUF_SIZE*sizeof(UInt8));//pucOutBuf0;
	ph->pOutBuf1        = (UInt8 *)malloc(MAX_OUTBUF_SIZE*sizeof(UInt8));//pucOutBuf1;

	char  cmd[30] = "null";
	int     val = -1;

	FILE *fpi = fopen(cfg, "r");
	if ( fpi==NULL ) {
		printf("Failed to open .cfg file!\n");
		exit(0);
	}

	while (!feof(fpi)) {
		fscanf(fpi, "%s %d\n", cmd, &val);
		if ( !strcmp(cmd, "ImageWidth") ) {
			ph->iWidth = val;
		}
		else if ( !strcmp(cmd, "ImageHeight") ) {
			ph->iHeight = val;
		}
		else if ( !strcmp(cmd, "QP") ) {
			ph->iQP = val;
		}
		else if ( !strcmp(cmd, "IntraPeriod") ) {
			ph->nIntraPeriod = val;
		}
		else if ( !strcmp(cmd, "UseRateControl") ) {
			ph->bUseRateCtrl = val;
		}
		else if ( !strcmp(cmd, "TargetKbps") ) {
			ph->targetKbps = val;
		}
		else if ( !strcmp(cmd, "FrameRate") ) {
			ph->nFrameRate = val;
		}
		else if ( !strcmp(cmd, "StrongIntraSmoothing") ) {
			ph->bStrongIntraSmoothing = val;
		}
		else if ( !strcmp(cmd, "NumberOfThreads") ) {
			ph->nThreads = val;
		}
		else if ( !strcmp(cmd, "SceneChangeDetection") ) {
			ph->bUseSceneChangeDetection = val;
		}
		else {
			printf("Unknown option [%s]\n", cmd);
		}
	}

	CInt32   iWidth             = ph->iWidth;
	CInt32   iHeight            = ph->iHeight;
	Int iCUWidth  = (ph->iWidth +MAX_CU_SIZE-1)/MAX_CU_SIZE;
	Int iCUHeight = (ph->iHeight+MAX_CU_SIZE-1)/MAX_CU_SIZE;
	ph->iCUWidth = iCUWidth;
	ph->iCUHeight = iCUHeight;

	Int iWidthAligned  = iCUWidth*MAX_CU_SIZE;
	Int iHeightAligned = iCUHeight*MAX_CU_SIZE;
	Int iSizePaddedX   = iWidthAligned - iWidth;
	Int iSizePaddedY   = iHeightAligned - iHeight;
	bool bFrameAlignment = (iSizePaddedX>0 || iSizePaddedY>0);
	ph->bFrameAlignment = (Int)bFrameAlignment;

	ph->iWidthOrg        = iWidth;
	ph->iHeightOrg       = iHeight;
	ph->iWidth           = (bFrameAlignment? iWidthAligned  : iWidth);
	ph->iHeight          = (bFrameAlignment? iHeightAligned : iHeight);

	ph->iSceneChangeIntervalCurr = 0;

	if(ph->bUseRateCtrl){
		xInitRCSequence(ph, ph->targetKbps*1000);
	}

	xCheckParams( ph );
	xEncInit( ph );

	fclose(fpi);
	*ppParam = ph;

	return 0;
}

int x265_encode(void *pParam, unsigned char* p_data, int* nal_num, int* nal_len, unsigned char* p_yuv )
{
	X265_t *ph = (X265_t *)pParam;
	(*nal_num) = 0 ;

	CInt32   iWidthOrg          = ph->iWidthOrg; //before aligning
	CInt32   iHeightOrg         = ph->iHeightOrg;//before aligning
	CInt32   iWidth             = ph->iWidth;    //Aligned width
	CInt32   iHeight            = ph->iHeight;   //Aligend height

	CInt32 iYSizeOrg   = iWidthOrg*iHeightOrg;
	CInt32 iYSize      = iWidth*iHeight;
	xFrame frame;
	xFrame frameAligned;
	
	frame.pucY = p_yuv;
	frame.pucU = p_yuv + iYSizeOrg;
	frame.pucV = p_yuv + iYSizeOrg * 5 / 4;
	frame.iStrideY = ph->iWidthOrg;
	frame.iStrideC = ph->iWidthOrg >> 1;

	UInt8 *buf1 = ph->buf1;
	frameAligned.pucY = buf1;
	frameAligned.pucU = buf1 + iYSize;
	frameAligned.pucV = buf1 + iYSize * 5 / 4;
	frameAligned.iStrideY = iWidth;
	frameAligned.iStrideC = iWidth >> 1;

	if (ph->bFrameAlignment) {
		xFrameAligning(&frameAligned, iWidth, iHeight, &frame, iWidthOrg, iHeightOrg);
	}

	Int64    iFrameNumCurr      = ph->uiFrames;
	Int64    iFramesToBeEncoded = ph->nFramesToBeEncoded;
	Int      bSceneChanged = 0;

	if ( ph->bUseSceneChangeDetection && ( (iFrameNumCurr+1)%ph->nIntraPeriod!=0 ) && (ph->iSceneChangeIntervalCurr>SCENE_CHANGE_INTERVAL) )	{
		bSceneChanged = sceneChangeDetection(ph, (frameAligned.pucY));
	}

	/* Add logo "ZenHEVC /n from SJTU" */
	if (iWidth==1920 && iHeight==1088) { //only for 1080P now
		for (Int j=0; j<LOGO_HEIGHT; j++)	{
			for (Int i=0; i<LOGO_WIDTH; i++)	{
				Int iOffset = (j+928)*iWidth+(i+1664);
				UInt8 iY = frameAligned.pucY[iOffset];
				frameAligned.pucY[iOffset] = Clip3(0, 255, iLogoY[j*LOGO_WIDTH+i]+iY);
			}
		}

		//for (Int j=0; j<LOGO_HEIGHT/2; j++)	{
		//	for (Int i=0; i<LOGO_WIDTH/2; i++)	{
		//		Int iOffset = (j+928/2)*iWidth/2+(i+1664/2);
		//		UInt8 iU = frameAligned.pucU[iOffset];
		//		frameAligned.pucU[iOffset] = iU;//Clip3(0, 255, iLogoU[j*LOGO_WIDTH/2+i]+iU);

		//		UInt8 iV = frameAligned.pucV[iOffset];
		//		frameAligned.pucV[iOffset] = iV;//Clip3(0, 255, iLogoV[j*LOGO_WIDTH/2+i]+iV);
		//	}
		//}
	}
	

	if ( ((iFrameNumCurr % ph->nIntraPeriod == 0 ) || bSceneChanged) && (ph->iPoc!=0) ){ // make sure two continuous frames not both be IDR frames
		/*Last POC be 0 means previous frame is an IDR, so current frame should not be an IDR*/
		ph->eSliceType = SLICE_I;
		//if (bSceneChanged) {
		//	printf("Scene change detected, frame number %d, interval %d \n", iFrameNumCurr, ph->iSceneChangeIntervalCurr);
		//}
		ph->iSceneChangeIntervalCurr = 0; //reset current scene change interval
	}
	else{
		ph->eSliceType = SLICE_P;
		ph->iSceneChangeIntervalCurr++; //increase scene change interval
	}

	if (ph->bUseRateCtrl)	{
		Int iGopSize = ph->nGOPSize;
		if (iFrameNumCurr==0 || iFrameNumCurr % iGopSize==1){
			Int numPic = (iFrameNumCurr==0? 1: (iFramesToBeEncoded-iFrameNumCurr>iGopSize? iGopSize:iFramesToBeEncoded-iFrameNumCurr));
			xInitRCGOP(ph, numPic);
		}

		Int sliceQP              = ph->iQP;
		Int frameLevel = ( (ph->eSliceType)==SLICE_I? 0: 1);//0: I frame; 1: ref; 2: non-ref
		xInitRCPic( ph );

		RateCtrl* rc        = &(ph->rc);	
		xRCSeq*   rcSeq     = &rc->rcSeq;
		xRCPic*   rcPic     = &rc->rcPic;
		double lambda = 0.0;
		if ( frameLevel == 0 ){   
			if ( ph->nIntraPeriod != 1 ){ 
				Int64 bits = rcSeq->m_bitsLeft / rcSeq->m_framesLeft;
				double bpp = ( (double)bits ) / iHeight/iWidth;
				bits *= ( bpp>0.2? 5: (bpp>0.1? 7:10) );
				if ( bits < 200 ){
					bits = 200;
				}

				rcPic->m_targetBits =  Int64(bits);
			}

			lambda  = xRCPicEstPicLambda( ph );
			sliceQP = xRCPicEstPicQP( ph, lambda );
		}
		else{    // normal case
			lambda  = xRCPicEstPicLambda( ph );
			sliceQP = xRCPicEstPicQP( ph, lambda );
		}

		if (iFrameNumCurr==0){
			//sliceQP = Clip3( ph->iQP-1, ph->iQP+1, sliceQP );
			sliceQP = Clip3( 30, 40, ph->iQP+5 );
		}
		else{
			sliceQP = Clip3( ph->iQP-3, ph->iQP+3, sliceQP );
		}
		sliceQP = Clip3( 0, MAX_QP, sliceQP );

		ph->iQP = sliceQP;
		rcPic->m_picQP = sliceQP;
		rcPic->m_picLambda = lambda;
	}
	
	if (DEBLOCK){
		memset(ph->qp_y_tab, ph->iQP, ph->iWidth/ph->ucMaxCUDepth * ph->iHeight/ph->ucMaxCUDepth);
	}
	
	if (STATIC_PSNR) {
		printf("Encoding Frame[%5d, %c, %d]", iFrameNumCurr, (ph->eSliceType==SLICE_I? 'I':'P'), ph->iQP);
	}
	Int32 iEncSize = xEncodeFrameAPI( ph, &frameAligned, p_data, MAX_OUTBUF_SIZE, nal_num, nal_len);
	
	return iEncSize;
}

void x265_encoder_free(void *pParam)
{
	X265_t *ph = (X265_t *)pParam;
	free(ph->pOutBuf0);
	free(ph->pOutBuf1);
	xEncFree(ph);
}
