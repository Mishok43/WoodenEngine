﻿#pragma once

#include <ppltasks.h>	// For create_task
#include <D3Dcompiler.h>
#include "pch.h"


namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Function that reads from a binary file asynchronously.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([](StorageFile^ file)
		{
			return FileIO::ReadBufferAsync(file);
		}).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}


	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target)
	{
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hr = S_OK;

		ComPtr<ID3DBlob> byteCode = nullptr;
		ComPtr<ID3DBlob> errors;
		hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

		if (errors != nullptr)
			OutputDebugStringA((char*)errors->GetBufferPointer());

		ThrowIfFailed(hr);

		return byteCode;

	}

	// Assign a name to the object to aid with debugging.
#if defined(_DEBUG)
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}
#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
#endif


	//------------------------------------------------------------------------------------------------
	struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE() {}
		explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE &o) :
			D3D12_CPU_DESCRIPTOR_HANDLE(o)
		{
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) { ptr = 0; }
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(other, offsetScaledByIncrementSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			ptr += offsetInDescriptors * descriptorIncrementSize;
			return *this;
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncrementSize)
		{
			ptr += offsetScaledByIncrementSize;
			return *this;
		}
		bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
		{
			return (ptr == other.ptr);
		}
		bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
		{
			return (ptr != other.ptr);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE &operator=(const D3D12_CPU_DESCRIPTOR_HANDLE &other)
		{
			ptr = other.ptr;
			return *this;
		}

		inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(*this, base, offsetScaledByIncrementSize);
		}

		inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
		}

		static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize)
		{
			handle.ptr = base.ptr + offsetScaledByIncrementSize;
		}

		static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			handle.ptr = base.ptr + offsetInDescriptors * descriptorIncrementSize;
		}
	};

	//------------------------------------------------------------------------------------------------
	struct CD3DX12_GPU_DESCRIPTOR_HANDLE : public D3D12_GPU_DESCRIPTOR_HANDLE
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE() {}
		explicit CD3DX12_GPU_DESCRIPTOR_HANDLE(const D3D12_GPU_DESCRIPTOR_HANDLE &o) :
			D3D12_GPU_DESCRIPTOR_HANDLE(o)
		{
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) { ptr = 0; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE &other, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(other, offsetScaledByIncrementSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			ptr += offsetInDescriptors * descriptorIncrementSize;
			return *this;
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncrementSize)
		{
			ptr += offsetScaledByIncrementSize;
			return *this;
		}
		inline bool operator==(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other)
		{
			return (ptr == other.ptr);
		}
		inline bool operator!=(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other)
		{
			return (ptr != other.ptr);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE &operator=(const D3D12_GPU_DESCRIPTOR_HANDLE &other)
		{
			ptr = other.ptr;
			return *this;
		}

		inline void InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(*this, base, offsetScaledByIncrementSize);
		}

		inline void InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
		}

		static inline void InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize)
		{
			handle.ptr = base.ptr + offsetScaledByIncrementSize;
		}

		static inline void InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			handle.ptr = base.ptr + offsetInDescriptors * descriptorIncrementSize;
		}
	};


}

// Naming helper function for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
#define NAME_D3D12_OBJECT(x) DX::SetName(x.Get(), L#x)
