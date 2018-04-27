#pragma once
#include "pch.h"

namespace DirectXEngine
{

	using namespace DirectX;

	struct SConstData
	{
		XMFLOAT4X4 ViewMatrix;
		XMFLOAT4X4 ProjMatrix;
		XMFLOAT4X4 ViewProjMatrix;

		float GameTime;
	}; // 256B-256B=56B

	struct SObjectData
	{
		XMFLOAT4X4 WorldMatrix; // 64B
		float Time;
	};

	struct SVertexData
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};
}