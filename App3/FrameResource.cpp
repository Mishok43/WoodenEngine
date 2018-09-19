#include "FrameResource.h"
#include "Common/DirectXHelper.h"

namespace WoodenEngine
{
	FFrameResource::FFrameResource(ComPtr<ID3D12Device> Device, const uint64 NumObjects, const uint64 NumMaterials)
	{
		assert(Device != nullptr);
		assert(NumObjects != 0);

		DX::ThrowIfFailed(Device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&CmdListAllocator)));

		FrameDataBuffer = std::make_unique<DX::FUploadBuffer<SFrameData>>(Device, 2, true);
		ObjectsDataBuffer = std::make_unique<DX::FUploadBuffer<SObjectData>>(Device, NumObjects, true);
		MaterialsDataBuffer = std::make_unique<DX::FUploadBuffer<SMaterialData>>(Device, NumMaterials, true);
	}
}
