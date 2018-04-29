#pragma once
#include "pch.h"


namespace WoodenEngine
{
	// number frame buffers for swap chain
	static const uint8 NMR_SWAP_BUFFERS = 3;

	static const DXGI_FORMAT BufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

}