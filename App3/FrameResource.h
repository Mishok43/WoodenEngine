#pragma once

#include "Common/DirectXHelper.h"
#include "ShaderStructures.h"

namespace WoodenEngine
{
	/*!
	 * \class FFrameResources
	 *
	 * \brief Stores the resources needed for the CPU to build the command lists
	 *
	 * \author devmi
	 * \date April 2018
	 */
	struct FFrameResource
	{
	public:
		FFrameResource() = default;
		FFrameResource(ComPtr<ID3D12Device> Device, const uint64 NumberObjects);
		~FFrameResource();

		FFrameResource(const FFrameResource& FrameResources) = delete;
		FFrameResource(FFrameResource&& FrameResource) = delete;
		FFrameResource& operator=(const FFrameResource& FrameResources) = delete;

		ComPtr<ID3D12CommandAllocator> CmdListAllocator;

		// Const data for shaders
		std::unique_ptr<DX::FUploadBuffer<SFrameData>> FrameDataBuffer = nullptr;

		// Per object data for shaders
		std::unique_ptr<DX::FUploadBuffer<SObjectData>> ObjectsDataBuffer = nullptr;
		
		// Fence of command list
		uint64 Fence = 0;
	};
}