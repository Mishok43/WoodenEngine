#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "ShaderStructures.h"
#include "MeshData.h"
#include "BillboardData.h"
#include "MaterialData.h"
#include "TextureData.h"

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
	class FGameResource
	{
	public:
		using FMaterialsData =
			std::unordered_map<std::string, std::unique_ptr<FMaterialData>>;

		using FMeshesData =
			std::unordered_map<std::string, std::unique_ptr<FMeshData>>;

		using FTexturesData =
			std::unordered_map<std::string, std::unique_ptr<FTextureData>>;

		FGameResource();
		FGameResource(ComPtr<ID3D12Device> Device);
		~FGameResource();

		FGameResource& operator=(const FGameResource& GameResources) = delete;
		FGameResource(const FGameResource& GameResources) = delete;
		FGameResource(FGameResource&& GameResources) = delete;


		/** @brief Method set dx12 device. It'll be used for comitting resources
		  * @param Device DX12 Device (ComPtr<ID3D12Device>)
		  * @return (void)
		  */
		void SetDevice(ComPtr<ID3D12Device> Device);

		/** @brief Method load an array of meshes to video and cpu memory.
		  *	(The Device must be set!)
		  * @param SubmeshesData An array of submeshes
		  * It'll have only nullptrs after executing function 
		  * (const std::vector<std::unique_ptr<FMeshRawData>> &&)
		  * @param MeshName Mesh name of group of the submehes (const std::string& )
		  * @param CmdList A Graphics Command List for comitting vertex and indices resources
		  * @return (void)
		  */
		void LoadStaticMesh(
			std::vector<std::unique_ptr<FMeshRawData>>&& SubmeshesData,
			const std::string& MeshName,
			ComPtr<ID3D12GraphicsCommandList> CmdList);


		/** @brief Add billboards
		  * @param Vertices Vector of billboards vertices (const std::vector<SVertexBillboardData> &)
		  * @param MeshName Name of mesh (const std::string &)
		  * @param SubmeshName Name of submesh (const std::string &)
		  * @param CMDList (ComPtr<ID3D12GraphicsCommandList>)
		  * @return (void)
		  */
		void LoadBillboards(
			const std::vector<SVertexBillboardData>& Vertices,
			const std::string& MeshName,
			const std::string& SubmeshName,
			ComPtr<ID3D12GraphicsCommandList> CMDList
		);

		/** @brief Adds material data to cache for future access
		  * @param MaterialData Unique ptr to material data (std::unique_ptr<FMaterialData>)
		  * @return (void)
		  */
		void AddMaterial(std::unique_ptr<FMaterialData> MaterialData);
		/** @brief Loads texture data to gpu memory
		  * @param FileName Texture's file name (const std::wstring &)
		  * @param Name Texture's name (const std::string &)
		  * @return (void)
		  */
		void LoadTexture(const std::wstring& FileName,
						 const std::string& Name,
						 ComPtr<ID3D12GraphicsCommandList> CmdList, 
						 D3D12_SRV_DIMENSION ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D);

		/** @brief Finds a material data by name and returns it's iConstBuffer
		  * @param MaterialName (const std::string &)
		  * @return Const Buffer Index (default::uint64)
		  */
		uint64 GetMaterialConstBufferIndex(const std::string& MaterialName) const;

		/** @brief Finds a material data by name and returns it
		  * @param MaterialName (const std::string &)
		  * @return Material Data(const WoodenEngine::FMaterialData&)
		  */
		FMaterialData* GetMaterialData(const std::string& MaterialName) const;

		/** @brief Finds a submesh data by its name and name of mesh which contains it
			and returns it.
		  * Throws exception if no mesh data is associated with the names
		  * @param MeshName Name of mesh data which contains this submesh (const std::string &)
		  * @param Submesh Name of submesh data (const std::string&)
		  * @return Submesh data associated with the name (WoodenEngine::FMeshData)
		  */
		const FSubmeshData& GetSubmeshData(
			const std::string& MeshName,
			const std::string& SubmeshName) const;

		/** @brief Finds a mesh data by its name
		   * Throws invalid_argument exception if there's no mesh data with the name
		  * @param MeshName Mesh name (const std::string &)
		  * @return MeshData (const WoodenEngine::FMeshData&)
		  */
		const FMeshData& GetMeshData(const std::string& MeshName) const;

		/** @brief Returns number of materials
		  * @return Number of materials (default::uint64)
		  */
		uint64 GetNumMaterials() const noexcept;

		/** @brief Returns materials data
		  * @return Material Data (const WoodenEngine::FGameResource::FMaterialsData&)
		  */
		const FMaterialsData& GetMaterialsData() const noexcept;

		/** @brief Returns textures data
		  * @return Textures data (const WoodenEngine::FGameResource::FTexturesData&)
		  */
		const FTexturesData& GetTexturesData() const noexcept;

		/** @brief Returns texture data
		  * @param Name Texture Name (const std::string&)
		  * @return Texture data (const FTextureData*)
		  */
		const FTextureData* GetTextureData(const std::string& Name) const;

		/** @brief Returns number of textures data in cache
		  * @return Number of textures data (const uint32)
		  */
		const uint32 GetNumTexturesData() const noexcept;

	private:
		// Hash-Table consists of static meshes data, where key is a mesh's name
		FMeshesData StaticMeshesData;

		// Hash-table consists of materials data, where key is a material's name
		FMaterialsData MaterialsData;

		// Hash-table consists of textures data, where key is a texture's name
		FTexturesData TexturesData;

		// DX12 Device
		ComPtr<ID3D12Device> Device;
	};
}