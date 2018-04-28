#pragma once
#include "pch.h"

template<typename T>
class FUploadBuffer
{
public:
	FUploadBuffer() = delete;


private:
	ComPtr<ID3D12Resource> Buffer;
	UINT64 ByteSize;
	bool bIsConstantBuffer;
};