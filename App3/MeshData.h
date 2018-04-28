#pragma once
#include <iostream>
#include <string>

#include "ShaderStructures.h"

namespace WoodenEngine
{

	struct FVertex{
		FVertex()
		{}

		FVertex(const float x, const float y, const float z) :
			Position(x, y, z)
		{
		}

		FVertex(DirectX::XMFLOAT3 Position) :
			Position(Position)
		{}

		DirectX::XMFLOAT3 Position;
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
		FMeshData CreateBox(
			float Width,
			float Height,
			float Depth
		) const noexcept;
		
		/** @brief Method generate a sphere with specific dimensions 
		  * @param Radius (float)
		  * @param NumVSubdivisions (float)
		  * @param NumHSubdivisions (float)
		  * @return Mesh data of generated sphere (DirectXEngine::MeshData)
		  */
		FMeshData CreateSphere(
			float Radius,
			float NumVSubdivisions,
			float NumHSubdivisions
		) const noexcept;
	};


}