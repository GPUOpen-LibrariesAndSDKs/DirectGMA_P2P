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
#include <string>

#include <GL/glew.h>
#include "GLCopyApp.h"


// Size of the frames
unsigned int g_uiWidth  = 1280;
unsigned int g_uiHeight =  720;


LRESULT CALLBACK    WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


using namespace std;

  

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEX      wndclass;
    const LPCSTR    cClassName  = "OGL";
    const LPCSTR    cWindowName = "GPU P2P Copy";

    // Register WindowClass
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_OWNDC;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = (HINSTANCE)GetModuleHandle(NULL);
    wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION); 
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = NULL;
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = cClassName;
    wndclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wndclass))
        return WM_QUIT;

    GLCopyApp* pApp = new GLCopyApp;

	std::string errorMessageFromInit = "";
    if (!pApp->init(g_uiWidth, g_uiHeight, cClassName, errorMessageFromInit))
    {
		MessageBoxA(NULL,errorMessageFromInit.c_str(),"DirectGMA",NULL);
        return WM_QUIT;
    }

    if (pApp->start(true))
    {
        // Run message loop
        MSG	    Msg;

        bool bDone = !pApp->isRunning();

        while (!bDone)
        {
            bDone = !pApp->isRunning();

            if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
			{
				if (Msg.message == WM_QUIT)
				{
					bDone = true;
				}
				else
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
			}
            Sleep(100); // sleep since we do not want to spend too many cycles in the window message management
        }

        pApp->stop();
    }

    delete pApp;

    return WM_QUIT;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int nLastx = 0;
    static int nLasty = 0;

    switch (uMsg)
    {
        char c;

        case WM_CHAR:
            c = (char)wParam;

            switch (c)
            {
            case VK_ESCAPE:
                PostQuitMessage(0);
		        break;
            }
            return 0;

        case WM_CREATE:
            return 0;

        case WM_SIZE:
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

