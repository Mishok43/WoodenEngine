#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

#include "ShaderStructures.h"

namespace WoodenEngine
{

	struct FVertex{
		FVertex() = default;


		FVertex(
			const DirectX::XMFLOAT3& Position,
			const DirectX::XMFLOAT3& Normal = { 0.0f, 0.0f, 0.0f },
			const DirectX::XMFLOAT3& Tangent = { 0.0f, 0.0f, 0.0f },
			const DirectX::XMFLOAT2& UV = { 0.0f, 0.0f }) :
			Position(Position),
			Normal(Normal),
			Tangent(Tangent),
			TexC(UV)
		{}

		FVertex(
			float PositionX, float PositionY, float PositionZ,
			float NormalX, float NormalY, float NormalZ,
			float TangentX, float TangentY, float TangentZ,
			float u=0.0f, float v=0.0f) :
			Position(PositionX, PositionY, PositionZ),
			Normal(NormalX, NormalY, NormalZ),
			Tangent(TangentX, TangentY, TangentZ),
			TexC(u, v)
		{}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT2 TexC;

		static FVertex MidPoint(const FVertex& V0, const FVertex& V1) noexcept
		{
			auto Position0 = XMLoadFloat3(&V0.Position);
			auto Position1 = XMLoadFloat3(&V1.Position);
			
			auto Normal0 = XMLoadFloat3(&V0.Normal);
			auto Normal1 = XMLoadFloat3(&V1.Normal);

			auto Tangent0 = XMLoadFloat3(&V0.Tangent);
			auto Tangent1 = XMLoadFloat3(&V1.Tangent);

			auto Tex0 = XMLoadFloat2(&V0.TexC);
			auto Tex1 = XMLoadFloat2(&V1.TexC);

			auto MidPosition = 0.5f*(Position0 + Position1);
			auto MidNormal = XMVector3Normalize(0.5f*(Normal0 + Normal1));
			auto MidTangent = XMVector3Normalize(0.5f*(Tangent0 + Tangent1));
			auto MidTexC = 0.5f*(Tex0 + Tex1);

			FVertex MidVertex;
			XMStoreFloat3(&MidVertex.Position, MidPosition);
			XMStoreFloat3(&MidVertex.Normal, MidNormal);
			XMStoreFloat3(&MidVertex.Tangent, MidTangent);
			XMStoreFloat2(&MidVertex.TexC, MidTexC);
			return MidVertex;
		}
	};


	/*!
	* \struct FMeshRawData
	*
	* \brief Contains raw meshes data - only vertices and indices
	*
	* \author devmi
	* \date April 2018
	*/
	
	struct FMeshRawData
	{
		FMeshRawData() = default;

		FMeshRawData(const std::string& Name):
			Name(Name)
		{ }

		std::vector<FVertex> Vertices;
		std::vector<uint16> Indices;

		std::string Name;
	};

	/*!
	 * \class FSubmeshData
	 *
	 * \brief Contains submesh data
	 *
	 * \author devmi
	 * \date May 2018
	 */
	struct FSubmeshData
	{
		FSubmeshData() = default;

		FSubmeshData(const std::string& Name) :
			Name(Name)
		{
		}

		std::string Name;

		uint64 IndexBegin;
		uint64 NumIndices;
		uint16 VertexBegin;
	};

	/*!
	 * \class FMeshData
	 *
	 * \brief Contains resources, views for vertices, indices of all submeshes
	 *
	 * \author devmi
	 * \date May 2018
	 */
	struct FMeshData
	{
		FMeshData() = default;

		FMeshData(const FMeshData& MeshData) = delete;
		FMeshData& operator=(const FMeshData& MeshData) = delete;

		FMeshData(const std::string& Name):
			Name(Name)
		{ }

		std::string Name;

		// DX12 Buffer of vertices of static meshes
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> VertexUploadBuffer;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

		// DX12 Buffer of indices of static meshes
		ComPtr<ID3D12Resource> IndexBuffer;
		ComPtr<ID3D12Resource> IndexUploadBuffer;

		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		
		std::unordered_map<std::string, std::unique_ptr<FSubmeshData>> SubmeshesData;
	};

	/*!
	 * \class FMeshGenerator
	 *
	 * \brief It generators mesh data of various geometric figures 
	 *
	 * \author devmi
	 * \date April 2018
	 */
	class FMeshGenerator
	{
	public:
		FMeshGenerator();
		~FMeshGenerator();

		FMeshGenerator& operator=(const FMeshGenerator& Generator) = delete;
		FMeshGenerator(const FMeshGenerator& Generator) = delete;
		FMeshGenerator(FMeshGenerator&& Generator) = delete;

		/** @brief Method generate a box with specific dimensions
		  * @param Width (float)
		  * @param Height (float)
		  * @param Depth (float)
		  * @param NumSubdivisions (uint32)
		  * @return Mesh data of generated box (DirectXEngine::MeshData)
		  */
		std::unique_ptr<FMeshRawData> CreateBox(
			float Width,
			float Height,
			float Depth,
			uint32 NumSubdivisions = 0
		) const noexcept;
		
		/** @brief Method generate a sphere with specific dimensions 
		  * @param Radius (float)
		  * @param NumVSubdivisions (uint32)
		  * @param NumHSubdivisions (uint32)
		  * @return Mesh data of generated sphere (DirectXEngine::MeshData)
		  */
		std::unique_ptr<FMeshRawData> CreateSphere(
			float Radius,
			uint32 NumVSubdivisions,
			uint32 NumHSubdivisions
		) const noexcept;

		/** @brief Generates a geosphere with regular triangles
		  * @param Radius of geosphere (float)
		  * @param NumSubdivision (uint16_t)
		  * @return (std::unique_ptr<WoodenEngine::FMeshRawData>)
		  */
		std::unique_ptr<FMeshRawData> CreateGeoSphere(
			float Radius,
			uint16_t NumSubdivision
		) const noexcept;

		/** @brief Generates a grid NumVSubdivions*NumHSubdivisions 
		  * @param Width Object's width (float)
		  * @param Height Object's height (float)
		  * @param NumVSubdivisions Number verticals squares (uint32)
		  * @param NumHSubdivisions Number horizontal squares (uint32)
		  * @return (std::unique_ptr<WoodenEngine::FMeshData>)
		  */
		std::unique_ptr<FMeshRawData> CreateGrid(
			float Width,
			float Height,
			uint32 NumVSubdivisions,
			uint32 NumHSubdivisions
		) const noexcept;


		/** @brief Generates a landscape grid based of function:
		  * y = 3(sin(x) + cos(z))
		  * @param Width (float)
		  * @param Height (float)
		  * @param NumVSubdivisions (uint32)
		  * @param NumHSubdivisions (uint32)
		  * @return (std::unique_ptr<WoodenEngine::FMeshData>)
		  */
		std::unique_ptr<FMeshRawData> CreateLandscapeGrid(
			float Width,
			float Height,
			uint32 NumVSubdivisions,
			uint32 NumHSubdivisions
		) const noexcept;

		/** @brief Subdivide mesh data (every triangle of it's to 3 triangles)
		  * @param MeshData Mesh data (FMeshData* MeshData)
		  * @return (void)
		  */
		void Subdivide(FMeshRawData* MeshData) const noexcept;
	};

	/*!
	 * \class FMeshParser
	 *
	 * \brief Helper class for parsing external data files with mesh for exporting them to MeshData
	 *
	 * \author devmi
	 * \date May 2018
	 */
	class FMeshParser
	{
	public:
		FMeshParser() = default;
		~FMeshParser() = default;

		FMeshParser& operator=(const FMeshParser& MeshParser) = delete;
		FMeshParser(const FMeshParser& MeshParser) = delete;
		FMeshParser(FMeshParser&& MeshParser) = delete;

		/** @brief Parses external file with mesh data
		* @param FilePath Path to file (format: *.txt) with mesh data (const std::string &)
		* @return Parsed mesh data (WoodenEngine::FMeshData)
		*/
		std::unique_ptr<FMeshRawData>ParseTxtData(const std::string& FilePath) const;

		/** @brief Parses external .obj file with mesh data
		* @param FilePath Path to file (format: *.obj) with mesh data (const std::string &)
		* @return Parsed mesh data (WoodenEngine::FMeshData)
		*/
		std::unique_ptr<FMeshRawData> ParseObjFile(const std::string& FilePath) const;
	};
}