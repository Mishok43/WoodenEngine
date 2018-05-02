#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "MeshData.h"
#include "ShaderStructures.h"

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
		FGameResources() = default;
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
		  * @param MeshesData An array of meshes (const std::vector<FMeshData> &)
		  * @param CmdList A Graphics Command List for comitting vertex and indices resources
		  * @return (void)
		  */
		void LoadMeshes(
			const std::vector<FMeshData>& MeshesData,
			ComPtr<ID3D12GraphicsCommandList> CmdList);

		/** @brief Return mesh data by name. Throws exception if no mesh data is associated with the name 
		  * @param MeshName Name of mesh data(const std::string &)
		  * @return Mesh data associated with the name (WoodenEngine::FMeshData)
		  */
		FMeshData GetMeshData(const std::string& MeshName) const;

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
		std::unordered_map<std::string, const FMeshData> StaticMeshesData;

		// Dynamic array of all static meshes' vertices 
		std::vector<SVertexData> VertexData;

		// Dynamic array of all static meshes' indices
		std::vector<uint16> IndexData;
		
		// DX12 Device
		ComPtr<ID3D12Device> Device;
	
		// DX12 Buffer of vertices of static meshes
		ComPtr<ID3D12Resource> VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

		// DX12 Buffer of indices of static meshes
		ComPtr<ID3D12Resource> IndexBuffer;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	};
}