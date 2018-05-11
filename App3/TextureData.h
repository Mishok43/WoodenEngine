#pragma once
#include <iostream>
#include <string.h>
#include "pch.h"

/*!
 * \struct FTexture
 *
 * \brief Contains info about a texture and pointer to it in gpu memory
 *
 * \author devmi
 * \date May 2018
 */
struct FTextureData
{
	// Texture name
	std::string Name;

	// File name (because of different languages it's wstring)
	std::wstring FileName;

	ComPtr<ID3D12Resource> Resource = nullptr;

	ComPtr<ID3D12Resource> UploadResource = nullptr;

	uint32 iSRVHeap = UINT32_MAX;
};

