/*****************************************************************************
 * x265dll.h: Declarations of x265 encoder APIs 
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

#ifndef __X265DLL_H_
#define __X265DLL_H_

#ifdef X265CODEC_API_DLL_
//export all dll functions
#else
#define X265CODEC_API_DLL_ __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C" {
#endif

	X265CODEC_API_DLL_ int x265_encoder_init(void **ppParam, int format, char* cfg );
	X265CODEC_API_DLL_ int x265_encode(void *pParam, unsigned char* p_data, int* nal_num, int* nal_len, unsigned char* p_yuv );
	X265CODEC_API_DLL_ void x265_encoder_free(void *pParam);

#ifdef __cplusplus
}
#endif



#endif // __X265DLL_H