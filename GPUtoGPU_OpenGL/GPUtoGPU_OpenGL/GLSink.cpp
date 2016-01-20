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




#include "os_include.h"
#include <assert.h>
#include <GL/glew.h>


#include "defines.h"
#include "FormatInfo.h"
#include "BufferQueue.h"
#include "GLTransferBuffers.h"
#include "GLSink.h"



GLSink::GLSink()
{
    m_bUseP2P        = false;

    m_uiWindowWidth  = 0;
    m_uiWindowHeight = 0;

    m_uiTextureWidth  = 0;
    m_uiTextureHeight = 0;
    m_uiTexture       = 0;
    m_uiTextureSize   = 0;

    m_nIntFormat = GL_RGB8;
    m_nExtFormat = GL_RGB;
    m_nType      = GL_UNSIGNED_BYTE;

    m_uiQuad = 0;
 
    m_fAspectRatio = 1.0f;

    m_pInputBuffer      = NULL;
    m_pSyncBuffer       = NULL;
    m_pFrameData        = NULL;
}


GLSink::~GLSink()
{
    // make sure now other thread is blocked
    release();

    if (m_pFrameData)
        delete [] m_pFrameData;
  
    if (m_pInputBuffer)
        delete m_pInputBuffer;

    if (m_pSyncBuffer)
        delete m_pSyncBuffer;
}


void GLSink::initGL()
{
    const float pArray [] = { -0.5f, -0.5f, 0.0f,       0.0f, 0.0f,
                               0.5f, -0.5f, 0.0f,       1.0f, 0.0f,
                               0.5f,  0.5f, 0.0f,       1.0f, 1.0f,
                              -0.5f,  0.5f, 0.0f,       0.0f, 1.0f 
                            };
    
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_SMOOTH);

    glPolygonMode(GL_FRONT, GL_FILL);
    
    // create quad that is used to map SDi texture to
    glGenBuffers(1, &m_uiQuad);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiQuad);

    glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), pArray, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_uiQuad);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT,   5*sizeof(float), 0);
    glTexCoordPointer(2, GL_FLOAT, 5*sizeof(float), (char*)NULL + 3*sizeof(float));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Activate Sync to VBlank to avoid tearing
    //wglSwapIntervalEXT(1);
}


void GLSink::resize(unsigned int w, unsigned int h)
{
    m_uiWindowWidth  = w;
    m_uiWindowHeight = h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-(double)w/(double)h*0.5, (double)w/(double)h*0.5, -0.5, 0.5, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


bool GLSink::createDownStream(unsigned int w, unsigned int h, int nIntFormat, int nExtFormat, int nType, bool bUseP2P)
{
    unsigned int uiBufferSize = 0;

    m_uiTextureWidth  = w;
    m_uiTextureHeight = h;

    m_nIntFormat = nIntFormat;
    m_nExtFormat = nExtFormat;
    m_nType      = nType;

    m_fAspectRatio = (float)w/(float)h;

    uiBufferSize = FormatInfo::getInternalFormatSize(m_nIntFormat) * m_uiTextureWidth * m_uiTextureHeight;

    if (uiBufferSize == 0)
        return false;

    // check if format is supported
    if (FormatInfo::getExternalFormatSize(m_nExtFormat, m_nType) == 0)
        return false;

    // Create texture that will be used to store frames from remote device
    glGenTextures(1, &m_uiTexture);

    glBindTexture(GL_TEXTURE_2D, m_uiTexture);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, m_nIntFormat, m_uiTextureWidth, m_uiTextureHeight, 0, m_nExtFormat, m_nType, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_uiTextureSize = m_uiTextureWidth*m_uiTextureHeight*FormatInfo::getExternalFormatSize(m_nExtFormat, m_nType);

    // Create transfer buffer
    m_pInputBuffer = new GLTransferBuffers;

    m_pInputBuffer->createBuffers(NUM_BUFFERS, uiBufferSize, GL_PIXEL_UNPACK_BUFFER, bUseP2P);

    // Create synchronization buffers
    m_pSyncBuffer = new BufferQueue(NUM_BUFFERS);

    // Allocate memory to store frame data
    m_pFrameData = new FrameData[NUM_BUFFERS];
   
    for (unsigned int i = 0; i < NUM_BUFFERS; i++)
    {
        m_pFrameData[i].uiTransferId        = 0;
        m_pFrameData[i].uiBufferId          = i;
        m_pFrameData[i].ullBufferBusAddress = m_pInputBuffer->getBufferBusAddress(i);
        m_pFrameData[i].ullMarkerBusAddress = m_pInputBuffer->getMarkerBusAddress(i);

        m_pSyncBuffer->setBufferMemory(i, (void*)(&m_pFrameData[i]));
    }

    m_bUseP2P = bUseP2P;

    return true;
}


void GLSink::draw()
{
    unsigned int        uiBufferIdx;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update stream 
    glBindTexture(GL_TEXTURE_2D, m_uiTexture);

    FrameData* pFrame = NULL;

    // block until new frame is available
    uiBufferIdx = m_pSyncBuffer->getBufferForReading((void*&)pFrame);

    m_pInputBuffer->bindBuffer(pFrame->uiBufferId);

    if (m_bUseP2P)
    {
        // This is a non-blocking call, but the GPU is instructed to wait until
        // the marker value is pFrame->uiTransferId before processing the subsequent
        // instructions
        m_pInputBuffer->waitMarker(pFrame->uiTransferId);
    }

    // Copy bus addressable buffer into texture object
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_uiTextureWidth, m_uiTextureHeight, m_nExtFormat, m_nType, NULL);

    // Insert fence to determine when the buffer was copied into the texture
    // and we can release the buffer
    GLsync Fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // Draw quad with mapped texture
    glPushMatrix();
   
    // Scale quad to the AR of the incoming texture
    glScalef(m_fAspectRatio, 1.0f, 1.0f);
   
    // Draw quad with mapped texture
    glBindBuffer(GL_ARRAY_BUFFER, m_uiQuad);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);

    // Wait until buffer is no longer needed and release it
    if (glIsSync(Fence))
    {
        glClientWaitSync(Fence, GL_SYNC_FLUSH_COMMANDS_BIT, OneSecond);
        glDeleteSync(Fence);
    }

    m_pSyncBuffer->releaseReadBuffer();
}



unsigned long long* GLSink::getBufferBusAddress()
{
    if (m_pInputBuffer)
    {
        return (unsigned long long*)m_pInputBuffer->getBufferBusAddresses();
    }

    return NULL;
}


unsigned long long* GLSink::getMarkerBusAddress()
{
    if (m_pInputBuffer)
    {
        return (unsigned long long*)m_pInputBuffer->getMarkerBusAddresses();
    }

    return NULL;
}


AlignedMem* GLSink::getPinnedMemory()
{
    return m_pInputBuffer->getPinndeMemory();
}


void GLSink::release()
{
    if (m_pSyncBuffer)
        m_pSyncBuffer->releaseReadBuffer();

}
