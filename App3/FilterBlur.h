#pragma once

#include <array>
#include "pch.h"

namespace WoodenEngine
{
	class FFilterBlur
	{
	public:
		FFilterBlur();
		~FFilterBlur();

		FFilterBlur(const FFilterBlur& FilterBlur) = delete;
		FFilterBlur& operator=(const FFilterBlur& FilterBlur) = delete;
		FFilterBlur(FFilterBlur&& FilterBlur) = delete;
		FFilterBlur& operator=(FFilterBlur&& FilterBlur) = delete;

		void Init(
			uint16_t RenderTargetWidth,
			uint16_t RenderTargetHeight,
			CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUDescriptorHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUDescriptorHandle,
			uint16 SRVUAVDescriptorHandleIncrementSize,
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

		void Execute(
			ComPtr<ID3D12PipelineState> HorizBlurPSO,
			ComPtr<ID3D12PipelineState> VertBlurPSO,
			ComPtr<ID3D12RootSignature> RootSignature,
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ID3D12Resource* Input,
			uint8_t BlurCount=1);


		inline ID3D12Resource* Output() const noexcept
		{
			return BlurResourceA.Get();
		}
	
	protected:
		void InitResources(
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

		void BuildDescriptors(
			CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUDescriptorHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUDescriptorHandle,
			uint16 SRVUAVDescriptorHandleIncrementSize,
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

		std::vector<float> CalcGaussWeights(float Sigma) const;

	private:
		// CPU Descriptor Handles for BlurA and BlurB Resources
		CD3DX12_CPU_DESCRIPTOR_HANDLE BlurASRVCPUDescriptorHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE BlurAUAVCPUDescriptorHandle;

		CD3DX12_CPU_DESCRIPTOR_HANDLE BlurBSRVCPUDescriptorHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE BlurBUAVCPUDescriptorHandle;

		// GPU Descriptor Handles for BlurA and BlurB Resources
		CD3DX12_GPU_DESCRIPTOR_HANDLE BlurASRVGPUDescriptorHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE BlurAUAVGPUDescriptorHandle;

		CD3DX12_GPU_DESCRIPTOR_HANDLE BlurBSRVGPUDescriptorHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE BlurBUAVGPUDescriptorHandle;

		uint16_t RenderTargetWidth;
		uint16_t RenderTargetHeight;

		ComPtr<ID3D12Resource> BlurResourceA;
		ComPtr<ID3D12Resource> BlurResourceB;
		uint8_t MaxBlurRadius;

	};
}
