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
			4, 1, 5,

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
		SphereMeshData.Indices.resize((2*2*NumHSubdivisions + (NumVSubdivisions-2)*(NumHSubdivisions)*2)*3);
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
			for (auto iHSubdivision = 0; iHSubdivision < NumHSubdivisions; ++iHSubdivision, VertexIndex+=6)
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

	FMeshData FMeshParser::ParseMeshData(const std::string& FilePath) const
	{
		auto MeshData = FMeshData{};

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

		MeshData.Vertices.reserve(std::stoi(StrNumVertex));		
		
		std::string StrNumIndex;
		std::getline(File, StrNumIndex);

		MeshData.Indices.reserve(std::stoi(StrNumIndex));

		auto VerticesIter = MeshData.Vertices.cend();
		std::string StrVertexData;
		for (auto iVertex = 0; iVertex < MeshData.Vertices.capacity(); ++iVertex)
		{
			std::getline(File, StrVertexData);
			auto VertexData = split(StrVertexData, ' ');
			
			FVertex Vertex;
			Vertex.Position.x = std::stof(std::move(VertexData[0]));
			Vertex.Position.y = std::stof(std::move(VertexData[1]));
			Vertex.Position.z = std::stof(std::move(VertexData[2]));

			VerticesIter = MeshData.Vertices.insert(VerticesIter, std::move(Vertex));
			VerticesIter++;
		}

		auto IndicesIter = MeshData.Indices.cend();
		std::string StrIndexData;
		for (auto iIndex = 0; iIndex < MeshData.Indices.capacity()/3; ++iIndex)
		{
			std::getline(File, StrIndexData);
			auto IndexData = split(StrIndexData, ' ');

			IndicesIter = MeshData.Indices.insert(IndicesIter, std::stoi(std::move(IndexData[0])));
			IndicesIter++;

			IndicesIter = MeshData.Indices.insert(IndicesIter, std::stoi(std::move(IndexData[1])));
			IndicesIter++;

			IndicesIter = MeshData.Indices.insert(IndicesIter, std::stoi(std::move(IndexData[2])));
			IndicesIter++;
		}

		File.close();
		return MeshData;
	}
}