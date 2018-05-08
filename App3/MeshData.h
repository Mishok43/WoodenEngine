#pragma once
#include <iostream>
#include <string>

#include "ShaderStructures.h"

namespace WoodenEngine
{

	struct FVertex{
		FVertex() = default;

		FVertex(
			const DirectX::XMFLOAT3& Position,
			const DirectX::XMFLOAT3& Normal,
			const DirectX::XMFLOAT3& Tangent,
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
			Normal(NormalX, NormalY, NormalZ)
		{}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT2 TexC;
	};


	/*!
	* \struct FMeshData
	*
	* \brief Structure contains mesh data - vertices and indices
	*
	* \author devmi
	* \date April 2018
	*/
	struct FMeshData
	{
		FMeshData() = default;

		FMeshData(const std::string& Name):
			Name(Name)
		{}

		std::string Name;
		std::vector<FVertex> Vertices;
		std::vector<uint16> Indices;
		uint16 IndexBegin;
		uint16 VertexBegin;
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
		std::unique_ptr<FMeshData> CreateBox(
			float Width,
			float Height,
			float Depth
		) const noexcept;
		
		/** @brief Method generate a sphere with specific dimensions 
		  * @param Radius (float)
		  * @param NumVSubdivisions (uint32)
		  * @param NumHSubdivisions (uint32)
		  * @return Mesh data of generated sphere (DirectXEngine::MeshData)
		  */
		std::unique_ptr<FMeshData> CreateSphere(
			float Radius,
			uint32 NumVSubdivisions,
			uint32 NumHSubdivisions
		) const noexcept;
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
		std::unique_ptr<FMeshData> ParseMeshData(const std::string& FilePath) const;
	};
}