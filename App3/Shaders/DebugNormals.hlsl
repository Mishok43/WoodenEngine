//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

// Number of direction lights

#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

// Number of point lights
#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 1
#endif

// Number of spot lights
#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 1
#endif

#include "LightingUtils.hlsl"

cbuffer cbObject: register(b0)
{
	// World matrix
	float4x4 cbWorld;

	// Texture transform matrix
    float4x4 cbTexTransform;

	// Object's life time
	float cbTime;

    int cbIsWater;

    int cbWaterFactor;
}

cbuffer cbMaterial: register(b1)

{	
	// Diffuse reflection factor
	float4 cbDiffuseAlbedo;

	float3 cbFresnelR0;
	
	float cbRoughness;

    float4x4 cbMatTransform;
}

Texture2D tDiffuseMap : register(t0);

SamplerState sPointWrap : register(s0);
SamplerState sPointClamp : register(s1);
SamplerState sLinearWrap : register(s2);
SamplerState sLinearClamp : register(s3);
SamplerState sAnisotropicWrap : register(s4);
SamplerState sAnisotropicClamp : register(s5);

cbuffer cbFrame: register(b2)
{
	// View matrix
	float4x4 cbView;

	// Proj matrix
	float4x4 cbProjMatrix;
	
	// View proj matrix
	float4x4 cbViewProj;

	// Lights in the frame
	Light cbLights[MaxLights];

	// World camera position
	float3 cbCameraPosW;

	// Game time
	float cbGameTime;

	// Game's ambient light 
	float4 cbAmbientLight;

	// Fog color
	float4 cbFogColor;

	// Distance from camera to fog start
	float cbFogStart;

	// Distance from fog start to fog end
	float cbFogRange;
};

struct VertexIn
{
	// Local position
	float3 PosL  : POSITION;

	// Normal's local position
	float3 NormalL: NORMAL;

	// Texture coordinates
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
	// Local position
	float3 PosL  : POSITION;

	// Normal's local position
	float3 NormalL: NORMAL;
};

struct GeoOut
{
	// Projected position
	float4 PosP : SV_POSITION;
};

static const float PI = 3.14159265f;

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosL = vin.PosL;
	vout.NormalL = vin.NormalL;
	return vout;
}


[maxvertexcount(12)]
void GS(triangle VertexOut gin[3],
		uint primID: SV_PrimitiveID,
		inout LineStream<GeoOut> lineStream
)
{   
    GeoOut gout[2];

    float3 MeanPosL = 0.33f * (gin[0].PosL + gin[1].PosL + gin[2].PosL);
    float3 MeanNormalL = 0.33f * (gin[0].NormalL + gin[1].NormalL + gin[2].NormalL);
    float4 FirstPosW = mul(float4(MeanPosL, 1.0f), cbWorld);
    float3 Shift = normalize(mul(MeanNormalL, (float3x3) cbWorld));
    float4 SecondPosW = float4(FirstPosW.xyz +  Shift, 1.0f);

    gout[0].PosP = mul(FirstPosW, cbViewProj);
    gout[1].PosP = mul(SecondPosW, cbViewProj);
    
    lineStream.Append(gout[0]);
    lineStream.Append(gout[1]);
}

float4 PS(GeoOut pin) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}


