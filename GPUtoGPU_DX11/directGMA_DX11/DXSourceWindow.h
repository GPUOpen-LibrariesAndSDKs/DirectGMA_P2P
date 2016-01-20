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

#include "DXwindow.h"



#include <D3dx9math.h>

class BufferQueue;


class DXSourceWindow : public DXwindow
{
public:

	DXSourceWindow();
	~DXSourceWindow();

	HRESULT Render(BufferQueue* pSyncBuffer,unsigned int uiTransferId);

	HRESULT InitSDItexture(const AmdDxLocalSDISurfaceList& surfaceInfo);

private:

	HRESULT CreateShaderResources();

	std::vector<ID3D11Texture2D*>  m_source_SDItexture;

	

	//background clear color
	float m_clearColorR;
	float m_clearColorG;
	float m_clearColorB;


	struct CUBE_CB
	{
		D3DXMATRIX f4x4WorldViewProjection; // World * View * Projection matrix 
		D3DXMATRIX f4x4WorldView; // World * View 
	};

	struct CUBE_Vertex1 
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 color;
	};


	ID3D11Buffer*                m_Cube_pcb ;  
	float m_Cube_angle;
	ID3D11VertexShader*          m_Cube_pVertexShader ;
	ID3D11PixelShader*           m_Cube_pPixelShader ;
	ID3D11InputLayout*           m_Cube_pVertexLayout11 ;
	ID3D11Buffer*                m_Cube_pVertexBuffer ;
	ID3D11Buffer*                m_Cube_pIndexBuffer ;
	ID3D11RasterizerState*       m_Cube_pRasterizerStateSolid ;
	ID3D11ShaderResourceView*    m_Cube_pTextureRV;
	ID3D11SamplerState*          m_Cube_pSamplerLinear;

};

