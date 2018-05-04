#include "MeshData.h"

namespace WoodenEngine
{
	FMeshGenerator::FMeshGenerator()
	{

	}

	FMeshGenerator::~FMeshGenerator()
	{

	}

	FMeshData FMeshGenerator::CreateBox(
		float Width, 
		float Height, 
		float Depth
	) const noexcept
	{
		FMeshData BoxMeshData;
		BoxMeshData.Name = "Box";

		const auto WidthHalf = Width / 2.0f;
		const auto HeightHalf = Height / 2.0f;
		const auto DepthHalf = Depth / 2.0f;

		BoxMeshData.Vertices = {
			{ -WidthHalf, -HeightHalf, -DepthHalf },
			{ WidthHalf, -HeightHalf, -DepthHalf },
			{ WidthHalf, HeightHalf, -DepthHalf },
			{ -WidthHalf, HeightHalf, -DepthHalf },
			{ -WidthHalf, -HeightHalf, DepthHalf },
			{ WidthHalf, -HeightHalf, DepthHalf },
			{ WidthHalf, HeightHalf, DepthHalf },
			{ -WidthHalf, HeightHalf, DepthHalf }
		};

		BoxMeshData.Indices = {
			// front face
			0, 2, 1,
			0, 3, 2,

			// back face
			5, 6, 7,
			5, 7, 4,

			// top face
			3, 7, 6,
			3, 6, 2,

			// back face
			4, 0, 1,
			4, 1, 6,

			// left face
			4, 3, 0,
			4, 7, 3,

			// right face
			1, 2, 6,
			1, 6, 5
		};

		return BoxMeshData;
	}

	FMeshData FMeshGenerator::CreateSphere(
		float Radius,
		float NumVSubdivisions,
		float NumHSubdivisions) const noexcept
	{
		FMeshData SphereMeshData;
		SphereMeshData.Name = "Sphere";
		SphereMeshData.Vertices.resize((NumVSubdivisions+1)*(NumHSubdivisions));
		SphereMeshData.Indices.resize((2*NumHSubdivisions + (NumVSubdivisions-2)*NumHSubdivisions*2)*3);
		const auto TopVertexY = Radius;
		const auto BottomVertexY = -Radius;
		const auto VAngleOffset = DirectX::XM_PI / NumVSubdivisions;
		const auto HAngleOffset = DirectX::XM_2PI / NumHSubdivisions;

		
		uint16 iVertex = 0;
		
		// Add top vertices
		for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions; iHSubdivision++, iVertex++)
		{
			SphereMeshData.Vertices[iVertex] = { 0.0f, TopVertexY, 0.0f };
		}

		// Add intermediate vertices
		for (auto iVSubdivision = 0; iVSubdivision < NumVSubdivisions-1; ++iVSubdivision)
		{
			const auto VAngle = DirectX::XM_PIDIV2 - (iVSubdivision + 1)*VAngleOffset;
			const auto SubCircleRadius = cos(VAngle)*Radius;
			const auto VertexY = sin(VAngle)*Radius;

			for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions; ++iHSubdivision, ++iVertex)
			{
				const auto HAngle = (iHSubdivision + 1)*HAngleOffset;
				const auto VertexX = cos(HAngle)*SubCircleRadius;
				const auto VertexZ = sin(HAngle)*SubCircleRadius;
				SphereMeshData.Vertices[iVertex] = { VertexX, VertexY, VertexZ };
			}
		}

		// Add bottom vertices
		for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions; iHSubdivision++, iVertex++)
		{
			SphereMeshData.Vertices[iVertex] = { 0.0f, BottomVertexY, 0.0f };
		}

		// Add indices
		uint16 VertexIndex = 0;
		for (auto iVSubdivision = 1; iVSubdivision < NumVSubdivisions+1; ++iVSubdivision)
		{
			for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions-1; ++iHSubdivision, VertexIndex+=6)
			{
				const auto TopLeftVIndex = (iVSubdivision - 1)*NumHSubdivisions +iHSubdivision;
				const auto TopRightVIndex = TopLeftVIndex+1;
				const auto DownLeftVIndex = iVSubdivision * NumHSubdivisions + iHSubdivision;
				const auto DownRightVIndex = DownLeftVIndex + 1;
				SphereMeshData.Indices[VertexIndex] = DownLeftVIndex;
				SphereMeshData.Indices[VertexIndex+1] = TopLeftVIndex;
				SphereMeshData.Indices[VertexIndex+2] = TopRightVIndex;

				SphereMeshData.Indices[VertexIndex+3] = DownLeftVIndex;
				SphereMeshData.Indices[VertexIndex+4] = TopRightVIndex;
				SphereMeshData.Indices[VertexIndex+5] = DownRightVIndex;
			}
		}

		return SphereMeshData;
	}
}