//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 1
#endif
#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif
#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#include "LightingUtils.hlsl"

cbuffer cbObject: register(b0)
{
	float4x4 WorldMatrix;
	float4 Color;
	float Time;
}

cbuffer cbMaterial: register(b1)
{
	float4 DiffuzeAlbedo;
	float3 FresnelR0;
	float Roughness;
}

cbuffer cbFrame: register(b2)
{
	float4x4 ViewMatrix;
	float4x4 ProjMatrix;
	float4x4 ViewProjMatrix;

	Light gLights[MaxLights];
	float3 gEyePosW;
	float GameTime;

	float4 gAmbientLight;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float3 NormalL: NORMAL;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW: Normal;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	float3 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW = posW.xyz;

	vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

	vout.PosH = mul(posW, gViewProj);
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	pin.NormalW = normalize(pin.NormalW);
	
	float3 toEyeW = normalize(gEyePosW - pin.PosW);
	float4 ambient = gAmbientLight * gDiffuseAlbedo;

	const float shininess = 1.0f - gRoughness;
	Material mat = { gDiffuseAlbedo, gFresnelR0, shininess };

	float3 shadowFactor = 1.0f;
	float4 directLight = ComputeLighting(gLights, mat,
		pin.PosW, pin.NormalW, toEyeW, shadowFactor);

	float4 litColor = ambient + directLight;

	litColor.a = gDiffuseAlbedo.a;

	return litColor;
}


