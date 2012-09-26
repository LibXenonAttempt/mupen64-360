/*
Rice_GX - Texture.cpp
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

#include "stdafx.h"



//////////////////////////////////////////
// Constructors / Deconstructors

// Probably shouldn't need more than 4096 * 4096

CTexture::CTexture(uint32 dwWidth, uint32 dwHeight, TextureUsage usage) :
    m_dwWidth(dwWidth),
    m_dwHeight(dwHeight),
    m_dwCreatedTextureWidth(dwWidth),
    m_dwCreatedTextureHeight(dwHeight),
    m_fXScale(1.0f),
    m_fYScale(1.0f),
    m_bScaledS(false),
    m_bScaledT(false),
    m_bClampedS(false),
    m_bClampedT(false),
    m_bIsEnhancedTexture(false),
    m_Usage(usage),
#ifdef __GX__
		GXinited(false),
		GXcacheType(UNKNOWN_CACHE),
		GXtexfmt(GX_TF_RGBA8),
		GXwrapS(GX_REPEAT),
		GXwrapT(GX_REPEAT),
#endif //__GX__
        m_pTexture(NULL),
        m_dwTextureFmt(TEXTURE_FMT_A8R8G8B8)
{
    // fix me, do something here
}

CTexture::~CTexture(void)
{
#ifdef __GX__
	if (m_pTexture)	
	{
		__lwp_heap_free(GXtexCache, m_pTexture);
		gGX.GXnumTex--;
		gGX.GXnumTexBytes -= GXtextureBytes;
	}
	m_pTexture = NULL;
#endif //__GX__
}

TextureFmt CTexture::GetSurfaceFormat(void)
{
    if (m_pTexture == NULL)
        return TEXTURE_FMT_UNKNOWN;
    else
        return m_dwTextureFmt;
}

uint32 CTexture::GetPixelSize()
{
#ifndef __GX__
    if( m_dwTextureFmt == TEXTURE_FMT_A8R8G8B8 )
        return 4;
    else
        return 2;
#else //!__GX__
    if( GXtexfmt == GX_TF_RGBA8 )
        return 4;
    else if ( GXtexfmt == GX_TF_IA4 )
        return 1;
	else //GX_TF_RGB5A3 or GX_TF_IA8
		return 2;
#endif //__GX__
}


// There are reasons to create this function. D3D and OGL will only create surface of width and height
// as 2's pow, for example, N64's 20x14 image, D3D and OGL will create a 32x16 surface.
// When we using such a surface as D3D texture, and the U and V address is for the D3D and OGL surface
// width and height. It is still OK if the U and V addr value is less than the real image within
// the D3D surface. But we will have problems if the U and V addr value is larger than it, or even
// large then 1.0.
// In such a case, we need to scale the image to the D3D surface dimension, to ease the U/V addr
// limition
void CTexture::ScaleImageToSurface(bool scaleS, bool scaleT)
{
#ifdef __GX__
	return; // This function is never used.
#else //__GX__
    uint8 g_ucTempBuffer[1024*1024*4];

    if( scaleS==false && scaleT==false) return;

    // If the image is not scaled, call this function to scale the real image to
    // the D3D given dimension

    uint32 width = scaleS ? m_dwWidth : m_dwCreatedTextureWidth;
    uint32 height = scaleT ? m_dwHeight : m_dwCreatedTextureHeight;

    uint32 xDst, yDst;
    uint32 xSrc, ySrc;

    DrawInfo di;

    if (!StartUpdate(&di))
    {
        return;
    }

    int pixSize = GetPixelSize();

    // Copy across from the temp buffer to the surface
    switch (pixSize)
    {
    case 4:
        {
            memcpy((uint8*)g_ucTempBuffer, (uint8*)(di.lpSurface), m_dwHeight*m_dwCreatedTextureWidth*4);

            uint32 * pDst;
            uint32 * pSrc;
            
            for (yDst = 0; yDst < m_dwCreatedTextureHeight; yDst++)
            {
                // ySrc ranges from 0..m_dwHeight
                // I'd rather do this but sometimes very narrow (i.e. 1 pixel)
                // surfaces are created which results in  /0...
                //ySrc = (yDst * (m_dwHeight-1)) / (d3dTextureHeight-1);
                ySrc = (uint32)((yDst * height) / m_dwCreatedTextureHeight+0.49f);
                
                pSrc = (uint32*)((uint8*)g_ucTempBuffer + (ySrc * m_dwCreatedTextureWidth * 4));
                pDst = (uint32*)((uint8*)di.lpSurface + (yDst * di.lPitch));
                
                for (xDst = 0; xDst < m_dwCreatedTextureWidth; xDst++)
                {
                    xSrc = (uint32)((xDst * width) / m_dwCreatedTextureWidth+0.49f);
                    pDst[xDst] = pSrc[xSrc];
                }
            }
        }
        
        break;
    case 2:
        {
            memcpy((uint8*)g_ucTempBuffer, (uint8*)(di.lpSurface), m_dwHeight*m_dwCreatedTextureWidth*2);

            uint16 * pDst;
            uint16 * pSrc;
            
            for (yDst = 0; yDst < m_dwCreatedTextureHeight; yDst++)
            {
                // ySrc ranges from 0..m_dwHeight
                ySrc = (yDst * height) / m_dwCreatedTextureHeight;
                
                pSrc = (uint16*)((uint8*)g_ucTempBuffer + (ySrc * m_dwCreatedTextureWidth * 2));
                pDst = (uint16*)((uint8*)di.lpSurface + (yDst * di.lPitch));
                
                for (xDst = 0; xDst < m_dwCreatedTextureWidth; xDst++)
                {
                    xSrc = (xDst * width) / m_dwCreatedTextureWidth;
                    pDst[xDst] = pSrc[xSrc];
                }
            }
        }
        break;
            
    }
            
    EndUpdate(&di);

    if( scaleS ) m_bScaledS = true;
    if( scaleT ) m_bScaledT = true;
#endif //!__GX__
}

void CTexture::ClampImageToSurfaceS()
{
#ifdef __GX__
	return;	// In GX we can always use the exact texture width.
#else //__GX__
    if( !m_bClampedS && m_dwWidth < m_dwCreatedTextureWidth )
    {       
        DrawInfo di;
        if( StartUpdate(&di) )
        {
            if(  m_dwTextureFmt == TEXTURE_FMT_A8R8G8B8 )
            {
                for( uint32 y = 0; y<m_dwHeight; y++ )
                {
                    uint32* line = (uint32*)((uint8*)di.lpSurface+di.lPitch*y);
                    uint32 val = line[m_dwWidth-1];
                    for( uint32 x=m_dwWidth; x<m_dwCreatedTextureWidth; x++ )
                    {
                        line[x] = val;
                    }
                }
            }
            else
            {
                for( uint32 y = 0; y<m_dwHeight; y++ )
                {
                    uint16* line = (uint16*)((uint8*)di.lpSurface+di.lPitch*y);
                    uint16 val = line[m_dwWidth-1];
                    for( uint32 x=m_dwWidth; x<m_dwCreatedTextureWidth; x++ )
                    {
                        line[x] = val;
                    }
                }
            }
            EndUpdate(&di);
        }
    }
    m_bClampedS = true;
#endif //!__GX__
}

void CTexture::ClampImageToSurfaceT()
{
#ifdef __GX__
	return;	// In GX we can always use the exact texture height.
#else //__GX__
    if( !m_bClampedT && m_dwHeight < m_dwCreatedTextureHeight )
    {
        DrawInfo di;
        if( StartUpdate(&di) )
        {
            if(  m_dwTextureFmt == TEXTURE_FMT_A8R8G8B8 )
            {
                uint32* linesrc = (uint32*)((uint8*)di.lpSurface+di.lPitch*(m_dwHeight-1));
                for( uint32 y = m_dwHeight; y<m_dwCreatedTextureHeight; y++ )
                {
                    uint32* linedst = (uint32*)((uint8*)di.lpSurface+di.lPitch*y);
                    for( uint32 x=0; x<m_dwCreatedTextureWidth; x++ )
                    {
                        linedst[x] = linesrc[x];
                    }
                }
            }
            else
            {
                uint16* linesrc = (uint16*)((uint8*)di.lpSurface+di.lPitch*(m_dwHeight-1));
                for( uint32 y = m_dwHeight; y<m_dwCreatedTextureHeight; y++ )
                {
                    uint16* linedst = (uint16*)((uint8*)di.lpSurface+di.lPitch*y);
                    for( uint32 x=0; x<m_dwCreatedTextureWidth; x++ )
                    {
                        linedst[x] = linesrc[x];
                    }
                }
            }
            EndUpdate(&di);
        }
    }
    m_bClampedT = true;
#endif //!__GX__
}

void CTexture::RestoreAlphaChannel(void) //This is never called..
{
    DrawInfo di;

    if ( StartUpdate(&di) )
    {
        uint32 *pSrc = (uint32 *)di.lpSurface;
        LONG lPitch = di.lPitch;

        for (uint32 y = 0; y < m_dwHeight; y++)
        {
            uint32 * dwSrc = (uint32 *)((uint8 *)pSrc + y*lPitch);
            for (uint32 x = 0; x < m_dwWidth; x++)
            {
                uint32 dw = dwSrc[x];
                uint32 dwRed   = (uint8)((dw & 0x00FF0000)>>16);
                uint32 dwGreen = (uint8)((dw & 0x0000FF00)>>8 );
                uint32 dwBlue  = (uint8)((dw & 0x000000FF)    );
                uint32 dwAlpha = (dwRed+dwGreen+dwBlue)/3;
                dw &= 0x00FFFFFF;
                dw |= (dwAlpha<<24);

                /*
                uint32 dw = dwSrc[x];
                if( (dw&0x00FFFFFF) > 0 )
                    dw |= 0xFF000000;
                else
                    dw &= 0x00FFFFFF;
                    */
            }
        }
        EndUpdate(&di);
    }
    else
    {
        //TRACE0("Cannot lock texture");
    }
}

#ifdef __GX__
int CTexture::GXallocateTexture(void)
{
	//Set texture size based on
	uint32 GXtileX = 4;
	uint32 GXtileY = 4;
	uint32 GXbpp = 4;

	switch (GXtexfmt)
	{
	case GX_TF_IA4:
		GXtileX = 8;
		GXbpp = 1;
		break;
	case GX_TF_IA8:
	case GX_TF_RGB5A3:
		GXbpp = 2;
		break;
	case GX_TF_RGBA8:
		GXbpp = 4;
	}

//	GXwidth = m_dwCreatedTextureWidth + GXtileX - (m_dwCreatedTextureWidth%GXtileX);
//	GXheight = m_dwCreatedTextureHeight + GXtileY - (m_dwCreatedTextureHeight%GXtileY);

	if(m_dwCreatedTextureWidth % GXtileX)
		GXwidth = m_dwCreatedTextureWidth + GXtileX - (m_dwCreatedTextureWidth%GXtileX);
	else
		GXwidth = m_dwCreatedTextureWidth;

	if(m_dwCreatedTextureHeight % GXtileY)
		GXheight = m_dwCreatedTextureHeight + GXtileY - (m_dwCreatedTextureHeight%GXtileY);
	else
		GXheight = m_dwCreatedTextureHeight;

	GXtextureBytes = GXwidth*GXheight*GXbpp;

	if (GXcacheType == TEX_CACHE)
	{
		m_pTexture = (u32*) __lwp_heap_allocate(GXtexCache,GXtextureBytes);
		while(!m_pTexture)
		{
			//TODO: g_pFrameBufferManager->CloseUp() needed?
			if (gTextureManager.PurgeOldestTexture(this)) break;
			m_pTexture = (u32*) __lwp_heap_allocate(GXtexCache,GXtextureBytes);
		}
	}

	if (m_pTexture) 
	{
		gGX.GXnumTex++;
		gGX.GXnumTexBytes += GXtextureBytes;
		memset (m_pTexture, 0, GXtextureBytes);
#if 0
		sprintf(txtbuffer,"TexAlloc: NumTex %d; NumTexBytes %d", gGX.GXnumTex, gGX.GXnumTexBytes);
		DEBUG_print(txtbuffer,DBG_CACHEINFO); 
#endif
		return 0;
	}
	else
	{
#if 0
		static int callcount = 0;
		sprintf(txtbuffer,"GXalcTx %d fail, Ht %d, Wd %d, Byts %d; nTex %d, nByts %d", callcount++, GXheight, GXwidth, GXtextureBytes, gGX.GXnumTex, gGX.GXnumTexBytes);
		DEBUG_print(txtbuffer,DBG_CACHEINFO+1); 
#endif
		return -1;
	}
}
#endif

