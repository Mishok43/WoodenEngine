#pragma once

#include "pch.h"

namespace WoodenEngine
{
	/*!
	* \class FFilterSobel
	*
	* \brief PostProcess sobel effect manager class
	*
	* \author devmi
	* \date September 2018
	*/
	class FFilterSobel
	{
	public:
		FFilterSobel();
		~FFilterSobel();

		FFilterSobel(const FFilterSobel& FilterBlur) = delete;
		FFilterSobel& operator=(const FFilterSobel& FilterBlur) = delete;
		FFilterSobel(FFilterSobel&& FilterBlur) = delete;
		FFilterSobel& operator=(FFilterSobel&& FilterBlur) = delete;

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
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

		/** @brief Applies sobel post-process effect to input resource
		* @param SobelPSO Sobel Pipepline State Object (ComPtr<ID3D12PipelineState>)
		* @param RootSignature Compute shader root signature (ComPtr<ID3D12RootSignature>)
		* @param CMDList Command List (ComPtr<ID3D12GraphicsCommandList>)
		* @param Input Input Resource (ID3D12Resource *)
		* @return ID3D12Resource* Output (void)
		*/
		ID3D12Resource* Execute(
			ComPtr<ID3D12PipelineState> SobelPSO,
			ComPtr<ID3D12RootSignature> RootSignature,
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ID3D12Resource* Input,
			CD3DX12_GPU_DESCRIPTOR_HANDLE InputGPUDescriptorHandle
		);

		
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
			ComPtr<ID3D12GraphicsCommandList> CMDList,
			ComPtr<ID3D12Device> Device
		);

	private:
		CD3DX12_CPU_DESCRIPTOR_HANDLE OutputUAVCPUDescriptorHandle;

		CD3DX12_GPU_DESCRIPTOR_HANDLE OutputUAVGPUDescriptorHandle;

		uint16_t RenderTargetWidth;
		uint16_t RenderTargetHeight;

		ComPtr<ID3D12Resource> OutputResource;
	};
}
