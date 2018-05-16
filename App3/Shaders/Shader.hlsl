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
#define NUM_SPOT_LIGHTS 0
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
	// Projected position
	float4 PosP : SV_POSITION;

	// World position
	float3 PosW : POSITION;

	// Normal's world position
	float3 NormalW: Normal;

	// Texture coordinates
    float2 TexC : TEXCOORD;
};

static const float PI = 3.14159265f;

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    if (cbIsWater > 0)
    {
        float sint = sin(cbTime / 15.0f);
		vin.PosL.y = sin(vin.PosL.x + vin.PosL.z) * sint*0.5f;
		vin.NormalL = float3(-sint * 0.5f*cos(vin.PosL.x), 1, -sint * 0.5f*cos(vin.PosL.z));
        vin.NormalL = normalize(vin.NormalL);
    }
    else
    {
        float sint = sin(cbTime / 15.0f);
        vin.PosL.y += cbWaterFactor * sint*0.5f;
    }

	// Compute world position
        float4 posW = mul(float4(vin.PosL, 1.0f), cbWorld);
        vout.PosW = posW.xyz;

	// Compute world normal
        vout.NormalW = mul(vin.NormalL, (float3x3) cbWorld);

	// Compute projected position
        vout.PosP = mul(posW, cbViewProj);
	
    // Apply texture tranformation for creating some effects
    // TexC - 2D coordinates, converts them to homogolenous space (z = 0 and w = 1.0f)
        vout.TexC = mul(mul(float4(vin.TexC, 0.0f, 1.0f), cbTexTransform), cbMatTransform).xy;
    
        return vout;
    }

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = tDiffuseMap.Sample(sAnisotropicWrap, pin.TexC) * cbDiffuseAlbedo;

#ifdef ALPHA_TEST
	clip(diffuseAlbedo.a - 0.1f);
#endif 

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

	float distToCamera = distance(cbCameraPosW, pin.PosW);

#ifdef FOG
	float fogDensity = saturate((distToCamera- cbFogStart) / cbFogRange);
	litColor = lerp(litColor, cbFogColor, fogDensity);
#endif 

    litColor.a = diffuseAlbedo.a;

	return litColor;
}


