#pragma once
#include "pch.h"

namespace WoodenEngine
{

	using namespace DirectX;

	struct SFrameData
	{
		XMFLOAT4X4 ViewMatrix;
		XMFLOAT4X4 ProjMatrix;
		XMFLOAT4X4 ViewProjMatrix;

		float GameTime;
	}; // 256B-256B=56B

	struct SObjectData
	{
		XMFLOAT4X4 WorldMatrix; // 64B
		XMFLOAT4 Color; // 16B
		float Time; // 4B
	}; // 84B

	struct SVertexData
	{
		XMFLOAT3 Position;
	};
}