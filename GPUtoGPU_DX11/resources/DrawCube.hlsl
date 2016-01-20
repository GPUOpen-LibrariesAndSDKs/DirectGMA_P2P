
//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

cbuffer cbPerFrame : register( b0 )
{
	matrix		g_mWorldViewProjection;
	matrix		g_mWorldView;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float2 vTexture		: TEXTURE;
	float3 vNormalWS    : NORMAL;
	float3 vColor	    : COLOR;
};

struct PS_INPUT
{
    float4 vPosition	: SV_POSITION;
	float2 vTexture		: TEXTURE0;
	float4 vPositionVS	: TEXTURE1;
	float3 vNormalVS    : NORMAL;
	float3 vColor	    : COLOR;
};

struct PS_RenderOutput
{
    float4 f4Color      : SV_Target0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VSMain( VS_INPUT Input )
{
	PS_INPUT Output;

	Output.vPosition = mul( float4(Input.vPosition,1.0f), g_mWorldViewProjection );
	Output.vPositionVS = mul( float4(Input.vPosition,1.0f), g_mWorldView );
	Output.vColor = Input.vColor;
	Output.vTexture = Input.vTexture;
	Output.vNormalVS = mul( float4(Input.vNormalWS,0.0f), g_mWorldView );

	return Output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

PS_RenderOutput PSMain( PS_INPUT I ) 
{
    PS_RenderOutput O;


	

	float4 vTexColor = txDiffuse.Sample( samLinear, I.vTexture.xy ) ;
	float4 DiffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 AmbientColor = float4(0.2f, 0.2f, 0.2f, 1.0f);
	float4 SpecularColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float3 LightDir = float3(+0.707107f, 0, +0.707107f);



	float4 vBaseColor = lerp(float4(0.4f, 0.4f, 0.4f, 1.0f), vTexColor, vTexColor.a); 
	float3 vN = normalize(I.vNormalVS);
	float NdotL = max(dot(vN, -LightDir.xyz), 0.0);
	float3 vR    = reflect(LightDir.xyz, vN);  
	float Spec = pow(max(dot(normalize(vR.xyz), normalize(-I.vPositionVS.xyz)), 0.0), 6.0);
	O.f4Color = vBaseColor * DiffuseColor   * NdotL + vBaseColor * AmbientColor + Spec * SpecularColor ;

    
	//O.f4Color = float4(I.vColor.xyz,1);
	//O.f4Color = float4(I.vTexture.xy,1.0,1.0);
	//O.f4Color = txDiffuse.Sample( samLinear, I.vTexture.xy ) * float4(I.vColor,1);
    //O.f4Color = float4((I.vNormal + 1.0f) / 2.0f,1);


    return O;
}
