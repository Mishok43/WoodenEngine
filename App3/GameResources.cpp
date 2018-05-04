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

		uint64 NumVertices = 0;
		uint64 NumIndices = 0;
		for (auto MeshData : MeshesData)
		{
			NumVertices += MeshData.Vertices.size();
			NumIndices += MeshData.Indices.size();
		}

		VertexData.reserve(NumVertices);
		IndexData.reserve(NumIndices);
		
		auto VertexDataIter = VertexData.cend();
		auto IndexDataIter = IndexData.cend();
		// Adding vertices and indices of every mesh to common arrays
		for (auto MeshData : MeshesData)
		{
			MeshData.VertexBegin = VertexData.size();

			const auto VertexDataNewSize = VertexData.size() + MeshData.Vertices.size();

			for (auto i = MeshData.VertexBegin; i < VertexDataNewSize; i++)
			{
				const SVertexData Vertex = { MeshData.Vertices[i - MeshData.VertexBegin].Position };
				VertexDataIter = VertexData.insert(VertexDataIter, std::move(Vertex));
				VertexDataIter++;
			}

			MeshData.IndexBegin = IndexData.size();

			const auto IndexDataNewSize = IndexData.size() + MeshData.Indices.size();
			
			for (auto i = MeshData.IndexBegin; i < IndexDataNewSize; i++)
			{
				const auto Index = MeshData.Indices[i - MeshData.IndexBegin];
				IndexDataIter = IndexData.insert(IndexDataIter, std::move(Index));
				IndexDataIter++;
			}

			StaticMeshesData.insert(std::make_pair(MeshData.Name, MeshData));
		}

		// Create vertex buffer 
		const auto VertexBufferSize = sizeof(SVertexData)*VertexData.size();

		VertexBuffer = DX::CreateBuffer(Device, CMDList, VertexBufferSize, VertexData.data(), VertexUploadBuffer);

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Setup vertex buffer view
		VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		VertexBufferView.SizeInBytes = VertexBufferSize;
		VertexBufferView.StrideInBytes = sizeof(SVertexData);

		// Create index buffer
		const auto IndexBufferSize = 2*IndexData.size();
		IndexBuffer = DX::CreateBuffer(Device, CMDList, IndexBufferSize, IndexData.data(), IndexUploadBuffer);

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