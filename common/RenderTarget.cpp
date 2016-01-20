//--------------------------------------------------------------------------------------
//
//
// Copyright 2014 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
//
// AMD is granting you permission to use this software and documentation (if
// any) (collectively, the "Materials") pursuant to the terms and conditions
// of the Software License Agreement included with the Materials.  If you do
// not have a copy of the Software License Agreement, contact your AMD
// representative for a copy.
// You agree that you will not reverse engineer or decompile the Materials,
// in whole or in part, except as allowed by applicable law.
//
// WARRANTY DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND.  AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT, THAT THE SOFTWARE
// WILL RUN UNINTERRUPTED OR ERROR-FREE OR WARRANTIES ARISING FROM CUSTOM OF
// TRADE OR COURSE OF USAGE.  THE ENTIRE RISK ASSOCIATED WITH THE USE OF THE
// SOFTWARE IS ASSUMED BY YOU.
// Some jurisdictions do not allow the exclusion of implied warranties, so
// the above exclusion may not apply to You.
//
// LIMITATION OF LIABILITY AND INDEMNIFICATION:  AMD AND ITS LICENSORS WILL
// NOT, UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
// INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
// THE SOFTWARE OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
// In no event shall AMD's total liability to You for all damages, losses,
// and causes of action (whether in contract, tort (including negligence) or
// otherwise) exceed the amount of $100 USD.  You agree to defend, indemnify
// and hold harmless AMD and its licensors, and any of their directors,
// officers, employees, affiliates or agents from and against any and all
// loss, damage, liability and other expenses (including reasonable attorneys'
// fees), resulting from Your use of the Software or violation of the terms and
// conditions of this Agreement. 
//
// U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with "RESTRICTED
// RIGHTS." Use, duplication, or disclosure by the Government is subject to the
// restrictions as set forth in FAR 52.227-14 and DFAR252.227-7013, et seq., or
// its successor.  Use of the Materials by the Government constitutes
// acknowledgement of AMD's proprietary rights in them.
//
// EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
// stated in the Software License Agreement.
//
//--------------------------------------------------------------------------------------




#include <GL/glew.h>

#include "RenderTarget.h"

RenderTarget::RenderTarget(void)
{
    m_uiBufferId        = 0;
    m_uiBufferWidth     = 0;
    m_uiBufferHeight    = 0;
    m_nBufferFormat     = 0;
}


RenderTarget::~RenderTarget(void)
{
    deleteBuffer();
}


bool RenderTarget::createBuffer(unsigned int uiWidth, unsigned int uiHeight, int nBufferFormat, int nExtFormat, int nType)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (m_uiBufferId != 0)
    {
        return false;
    }

    m_uiBufferWidth  = uiWidth;
    m_uiBufferHeight = uiHeight;
    m_nBufferFormat  = nBufferFormat;
    m_nExtFormat     = nExtFormat;
    m_nType          = nType;

    // Set up texture to be used as color attachment
    glGenTextures(1, &m_uiColorTex);

    glBindTexture(GL_TEXTURE_2D, m_uiColorTex);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	
    glTexImage2D(GL_TEXTURE_2D, 0, m_nBufferFormat, m_uiBufferWidth, m_uiBufferHeight, 0, m_nExtFormat, m_nType, 0);

    // Create FBO with color and depth attachment
    glGenFramebuffers(1,  &m_uiBufferId);
    glGenRenderbuffers(1, &m_uiDepthBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, m_uiBufferId);

    glBindRenderbuffer(GL_RENDERBUFFER, m_uiDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_uiBufferWidth, m_uiBufferHeight);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiColorTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_RENDERBUFFER, m_uiDepthBuffer);

    bool bFBOStatus = false;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {  
        bFBOStatus = true;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return bFBOStatus;
}


void RenderTarget::deleteBuffer()
{
    if (m_uiColorTex)
        glDeleteTextures(1, &m_uiColorTex);

    if (m_uiDepthBuffer)
        glDeleteRenderbuffers(1, &m_uiDepthBuffer);

    if (m_uiBufferId)
        glDeleteFramebuffers(1, &m_uiBufferId);

    m_uiBufferId = 0;
}


void RenderTarget::bind(GLenum nTarget)
{
    if (m_uiBufferId)
    {
        glBindFramebuffer(nTarget, m_uiBufferId);
    }
}


void RenderTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void RenderTarget::draw()
{
    int nViewport[4];

    glGetIntegerv(GL_VIEWPORT, nViewport);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_uiBufferId);
    glBlitFramebuffer(0, 0, m_uiBufferWidth, m_uiBufferHeight, nViewport[0], nViewport[1], nViewport[2], nViewport[3], GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}


unsigned int RenderTarget::getBufferHeight()
{
    if (m_uiBufferId)
    {
        return m_uiBufferHeight;
    }

    return 0;
}


unsigned int RenderTarget::getBufferWidth()
{
    if (m_uiBufferId)
    {
        return m_uiBufferWidth;
    }

    return 0;
}


int RenderTarget::getBufferFormat()
{
    if (m_uiBufferId)
    {
        return m_nBufferFormat;
    }

    return 0;
}
