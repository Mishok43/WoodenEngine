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

	// Texture coordinates
	float2 TexC : TEXCOORD;
};

struct GeoOut
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
	VertexOut vout;
	vout.PosL = vin.PosL;
	vout.NormalL = vin.NormaL;
	vout.TexC = vin.TexC;
	return vout;
}

void Subdivide(VertexOut inVerts[3],
			   out VertexOut outVerts[6])
{
	VertexOut NewVerts[3];
	NewVerts[0].PosL = 0.5f*(inVerts[0].PosL + inVerts[1].PosL);
	NewVerts[1].PosL = 0.5f*(inVerts[1].PosL + inVerts[2].PosL);
	NewVerts[2].PosL = 0.5f*(inVerts[0].PosL + inVerts[2].PosL);

	NewVerts[0].PosL = normalize(NewVerts[0].PosL);
	NewVerts[1].PosL = normalize(NewVerts[1].PosL);
	NewVerts[2].PosL = normalize(NewVerts[2].PosL);

	NewVerts[0].NormalL = NewVerts[0].PosL;
	NewVerts[1].NormalL = NewVerts[1].PosL;
	NewVerts[2].NormalL = NewVerts[2].PosL;

	NewVerts[0].TexC = 0.5f*(inVerts[0].TexC + inVerts[1].TexC);
	NewVerts[1].TexC = 0.5f*(inVerts[1].TexC + inVerts[2].TexC);
	NewVerts[2].TexC = 0.5f*(inVerts[0].TexC + inVerts[2].TexC);


	outVerts[0] = inVerts[0];
	outVerts[1] = NewVerts[0];
	outVerts[2] = NewVerts[2];
	outVerts[3] = NewVerts[1];
	outVerts[4] = inVerts[2];
	outVerts[5] = inVerts[1];
}


[maxvertexcout(12)]
void GS(triangle VertexOut gin[3],
		uint primID: SV_PrimitiveID,
		inout TriangleStream<GeoOut> triStream
)
{
	VertexOut v[6];
	Subdivide(gin, v);

	GeoOut gout[6];
	[unroll]
	for (int i = 0; i < 6; ++i)
	{
		gout[i].PosW = mul(float4(v[i].PosL, 1.0f), cbWorld);
		gout[i].NormalW = mul(v[i].NormalL, (float3x3)cbWorld); // normalize in PS because optimization
		gout[i].PosP = mul(gout[i].PosW, cbViewProj);
		gout[i].TexC = mul(mul(float4(v[i].TexC, 0.0f, 1.0f), cbTexTransform), cbMatTransform).xy;
	}

	[unroll]
	for (int i = 0; i < 5; ++i)
	{
		triStream.Append(gout[i]);
	}

	triStream.RestartStrip();

	triStream.Append(gout[1]);
	triStream.Append(gout[5]);
	triStream.Append(gout[3]);
}

float4 PS(GeoOut pin) : SV_Target
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


