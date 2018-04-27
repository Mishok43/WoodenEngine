#include "MeshData.h"

namespace DirectXEngine
{
	MeshGenerator::MeshGenerator()
	{

	}

	MeshGenerator::~MeshGenerator()
	{

	}

	MeshData MeshGenerator::CreateBox(
		float Width, 
		float Height, 
		float Depth
	) const noexcept
	{
		MeshData BoxMeshData;
		
		const auto WidthHalf = Width / 2.0f;
		const auto HeightHalf = Height / 2.0f;
		const auto DepthHalf = Depth / 2.0f;

		BoxMeshData.Vertices = {
			{ -WidthHalf, -HeightHalf, -DepthHalf },
			{ WidthHalf, -HeightHalf, -DepthHalf },
			{ -WidthHalf, HeightHalf, -DepthHalf },
			{ WidthHalf, HeightHalf, -DepthHalf },
			{ -WidthHalf, -HeightHalf, DepthHalf },
			{ WidthHalf, -HeightHalf, DepthHalf },
			{ -WidthHalf, HeightHalf, DepthHalf },
			{ WidthHalf, HeightHalf, DepthHalf }
		};

		BoxMeshData.Indices = {
			// front face
			0, 2, 1,
			0, 3, 2,

			// back face
			4, 6, 5,
			4, 7, 6,

			// top face
			3, 7, 6,
			3, 6, 2,

			// back face
			0, 4, 5,
			0, 5, 1
		};

		return BoxMeshData;
	}

	MeshData MeshGenerator::CreateSphere(
		float Radius,
		float NumVSubdivisions,
		float NumHSubdivisions) const noexcept
	{
		for (int i = 0; i < NumVSubdivisions+1; i++)
		{

		}
	}
}