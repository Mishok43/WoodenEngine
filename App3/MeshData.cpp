#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MeshData.h"

namespace WoodenEngine
{
	FMeshGenerator::FMeshGenerator()
	{

	}

	FMeshGenerator::~FMeshGenerator()
	{

	}

	std::unique_ptr<FMeshData> FMeshGenerator::CreateBox(
		float Width, 
		float Height, 
		float Depth,
		uint32 NumSubdivisions
	) const noexcept
	{
		auto BoxMeshData = std::make_unique<FMeshData>("Box");

		const auto WidthHalf = Width / 2.0f;
		const auto HeightHalf = Height / 2.0f;
		const auto DepthHalf = Depth / 2.0f;

		BoxMeshData->Vertices = {
			// Front face
			{ -WidthHalf, -HeightHalf, -DepthHalf, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{ WidthHalf, -HeightHalf, -DepthHalf, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
			{ WidthHalf, HeightHalf, -DepthHalf , 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
			{ -WidthHalf, HeightHalf, -DepthHalf , 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},

			// Back face
			{ -WidthHalf, -HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
			{ WidthHalf, -HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
			{ WidthHalf, HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			{ -WidthHalf, HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f},

			// Top face
			{ -WidthHalf, HeightHalf, -DepthHalf , 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ WidthHalf, HeightHalf, -DepthHalf , 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f },
			{ WidthHalf, HeightHalf, DepthHalf , 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f},
			{ -WidthHalf, HeightHalf, DepthHalf , 0.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f, 0.0f, 0.0f },

			// Bottom face
			{ -WidthHalf, -HeightHalf, -DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			{ WidthHalf, -HeightHalf, -DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
			{ WidthHalf, -HeightHalf, DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
			{ -WidthHalf, -HeightHalf, DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f},

			// Left face
			{ -WidthHalf, -HeightHalf, -DepthHalf, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},
			{ -WidthHalf, HeightHalf, -DepthHalf , -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f},
			{ -WidthHalf, HeightHalf, DepthHalf , -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f},
			{ -WidthHalf, -HeightHalf, DepthHalf , -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f },

			// Right face
			{ WidthHalf, -HeightHalf, -DepthHalf, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
			{ WidthHalf, HeightHalf, -DepthHalf , 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
			{ WidthHalf, HeightHalf, DepthHalf , 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f , 1.0f, 0.0f},
			{ WidthHalf, -HeightHalf, DepthHalf , 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f , 1.0f, 1.0f},
		};

		BoxMeshData->Indices = {
			// front face
			0, 2, 1,
			0, 3, 2,

			// back face
			5, 6, 7,
			5, 7, 4,

			// top face
			8, 11, 10,
			8, 10, 9,

			// bottom face
			15, 12, 13,
			15, 13, 14,

			// left face
			19, 18, 17,
			19, 17, 16,

			// right face
			20, 21, 22,
			20, 22, 23
		};

		for (auto i = 0; i < NumSubdivisions; ++i)
		{
			Subdivide(BoxMeshData.get());
		}

		return BoxMeshData;
	}

	std::unique_ptr<FMeshData> FMeshGenerator::CreateSphere(
		float Radius,
		uint32 NumVSubdivisions,
		uint32 NumHSubdivisions) const noexcept
	{
		auto SphereMeshData = std::make_unique<FMeshData>("Sphere");
		SphereMeshData->Vertices.resize(2 + (NumVSubdivisions - 1)*(NumHSubdivisions + 1));
		SphereMeshData->Indices.resize(NumHSubdivisions*NumVSubdivisions * 2 * 3);

		FVertex TopVertex = { 0.0f, Radius, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		FVertex BottomVertex = { 0.0f, -Radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
		
		SphereMeshData->Vertices[0] = std::move(TopVertex);

		float PhiStep = XM_PI / NumVSubdivisions;
		float ThetaStep = 2.0f*XM_PI / NumHSubdivisions;

		uint64 iVertex = 1;
		for (auto iVSubdivision = 1; iVSubdivision <= NumVSubdivisions - 1; ++iVSubdivision)
		{
			float Phi = PhiStep * iVSubdivision;

			for (auto iHSubdivision = 0; iHSubdivision <= NumHSubdivisions; ++iHSubdivision, ++iVertex)
			{
				float Theta = ThetaStep*iHSubdivision;

				const float SinPhi = sinf(Phi);
				const float CosPhi = cosf(Phi);
				const float SinTheta= sinf(Theta);
				const float CosTheta = cosf(Theta);

				FVertex Vertex;

				// Convert Spherical coordinates to Cartesian
				Vertex.Position.x = Radius*SinPhi*CosTheta;
				Vertex.Position.y = Radius*CosPhi;
				Vertex.Position.z = Radius*SinPhi*SinTheta;

				// Partial derivative of Position with respect to Theta
				Vertex.Tangent.x = -Radius*SinPhi*SinTheta;
				Vertex.Tangent.y = 0.0f;
				Vertex.Tangent.z = Radius*SinPhi*CosTheta;

				Vertex.TexC.x = Theta / XM_2PI;
				Vertex.TexC.y = Phi / XM_PI;

				// Normalize tangent vector and store it as tangent vector
				XMStoreFloat3(&Vertex.Tangent, XMVector3Normalize(XMLoadFloat3(&Vertex.Tangent)));

				// Normalize position vector and store it as normal vector
				XMStoreFloat3(&Vertex.Normal, XMVector3Normalize(XMLoadFloat3(&Vertex.Position)));

				SphereMeshData->Vertices[iVertex] = std::move(Vertex);
			}
		}

		SphereMeshData->Vertices[iVertex] = std::move(BottomVertex);


		int iIndex = 0;
		for (auto iVertex = 1; iVertex <= NumHSubdivisions; ++iVertex, iIndex+=3)
		{
			SphereMeshData->Indices[iIndex] = 0;
			SphereMeshData->Indices[iIndex + 1] = iVertex + 1;
			SphereMeshData->Indices[iIndex + 2] = iVertex;
		}


		const auto NumVertexInRing = NumHSubdivisions + 1;
		for (auto iVSubdivision = 0; iVSubdivision < NumVSubdivisions-2; ++iVSubdivision)
		{
			for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions; ++iHSubdivision, iIndex +=6)
			{

				SphereMeshData->Indices[iIndex] = 1 + iVSubdivision * NumVertexInRing + iHSubdivision;
				SphereMeshData->Indices[iIndex+1] = 1 + iVSubdivision * NumVertexInRing + iHSubdivision+1;
				SphereMeshData->Indices[iIndex+2] = 1 + (iVSubdivision+1) * NumVertexInRing + iHSubdivision;
				
				SphereMeshData->Indices[iIndex+3] = 1 + (iVSubdivision+1) * NumVertexInRing+ iHSubdivision;
				SphereMeshData->Indices[iIndex+4] = 1 + iVSubdivision * NumVertexInRing+ iHSubdivision+1;
				SphereMeshData->Indices[iIndex+5] = 1 + (iVSubdivision+1) * NumVertexInRing+ iHSubdivision+1;
			}
		}

		const auto iBottomVertex = SphereMeshData->Vertices.size() - 1;
		const auto iLastRingBaseVertex = iBottomVertex - NumVertexInRing;
		for (auto iVertex = 0; iVertex < NumHSubdivisions; ++iVertex, iIndex += 3)
		{
			SphereMeshData->Indices[iIndex] = iBottomVertex;
			SphereMeshData->Indices[iIndex + 1] = iLastRingBaseVertex + iVertex;
			SphereMeshData->Indices[iIndex + 2] = iLastRingBaseVertex + iVertex+1;
		}

		return SphereMeshData;
	}

	std::unique_ptr<FMeshData> FMeshGenerator::CreateGrid(float Width, float Height, uint32 NumVSubdivisions, uint32 NumHSubdivisions) const noexcept
	{
		auto MeshData = std::make_unique<FMeshData>("Grid");

		uint32 NumVertex = (NumHSubdivisions+1) * (NumVSubdivisions+1);
		MeshData->Vertices.resize(NumVertex);
		
		uint32 NumIndices = (NumHSubdivisions * NumVSubdivisions)*6;
		MeshData->Indices.resize(NumIndices);

		const auto HalfHeight = Height/2.0f;
		const auto HalfWidth = Width / 2.0f;

		const auto HeightStep = Height / NumVSubdivisions;
		const auto WidthStep = Width / NumHSubdivisions;

		for (auto iVSubdivision = 0, iVertex=0; iVSubdivision < NumVSubdivisions; ++iVSubdivision)
		{
			float Z = -HalfHeight + iVSubdivision * HeightStep;
			for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions; ++iHSubdivision, ++iVertex)
			{
				float X = -HalfWidth + iHSubdivision * WidthStep;

				FVertex VertexData;
				VertexData.Position = { X, 0.0f, Z };
				VertexData.Normal = { 0.0f, 1.0f, 0.0f };
				VertexData.Tangent = { 1.0f, 0.0f, 0.0f };
				VertexData.TexC.x = (float)iHSubdivision / (NumHSubdivisions-1);
				VertexData.TexC.y = (float)iVSubdivision / (NumVSubdivisions- 1);


				MeshData->Vertices[iVertex] = std::move(VertexData);
			}
		}

		for (auto iVSubdivision = 0, iIndex=0; iVSubdivision < NumVSubdivisions-1; ++iVSubdivision)
		{
			for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions-1; ++iHSubdivision, iIndex+=6)
			{
				MeshData->Indices[iIndex] = iVSubdivision*NumHSubdivisions+ iHSubdivision;
				MeshData->Indices[iIndex+1] = (iVSubdivision + 1)*NumHSubdivisions+ iHSubdivision;
				MeshData->Indices[iIndex+2] = (iVSubdivision + 1)*NumHSubdivisions + iHSubdivision + 1;

				MeshData->Indices[iIndex+3] = iVSubdivision * NumHSubdivisions + iHSubdivision;
				MeshData->Indices[iIndex+4] = (iVSubdivision + 1)*NumHSubdivisions+ iHSubdivision + 1;
				MeshData->Indices[iIndex+5] = iVSubdivision*NumHSubdivisions+iHSubdivision+1;
			}
		}

		return MeshData;
	}

	std::unique_ptr<WoodenEngine::FMeshData> FMeshGenerator::CreateLandscapeGrid(
		float Width,
		float Height, 
		uint32 NumVSubdivisions, 
		uint32 NumHSubdivisions) const noexcept
	{
		auto GridData = CreateGrid(Width, Height, NumVSubdivisions, NumHSubdivisions);

		/* Change Y coordinate of grid's vertices, based on the formula:
		 * y = 0.5*(sin(0.2x)x + cos(0.2z)z)
		 * By finding partial derivatives, we can find normal vector:
		 * diff(y, x) = 3*cos(x)
		 * diff(y, z) = -3*sin(x)
		 * TangentX = [1 3*cos(x) 0]
		 * TangentZ = [0 -3*sin(z) 1]
		 * Normal = Cross(TangentZ, TangentX)
		 * Normal = [-3cosx 1 3sinz]
		 */

		
		for (auto& Vertex : GridData->Vertices)
		{
			float z = Vertex.Position.z;
			float x = Vertex.Position.x;

			Vertex.Position.y = 0.5*(sinf(x*0.2)*x + cosf(z*0.2)*z);
			
			Vertex.Tangent.x = 1;
			Vertex.Tangent.y = 
				cosf(z / 5) / 2.0f + sinf(x / 5) / 2.0f + x * cosf(x / 5) / 10.0f - z * sinf(z / 5) / 10;
			Vertex.Tangent.z = 1;
			XMStoreFloat3(&Vertex.Tangent, XMVector3Normalize(XMLoadFloat3(&Vertex.Tangent)));

			Vertex.Normal.x = -sinf(x / 5) / 2 - x * cosf(x / 5) / 10;
			Vertex.Normal.y = 1;
			Vertex.Normal.z = z * sinf(z / 5) / 10 - cosf(z / 5) / 2;
			XMStoreFloat3(&Vertex.Normal, XMVector3Normalize(XMLoadFloat3(&Vertex.Normal)));
		}

		return GridData;
	}

	void FMeshGenerator::Subdivide(FMeshData* MeshData) const noexcept
	{
		const auto CopiedMeshData = *MeshData;

		MeshData->Vertices.resize(0);
		MeshData->Indices.resize(0);

		const auto NumTriangles = CopiedMeshData.Indices.size() / 3;
		for (auto i = 0; i < NumTriangles; ++i)
		{
			auto V0 = CopiedMeshData.Vertices[CopiedMeshData.Indices[i * 3 + 0]];
			auto V1 = CopiedMeshData.Vertices[CopiedMeshData.Indices[i * 3 + 1]];
			auto V2 = CopiedMeshData.Vertices[CopiedMeshData.Indices[i * 3 + 2]];

			auto M0 = FVertex::MidPoint(V0, V1);
			auto M1 = FVertex::MidPoint(V1, V2);
			auto M2 = FVertex::MidPoint(V0, V2);

			MeshData->Vertices.push_back(std::move(V0));
			MeshData->Vertices.push_back(std::move(V1));
			MeshData->Vertices.push_back(std::move(V2));
			MeshData->Vertices.push_back(std::move(M0));
			MeshData->Vertices.push_back(std::move(M1));
			MeshData->Vertices.push_back(std::move(M2));

			MeshData->Indices.push_back(i * 6 + 0);
			MeshData->Indices.push_back(i * 6 + 3);
			MeshData->Indices.push_back(i * 6 + 5);

			MeshData->Indices.push_back(i * 6 + 3);
			MeshData->Indices.push_back(i * 6 + 4);
			MeshData->Indices.push_back(i * 6 + 5);

			MeshData->Indices.push_back(i * 6 + 5);
			MeshData->Indices.push_back(i * 6 + 4);
			MeshData->Indices.push_back(i * 6 + 2);

			MeshData->Indices.push_back(i * 6 + 3);
			MeshData->Indices.push_back(i * 6 + 1);
			MeshData->Indices.push_back(i * 6 + 4);
		}
	}

	std::unique_ptr<FMeshData> FMeshParser::ParseTxtData(const std::string& FilePath) const
	{
		// use smart pointer for preventing memory leak if exception is thrown
		auto MeshData = std::make_unique<FMeshData>();

		std::ifstream File;
		File.open(FilePath, std::ios_base::in);

		if (!File.is_open())
		{
			throw std::invalid_argument("File can't be opened by path " + FilePath);
		}

		if (File.eof())
		{
			throw std::invalid_argument("File " + FilePath + "is empty");
		}
		
		uint16 NumVertices;
		File >> NumVertices;
		MeshData->Vertices.reserve(NumVertices);
		
		uint16 NumIndices;
		File >> NumIndices;
		MeshData->Indices.reserve(NumIndices*3);

		auto VerticesIter = MeshData->Vertices.cend();
		std::string StrVertexData;
		for (auto iVertex = 0; iVertex < MeshData->Vertices.capacity(); ++iVertex)
		{
			FVertex Vertex;
			File >> Vertex.Position.x >> Vertex.Position.y >> Vertex.Position.z;
			File >> Vertex.Normal.x >> Vertex.Normal.y >> Vertex.Normal.z;

			VerticesIter = MeshData->Vertices.insert(VerticesIter, std::move(Vertex));
			VerticesIter++;
		}

		auto IndicesIter = MeshData->Indices.cend();
		std::string StrIndexData;
		for (auto iIndex = 0; iIndex < MeshData->Indices.capacity(); ++iIndex)
		{
			uint16 Index;
			File >> Index;
			IndicesIter = MeshData->Indices.insert(IndicesIter, std::move(Index));
			IndicesIter++;
		}

		File.close();
		return MeshData;
	}
}