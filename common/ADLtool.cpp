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




#include "ADLtool.h"


#if defined(WIN32)
    #define GETPROC GetProcAddress
#elif defined (LINUX)
    #define GETPROC dlsym
#endif


std::vector<ADLtool::DisplayData*>  ADLtool::m_DisplayInfo;
ADLtool::ADLFunctions ADLtool::g_AdlCalls = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int ADLtool::m_uiNumGPU = 0;


// Memory Allocation function needed for ADL
void* __stdcall ADLtool::ADL_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

// Memory Free function needed for ADL
void __stdcall ADLtool::ADL_Free ( void* lpBuffer )
{
    if ( NULL != lpBuffer )
    {
        free ( lpBuffer );
        lpBuffer = NULL;
    }
}

unsigned int ADLtool::getNumDisplaysOnGPU(unsigned int uiGPU)
{
    unsigned int uiNumDsp = 0;

	std::vector<DisplayData*>::iterator itr;
	// should only loop twice
    for (itr = m_DisplayInfo.begin(); itr != m_DisplayInfo.end(); ++itr)
    {
        if ((*itr)->uiGPUId == uiGPU)
        {
            ++uiNumDsp;
        }
    }

    return uiNumDsp;
}

unsigned int ADLtool::getDisplayOnGPU(unsigned int uiGPU, unsigned int n)
{
    unsigned int uiCurrentDsp = 0;

   std::vector<DisplayData*>::iterator itr;

    for (itr = m_DisplayInfo.begin(); itr != m_DisplayInfo.end(); ++itr)
    {
        if ((*itr)->uiGPUId == uiGPU)
        {
            if (uiCurrentDsp == n)
            {
                return (*itr)->uiDisplayId;
            }

            ++uiCurrentDsp;
        }
    }

    return 0;
}

ADLtool::DisplayData* ADLtool::getDisplayData(unsigned int uiDspId)
{
	std::vector<DisplayData*>::iterator itr;

    for (itr = m_DisplayInfo.begin(); itr != m_DisplayInfo.end(); ++itr)
    {
        if ((*itr)->uiDisplayId == uiDspId)
        {
            return (*itr);
        }
    }

    return NULL;
}



bool ADLtool::setupADL()
{
    // check if ADL was already loaded
    if (g_AdlCalls.hDLL)
    {
        return true;
    }

#ifdef WIN32
    g_AdlCalls.hDLL = (void*)LoadLibraryW(L"atiadlxx.dll");

	if (g_AdlCalls.hDLL == NULL)
       g_AdlCalls.hDLL = (void*)LoadLibraryW(L"atiadlxy.dll");
#endif

#ifdef LINUX
    g_AdlCalls.hDLL = dlopen("libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
    
    if (g_AdlCalls.hDLL == NULL)
        g_AdlCalls.hDLL = dlopen("libatiadlxy.so", RTLD_LAZY|RTLD_GLOBAL);
#endif

	if (!g_AdlCalls.hDLL)
		return false;

	// Get proc address of needed ADL functions
	g_AdlCalls.ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Main_Control_Create");
	if (!g_AdlCalls.ADL_Main_Control_Create)
		return false;

	g_AdlCalls.ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GETPROC((HMODULE)g_AdlCalls.hDLL, "ADL_Main_Control_Destroy");
	if (!g_AdlCalls.ADL_Main_Control_Destroy)
		return false;

	g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Adapter_NumberOfAdapters_Get");
	if (!g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get)
		return false;

	g_AdlCalls.ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Adapter_AdapterInfo_Get");
	if (!g_AdlCalls.ADL_Adapter_AdapterInfo_Get)
		return false;

	g_AdlCalls.ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Display_DisplayInfo_Get");
	if (!g_AdlCalls.ADL_Display_DisplayInfo_Get)
		return false;

	g_AdlCalls.ADL_Adapter_Active_Get = (ADL_ADAPTER_ACTIVE_GET)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Adapter_Active_Get");
	if (!g_AdlCalls.ADL_Adapter_Active_Get)
		return false;

	g_AdlCalls.ADL_Display_Position_Get = (ADL_DISPLAY_POSITION_GET)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Display_Position_Get");
	if (!g_AdlCalls.ADL_Display_Position_Get)
		return false;

    g_AdlCalls.ADL_Display_Size_Get = (ADL_DISPLAY_POSITION_GET)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Display_Size_Get");
	if (!g_AdlCalls.ADL_Display_Size_Get)
		return false;
 
    g_AdlCalls.ADL_Display_Modes_Get = (ADL_DISPLAY_MODES_GET)GETPROC((HMODULE)g_AdlCalls.hDLL, "ADL_Display_Modes_Get");
    if (!g_AdlCalls.ADL_Display_Modes_Get)
        return false;


	g_AdlCalls.ADL_Display_Property_Get = (ADL_DISPLAY_PROPERTY_GET)GETPROC((HMODULE)g_AdlCalls.hDLL,"ADL_Display_Property_Get");
	if (!g_AdlCalls.ADL_Display_Property_Get)
		return false;
 
	// Init ADL
	if (g_AdlCalls.ADL_Main_Control_Create(ADL_Alloc, 0) != ADL_OK)
		return false;

	return true;
}

bool ADLtool::InitClass()
{
	int				nNumDisplays = 0;
	int				nNumAdapters = 0;
    int             nCurrentBusNumber = 0;
	LPAdapterInfo   pAdapterInfo = NULL;
    unsigned int    uiCurrentGPUId     = 0;
    unsigned int    uiCurrentDisplayId = 0;

	m_uiNumGPU=0;

    // load all required ADL functions
    if (!setupADL())
        return false;

    // Determine how many adapters and displays are in the system
	g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get(&nNumAdapters);

	if (nNumAdapters > 0)
	{
		pAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * nNumAdapters );
        memset ( pAdapterInfo,'\0', sizeof (AdapterInfo) * nNumAdapters );
	}

	g_AdlCalls.ADL_Adapter_AdapterInfo_Get (pAdapterInfo, sizeof (AdapterInfo) * nNumAdapters); // not getting correct pAdapterInfo

    // Loop through all adapters 
	for (int i = 0; i < nNumAdapters; ++i)
	{
		int				nAdapterIdx; 
		int				nAdapterStatus;
		
		nAdapterIdx = pAdapterInfo[i].iAdapterIndex; // always return to 0 instead of 0-17

		g_AdlCalls.ADL_Adapter_Active_Get(nAdapterIdx, &nAdapterStatus);

		if (nAdapterStatus)
		{
			LPADLDisplayInfo	pDisplayInfo = NULL;

			g_AdlCalls.ADL_Display_DisplayInfo_Get(nAdapterIdx, &nNumDisplays, &pDisplayInfo, 0);

			for (int j = 0; j < nNumDisplays; ++j)
			{
				// check if display is connected and mapped
				if (pDisplayInfo[j].iDisplayInfoValue & ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED)
                {
					// check if display is mapped on adapter
					if (pDisplayInfo[j].iDisplayInfoValue & ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED && pDisplayInfo[j].displayID.iDisplayLogicalAdapterIndex == nAdapterIdx)
					{
                        if (nCurrentBusNumber == 0)
                        {
                            // Found first GPU in the system
                            ++m_uiNumGPU;
                            nCurrentBusNumber = pAdapterInfo[nAdapterIdx].iBusNumber;
                        }
                        else if (nCurrentBusNumber != pAdapterInfo[nAdapterIdx].iBusNumber)
                        {
                            // found new GPU
                            ++m_uiNumGPU;
                            ++uiCurrentGPUId;
                            nCurrentBusNumber = pAdapterInfo[nAdapterIdx].iBusNumber;
                        }

                        ++uiCurrentDisplayId;

                        

                        // Found first mapped display
                        DisplayData* pDsp = new DisplayData;
                        
                        pDsp->uiGPUId               = uiCurrentGPUId;
                        pDsp->uiDisplayId           = uiCurrentDisplayId;
                        pDsp->uiDisplayLogicalId    = pDisplayInfo[j].displayID.iDisplayLogicalIndex;
                        pDsp->strDisplayname        = pAdapterInfo[i].strDisplayName;
                        pDsp->iOriginX             = 0;
                        pDsp->iOriginY             = 0;
                        pDsp->uiWidth               = 0;
                        pDsp->uiHeight              = 0;

#ifdef WIN32
                        //DEVMODE DevMode;
						DEVMODEA DevMode;
						EnumDisplaySettingsA(pAdapterInfo[i].strDisplayName, ENUM_CURRENT_SETTINGS, &DevMode);

                        pDsp->iOriginX             = DevMode.dmPosition.x;
                        pDsp->iOriginY             = DevMode.dmPosition.y;
                        pDsp->uiWidth               = DevMode.dmPelsWidth;
                        pDsp->uiHeight              = DevMode.dmPelsHeight;
#endif
					

                        m_DisplayInfo.push_back(pDsp);
                    }
                }
            }
        }
    }



    return true;
}

void ADLtool::CleanClass()
{
	std::vector<DisplayData*>::iterator itr;
    for (itr = m_DisplayInfo.begin(); itr != m_DisplayInfo.end(); ++itr)
    {
        if ((*itr))
        {
            delete (*itr); *itr=NULL;
        }
    }
	m_DisplayInfo.clear();

	    
	if (g_AdlCalls.hDLL)
    {
        g_AdlCalls.ADL_Main_Control_Destroy();
    }

}

