#include "FilterSobel.h"

#include <array>

#include "EngineSettings.h"
#include "Common/DirectXHelper.h"

namespace WoodenEngine
{

	FFilterSobel::FFilterSobel()
	{
	}

	FFilterSobel::~FFilterSobel()
	{
	}

	void FFilterSobel::Init(
		uint16_t RenderTargetWidth,
		uint16_t RenderTargetHeight,
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUDescriptorHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUDescriptorHandle,
		ComPtr<ID3D12GraphicsCommandList> CMDList,
		ComPtr<ID3D12Device> Device)
	{
		this->RenderTargetWidth = RenderTargetWidth;
		this->RenderTargetHeight = RenderTargetHeight;

		InitResources(CMDList, Device);
		BuildDescriptors(SRVUAVCPUDescriptorHandle, 
			SRVUAVGPUDescriptorHandle, 
			CMDList, Device);
	}

	void FFilterSobel::InitResources(
		ComPtr<ID3D12GraphicsCommandList> CMDList, 
		ComPtr<ID3D12Device> Device)
	{
		CD3DX12_RESOURCE_DESC OutputResourceDesc;
		ZeroMemory(&OutputResourceDesc, sizeof(CD3DX12_RESOURCE_DESC));
		OutputResourceDesc.Format = BufferFormat;
		OutputResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		OutputResourceDesc.MipLevels = 1;
		OutputResourceDesc.Width = RenderTargetWidth;
		OutputResourceDesc.Height = RenderTargetHeight;
		OutputResourceDesc.DepthOrArraySize = 1;
		OutputResourceDesc.SampleDesc.Count = 1;
		OutputResourceDesc.SampleDesc.Quality = 0;
		OutputResourceDesc.Alignment = 0;
		OutputResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		DX::ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&OutputResourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&OutputResource)
		));
	}

	void FFilterSobel::BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUDescriptorHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUDescriptorHandle,
		ComPtr<ID3D12GraphicsCommandList> CMDList,
		ComPtr<ID3D12Device> Device)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC OutputUAVDesc;
		ZeroMemory(&OutputUAVDesc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
		OutputUAVDesc.Format = BufferFormat;
		OutputUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		OutputUAVCPUDescriptorHandle = SRVUAVCPUDescriptorHandle;
		OutputUAVGPUDescriptorHandle = SRVUAVGPUDescriptorHandle;

		Device->CreateUnorderedAccessView(
			OutputResource.Get(), nullptr,
			&OutputUAVDesc, OutputUAVCPUDescriptorHandle);
	}

	ID3D12Resource* FFilterSobel::Execute(
		ComPtr<ID3D12PipelineState> SobelPSO,
		ComPtr<ID3D12RootSignature> RootSignature,
		ComPtr<ID3D12GraphicsCommandList> CMDList,
		ID3D12Resource* Input,
		CD3DX12_GPU_DESCRIPTOR_HANDLE InputGPUDescriptorHandle)
	{
		CMDList->SetPipelineState(SobelPSO.Get());
		CMDList->SetComputeRootSignature(RootSignature.Get());


		CMDList->SetComputeRootDescriptorTable(0, InputGPUDescriptorHandle);
		CMDList->SetComputeRootDescriptorTable(1, OutputUAVGPUDescriptorHandle);

		std::array<D3D12_RESOURCE_BARRIER, 2> Barriers_0 = {
			CD3DX12_RESOURCE_BARRIER::Transition(
				OutputResource.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
			CD3DX12_RESOURCE_BARRIER::Transition(
				Input,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ)
		};
		
		CMDList->ResourceBarrier(Barriers_0.size(), Barriers_0.data());

		auto NumXThreadGroups = (uint8)ceilf(RenderTargetWidth / 16.0f);
		auto NumYThreadGroups = (uint8)ceilf(RenderTargetHeight / 16.0f);

		CMDList->Dispatch(NumXThreadGroups, NumYThreadGroups, 1);

		return OutputResource.Get();
	}
}