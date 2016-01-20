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

#include "ADL/adl_sdk.h"



typedef int ( *ADL_MAIN_CONTROL_CREATE )            (ADL_MAIN_MALLOC_CALLBACK, int );
typedef int ( *ADL_MAIN_CONTROL_DESTROY )           ();
typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET )   ( int* );
typedef int ( *ADL_ADAPTER_ACTIVE_GET )             ( int, int* );
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET )        ( LPAdapterInfo, int );
typedef int ( *ADL_ADAPTER_ACTIVE_GET )             ( int, int* );
typedef int ( *ADL_DISPLAY_DISPLAYINFO_GET )        ( int, int *, ADLDisplayInfo **, int );
typedef int ( *ADL_DISPLAY_PROPERTY_GET )           ( int, int, ADLDisplayProperty* );
typedef int ( *ADL_DISPLAY_MODES_GET )              (int, int, int*, ADLMode**);
typedef int ( *ADL_DISPLAY_POSITION_GET )           (int, int, int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);
typedef int ( *ADL_DISPLAY_SIZE_GET )               (int, int, int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);


typedef struct
{
    void*                               hDLL;
    ADL_MAIN_CONTROL_CREATE             ADL_Main_Control_Create;
    ADL_MAIN_CONTROL_DESTROY            ADL_Main_Control_Destroy;
    ADL_ADAPTER_NUMBEROFADAPTERS_GET    ADL_Adapter_NumberOfAdapters_Get;
    ADL_ADAPTER_ACTIVE_GET              ADL_Adapter_Active_Get;
    ADL_ADAPTER_ADAPTERINFO_GET         ADL_Adapter_AdapterInfo_Get;
    ADL_DISPLAY_DISPLAYINFO_GET         ADL_Display_DisplayInfo_Get;
    ADL_DISPLAY_PROPERTY_GET		    ADL_Display_Property_Get;
    ADL_DISPLAY_MODES_GET               ADL_Display_Modes_Get;
    ADL_DISPLAY_POSITION_GET		    ADL_Display_Position_Get;
    ADL_DISPLAY_SIZE_GET                ADL_Display_Size_Get;
} ADLFunctions;

