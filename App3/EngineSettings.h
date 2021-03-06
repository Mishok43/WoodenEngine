#pragma once
#include "pch.h"


namespace WoodenEngine
{
	// number frame buffers for swap chain
	static constexpr uint8 NMR_SWAP_BUFFERS = 3;

	static const DXGI_FORMAT BufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
}