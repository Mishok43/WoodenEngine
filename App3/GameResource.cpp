#pragma once
#include "Common/DirectXHelper.h"
#include "Common/DDSTextureLoader.h"
#include "GameResource.h"

namespace WoodenEngine
{
	FGameResource::FGameResource()
	{
	}

	FGameResource::FGameResource(ComPtr<ID3D12Device> Device):
		Device(Device)
	{
	}

	FGameResource::~FGameResource()
	{
	}

	void FGameResource::LoadMeshes(
		std::vector<std::unique_ptr<FMeshData>>&& MeshesData,
		ComPtr<ID3D12GraphicsCommandList> CMDList)
	{
		assert(MeshesData.size() != 0);

		uint64 NumVertices = 0;
		uint64 NumIndices = 0;
		
		for (auto iMesh = 0; iMesh < MeshesData.size(); ++iMesh)
		{
			auto MeshData = MeshesData[iMesh].get();

			NumVertices += MeshData->Vertices.size();
			NumIndices += MeshData->Indices.size();
		}

		VerticesData.reserve(NumVertices);
		IndicesData.reserve(NumIndices);
		
		auto VertexDataIter = VerticesData.cend();
		auto IndexDataIter = IndicesData.cend();
		// Adding vertices and indices of every mesh to common arrays
		for (auto iMesh = 0; iMesh < MeshesData.size(); ++iMesh)
		{
			auto MeshData = MeshesData[iMesh].get();
			MeshData->VertexBegin = VerticesData.size();

			const auto VertexDataNewSize = VerticesData.size() + MeshData->Vertices.size();

			for (auto i = MeshData->VertexBegin; i < VertexDataNewSize; i++)
			{
				const auto iVertex = i - MeshData->VertexBegin;

				const SVertexData Vertex = 
				{ MeshData->Vertices[iVertex].Position, MeshData->Vertices[iVertex].Normal, MeshData->Vertices[iVertex].TexC };
				VertexDataIter = VerticesData.insert(VertexDataIter, std::move(Vertex));
				VertexDataIter++;
			}

			MeshData->IndexBegin = IndicesData.size();

			const auto IndexDataNewSize = IndicesData.size() + MeshData->Indices.size();
			
			for (auto i = MeshData->IndexBegin; i < IndexDataNewSize; i++)
			{
				const auto Index = MeshData->Indices[i - MeshData->IndexBegin];
				IndexDataIter = IndicesData.insert(IndexDataIter, std::move(Index));
				IndexDataIter++;
			}

			StaticMeshesData.insert(std::make_pair(MeshData->Name, std::move(MeshesData[iMesh])));
		}

		// Create vertex buffer 
		const auto VertexBufferSize = sizeof(SVertexData)*VerticesData.size();

		VertexBuffer = DX::CreateBuffer(Device, CMDList, VertexBufferSize, VerticesData.data(), VertexUploadBuffer);

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Setup vertex buffer view
		VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		VertexBufferView.SizeInBytes = VertexBufferSize;
		VertexBufferView.StrideInBytes = sizeof(SVertexData);

		// Create index buffer
		const auto IndexBufferSize = 2*IndicesData.size();
		IndexBuffer = DX::CreateBuffer(Device, CMDList, IndexBufferSize, IndicesData.data(), IndexUploadBuffer);

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		// Setup index buffer view
		IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
		IndexBufferView.SizeInBytes = IndexBufferSize;
		IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	void FGameResource::AddMaterial(std::unique_ptr<FMaterialData> MaterialData)
	{
		if (MaterialData->Name.empty())
		{
			throw std::invalid_argument("Material's name must be not empty");
		}

		if (MaterialsData.find(MaterialData->Name) != MaterialsData.end())
		{
			throw std::invalid_argument("A material with the same name exists");
		}

		MaterialsData[MaterialData->Name] = std::move(MaterialData);
	}

	void FGameResource::LoadTexture(
		const std::wstring& FileName,
		const std::string& Name,
		ComPtr<ID3D12GraphicsCommandList> CmdList)
	{
		if (CmdList == nullptr)
		{
			throw std::invalid_argument("CmdList must be not nullptr");
		}

		if (Name.empty())
		{
			throw std::invalid_argument("Name must be not empty");
		}

		if (TexturesData.find(Name) != TexturesData.end())
		{
			throw std::invalid_argument("A texture with same name exists");
		}

		auto Texture = std::make_unique<FTextureData>();
		DX::ThrowIfFailed(CreateDDSTextureFromFile12(Device.Get(), CmdList.Get(), FileName.c_str(), 
			Texture->Resource, Texture->UploadResource));

		Texture->Name = Name;

		TexturesData[Texture->Name] = std::move(Texture);
	}

	void FGameResource::SetDevice(ComPtr<ID3D12Device> Device)
	{
		assert(Device != nullptr);
		this->Device = Device;
	}

	uint64 FGameResource::GetMaterialConstBufferIndex(const std::string& MaterialName) const
	{
		auto MaterialDataIt = MaterialsData.find(MaterialName);
		if (MaterialDataIt == MaterialsData.end())
		{
			throw std::invalid_argument("Hasn't found any material data with name - " + MaterialName);
		}
		return MaterialDataIt->second->iConstBuffer;
	}

	FMaterialData* FGameResource::GetMaterialData(const std::string& MaterialName) const
	{
		auto MaterialDataIt = MaterialsData.find(MaterialName);
		if (MaterialDataIt == MaterialsData.end())
		{
			throw std::invalid_argument("Hasn't found any material data with name - " + MaterialName);
		}
		return MaterialDataIt->second.get();
	}

	const FMeshData& FGameResource::GetMeshData(const std::string& MeshName) const
	{
		auto MeshDataIt = StaticMeshesData.find(MeshName);
		if (MeshDataIt == StaticMeshesData.end())
		{
			throw std::invalid_argument("Hasn't found any mesh data with name - " + MeshName);
		}
		return *MeshDataIt->second;
	}

	default::uint64 FGameResource::GetNumMaterials() const noexcept
	{
		return MaterialsData.size();
	}

	const FGameResource::FMaterialsData& FGameResource::GetMaterialsData() const noexcept
	{
		return MaterialsData;
	}

	const WoodenEngine::FGameResource::FTexturesData& FGameResource::GetTexturesData() const noexcept
	{
		return TexturesData;
	}

	const FTextureData* FGameResource::GetTextureData(const std::string& Name) const
	{
		auto TextureDataIter = TexturesData.find(Name);
		if (TextureDataIter == TexturesData.end())
		{
			throw std::invalid_argument("Texture with name " + Name + " doesn't exist");
		}

		return TextureDataIter->second.get();
	}

	const uint32 FGameResource::GetNumTexturesData() const noexcept
	{
		return TexturesData.size();
	}

	D3D12_VERTEX_BUFFER_VIEW FGameResource::GetVertexBufferView() const noexcept
	{
		return VertexBufferView;
	}

	D3D12_INDEX_BUFFER_VIEW FGameResource::GetIndexBufferView() const noexcept
	{
		return IndexBufferView;
	}
}