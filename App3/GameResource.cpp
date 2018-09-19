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

	void FGameResource::LoadStaticMesh(
		std::vector<std::unique_ptr<FMeshRawData>>&& SubmeshesData,
		const std::string& MeshName,
		ComPtr<ID3D12GraphicsCommandList> CMDList)
	{
		if (SubmeshesData.empty())
		{
			throw std::invalid_argument("SubmeshesData must be not empty");
		}

		if (MeshName.empty())
		{
			throw std::invalid_argument("MeshName must be not empty");
		}

		auto StaticMeshesDataIter = StaticMeshesData.find(MeshName);
		if (StaticMeshesDataIter != StaticMeshesData.end())
		{
			throw std::invalid_argument("A mesh with the name " + MeshName + " exists yet");
		}

		auto MeshData = std::make_unique<FMeshData>(MeshName);

		uint16 NumVertices = 0;
		uint64 NumIndices = 0;
		
		for (auto iMesh = 0; iMesh < SubmeshesData.size(); ++iMesh)
		{
			auto MeshData = SubmeshesData[iMesh].get();

			NumVertices += MeshData->Vertices.size();
			NumIndices += MeshData->Indices.size();
		}


		std::vector<SVertexData> VerticesData;
		std::vector<uint16> IndicesData;

		VerticesData.resize(NumVertices);
		IndicesData.reserve(NumIndices);
		
		auto VertexDataIter = VerticesData.cend();
		auto IndexDataIter = IndicesData.cend();

		auto NumFilledVertices = 0;
		// Adding vertices and indices of every mesh to common arrays
		for (auto iMesh = 0; iMesh < SubmeshesData.size(); ++iMesh)
		{
			auto SubmeshRawData = SubmeshesData[iMesh].get();
			
			auto SubmeshData = std::make_unique<FSubmeshData>(SubmeshRawData->Name);
			SubmeshData->VertexBegin = NumFilledVertices;
			SubmeshData->IndexBegin = IndicesData.size();
			SubmeshData->NumIndices = SubmeshRawData->Indices.size();

			const auto VertexDataNewSize = NumFilledVertices + SubmeshRawData->Vertices.size();

			for (auto i = SubmeshData->VertexBegin; i < VertexDataNewSize; i++)
			{
				const auto iVertex = i - SubmeshData->VertexBegin;

				SVertexData Vertex = { 
					SubmeshRawData->Vertices[iVertex].Position,
					SubmeshRawData->Vertices[iVertex].Normal, 
					SubmeshRawData->Vertices[iVertex].TexC 
				};

				VerticesData[i] = std::move(Vertex);
			}

			NumFilledVertices += SubmeshRawData->Vertices.size();

			const auto IndexDataNewSize = IndicesData.size() + SubmeshRawData->Indices.size();
			
			for (auto i = SubmeshData->IndexBegin; i < IndexDataNewSize; i++)
			{
				auto Index = SubmeshRawData->Indices[i - SubmeshData->IndexBegin];
				
				IndexDataIter = IndicesData.insert(IndexDataIter, std::move(Index));
				IndexDataIter++;
			}

			MeshData->SubmeshesData.insert(std::make_pair(SubmeshRawData->Name, std::move(SubmeshData)));
		}

		// Create vertex buffer 
		const auto VertexBufferSize = sizeof(SVertexData)*VerticesData.size();

		MeshData->VertexBuffer = DX::CreateBuffer(Device, CMDList, 
			VertexBufferSize, VerticesData.data(), MeshData->
												  VertexUploadBuffer);

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			MeshData->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, 
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Setup vertex buffer view
		MeshData->VertexBufferView.BufferLocation = MeshData->VertexBuffer->GetGPUVirtualAddress();
		MeshData->VertexBufferView.SizeInBytes = VertexBufferSize;
		MeshData->VertexBufferView.StrideInBytes = sizeof(SVertexData);

		// Create index buffer
		const auto IndexBufferSize = sizeof(uint16)*IndicesData.size();
		MeshData->IndexBuffer = DX::CreateBuffer(Device, CMDList,
			IndexBufferSize, IndicesData.data(), MeshData->IndexUploadBuffer);

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			MeshData->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, 
			D3D12_RESOURCE_STATE_INDEX_BUFFER));

		// Setup index buffer view
		MeshData->IndexBufferView.BufferLocation = MeshData->IndexBuffer->GetGPUVirtualAddress();
		MeshData->IndexBufferView.SizeInBytes = IndexBufferSize;
		MeshData->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;

		StaticMeshesData[MeshData->Name] = std::move(MeshData);
	}

	void FGameResource::LoadBillboards(
		const std::vector<SVertexBillboardData>& VerticesData,
		const std::string& MeshName,
		const std::string& SubmeshName,
		ComPtr<ID3D12GraphicsCommandList> CMDList)
	{
		if (MeshName.empty())
		{
			throw std::invalid_argument("MeshName must be not empty");
		}

		auto StaticMeshesDataIter = StaticMeshesData.find(MeshName);
		if (StaticMeshesDataIter != StaticMeshesData.end())
		{
			throw std::invalid_argument("A mesh with the name " + MeshName + " exists yet");
		}

		auto MeshData = std::make_unique<FMeshData>(MeshName);

		uint16_t NumVertices = VerticesData.size();

		std::vector<uint16_t> IndicesData;
		IndicesData.resize(NumVertices);
		
		for (uint16_t i = 0; i < NumVertices; ++i)
		{
			IndicesData[i] = i;
		}
		
		auto SubmeshData = std::make_unique<FSubmeshData>(SubmeshName);
		SubmeshData->NumIndices = NumVertices;
		SubmeshData->IndexBegin = 0;
		SubmeshData->VertexBegin = 0;

		MeshData->SubmeshesData.insert(std::make_pair(SubmeshName, std::move(SubmeshData)));

		// Create vertex buffer 
		const auto VertexBufferSize = sizeof(SVertexBillboardData)*VerticesData.size();

		MeshData->VertexBuffer = DX::CreateBuffer(Device, CMDList,
												  VertexBufferSize, VerticesData.data(), MeshData->
												  VertexUploadBuffer);

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			MeshData->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Setup vertex buffer view
		MeshData->VertexBufferView.BufferLocation = MeshData->VertexBuffer->GetGPUVirtualAddress();
		MeshData->VertexBufferView.SizeInBytes = VertexBufferSize;
		MeshData->VertexBufferView.StrideInBytes = sizeof(SVertexBillboardData);

		// Create index buffer
		const auto IndexBufferSize = sizeof(uint16)*IndicesData.size();
		MeshData->IndexBuffer = DX::CreateBuffer(Device, CMDList,
												 IndexBufferSize, IndicesData.data(), MeshData->IndexUploadBuffer);

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			MeshData->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER));

		// Setup index buffer view
		MeshData->IndexBufferView.BufferLocation = MeshData->IndexBuffer->GetGPUVirtualAddress();
		MeshData->IndexBufferView.SizeInBytes = IndexBufferSize;
		MeshData->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;

		StaticMeshesData[MeshData->Name] = std::move(MeshData);
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
		ComPtr<ID3D12GraphicsCommandList> CmdList,
		D3D12_SRV_DIMENSION ViewDimension)
	{
		if (CmdList == nullptr)
		{
			throw std::invalid_argument("CmdList must be not nullptr");
		}

		if (Name.empty())
		{
			throw std::invalid_argument("Name must be not empty");
		}

		if (TexturesData.find(Name) != TexturesData.cend())
		{
			throw std::invalid_argument("A texture with same name exists");
		}

		auto Texture = std::make_unique<FTextureData>();
		DX::ThrowIfFailed(CreateDDSTextureFromFile12(Device.Get(), CmdList.Get(), FileName.c_str(), 
			Texture->Resource, Texture->UploadResource));

		Texture->Name = Name;
		Texture->FileName = FileName;
		Texture->ViewDimension = ViewDimension;

		TexturesData[Texture->Name] = std::move(Texture);
	}

	void FGameResource::SetDevice(ComPtr<ID3D12Device> Device)
	{
		if (Device == nullptr)
		{
			throw std::invalid_argument("Device must be not nullptr");
		}

		this->Device = Device;
	}

	uint64 FGameResource::GetMaterialConstBufferIndex(const std::string& MaterialName) const
	{
		auto MaterialDataIter = MaterialsData.find(MaterialName);
		if (MaterialDataIter == MaterialsData.cend())
		{
			throw std::invalid_argument("Hasn't found any material data with name - " + MaterialName);
		}
		return MaterialDataIter->second->iConstBuffer;
	}

	FMaterialData* FGameResource::GetMaterialData(const std::string& MaterialName) const
	{
		auto MaterialDataIter = MaterialsData.find(MaterialName);
		if (MaterialDataIter == MaterialsData.cend())
		{
			throw std::invalid_argument("Hasn't found any material data with name - " + MaterialName);
		}
		return MaterialDataIter->second.get();
	}

	const FSubmeshData& FGameResource::GetSubmeshData(
		const std::string& MeshName,
		const std::string& SubmeshName) const
	{
		const auto& MeshData = GetMeshData(MeshName);

		auto SubmeshDataIter = MeshData.SubmeshesData.find(SubmeshName);
		if (SubmeshDataIter == MeshData.SubmeshesData.cend())
		{
			throw std::invalid_argument("Hasn't found any submesh data with name " + SubmeshName +
				" in the mesh " + MeshName);
		}

		return *SubmeshDataIter->second;
	}

	const FMeshData& FGameResource::GetMeshData(const std::string& MeshName) const
	{
		auto MeshDataIter = StaticMeshesData.find(MeshName);
		if (MeshDataIter == StaticMeshesData.cend())
		{
			throw std::invalid_argument("Hasn't found any mesh data with name - " + MeshName);
		}

		return *MeshDataIter->second;
	}

	uint64 FGameResource::GetNumMaterials() const noexcept
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
		if (TextureDataIter == TexturesData.cend())
		{
			throw std::invalid_argument("Texture with name " + Name + " doesn't exist");
		}

		return TextureDataIter->second.get();
	}

	const uint32 FGameResource::GetNumTexturesData() const noexcept
	{
		return TexturesData.size();
	}
}