#include "FrameResource.h"

namespace WoodenEngine
{
	FFrameResource::FFrameResource(ComPtr<ID3D12Device> Device, const uint64 NumberObjects)
	{
		assert(Device != nullptr);
		assert(NumberObjects != 0);

		DX::ThrowIfFailed(Device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&CmdListAllocator)));

		FrameDataBuffer = std::make_unique<DX::FUploadBuffer<SFrameData>>(Device, 1, true);
		ObjectsDataBuffer = std::make_unique<DX::FUploadBuffer<SObjectData>>(Device, NumberObjects, true);
	}
}