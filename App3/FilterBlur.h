#pragma once

#include <array>
#include "pch.h"

namespace WoodenEngine
{
	/*!
	 * \class FFilterBlur
	 *
	 * \brief PostProcess blurring effect manager class
	 *
	 * \author devmi
	 * \date September 2018
	 */
	class FFilterBlur
	{
	public:
		FFilterBlur();
		~FFilterBlur();

		FFilterBlur(const FFilterBlur& FilterBlur) = delete;
		FFilterBlur& operator=(const FFilterBlur& FilterBlur) = delete;
		FFilterBlur(FFilterBlur&& FilterBlur) = delete;
		FFilterBlur& operator=(FFilterBlur&& FilterBlur) = delete;

		/** @brief Initializes resources, descriptors
		  * @param RenderTargetWidth Buffer width (uint16_t)
		  * @param RenderTargetHeight Buffer height (uint16_t)
		  * @param SRVUAVCPUDescriptorHandle CPU descriptor handle to heap of SRV,UAV,CBV (CD3DX12_CPU_DESCRIPTOR_HANDLE)
		  * @param SRVUAVGPUDescriptorHandle GPU descriptor handle to heap of SRV,UAV,CBV  (CD3DX12_GPU_DESCRIPTOR_HANDLE)
		  * @param SRVUAVDescriptorHandleIncrementSize Descriptor handle size (uint16)
		  * @param CMDList ComPtr to Graphics Command List (ComPtr<ID3D12GraphicsCommandList>)
		  * @param Device Device (ComPtr<ID3D12Device>)
		  * @return (void)
		  */
		void Init(
			uint16_t RenderTargetWidth,
			uint16_t RenderTargetHeight,
			CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUDescriptorHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUDescriptorHandle,
			uint16 SRVUAVDescriptorHandleIncrementSize,
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

		/** @brief Applies blurring post-process effect to input resource
		  * @param HorizBlurPSO Horizontal blurring Pipepline State Object (ComPtr<ID3D12PipelineState>)
		  * @param VertBlurPSO Vertical blurring Pipepline State Object (ComPtr<ID3D12PipelineState>)
		  * @param RootSignature Compute shader root signature (ComPtr<ID3D12RootSignature>)
		  * @param CMDList Command List (ComPtr<ID3D12GraphicsCommandList>)
		  * @param Input Input Resource (ID3D12Resource *)
		  * @param BlurCount Number of applied blurrings (uint8_t)
		  * @return (void)
		  */
		void Execute(
			ComPtr<ID3D12PipelineState> HorizBlurPSO,
			ComPtr<ID3D12PipelineState> VertBlurPSO,
			ComPtr<ID3D12RootSignature> RootSignature,
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ID3D12Resource* Input,
			uint8_t BlurCount=1);


		/** @brief Returns output of the effect
		  * @return outputs with applied effect (ID3D12Resource*)
		  */
		inline ID3D12Resource* Output() const noexcept
		{
			return BlurResourceA.Get();
		}
	
	protected:
		/** @brief Initializes resources
		  * @param CMDList (ComPtr<ID3D12GraphicsCommandList>)
		  * @param Device (ComPtr<ID3D12Device>)
		  * @return (void)
		  */
		void InitResources(
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

		/** @brief Initializes descriptors
		  * @param SRVUAVCPUDescriptorHandle (CD3DX12_CPU_DESCRIPTOR_HANDLE)
		  * @param SRVUAVGPUDescriptorHandle (CD3DX12_GPU_DESCRIPTOR_HANDLE)
		  * @param SRVUAVDescriptorHandleIncrementSize (uint16)
		  * @param CMDList (ComPtr<ID3D12GraphicsCommandList>)
		  * @param Device (ComPtr<ID3D12Device>)
		  * @return (void)
		  */
		void BuildDescriptors(
			CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUDescriptorHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUDescriptorHandle,
			uint16 SRVUAVDescriptorHandleIncrementSize,
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

		/** @brief Calculates array of gauss normalized weights for blurring
		  * @param Sigma (float)
		  * @return array of gauss normalized weights (std::vector<float>)
		  */
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
