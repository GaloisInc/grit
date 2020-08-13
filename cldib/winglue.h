//
//! \file winglue.h
//!  Required windows stuff for portability's sake
//!  Mostly ripped from FreeImage.h
//! \date 20060725 - 20070403
//! \author cearn
//=== NOTES ===
// * The switch-define is either _MSC_VER or _WIN32, I'm not sure which. 

#ifndef __WINGLUE_H__
#define __WINGLUE_H__


#ifdef _MSC_VER

// --------------------------------------------------------------------
// WINDOWS/Visual C++ platform
// --------------------------------------------------------------------

#include <windows.h>

#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif

// <sys/param.h> functionality
#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <limits.h>

/* These are useful for cross-compiling */ 
#define BIG_ENDIAN      4321
#define LITTLE_ENDIAN   1234
#define BYTE_ORDER      LITTLE_ENDIAN

#define MAXPATHLEN      PATH_MAX

#endif	// _SYS_PARAM_H

// Apparently MSVC2008 is really fussy about these. I just hope there's 
// no compilation problems because of the underscores >_>
#if _MSC_VER >= 1500
	#define strcasecmp	_stricmp
	#define strdup		_strdup
#else
	#define strcasecmp	stricmp
#endif	// _MSC_VER >= 1500

#else	// _MSC_VER


// --------------------------------------------------------------------
// NON-WINDOWS platform
// --------------------------------------------------------------------


// This defines BYTE_ORDER etc. If it doesn't exist, just C&P from 
// above
#include <sys/param.h>	

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// --- BASE TYPES -----------------------------------------------------

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long UINT;

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

typedef DWORD COLORREF;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct _RECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT;

// --- BITMAP STUFF ---------------------------------------------------
 

// --- INLINES --------------------------------------------------------

static inline COLORREF RGB( BYTE red, BYTE green, BYTE blue )
{
	return ( (blue << 16) | ( green << 8 ) | red );
}

static inline BYTE GetRValue( COLORREF color )
{
	return (BYTE)(color & 0xFF);
}

static inline BYTE GetGValue( COLORREF color )
{
	return (BYTE)(( color >> 8 )& 0xFF);
}

static inline BYTE GetBValue( COLORREF color )
{
	return (BYTE)(( color >> 16 ) & 0xFF);
}


#endif	// _MSC_VER


typedef struct tagBITMAPCOREHEADER {
  DWORD bcSize;
  WORD  bcWidth;
  WORD  bcHeight;
  WORD  bcPlanes;
  WORD  bcBitCount;
} BITMAPCOREHEADER, *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef enum
{
  BI_RGB = 0x0000,
  BI_RLE8 = 0x0001,
  BI_RLE4 = 0x0002,
  BI_BITFIELDS = 0x0003,
  BI_JPEG = 0x0004,
  BI_PNG = 0x0005,
  BI_CMYK = 0x000B,
  BI_CMYKRLE8 = 0x000C,
  BI_CMYKRLE4 = 0x000D
} Compression;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

typedef struct tagRGBTRIPLE {
  BYTE rgbtBlue;
  BYTE rgbtGreen;
  BYTE rgbtRed;
} RGBTRIPLE;

typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD          bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;


#endif	//__WINGLUE_H__

// EOF
