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

#include <GL/glew.h>

#if defined(WIN32)
    #include <GL/wglew.h>
#endif

#include "ADLtool.h"

#include "adl_prototypes.h"
#include "GLWindow.h"


using namespace std;






#ifdef WIN32
void MyDebugFunc(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
    MessageBox(NULL, message, "GL Debug Message", MB_ICONWARNING | MB_OK);
}
#endif


GLWindow::GLWindow(const char *strWinName, const char* strClsaaName)
{
    m_strClassName = strClsaaName;
    m_strWinName   = strWinName;

    m_hDC   = NULL;
    m_hGLRC = NULL;
    m_hWnd  = NULL;

    m_uiWidth = 800;
    m_uiHeight = 600;

    m_uiPosX = 0;
    m_uiPosY = 0;

    m_bADLReady   = false;
    m_bFullScreen = false;
}


GLWindow::~GLWindow(void)
{
    destroy();
}







void GLWindow::resize(unsigned int uiWidth, unsigned int uiHeight)
{
    m_uiWidth  = uiWidth;
    m_uiHeight = uiHeight;
}



//----------------------------------------------------------------------------------------------
//  Windows implementation functions
//----------------------------------------------------------------------------------------------

#ifdef WIN32

bool GLWindow::create(unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDspIndex, bool bDecoration)
{
    RECT        WinSize;
    DWORD		dwExStyle;
	DWORD		dwStyle;
	int			nPixelFormat;

    // Get information for the display with id uiDspId. This is the ID
    // shown by CCC. Use the origin of this display as base position for
    // opening the window. Like this a window can be opened on a particular
    // GPU.
    ADLtool::DisplayData* pDsp = ADLtool::getDisplayData(uiDspIndex);

    if (pDsp)
    {
        m_uiPosX += pDsp->iOriginX;
        m_uiPosY += pDsp->iOriginY;
    }


	if (m_bFullScreen)
	{
		dwExStyle = WS_EX_APPWINDOW;								
		dwStyle   = WS_POPUP;											
	}
	else
	{
        m_uiWidth  = uiWidth;
        m_uiHeight = uiHeight;

		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	

        if (bDecoration)
        {
		    dwStyle   = WS_OVERLAPPEDWINDOW;

            // Adjust window size so that the ClientArea has the initial size
            // of uiWidth and uiHeight
            WinSize.bottom = uiHeight; 
            WinSize.left   = 0;
            WinSize.right  = uiWidth;
            WinSize.top    = 0;

            AdjustWindowRect(&WinSize, WS_OVERLAPPEDWINDOW, false);

            m_uiWidth  = WinSize.right  - WinSize.left;
            m_uiHeight = WinSize.bottom - WinSize.top;    
        }
        else
            dwStyle   = WS_POPUP;
	}

	m_hWnd = CreateWindowEx( dwExStyle,
						     m_strClassName.c_str(), 
						     m_strWinName.c_str(),
						     dwStyle,
						     m_uiPosX,
						     m_uiPosY,
						     m_uiWidth,
						     m_uiHeight,
						     NULL,
						     NULL,
						    (HINSTANCE)GetModuleHandle(NULL),
						     NULL);

	if (!m_hWnd)
		return FALSE;

	static PIXELFORMATDESCRIPTOR pfd;

	pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR); 
	pfd.nVersion        = 1; 
	pfd.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL  | PFD_DOUBLEBUFFER ;
	pfd.iPixelType      = PFD_TYPE_RGBA; 
	pfd.cColorBits      = 24; 
	pfd.cRedBits        = 8; 
	pfd.cRedShift       = 0; 
	pfd.cGreenBits      = 8; 
	pfd.cGreenShift     = 0; 
	pfd.cBlueBits       = 8; 
	pfd.cBlueShift      = 0; 
	pfd.cAlphaBits      = 8;
	pfd.cAlphaShift     = 0; 
	pfd.cAccumBits      = 0; 
	pfd.cAccumRedBits   = 0; 
	pfd.cAccumGreenBits = 0; 
	pfd.cAccumBlueBits  = 0; 
	pfd.cAccumAlphaBits = 0; 
	pfd.cDepthBits      = 24; 
	pfd.cStencilBits    = 8; 
	pfd.cAuxBuffers     = 0; 
	pfd.iLayerType      = PFD_MAIN_PLANE;
	pfd.bReserved       = 0; 
	pfd.dwLayerMask     = 0;
	pfd.dwVisibleMask   = 0; 
	pfd.dwDamageMask    = 0;


	m_hDC = GetDC(m_hWnd);

	if (!m_hDC)
		return false;

	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);

    if (!nPixelFormat)
		return false;

	SetPixelFormat(m_hDC, nPixelFormat, &pfd);
	

	return true;
}


void GLWindow::destroy()
{
    if (m_hGLRC)
    {
        wglMakeCurrent(m_hDC, NULL);
        wglDeleteContext(m_hGLRC);
    }

    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
}

void GLWindow::open()
{
    ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	UpdateWindow(m_hWnd);
}





void GLWindow::makeCurrent()
{
    wglMakeCurrent(m_hDC, m_hGLRC);
}


void GLWindow::swapBuffers()
{
    SwapBuffers(m_hDC);
}


bool GLWindow::createContext()
{
    if (!m_hDC)
        return false;

    m_hGLRC = wglCreateContext(m_hDC);

    if (!m_hGLRC)
        return false;

    wglMakeCurrent( m_hDC, m_hGLRC );

    if (glewInit() != GLEW_OK)
    {
        return false;
    }
   
    if (WGLEW_ARB_create_context)
    {
        wglDeleteContext(m_hGLRC);

        int attribs[] = {
          WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
          WGL_CONTEXT_MINOR_VERSION_ARB, 1,
          WGL_CONTEXT_PROFILE_MASK_ARB , WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#ifdef DEBUG
          WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
          0
        }; 

        m_hGLRC = wglCreateContextAttribsARB(m_hDC, 0, attribs);

        if (m_hGLRC)
        {
            wglMakeCurrent(m_hDC, m_hGLRC);

            if (GLEW_AMD_debug_output)
                glDebugMessageCallbackAMD((GLDEBUGPROCAMD)&MyDebugFunc, NULL);

            return true;            
        }
    }

    return false;
}


void GLWindow::deleteContext()
{
    wglMakeCurrent(m_hDC, NULL);
    wglDeleteContext(m_hGLRC);
}

#endif


//----------------------------------------------------------------------------------------------
//  Linux implementation functions
//----------------------------------------------------------------------------------------------
#ifdef LINUX

bool GLWindow::create(unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDspIndex, bool bDecoration)
{
    XSetWindowAttributes WinAttr;

    int attrList[] = {  GLX_RGBA, GLX_DOUBLEBUFFER,
                        GLX_RED_SIZE, 8,
                        GLX_GREEN_SIZE, 8,
                        GLX_BLUE_SIZE, 8,
                        GLX_DEPTH_SIZE, 24,
                        GLX_STENCIL_SIZE, 8,
                        None };

    // Get information for the display with id uiDspId. This is the ID
    // shown by CCC. Use the origin of this display as base position for
    // opening the window. Like this a window can be opened on a particular
    // GPU.
    ADLtool::DisplayData* pDsp = ADLtool::getDisplayData(uiDspIndex);

    if (!pDsp)
        return false;

    m_uiWidth  = uiWidth;
    m_uiHeight = uiHeight;


    m_hDC = XOpenDisplay(pDsp->strDisplayname.c_str());

    if (m_hDC == NULL)
    {
		printf("Could not open display display \n");
		exit(1);
    }

    m_uiScreen = DefaultScreen(m_hDC);

    // get an appropriate visual 
    m_vi = glXChooseVisual(m_hDC, m_uiScreen, attrList);

    Window root = RootWindow(m_hDC, m_uiScreen);

    // window attributes 
    WinAttr.border_pixel = 0;
    WinAttr.colormap = XCreateColormap( m_hDC, root, m_vi->visual, AllocNone);
    WinAttr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    WinAttr.background_pixel = 0;
    unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    m_hWnd = XCreateWindow(m_hDC, root, 0, 0, m_uiWidth, m_uiHeight, 0, m_vi->depth, InputOutput, m_vi->visual, mask, &WinAttr);

    XSetStandardProperties(m_hDC, m_hWnd, m_strWinName.c_str(), m_strWinName.c_str(), None, NULL, 0, NULL);

	if (!bDecoration)
	{
       static const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
       static const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;

       typedef struct
       {
	      unsigned long       flags;
	      unsigned long       functions;
	      unsigned long       decorations;
	      long                inputMode;
	      unsigned long       status;
       } PropMotifWmHints;

       PropMotifWmHints motif_hints;
       Atom prop, proptype;
       unsigned long flags = 0;

       // set up the property 
       motif_hints.flags = MWM_HINTS_DECORATIONS;
       motif_hints.decorations = flags;

       // get the atom for the property 
       prop = XInternAtom( m_hDC, "_MOTIF_WM_HINTS", True );
       if (!prop) 
	   {
	      // something went wrong! 
	      return false;
       }
       proptype = prop;

       XChangeProperty( m_hDC, m_hWnd,           // display, window 
		         prop, proptype,                 // property, type 
		         32,                             // format: 32-bit datums 
		         PropModeReplace,                // mode 
		         (unsigned char *) &motif_hints, // data 
		         PROP_MOTIF_WM_HINTS_ELEMENTS    // nelements 
		       );
      
	}
	 
    return true;
}


void GLWindow::destroy()
{
    if (m_hGLRC)
    {
        glXMakeCurrent(m_hDC, NULL, NULL);
        glXDestroyContext(m_hDC, m_hGLRC);
    }
    
    if (m_hWnd)
    {
        XUnmapWindow(m_hDC, m_hWnd);
        XDestroyWindow(m_hDC, m_hWnd);
    }
}


void GLWindow::open()
{
    XMapWindow(m_hDC, m_hWnd);
}


void GLWindow::makeCurrent()
{
    // connect the glx-context to the window 
    glXMakeCurrent(m_hDC, m_hWnd, m_hGLRC);
}


void GLWindow::swapBuffers()
{
    glXSwapBuffers(m_hDC, m_hWnd);
}


bool GLWindow::createContext()
{
	// create a GLX context 
    m_hGLRC = glXCreateContext(m_hDC, m_vi, 0, GL_TRUE);
    
    glXMakeCurrent(m_hDC, m_hWnd, m_hGLRC);

    if (glewInit() != GLEW_OK)
    {
        return false;
    }

    return true;
}


void GLWindow::deleteContext()
{
    glXMakeCurrent(m_hDC, NULL, NULL);
    glXDestroyContext(m_hDC, m_hGLRC);
}


#endif





