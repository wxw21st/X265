/*****************************************************************************
 * testDll_main.cpp: x265 encoder API testing wrapper
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

#if 0

#include <stdio.h>
#include <Windows.h>

BYTE g_pbBitstreamBuffer[1024 * 1024 * 10] ;
BYTE g_pbYUVBuffer[1920 * 1088 * 3 / 2] ;
int g_iNalNum ;
int g_piNalLength[10] ;

void main()
{
	x265_encoder_init(1920, 1088, 1, "test.cfg") ;

	FILE *fp = fopen("D:\\YUV\\B\\First100_Cactus_1920x1088.yuv", "rb") ;

	int i ;
	int iRet ;
	for (i = 0 ; i < 25 ; i ++)
	{
		iRet = fread(g_pbYUVBuffer, 1, 1920 * 1088 * 3 / 2, fp) ;
		if (iRet != 1920 * 1088 * 3 / 2){
			printf("Read Buffer Failed!\n") ;
			break; ;
		}
		x265_encode(g_pbBitstreamBuffer, &g_iNalNum, g_piNalLength, g_pbYUVBuffer) ;
	}

	fclose(fp) ;

	x265_encoder_free() ;

	printf("Hello, world!\n") ;
	return ;
}

#endif