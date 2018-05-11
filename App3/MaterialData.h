#pragma once
#include <iostream>
#include <string>

#include "EngineSettings.h"
#include "TextureData.h"

namespace WoodenEngine
{

	using namespace DirectX;
	/*!
	 * \struct FMaterialData 
	 *
	 * \brief Contains setting data (Diffuze, Specular) of a specific material for rendering.
	 *
	 * \author devmi
	 * \date May 2018
	 */
	struct FMaterialData
	{
		FMaterialData() = default;

		FMaterialData(const std::string& Name):
			Name(Name){}

		// Unique material name
		std::string Name;

		// Index into const buffer of that material
		uint64 iConstBuffer = UINT64_MAX;

		// Number dirty const buffers/frames which has old-day data
		uint8 NumDirtyConstBuffers = NMR_SWAP_BUFFERS;

		// Rendering settings
		XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;

		// Material transform uv-coordinates (for implementing effects)
		XMFLOAT4X4 Transform = MathHelper::Identity4x4();

		const FTextureData* DiffuseTexture = nullptr;
	};
}