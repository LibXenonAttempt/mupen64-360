/*
Rice_GX - typedefs.h
Copyright (C) 2003 Rice1964
Copyright (C) 2010, 2011, 2012 sepp256 (Port to Wii/Gamecube/PS3)
Wii64 homepage: http://www.emulatemii.com
email address: sepp256@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#ifdef XENON
#include <xetypes.h>
#endif

#ifdef _BIG_ENDIAN
#define S8 0
#define S16 0
#else
#define S8 3
#define S16 1
#endif

#define uchar  unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define uint64 unsigned long long

typedef unsigned char               uint8;

#ifndef XENON
typedef signed char                 s8; 
typedef int                         s32;
typedef unsigned int                u32;
typedef unsigned char               u8;
#endif

//Note: These should be BE safe because they are matched with COLOR_RGBA()
#define RGBA_GETALPHA(rgb)      ((rgb) >> 24)
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)

typedef XMATRIX Matrix;
typedef void* LPRICETEXTURE ;

typedef struct 
{
    uint32 dwRGBA, dwRGBACopy;
    char x,y,z;         // Direction
    uint8 pad;
} N64Light;


typedef struct
{
    unsigned int    dwFormat:3;
    unsigned int    dwSize:2;
    unsigned int    dwWidth:10;
    uint32          dwAddr;
    uint32          bpl;
} SetImgInfo;

typedef struct 
{
    // Set by RDP_SetTile
    unsigned int dwFormat   :3;     // e.g. RGBA, YUV etc
    unsigned int dwSize     :2;     // e.g 4/8/16/32bpp
    unsigned int dwLine     :9;     // Ummm...
    unsigned int dwPalette  :4;     // 0..15 - a palette index?
    uint32 dwTMem;                  // Texture memory location

    unsigned int bClampS    :1;
    unsigned int bClampT    :1;
    unsigned int bMirrorS   :1;
    unsigned int bMirrorT   :1;

    unsigned int dwMaskS    :4;
    unsigned int dwMaskT    :4;
    unsigned int dwShiftS   :4;
    unsigned int dwShiftT   :4;

    // Set by RDP_SetTileSize
    int sl;     // Upper left S     - 8:3
    int tl;     // Upper Left T     - 8:3
    int sh;     // Lower Right S
    int th;     // Lower Right T

    int   hilite_sl;
    int   hilite_tl;
    int   hilite_sh;
    int   hilite_th;

    float fsl;      // Upper left S     - 8:3
    float ftl;      // Upper Left T     - 8:3
    float fsh;      // Lower Right S
    float fth;      // Lower Right T

    float   fhilite_sl;
    float   fhilite_tl;
    float   fhilite_sh;
    float   fhilite_th;

    uint32 dwDXT;

    uint32 dwPitch;

    uint32 dwWidth;
    uint32 dwHeight;

    float fShiftScaleS;
    float fShiftScaleT;

    uint32   lastTileCmd;
    bool  bSizeIsValid;

    bool bForceWrapS;
    bool bForceWrapT;
    bool bForceClampS;
    bool bForceClampT;

} Tile;


typedef struct
{
    float u;
    float v;
} TexCord;

typedef struct VECTOR2
{
    float x;
    float y;
    VECTOR2( float newx, float newy )   {x=newx; y=newy;}
    VECTOR2()   {x=0; y=0;}
} VECTOR2;

typedef struct
{
    short x;
    short y;
} IVector2;

typedef struct 
{
    short x;
    short y;
    short z;
} IVector3;

typedef struct {
    float x,y,z;
    float rhw;
    union {
        COLOR  dcDiffuse;
        struct {
#ifndef _BIG_ENDIAN
            uint8 b;
            uint8 g;
            uint8 r;
            uint8 a;
#else // !_BIG_ENDIAN - Big Endian fix.
            uint8 a;
            uint8 r;
            uint8 g;
            uint8 b;
#endif // _BIG_ENDIAN
        };
    };
    COLOR  dcSpecular;
    TexCord tcord[2];
} TLITVERTEX, *LPTLITVERTEX;

typedef struct {
    float x,y,z;
    union {
        COLOR  dcDiffuse;
        struct {
#ifndef _BIG_ENDIAN
            uint8 b;
            uint8 g;
            uint8 r;
            uint8 a;
#else // !_BIG_ENDIAN - Big Endian fix.
            uint8 a;
            uint8 r;
            uint8 g;
            uint8 b;
#endif // _BIG_ENDIAN
        };
    };
    COLOR  dcSpecular;
    TexCord tcord[2];
} UTLITVERTEX, *LPUTLITVERTEX;

typedef struct {
    float x,y,z;
    float rhw;
    union {
        COLOR  dcDiffuse;
        struct {
#ifndef _BIG_ENDIAN
            uint8 b;
            uint8 g;
            uint8 r;
            uint8 a;
#else // !_BIG_ENDIAN - Big Endian fix.
            uint8 a;
            uint8 r;
            uint8 g;
            uint8 b;
#endif // _BIG_ENDIAN
        };
    };
    COLOR  dcSpecular;
} LITVERTEX, *LPLITVERTEX;



typedef struct {
    float   x,y,z;
    float   rhw;
    COLOR dcDiffuse;
} FILLRECTVERTEX, *LPFILLRECTVERTEX;

#include "COLOR.h"
#include "IColor.h"

typedef struct
{
    float x,y,z;
    float nx,ny,nz;
    union {
        COLOR  dcDiffuse;
        struct {
#ifndef _BIG_ENDIAN
            uint8 b;
            uint8 g;
            uint8 r;
            uint8 a;
#else // !_BIG_ENDIAN - Big Endian fix.
            uint8 a;
            uint8 r;
            uint8 g;
            uint8 b;
#endif // _BIG_ENDIAN
        };
    };
    float u,v;
}EXTERNAL_VERTEX, *LPSHADERVERTEX;


typedef struct
{
    union {
        struct {
            float x;
            float y;
            float z;
            float range;        // Range == 0  for directional light
                                // Range != 0  for point light, Zelda MM
        };
    };

    union {
        struct {
            uint8 r;
            uint8 g;
            uint8 b;
            uint8 a;
        };
        uint32 col;
    };

    union {
        struct {
            float fr;
            float fg;
            float fb;
            float fa;
        };
        float fcolors[4];
    };

    union {
        struct {
            float tx;
            float ty;
            float tz;
            float tdummy;
        };
    };

    union {
        struct {
            float ox;
            float oy;
            float oz;
            float odummy;
        };
    };
} Light;

typedef struct
{
#ifndef _BIG_ENDIAN
    signed char na;
    signed char nz;    // b
    signed char ny;    //g
    signed char nx;    //r
#else // !_BIG_ENDIAN - Big Endian fix.
    signed char nx;    //r
    signed char ny;    //g
    signed char nz;    // b
    signed char na;
#endif // _BIG_ENDIAN
}NormalStruct;

typedef struct
{
#ifndef _BIG_ENDIAN
    short y;
    short x;
    
    short flag;
    short z;
    
    short tv;
    short tu;
    
    union {
        struct {
            uint8 a;
            uint8 b;
            uint8 g;
            uint8 r;
        } rgba;
        NormalStruct norma;
    };
#else // !_BIG_ENDIAN - Big Endian fix.
    short x;
    short y;
    
    short z;
    short flag;
    
    short tu;
    short tv;
    
    union {
        struct {
            uint8 r;
            uint8 g;
            uint8 b;
            uint8 a;
        } rgba;
        NormalStruct norma;
    };
#endif // _BIG_ENDIAN
} FiddledVtx;

typedef struct //Note: This struct isn't used anywhere.
{
    short y;
    short x;
    
    uint8 a;
    uint8 b;
    short z;
    
    uint8 g;
    uint8 r;
    
} FiddledVtxDKR;

typedef struct 
{
#ifndef _BIG_ENDIAN
    short y;
    short   x;
    uint16  cidx;
    short z;
    short t;
    short s;
#else // !_BIG_ENDIAN - Big Endian fix.
    short   x;
    short y;
    short z;
    uint16  cidx;
    short s;
    short t;
#endif // _BIG_ENDIAN
} N64VtxPD;

class CTexture;
class CxeTexture;
class COGLTexture;
class CDirectXTexture;
struct TxtrCacheEntry;

typedef struct {
    LPRICETEXTURE m_lpsTexturePtr;
    union {
        CTexture *          m_pCTexture;
        CDirectXTexture *   m_pCDirectXTexture;
        CxeTexture *        m_pCxeTexture;
        COGLTexture *       m_pCOGLTexture;
    };
    
    uint32 m_dwTileWidth;
    uint32 m_dwTileHeight;
    float m_fTexWidth;
    float m_fTexHeight;     // Float to avoid converts when processing verts
    TxtrCacheEntry *pTextureEntry;
} RenderTexture;


typedef struct
{
    unsigned int    dwFormat;
    unsigned int    dwSize;
    unsigned int    dwWidth;
    unsigned int    dwAddr;

    unsigned int    dwLastWidth;
    unsigned int    dwLastHeight;

    unsigned int    dwHeight;
    unsigned int    dwMemSize;

    bool                bCopied;
    unsigned int    dwCopiedAtFrame;

    unsigned int    dwCRC;
    unsigned int    lastUsedFrame;
    unsigned int    bUsedByVIAtFrame;
    unsigned int    lastSetAtUcode;
} RecentCIInfo;

typedef struct
{
    uint32      addr;
    uint32      FrameCount;
} RecentViOriginInfo;

typedef enum {
    SHADE_DISABLED,
    SHADE_FLAT,
    SHADE_SMOOTH,
} RenderShadeMode;

typedef enum {
    TEXTURE_UV_FLAG_WRAP,
    TEXTURE_UV_FLAG_MIRROR,
    TEXTURE_UV_FLAG_CLAMP,
} TextureUVFlag;

typedef struct
{
    TextureUVFlag   N64flag;
    uint32          realFlag;
} UVFlagMap;


typedef enum {
    FILTER_POINT,
    FILTER_LINEAR,
} TextureFilter;

typedef struct 
{
    TextureFilter   N64filter;
    uint32                  realFilter;
} TextureFilterMap;

typedef struct {
    const char* description;
    int number;
    uint32  setting;
} BufferSettingInfo;

typedef struct {
    const char* description;
    uint32 setting;
} SettingInfo;

typedef union {
    uint8   g_Tmem8bit[0x1000];
    short   g_Tmem16bit[0x800];
    uint32  g_Tmem32bit[0x400];
    uint64  g_Tmem64bit[0x200];
} TmemType;


typedef struct {
    uint32 dwFormat;
    uint32 dwSize;
    BOOL  bSetBy;

    uint32 dwLoadAddress;
    uint32 dwTotalWords;
    uint32 dxt;
    BOOL  bSwapped;

    uint32 dwWidth;
    uint32 dwLine;

    int sl;
    int sh;
    int tl;
    int th;

    uint32 dwTmem;
} TMEMLoadMapInfo;

#endif

