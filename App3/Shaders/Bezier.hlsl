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
	return vout;
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

PatchTess ConstantHS(InputPatch<VertexOut, 16> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	pt.EdgeTess[0] = 25;
	pt.EdgeTess[1] = 25;
	pt.EdgeTess[2] = 25;
	pt.EdgeTess[3] = 25;

	pt.InsideTess[0] = 25;
	pt.InsideTess[1] = 25;

	return pt;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 16> p,
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
};


float4 BernsteinBasis(float t)
{
	float invT = 1.0f - t;

	return float4(invT*invT*invT,
		3 * t*invT*invT,
		3 * t*t*invT,
		t*t*t);
}

float3 BezierSum(float4 uBasis, float4 vBasis, 
	const OutputPatch<HullOut, 16> bezpatch) 
{
	float3 sum = float3(0.0f, 0.0f, 0.0f);
	sum = vBasis.x*(
		uBasis.x*bezpatch[0].PosL +
		uBasis.y*bezpatch[1].PosL +
		uBasis.z*bezpatch[2].PosL +
		uBasis.w*bezpatch[3].PosL
		);
	sum += vBasis.y*(
		uBasis.x*bezpatch[4].PosL +
		uBasis.y*bezpatch[5].PosL +
		uBasis.z*bezpatch[6].PosL +
		uBasis.w*bezpatch[7].PosL
		);
	sum += vBasis.z*(
		uBasis.x*bezpatch[8].PosL +
		uBasis.y*bezpatch[9].PosL +
		uBasis.z*bezpatch[10].PosL +
		uBasis.w*bezpatch[11].PosL
		);
	sum += vBasis.w*(
		uBasis.x*bezpatch[12].PosL +
		uBasis.y*bezpatch[13].PosL +
		uBasis.z*bezpatch[14].PosL +
		uBasis.w*bezpatch[15].PosL
		);
	return sum;
}


[domain("quad")]
DomainOut DS(PatchTess patchTess,
	float2 uv: SV_DomainLocation,
	const OutputPatch<HullOut, 16> quad)
{
	DomainOut dout;
	
	float4 uBasis = BernsteinBasis(uv.x);
	float4 vBasis = BernsteinBasis(uv.y);

	float3 posL = BezierSum(uBasis, vBasis, quad);
	dout.PosW = mul(float4(posL, 1.0f), cbWorld).xyz;
	dout.PosH = mul(float4(dout.PosW, 1.0f), cbViewProj);

	return dout;
}


float4 PS(DomainOut pin) : SV_Target
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);

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


