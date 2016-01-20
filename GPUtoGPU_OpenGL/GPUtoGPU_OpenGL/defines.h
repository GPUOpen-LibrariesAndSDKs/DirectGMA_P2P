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
#include <GL/glew.h>
#endif

#ifdef LINUX
#include <GL/glxew.h>

typedef long long LONGLONG;

#define APIENTRY
#endif

#define GLX_ARB_get_proc_address 1

#define GL_EXTERNAL_VIRTUAL_MEMORY_AMD          0x9160

#define GL_BUS_ADDRESSABLE_MEMORY_AMD           0x9168
#define GL_EXTERNAL_PHYSICAL_MEMORY_AMD         0x9169

typedef GLvoid (APIENTRY * PFNGLWAITMARKERAMDPROC)          (GLuint buffer, GLuint marker);
typedef GLvoid (APIENTRY * PFNGLWRITEMARKERAMDPROC)         (GLuint buffer, GLuint marker, GLuint64 offset);
typedef GLvoid (APIENTRY * PFNGLMAKEBUFFERSRESIDENTAMDPROC) (GLsizei count, GLuint* buffers, GLuint64* baddrs, GLuint64* maddrs);
typedef GLvoid (APIENTRY * PFNGLBUFFERBUSADDRESSAMDPROC)    (GLenum target, GLsizeiptr size, GLuint64 baddrs, GLuint64 maddrs);


extern PFNGLWAITMARKERAMDPROC          glWaitMarkerAMD;
extern PFNGLWRITEMARKERAMDPROC         glWriteMarkerAMD;
extern PFNGLMAKEBUFFERSRESIDENTAMDPROC glMakeBuffersResidentAMD;
extern PFNGLBUFFERBUSADDRESSAMDPROC    glBufferBusAddressAMD;


#define OneSecond 1000*1000*1000L

#define MAX_SDI_BOARDS  2
#define MAX_STREAMS     4

#define NUM_BUFFERS 2
#define THREAD_WAIT_TIMEOUT 5000

// data structure to exchange P2P addresses between the GPUs
typedef struct
{
    unsigned int        uiTransferId;
    unsigned int        uiBufferId;
    unsigned long long  ullBufferBusAddress;
    unsigned long long  ullMarkerBusAddress;
} FrameData;

