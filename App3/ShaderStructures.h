#pragma once
#include "pch.h"

namespace App3
{

	using namespace DirectX;

	struct ConstData
	{
		XMFLOAT4X4 proj_matrix; // 64B
		XMFLOAT4X4 view_matrix; // 64B
		XMFLOAT4X4 view_proj_matrix; // 64B
		XMFLOAT4X4 world_view_proj_matrix; // 64B
	}; // 256B-256B=56B

	struct DataPerObject
	{
		XMFLOAT4X4 world_matrix; // 64B
	};

	struct VertexData
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};
}