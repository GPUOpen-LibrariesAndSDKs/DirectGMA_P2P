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


#pragma once

#ifdef WIN32
#include "GL/glew.h"
#endif

#ifdef LINUX
#include <GL/glxew.h>
#endif

#include "defines.h"


typedef struct
{
    char*           pBasePointer;
    char*           pAlignedPointer;
} AlignedMem;



class GLTransferBuffers
{
public:

    GLTransferBuffers(void);
    ~GLTransferBuffers(void);

    // Creates either bus addressable buffers or pinned memory buffers that can be used for data transfers.
    // uiNumBuffers: Number of buffers to be created
    // uiBufferSize: size of each buffer in bytes
    // uiTarget: Either GL_PIXEL_PACK_BUFFER or GL_PIXEL_UNPACK_BUFFER
    // bUseP2P: if true a bus addressable buffer is created, if false a pinned memory buffer
    // pPinnedMem: Pointer to memory that is already pinned and that should be reused for this buffer. If NULL new memory is allocated
    bool createBuffers(unsigned int uiNumBuffers, unsigned int uiBufferSize, unsigned int uiTarget, bool bUseP2P = false, AlignedMem *pPinnedMem = NULL);

    // In case of using a p2p PACK_BUFFER, the actual memory used by the buffer is located on a remote device. 
    // assignRemoteMemory passes the bus addresses of this memory to OpenGL and assigns it to the buffers used by this class.
    bool assignRemoteMemory(unsigned int uiNumBuffers, unsigned long long* pBufferBusAddress, unsigned long long* pMarkerBusAddress);

    void   waitMarker(unsigned int uiMarkerValue);
    void   writeMarker(unsigned long long ulBufferBusAddress, unsigned long long ulMarkerBusAddress, unsigned int uiMarkerValue);

    // Bind the buffer that is located at the bus address: ulBusAddress
    bool    bindBuffer(unsigned long long ulBusAddress);
    // Bind buffer with id: uiIdx
    bool    bindBuffer(unsigned int uiIdx);

    // In case of pinned memory returns the pointer to the aligned memory
    AlignedMem*         getPinndeMemory()              { return m_pBufferMemory; };
    // In case of pinned memory buffers, returns the pointer to the pinned system memory
    // that is used by buffer id: uiIdx
    char*               getPinnedMemoryPtr(unsigned int uiIdx);
    // In case of bus addressable memory, returns the bus address of the buffer memory used by
    // the buffer with id:  uiIdx
    GLuint64            getBufferBusAddress(unsigned int uiIdx);
    // In case of bus addressable memory, returns a pointer to the array that contains the buffer
    // bus addresses. The array length is m_uiNumBuffers
    GLuint64*           getBufferBusAddresses()        { return m_pBufferBusAddress; };
    // In case of bus addressable memory, returns the bus address of the marker memory used by
    // the buffer with id:  uiIdx
    GLuint64            getMarkerBusAddress(unsigned int uiIdx);
    // In case of bus addressable memory, returns a pointer to the array that contains the marker
    // bus addresses. The array length is m_uiNumBuffers
    GLuint64*           getMarkerBusAddresses()        { return m_pMarkerBusAddress; };

    unsigned int         getNumBuffers()                { return m_uiNumBuffers; };

private:

    bool                    m_bUseP2P;
    bool                    m_bBufferReady;

    unsigned int            m_uiTarget;
    unsigned int            m_uiNumBuffers;
    unsigned int            m_uiBufferSize;
    unsigned int            m_uiBufferIdx;
    unsigned int*           m_pBuffer;

    bool                    m_bAllocatedPinnedMem;
    AlignedMem*             m_pBufferMemory;
    GLuint64*               m_pBufferBusAddress;
    GLuint64*               m_pMarkerBusAddress;
};
