#pragma once
#include <pch.h>

namespace WoodenEngine
{
	using namespace DirectX;
	struct SVertexBillboardData;

	struct FBillboardsData
	{
		FBillboardsData() = default;

		FBillboardsData(const FBillboardsData& MeshData) = delete;
		FBillboardsData& operator=(const FBillboardsData& MeshData) = delete;

		// DX12 Buffer of vertices of static meshes
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> VertexUploadBuffer;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		std::vector<SVertexBillboardData> VerticesData;
	};
}