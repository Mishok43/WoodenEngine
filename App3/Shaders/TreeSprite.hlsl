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

Texture2DArray gTreeMapArray : register(t0);

SamplerState sPointWrap : register(s0);
SamplerState sPointClamp : register(s1);
SamplerState sLinearWrap : register(s2);
SamplerState sLinearClamp : register(s3);
SamplerState sAnisotropicWrap : register(s4);
SamplerState sAnisotropicClamp : register(s5);


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
	float3 CenterW  : POSITION;

	// Normal's local position
	float2 SizeW: SIZE;
};

struct VertexOut
{
	// Local position
	float3 CenterW  : POSITION;

	// Normal's local position
	float2 SizeW: SIZE;
};

struct GeoOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW: NORMAL;
	float2 TexC: TEXCOORD;
	uint   PrimID  : SV_PrimitiveID;
};

static const float PI = 3.14159265f;


VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.CenterW = vin.CenterW;
	vout.SizeW = vin.SizeW;
	return vout;
}




[maxvertexcount(4)]
void GS(point VertexOut gin[1],
		uint primID: SV_PrimitiveID,
		inout TriangleStream<GeoOut> triStream)
{
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = cbCameraPosW - gin[0].CenterW;
	look.y = 0.0f;
	look = normalize(look);
	float3 right = cross(up, look);

	float halfWidth = 0.5f*gin[0].SizeW.x;
	float halfHeight = 0.5f*gin[0].SizeW.y;
	float4 v[4];
	v[0] = float4(gin[0].CenterW + halfWidth * right -
				  halfHeight * up, 1.0f);
	v[1] = float4(gin[0].CenterW + halfWidth * right +
				  halfHeight * up, 1.0f);
	v[2] = float4(gin[0].CenterW - halfWidth * right -
				  halfHeight * up, 1.0f);
	v[3] = float4(gin[0].CenterW - halfWidth * right +
				  halfHeight * up, 1.0f);

	float2 texC[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	GeoOut gout;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		gout.PosH = mul(v[i], cbViewProj);
		gout.PosW = v[i].xyz;
		gout.NormalW = look;
		gout.TexC = texC[i];
		gout.PrimID = primID;
		triStream.Append(gout);

	}

}


float4 PS(GeoOut pin) : SV_Target
{
	float3 uvw = float3(pin.TexC, pin.PrimID % 3);
	float4 diffuseAlbedo = gTreeMapArray.Sample(
		sAnisotropicWrap, uvw) * cbDiffuseAlbedo;

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


