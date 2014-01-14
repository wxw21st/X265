/*****************************************************************************
 * utils.h: Util functions
 *****************************************************************************
 *****************************************************************************
 * Copyright (C) 2012-2015 x265 project
 *
 * Authors: Min Chen <chenm003@163.com>
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

#ifndef __UTILS_H__
#define __UTILS_H__

#if (_MSC_VER > 1400) && !defined(__INTEL_COMPILER)
    #include <intrin.h>
#endif

//extern int xLog2(UInt32);
static int xLog2(UInt32 x)
{
    unsigned long log2Size = 0;
#if (_MSC_VER > 1400) || defined(__INTEL_COMPILER)
    _BitScanReverse(&log2Size, x);
    log2Size++;
#else // ASM_NONE
    while(x > 0) {
        x >>= 1;
        log2Size++;
    }
#endif
    return(log2Size);
}


static Int32 av_clip_c(int a, int amin, int amax)
{
	if      (a < amin) return amin;
	else if (a > amax) return amax;
	else               return a;
}

static Int32 Clip3( Int32 minVal, Int32 maxVal, Int32 a )
{
    if ( a < minVal )
        a = minVal;
    else if ( a > maxVal )
        a = maxVal;
    return a;
}

#define Clip(x)         Clip3( 0, 255, (x))
#define xSHR(x, n)      ( (n)>=32 ? 0 : ((x)>>(n)) )
#define xSHL(x, n)      ( (n)>=32 ? 0 : ((x)<<(n)) )
#define MAX(a, b)       ( (a) > (b) ? (a) : (b) )
#define MIN(a, b)       ( (a) < (b) ? (a) : (b) )
#define MALLOC(n)       malloc(n)
#define FREE(p)         free(p)
#define ASIZE(x)        ( sizeof(x)/sizeof((x)[0]) )

#ifdef _OPENMP
    #ifdef _MSC_VER
        #include <windows.h>
        #define SLEEP(x)    Sleep(x)
    #else
        #include <unistd.h>
        #define SLEEP(x)    usleep((x)*100)
    #endif
#endif

#define BSWAP32(x)          ( ((x)<<24) + (((x)<<8)&0xff0000) + (((x)>>8)&0xff00) + ((x)>>24) )

#if !defined(__INTEL_COMPILER)
#define _bswap(x)           BSWAP32(x)
static UInt64 _bswap64(UInt64 x)
{
    UInt32 x0 = _bswap((UInt32)(x    ));
    UInt32 x1 = _bswap((UInt32)(x>>32));
    return (((UInt64)x0 << 32) | (UInt64)x1);
}
#endif

#endif /* __UTILS_H__ */
