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

struct PatchTess
{
	float EdgeTess[4] : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};


struct HullOut
{
	float3 PosL: POSITION;
};

struct VertexOut
{
	// Local position
	float3 PosL  : POSITION;

	// Normal's local position
	float3 NormalL: NORMAL;

	// Texture coordinates
	float2 TexC : TEXCOORD;
};

static const float PI = 3.14159265f;

// height function y = 0.5*(sin(0.2x)x + cos(0.2z)z)

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosL = vin.PosL;
	vout.NormalL = vin.NormalL;
	vout.TexC = vin.TexC;
}


struct HullOut
{
	// Local position
	float3 PosL  : POSITION;

	// Normal's local position
	float3 NormalL: NORMAL;

	// Texture coordinates
	float2 TexC : TEXCOORD;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	float3 centerL = 0.25f*(patch[0].PosL + 
							patch[1].PosL + 
							patch[2].PosL + 
							patch[3].PosL);

	float3 centerW = mul(float4(centerL, 1.0f), cbWorld).xyz;
	float d = distance(centerW, cbCameraPosW);
	

	const float d0 = 20.0f;
	const float d1 = 100.0f;
	float tess = 64 * saturate((d1 - d) / (d1 - d0));

	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	pt.EdgeTess[3] = tess;

	pt.InsideTess[0] = tess;
	pt.InsideTess[1] = tess;

	return pt;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
HullOut HS(InputPatch<VertexOut, 4> p,
	uint i : SV_OutputControlPointID,
	uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	hout.PosL = p[i].PosL;
	hout.NormalL = p[i].NormalL;
	hout.TexC = p[i].TexC;
	return hout;
}

struct DomainOut
{
	float4 PosH  : SV_POSITION;
	float3 PosW : POSITION;

	// Normal's local position
	float3 NormalW: NORMAL;

	// Texture coordinates
	float2 TexC : TEXCOORD;
}


[domain("quad")]
DomainOut DS(PatchTess patchTess,
	float2 uv: SV_DomainLocation,
	const OutputPatch<HullOut, 4> quad)
{
	DomainOut dout;
	float3 pos1 = lerp(quad[0].PosL, quad[1].PosL, uv.x);
	float3 pos2 = lerp(quad[2].PosL, quad[3].PosL, uv.x);
	float3 pos = lerp(pos1, pos2, uv.y);
	pos.y = 0.5*(sin(pos.x*0.2)*pos.x + cos(pos.z*0.2)*pos.z);

	dout.PosW = mul(pos, (float3x3)cbWorld);
	dout.PosH = mul(float4(pos, 1.0f), cbWorldViewProj);

	float3 uv1 = lerp(quad[0].TexC, quad[1].TexC, uv.x);
	float3 uv2 = lerp(quad[2].TexC, quad[3].TexC, uv.x);
	dout.TexC = lerp(uv1, uv2, uv.y);

	float3 normal1 = lerp(quad[0].NormalL, quad[1].NormaL, uv.x);
	float3 normal2 = lerp(quad[2].NormalL, quad[3].NormaL, uv.x);
	dout.NormalW = mul(float4(lerp(normal1, normal2, uv.y), 1.0f), cbWorld);

	return dout;
}






float4 PS(DomainOut pin) : SV_Target
{
	float4 diffuseAlbedo = tDiffuseMap.Sample(sAnisotropicWrap, pin.TexC) * cbDiffuseAlbedo;

#ifdef ALPHA_TEST
	clip(diffuseAlbedo.a - 0.1f);
#endif 

#ifdef LIGHTING
	// Normalize normal because of possible uniform world transform
	pin.NormalW = normalize(pin.NormalW);
	
	// Vector from point to eye
	float3 toCameraW = normalize(cbCameraPosW - pin.PosW);

	// Ambient light
    float4 ambient = cbAmbientLight * diffuseAlbedo;

	const float shininess = 1.0f - cbRoughness;

    Material mat = { diffuseAlbedo, cbFresnelR0, shininess };

	float3 shadowFactor = 1.0f;
	// Compute diffuse and specular light
	float4 directLight = ComputeLighting(cbLights, mat,
		pin.PosW, pin.NormalW, toCameraW, shadowFactor, cbGameTime);



	// Final color = ambient + directLight (diffuse light + specular light)
	float4 litColor = ambient + directLight;
#else
	float4 litColor = diffuseAlbedo;
#endif

#ifdef FOG
	float distToCamera = distance(cbCameraPosW, pin.PosW);
	float fogDensity = saturate((distToCamera- cbFogStart) / cbFogRange);
	litColor = lerp(litColor, cbFogColor, fogDensity);
#endif 

    litColor.a = diffuseAlbedo.a;

	return litColor;
}


