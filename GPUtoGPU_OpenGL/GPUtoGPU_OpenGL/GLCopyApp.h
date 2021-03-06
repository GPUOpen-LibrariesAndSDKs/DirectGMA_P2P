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

#include <string>

#include "GLWindow.h"
#include "Thread.h"

class GLSink;
class GLSource;
class GLWindow;



class GLCopyApp
{
public:

    GLCopyApp();
    virtual ~GLCopyApp();

    bool    init(unsigned int uiWidth, unsigned int uiHeight, const char* pClassName, std::string& strErrorMessage);
    bool    start(bool bUseP2P);
    void    stop();
	
    bool    isRunning() { return m_bRunning; };

private:

    static DWORD CALLAPI    SourceThreadFunc(void* pArg);
    static DWORD CALLAPI    SinkThreadFunc(void* pArg);

    bool    SourceLoop();
    bool    SinkLoop();

    void    processEvents(GLWindow* pWin);
    
    bool                m_bRunning;                 // Indicates that the thread is running
    bool                m_bStarted;                 // Indicates that the thread was started
    bool                m_bResizeSource;            // if true, the source window was resized
    bool                m_bResizeSink;              // if true, the sink window was resized

    bool                m_bUseP2P;
    bool                m_bCanDoP2P;

    unsigned int        m_uiBufferWidth;
    unsigned int        m_uiBufferHeight;

    GLWindow*           m_pSourceWin;
    GLWindow*           m_pSinkWin;

    GLSink*             m_pSink;                  // OpenGL consumer of frames rendered by GLSource
    GLSource*           m_pSource;                // OpenGL producer produces frames for GLSink

    Thread              m_SourceThread;
    Thread              m_SinkThread;

    Semaphore           m_SinkReady;
    Semaphore           m_SourceReady;
    Semaphore           m_SourceDone;

    
};
