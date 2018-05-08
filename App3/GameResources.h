#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "ShaderStructures.h"
#include "MeshData.h"
#include "MaterialData.h"

namespace WoodenEngine
{

	/*!
	 * \class FGameResources
	 *
	 * \brief It's manager of game's resources, such as textures, audio files etc
	 *
	 * \author devmi
	 * \date April 2018
	 */
	class FGameResources
	{
	public: 
		using FMaterialsData = 
			std::unordered_map<std::string, std::unique_ptr<FMaterialData>>;

		using FMeshesData =
			std::unordered_map<std::string, std::unique_ptr<FMeshData>>;

		FGameResources();
		FGameResources(ComPtr<ID3D12Device> Device);
		~FGameResources();

		FGameResources& operator=(const FGameResources& GameResources) = delete;
		FGameResources(const FGameResources& GameResources) = delete;
		FGameResources(FGameResources&& GameResources) = delete;


		/** @brief Method set dx12 device. It'll be used for comitting resources
		  * @param Device DX12 Device (ComPtr<ID3D12Device>)
		  * @return (void)
		  */
		void SetDevice(ComPtr<ID3D12Device> Device);

		/** @brief Method load an array of meshes to video and cpu memory. 
		  *	(The Device must be set!) 
		  * @param MeshesData An array of meshes.
		  * It'll have only nullptrs after executing function (const std::vector<FMeshData> &&)
		  * @param CmdList A Graphics Command List for comitting vertex and indices resources
		  * @return (void)
		  */
		void LoadMeshes(
			std::vector<std::unique_ptr<FMeshData>>&& MeshesData,
			ComPtr<ID3D12GraphicsCommandList> CmdList);
		
		/** @brief Adds material data to cache for future access
		  * @param MaterialData Unique ptr to material data (std::unique_ptr<FMaterialData>)
		  * @return (void)
		  */
		void AddMaterial(std::unique_ptr<FMaterialData> MaterialData);


		/** @brief Finds a material data by name and returns it's iConstBuffer
		  * @param MaterialName (const std::string &)
		  * @return Const Buffer Index (default::uint64)
		  */
		uint64 GetMaterialConstBufferIndex(const std::string& MaterialName) const;

		/** @brief Finds a material data by name and returns it
		  * @param MaterialName (const std::string &)
		  * @return Material Data(const WoodenEngine::FMaterialData&)
		  */
		const FMaterialData* GetMaterialData(const std::string& MaterialName) const;

		/** @brief Find a mesh data by name and returns it.
		  * Throws exception if no mesh data is associated with the name 
		  * @param MeshName Name of mesh data(const std::string &)
		  * @return Mesh data associated with the name (WoodenEngine::FMeshData)
		  */
		const FMeshData& GetMeshData(const std::string& MeshName) const;

		/** @brief Returns number of materils
		  * @return Number of materials (default::uint64)
		  */
		uint64 GetNumMaterials() const noexcept;

		/** @brief Returns materials data
		  * @return (const std::unordered_map<std::string, WoodenEngine::FMaterialData>&)
		  */
		const FMaterialsData& GetMaterialsData() const noexcept;


		/** @brief Returns vertex buffer view
		  * @return Vertex buffer view (D3D12_VERTEX_BUFFER_VIEW)
		  */
		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const noexcept;

		/** @brief Returns index buffer view
		  * @return Index buffer view (D3D12_INDEX_BUFFER_VIEW)
		  */
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const noexcept;
	private:
		// Hash-Table consists of static meshes data, where key is a mesh's name
		FMeshesData StaticMeshesData;
		
		// Hash-table consists of materials data, where key is a material's name
		FMaterialsData MaterialsData;

		// Dynamic array of all static meshes' vertices 
		std::vector<SVertexData> VerticesData;

		// Dynamic array of all static meshes' indices
		std::vector<uint16> IndicesData;
		
		// DX12 Device
		ComPtr<ID3D12Device> Device;
	
		// DX12 Buffer of vertices of static meshes
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> VertexUploadBuffer;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

		// DX12 Buffer of indices of static meshes
		ComPtr<ID3D12Resource> IndexBuffer;
		ComPtr<ID3D12Resource> IndexUploadBuffer;

		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	};
}