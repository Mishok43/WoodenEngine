#pragma once

#include "GameResources.h"
#include "Common/DirectXHelper.h"

namespace WoodenEngine
{
	FGameResources::FGameResources()
	{
	}

	FGameResources::FGameResources(ComPtr<ID3D12Device> Device):
		Device(Device)
	{
	}

	FGameResources::~FGameResources()
	{
	}

	void FGameResources::LoadMeshes(
		const std::vector<FMeshData>& MeshesData,
		ComPtr<ID3D12GraphicsCommandList> CMDList)
	{
		assert(MeshesData.size() != 0);

		// Adding vertices and indices of every mesh to common arrays
		for (auto MeshData : MeshesData)
		{
			MeshData.VertexBegin = VertexData.size();

			const auto VertexDataNewSize = VertexData.size() + MeshData.Vertices.size();
			VertexData.reserve(VertexDataNewSize*sizeof(SVertexData));

			for (int i = MeshData.VertexBegin; i < VertexDataNewSize; i++)
			{
				VertexData[i] = { MeshData.Vertices[i - MeshData.VertexBegin].Position };
			}

			MeshData.IndexBegin = IndexData.size();

			const auto IndexDataNewSize = IndexData.size() + MeshData.Indices.size();
			VertexData.reserve(IndexDataNewSize* sizeof(uint16));
			for (int i = MeshData.IndexBegin; i < IndexDataNewSize; i++)
			{
				IndexData[i] =  MeshData.Indices[i - MeshData.VertexBegin];
			}

			StaticMeshesData.insert(std::make_pair(MeshData.Name, MeshData));
		}

		// Create vertex buffer 
		const auto VertexBufferSize = sizeof(VertexData);

		VertexBuffer = DX::CreateBuffer(Device, CMDList, VertexBufferSize, VertexData.data());

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Setup vertex buffer view
		VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		VertexBufferView.SizeInBytes = VertexBufferSize;
		VertexBufferView.StrideInBytes = sizeof(SVertexData);

		// Create index buffer
		const auto IndexBufferSize = sizeof(IndexData);
		IndexBuffer = DX::CreateBuffer(Device, CMDList, IndexBufferSize, IndexData.data());

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		// Setup index buffer view
		IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
		IndexBufferView.SizeInBytes = IndexBufferSize;
		IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	void FGameResources::SetDevice(ComPtr<ID3D12Device> Device)
	{
		assert(Device != nullptr);
		this->Device = Device;
	}

	FMeshData FGameResources::GetMeshData(const std::string& MeshName) const
	{
		auto MeshDataIt = StaticMeshesData.find(MeshName);
		assert(MeshDataIt != StaticMeshesData.end());
		return MeshDataIt->second;
	}

	D3D12_VERTEX_BUFFER_VIEW FGameResources::GetVertexBufferView() const noexcept
	{
		return VertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW FGameResources::GetIndexBufferView() const noexcept
	{
		return IndexBufferView;
	}
}