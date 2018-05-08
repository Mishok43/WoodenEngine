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
		float Depth
	) const noexcept
	{
		auto BoxMeshData = std::make_unique<FMeshData>("Box");

		const auto WidthHalf = Width / 2.0f;
		const auto HeightHalf = Height / 2.0f;
		const auto DepthHalf = Depth / 2.0f;

		BoxMeshData->Vertices = {
			// Front face
			{ -WidthHalf, -HeightHalf, -DepthHalf, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f},
			{ WidthHalf, -HeightHalf, -DepthHalf, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f},
			{ WidthHalf, HeightHalf, -DepthHalf , 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f},
			{ -WidthHalf, HeightHalf, -DepthHalf , 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f},

			// Back face
			{ -WidthHalf, -HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f },
			{ WidthHalf, -HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f},
			{ WidthHalf, HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f},
			{ -WidthHalf, HeightHalf, DepthHalf , 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f},

			// Top face
			{ -WidthHalf, HeightHalf, -DepthHalf , 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{ WidthHalf, HeightHalf, -DepthHalf , 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{ WidthHalf, HeightHalf, DepthHalf , 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{ -WidthHalf, HeightHalf, DepthHalf , 0.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f },

			// Bottom face
			{ -WidthHalf, -HeightHalf, -DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f },
			{ WidthHalf, -HeightHalf, -DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f },
			{ WidthHalf, -HeightHalf, DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f },
			{ -WidthHalf, -HeightHalf, DepthHalf , 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f },

			// Left face
			{ -WidthHalf, -HeightHalf, -DepthHalf, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ -WidthHalf, HeightHalf, -DepthHalf , -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ -WidthHalf, HeightHalf, DepthHalf , -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },
			{ -WidthHalf, -HeightHalf, DepthHalf , -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f },

			// Right face
			{ WidthHalf, -HeightHalf, -DepthHalf, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ WidthHalf, HeightHalf, -DepthHalf , 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ WidthHalf, HeightHalf, DepthHalf , 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			{ WidthHalf, -HeightHalf, DepthHalf , 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
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

	template<typename Out>
	void split(const std::string &s, char delim, Out result)
	{
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
		{
			*(result++) = item;
		}
	}

	std::vector<std::string> split(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}

	std::unique_ptr<FMeshData> FMeshParser::ParseMeshData(const std::string& FilePath) const
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
		
		std::string StrNumVertex;
		std::getline(File, StrNumVertex);

		MeshData->Vertices.reserve(std::stoi(StrNumVertex));		
		
		std::string StrNumIndex;
		std::getline(File, StrNumIndex);

		MeshData->Indices.reserve(std::stoi(StrNumIndex));

		auto VerticesIter = MeshData->Vertices.cend();
		std::string StrVertexData;
		for (auto iVertex = 0; iVertex < MeshData->Vertices.capacity(); ++iVertex)
		{
			std::getline(File, StrVertexData);
			auto VertexData = split(StrVertexData, ' ');
			
			FVertex Vertex;
			Vertex.Position.x = std::stof(std::move(VertexData[0]));
			Vertex.Position.y = std::stof(std::move(VertexData[1]));
			Vertex.Position.z = std::stof(std::move(VertexData[2]));

			VerticesIter = MeshData->Vertices.insert(VerticesIter, std::move(Vertex));
			VerticesIter++;
		}

		auto IndicesIter = MeshData->Indices.cend();
		std::string StrIndexData;
		for (auto iIndex = 0; iIndex < MeshData->Indices.capacity()/3; ++iIndex)
		{
			std::getline(File, StrIndexData);
			auto IndexData = split(StrIndexData, ' ');

			IndicesIter = MeshData->Indices.insert(IndicesIter, std::stoi(std::move(IndexData[0])));
			IndicesIter++;

			IndicesIter = MeshData->Indices.insert(IndicesIter, std::stoi(std::move(IndexData[1])));
			IndicesIter++;

			IndicesIter = MeshData->Indices.insert(IndicesIter, std::stoi(std::move(IndexData[2])));
			IndicesIter++;
		}

		File.close();
		return MeshData;
	}
}