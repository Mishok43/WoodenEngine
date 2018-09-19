#pragma once
#include "pch.h"
#include "MathHelper.h"

namespace WoodenEngine
{

	using namespace DirectX;
	
	struct SLightData
	{
		// Light color
		XMFLOAT3 Strength;

		// Point/spot only
		float FalloffStart;

		// Directional/spot only
		XMFLOAT3 Direction;

		// Point/spot only
		float FalloffEnd;

		// Point/Spot light only
		XMFLOAT3 Position;

		// Spot only
		float SpotPower;
	};

	struct SFrameData
	{
		// Matrix converts world coordinates to view coordinates
		XMFLOAT4X4 ViewMatrix;
		
		// Matrix converts view coordinates to projected coordinates
		XMFLOAT4X4 ProjMatrix;
		
		// Matrix converts world coordinates to projected coordinates
		XMFLOAT4X4 ViewProjMatrix;

		// 16 - max lights amount
		SLightData Lights[16];

		// Camera's world coordinates
		XMFLOAT3 CameraPosition;
		
		float GameTime;

		// 'Whole-scene' light
		XMFLOAT4 AmbientLight = { 0.35f, 0.35f, 0.35f, 1.0f };

		// Fog color

		XMFLOAT4 FogColor = { 0.5f, 0.5f, 0.5f, 0.0f };
		// Distance from camera to fog start
		float FogStart = 20;

		// Distance from fog start to fog end
		float FogRange = 100;
	};

	struct SMaterialData
	{
		XMFLOAT4 DiffuzeAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };

		// For specular reflection
		XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };

		float Roughness = 0.25f;

		// Tex UV Coordinates transform matrix
		XMFLOAT4X4 MaterialTransform;
	};

	struct SObjectData
	{
		// Matrix for converting local coordinates to world space
		XMFLOAT4X4 WorldMatrix; 

		// Matrix for transforming material (UV-coordinates)
		XMFLOAT4X4 MaterialTransform;
		
		// Object's life time
		float Time; 

		// Cruch. 
		int IsWater = false;

		int WaterFactor = 1.0f;
	}; // 84B

	struct SVertexData
	{
		// Local coordinates
		XMFLOAT3 Position;

		// Local normal's direction
		XMFLOAT3 Normal;

		// UV-Coordinates
		XMFLOAT2 TexC;
	};

	struct SVertexBillboardData 
	{
		XMFLOAT3 Position;
		XMFLOAT2 Size;
	};
}