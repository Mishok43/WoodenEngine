#include "FilterBlur.h"
#include "EngineSettings.h"
#include "Common/DirectXHelper.h"

namespace WoodenEngine
{
	FFilterBlur::FFilterBlur()
	{
	}

	FFilterBlur::~FFilterBlur()
	{
	}

	void FFilterBlur::Init(
		uint16_t RenderTargetWidth,
		uint16_t RenderTargetHeight,
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUDescriptorHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUDescriptorHandle,
		uint16 SRVUAVDescriptorHandleIncrementSize,
		ComPtr<ID3D12GraphicsCommandList> CMDList,
		ComPtr<ID3D12Device> Device)
	{
		this->RenderTargetWidth = RenderTargetWidth;
		this->RenderTargetHeight = RenderTargetHeight;

		InitResources(CMDList, Device);
		BuildDescriptors(
			SRVUAVCPUDescriptorHandle, 
			SRVUAVGPUDescriptorHandle, 
			SRVUAVDescriptorHandleIncrementSize, 
			CMDList, Device);
	}

	void FFilterBlur::InitResources(
		ComPtr<ID3D12GraphicsCommandList> CMDList,
		ComPtr<ID3D12Device> Device)
	{
		D3D12_RESOURCE_DESC TexDesc;
		ZeroMemory(&TexDesc, sizeof(D3D12_RESOURCE_DESC));
		TexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TexDesc.Alignment = 0;
		TexDesc.Width = RenderTargetWidth;
		TexDesc.Height = RenderTargetHeight;
		TexDesc.DepthOrArraySize = 1;
		TexDesc.MipLevels = 1;
		TexDesc.Format = BufferFormat;
		TexDesc.SampleDesc.Count = 1;
		TexDesc.SampleDesc.Quality = 0;
		TexDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		TexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		DX::ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&TexDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
			IID_PPV_ARGS(&BlurResourceA)
		));

		DX::ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&TexDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
			IID_PPV_ARGS(&BlurResourceB)
		));
	}

	void FFilterBlur::BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVUAVCPUHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE SRVUAVGPUHandle,
		uint16 SRVUAVDescriptorHandleIncrementSize,
		ComPtr<ID3D12GraphicsCommandList> CMDList,
		ComPtr<ID3D12Device> Device)
	{

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVBlurResourceDesc;
		ZeroMemory(&SRVBlurResourceDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
		SRVBlurResourceDesc.Format = BufferFormat;
		SRVBlurResourceDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVBlurResourceDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVBlurResourceDesc.Texture2D.MostDetailedMip = 0;
		SRVBlurResourceDesc.Texture2D.MipLevels = 1;

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVBlurResourceDesc;
		ZeroMemory(&UAVBlurResourceDesc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
		UAVBlurResourceDesc.Format = BufferFormat;
		UAVBlurResourceDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;



		BlurASRVCPUDescriptorHandle = SRVUAVCPUHandle;
		BlurAUAVCPUDescriptorHandle = SRVUAVCPUHandle.Offset(1, SRVUAVDescriptorHandleIncrementSize);
		BlurBSRVCPUDescriptorHandle = SRVUAVCPUHandle.Offset(1, SRVUAVDescriptorHandleIncrementSize);
		BlurBUAVCPUDescriptorHandle = SRVUAVCPUHandle.Offset(1, SRVUAVDescriptorHandleIncrementSize);


		BlurASRVGPUDescriptorHandle = SRVUAVGPUHandle;
		BlurAUAVGPUDescriptorHandle = SRVUAVGPUHandle.Offset(1, SRVUAVDescriptorHandleIncrementSize);
		BlurBSRVGPUDescriptorHandle = SRVUAVGPUHandle.Offset(1, SRVUAVDescriptorHandleIncrementSize);
		BlurBUAVGPUDescriptorHandle = SRVUAVGPUHandle.Offset(1, SRVUAVDescriptorHandleIncrementSize);

		Device->CreateShaderResourceView(
			BlurResourceA.Get(), &SRVBlurResourceDesc, BlurASRVCPUDescriptorHandle);

		Device->CreateUnorderedAccessView(
			BlurResourceA.Get(), nullptr, &UAVBlurResourceDesc, BlurAUAVCPUDescriptorHandle);

		Device->CreateShaderResourceView(
			BlurResourceB.Get(), &SRVBlurResourceDesc, BlurBSRVCPUDescriptorHandle);

		Device->CreateUnorderedAccessView(
			BlurResourceB.Get(), nullptr, &UAVBlurResourceDesc, BlurBUAVCPUDescriptorHandle);
	}

	void FFilterBlur::Execute(
		ComPtr<ID3D12PipelineState> HorzBlurPSO,
		ComPtr<ID3D12PipelineState> VertBlurPSO,
		ComPtr<ID3D12RootSignature> RootSig,
		ComPtr<ID3D12GraphicsCommandList> CMDList,
		ID3D12Resource* Input,
		uint8_t BlurCount/* =1 */)
	{
		auto BlurGaussWeights = CalcGaussWeights(2.5f);
		auto BlurRadius = (uint32_t)BlurGaussWeights.size() / 2;

		CMDList->SetComputeRootSignature(RootSig.Get());
		CMDList->SetComputeRoot32BitConstants(0, 1, &BlurRadius, 0);
		CMDList->SetComputeRoot32BitConstants(0, BlurGaussWeights.size(), BlurGaussWeights.data(), 1);

		std::array<D3D12_RESOURCE_BARRIER, 2> Barriers_0 = {
			CD3DX12_RESOURCE_BARRIER::Transition(
				Input,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_COPY_SOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceA.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST),
		};

		CMDList->ResourceBarrier(2, Barriers_0.data());

		CMDList->CopyResource(BlurResourceA.Get(), Input);

		std::array<D3D12_RESOURCE_BARRIER, 2> Barriers_1 = {
			CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceA.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_GENERIC_READ),
			CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceB.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		};

		
		CMDList->ResourceBarrier(2, Barriers_1.data());

		std::array<D3D12_RESOURCE_BARRIER, 2> Barriers_2 = {
			CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceA.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
			CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceB.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_GENERIC_READ),
		};

		std::array<D3D12_RESOURCE_BARRIER, 2> Barriers_3 = {
			CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceA.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_GENERIC_READ),
			CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceB.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		};


		for (uint8_t i = 0; i < BlurCount; ++i)
		{
			CMDList->SetPipelineState(HorzBlurPSO.Get());
			CMDList->SetComputeRootDescriptorTable(1, BlurASRVGPUDescriptorHandle);
			CMDList->SetComputeRootDescriptorTable(2, BlurBUAVGPUDescriptorHandle);

			auto NumGroupsX = (uint16_t)ceilf(RenderTargetWidth / 256.0f);
			CMDList->Dispatch(NumGroupsX, RenderTargetHeight, 1);

			CMDList->ResourceBarrier(2, Barriers_2.data());

			CMDList->SetPipelineState(VertBlurPSO.Get());
			CMDList->SetComputeRootDescriptorTable(1, BlurBSRVGPUDescriptorHandle);
			CMDList->SetComputeRootDescriptorTable(2, BlurAUAVGPUDescriptorHandle);

			auto NumGroupsY = (uint16_t)ceilf(RenderTargetHeight / 256.0f);
			CMDList->Dispatch(RenderTargetWidth, NumGroupsY, 1);

			CMDList->ResourceBarrier(2, Barriers_3.data());
		}

		CMDList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				BlurResourceB.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
				D3D12_RESOURCE_STATE_COMMON));
	}

	std::vector<float> FFilterBlur::CalcGaussWeights(float Sigma) const
	{
		auto Denominator = 2.0*Sigma*Sigma;
		auto BlurRadius = (uint8_t)ceil(2.0f*Sigma);

		std::vector<float> Weights;
		Weights.resize(BlurRadius * 2 + 1);

		auto WeightsSum = 0.0f;
		for (int Position = -BlurRadius; Position  <= BlurRadius; Position+=BlurRadius)
		{
			auto X = (float)Position;
			Weights[Position+BlurRadius] = expf(-X*X/ Denominator);
			WeightsSum += Weights[Position+BlurRadius];
		}

		for (uint8_t i = 0; i < Weights.size(); ++i)
		{
			Weights[i] /= WeightsSum;
		}

		return Weights;
	}	
}