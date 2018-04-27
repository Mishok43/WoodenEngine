#pragma once
#include "ShaderStructures.h"

namespace DirectXEngine
{

	/*!
	 * \struct MeshData
	 *
	 * \brief Structure contains mesh data - vertices and indices
	 *
	 * \author devmi
	 * \date April 2018
	 */
	struct Vertex{
		Vertex()
		{}

		Vertex(const float x, const float y, const float z) :
			Position(x, y, z)
		{

		}

		Vertex(DirectX::XMFLOAT3 Position) :
			Position(Position)
		{}

		DirectX::XMFLOAT3 Position;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<uint16> Indices;
	};

	/*!
	 * \class MeshGenerator
	 *
	 * \brief It generators mesh data of various geometric figures 
	 *
	 * \author devmi
	 * \date April 2018
	 */
	class MeshGenerator
	{
	public:
		MeshGenerator();
		~MeshGenerator();

		MeshGenerator& operator=(const MeshGenerator& Generator) = delete;
		MeshGenerator(const MeshGenerator& Generator) = delete;
		MeshGenerator(MeshGenerator&& Generator) = delete;

		/** @brief Method generate a box with specific dimensions
		  * @param Width (float)
		  * @param Height (float)
		  * @param Depth (float)
		  * @param NumSubdivisions (uint32)
		  * @return Mesh data of generated box (DirectXEngine::MeshData)
		  */
		MeshData CreateBox(
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
		MeshData CreateSphere(
			float Radius,
			float NumVSubdivisions,
			float NumHSubdivisions
		) const noexcept;
	};


}