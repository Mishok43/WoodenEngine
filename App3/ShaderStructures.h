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
		XMFLOAT4X4 ViewMatrix;
		
		XMFLOAT4X4 ProjMatrix;
		
		XMFLOAT4X4 ViewProjMatrix;

		XMFLOAT3 EyePosition;
		float GameTime;

		XMFLOAT4 AmbientLight;
		
		// 16 - max lights amount
		SLightData Lights[16];

	};

	struct SMaterialData
	{
		XMFLOAT4 DiffuzeAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };

		// For specular reflection
		XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };

		float Roughness = 0.25f;
	};

	struct SObjectData
	{
		XMFLOAT4X4 WorldMatrix; // 64B
		float Time; // 4B
	}; // 84B

	struct SVertexData
	{
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
	};
}