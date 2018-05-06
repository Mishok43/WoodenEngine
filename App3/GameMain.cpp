
#include "pch.h"

#include "GameMain.h"
#include "FrameResource.h"
#include "GameResources.h"
#include "Object.h"
#include "Camera.h"
#include "Common/DirectXHelper.h"
#include "ShaderStructures.h"

using namespace WoodenEngine;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

#define _DEBUG

FGameMain::FGameMain()
{}

FGameMain::~FGameMain()
{}


bool FGameMain::Initialize(Windows::UI::Core::CoreWindow^ outWindow)
{
	Window = outWindow;

	InitDevice();
	InitCmdQueue();
	AddObjects();

	InitDescriptorHeap();

	InitSwapChain();
	InitDepthStencilBuffer();
	InitViewport();

	BuildFrameResources();
	InitConstBuffersViews();

	BuildRootSignature();
	InitShaders();
	BuildPipelineStateObject();

	DX::ThrowIfFailed(CmdList->Close());
	ID3D12CommandList* cmdLists[] = { CmdList.Get() };
	CmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	SignalAndWaitForGPU();

	return true;
}


ID3D12Resource* WoodenEngine::FGameMain::CurrentBackBuffer() const 
{
	return SwapChainBuffers[iCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE WoodenEngine::FGameMain::CurrentBackBufferView() const
{
	auto curBackBufferCPUHandle = RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	curBackBufferCPUHandle.ptr += iCurrBackBuffer * RTVDescriptorHandleIncrementSize;
	return curBackBufferCPUHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE WoodenEngine::FGameMain::CurrentCBVGPUHandle() const 
{
	auto currentCBVDescriptorHeapGPUHandle = CBVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	currentCBVDescriptorHeapGPUHandle.ptr += 0* CBVDescriptorHandleIncrementSize;
	return currentCBVDescriptorHeapGPUHandle;
}

void FGameMain::BuildFrameResources()
{
	for (uint8 iFrame = 0; iFrame < NMR_SWAP_BUFFERS; ++iFrame)
	{
		FramesResource[iFrame] = std::make_unique<FFrameResource>(Device, Objects.size());
	}
}

void FGameMain::InitViewport()
{
	ScreenViewport.Width = Window->Bounds.Width;
	ScreenViewport.Height = Window->Bounds.Height;
	ScreenViewport.TopLeftX = 0;
	ScreenViewport.TopLeftY = 0;
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;

	ScissorRect = { 0, 0, static_cast<long>(ScreenViewport.Width), static_cast<long>(ScreenViewport.Height) };
}

void FGameMain::InitDepthStencilBuffer()
{
	D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DepthStencilFormat, Window->Bounds.Width, Window->Bounds.Height, 1, 1);
	depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	CD3DX12_CLEAR_VALUE depthOptimizedClearValue(DepthStencilFormat, 1.0f, 0);

	DX::ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&DepthStencilBuffer)));

	DX::SetName(DepthStencilBuffer.Get(), L"DepthStencilBuffer");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilViewDesc.Format = DepthStencilFormat;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &depthStencilViewDesc, DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void FGameMain::InitConstBuffersViews()
{
	auto CBVDescriptorHandle = CBVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Initialize cbv for objects 
	for (uint8 iFrame = 0; iFrame < NMR_SWAP_BUFFERS; ++iFrame)
	{
		auto* ObjectsConstBuffer = FramesResource[iFrame]->ObjectsDataBuffer->Resource();
		const auto ObjectByteSize = FramesResource[iFrame]->ObjectsDataBuffer->GetElementByteSize();
		for (uint32 iObject = 0; iObject < Objects.size(); ++iObject)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC ConstBufferViewDesc = {};
			ConstBufferViewDesc.SizeInBytes = ObjectByteSize;
			ConstBufferViewDesc.BufferLocation = ObjectsConstBuffer->GetGPUVirtualAddress() + iObject*ObjectByteSize;

			Device->CreateConstantBufferView(&ConstBufferViewDesc, CBVDescriptorHandle);

			CBVDescriptorHandle.ptr += CBVDescriptorHandleIncrementSize;
		}
	}


	// Initialize last cbv for const frame data
	for (uint8 iFrame = 0; iFrame < NMR_SWAP_BUFFERS; ++iFrame)
	{
		auto* FrameConstBuffer = FramesResource[iFrame]->FrameDataBuffer->Resource();
		const auto FrameByteSize = FramesResource[iFrame]->FrameDataBuffer->GetElementByteSize();

		D3D12_CONSTANT_BUFFER_VIEW_DESC ConstBufferViewDesc = {};
		ConstBufferViewDesc.BufferLocation = FrameConstBuffer->GetGPUVirtualAddress();
		ConstBufferViewDesc.SizeInBytes = FrameByteSize;

		Device->CreateConstantBufferView(&ConstBufferViewDesc, CBVDescriptorHandle);

		CBVDescriptorHandle.ptr += CBVDescriptorHandleIncrementSize;
	}
}

void FGameMain::InitDevice()
{

#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	DX::ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)));

	RTVDescriptorHandleIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DSVDescriptorHandleIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CBVDescriptorHandleIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DX::ThrowIfFailed(Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleData = {};
	multisampleData.Format = BufferFormat;
	multisampleData.SampleCount = 4;
	multisampleData.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	DX::ThrowIfFailed(Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		(void*)&multisampleData,
		sizeof(multisampleData)));
}

void FGameMain::AddObjects()
{
	GameResources = std::make_unique<FGameResources>(Device);
	
	// Build and load static meshes
	FMeshGenerator* MeshGenerator = new FMeshGenerator();
	const auto BoxMesh = MeshGenerator->CreateBox(1.0f, 1.0f, 1.0f);
	const auto SphereMesh = MeshGenerator->CreateSphere(1.0f, 20.0f, 20.0f);

	const std::vector<FMeshData> Meshes = {
		std::move(BoxMesh), std::move(SphereMesh)
	};

	GameResources->LoadMeshes(Meshes, CmdList);

	// Create objects
	auto* BoxObject = new WObject(BoxMesh.Name);
	BoxObject->SetConstBufferIndex(0);
	BoxObject->SetPosition(1.5f, 0.0f, 0.0f);
	BoxObject->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	Objects.push_back(BoxObject);
	
	auto* SphereObject = new WObject(SphereMesh.Name);
	SphereObject->SetConstBufferIndex(1);
	SphereObject->SetPosition(2.5f, 2.0f, 2.0f);
	SphereObject->SetScale(1.25f, 1.25f, 1.25f);
	SphereObject->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	Objects.push_back(SphereObject);

	Camera = new WCamera(Window->Bounds.Width, Window->Bounds.Height);
	Objects.push_back(Camera);
}

void FGameMain::InitDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = NMR_SWAP_BUFFERS;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.NodeMask = 0;

	DX::ThrowIfFailed(Device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&RTVDescriptorHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorHeapDesc.NodeMask = 0;
	dsvDescriptorHeapDesc.NumDescriptors = 1;

	DX::ThrowIfFailed(Device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&DSVDescriptorHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC cbvDescriptorHeapDesc = {};
	cbvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvDescriptorHeapDesc.NodeMask = 0;
	cbvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvDescriptorHeapDesc.NumDescriptors = NMR_SWAP_BUFFERS*(Objects.size()+1); // frame's cbv = cube_cbv(world matrix) + const_cbv (viewProjMatrix + Game Time)
	DX::ThrowIfFailed(Device->CreateDescriptorHeap(&cbvDescriptorHeapDesc, IID_PPV_ARGS(&CBVDescriptorHeap)));
}

void FGameMain::InitCmdQueue()
{
	auto cmdQueueDesc = D3D12_COMMAND_QUEUE_DESC{};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	DX::ThrowIfFailed(Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&CmdQueue)));
	
	DX::ThrowIfFailed(Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&CmdAllocatorDefault)));

	DX::ThrowIfFailed(Device->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, CmdAllocatorDefault.Get(), 
		nullptr, IID_PPV_ARGS(&CmdList)));
}

void WoodenEngine::FGameMain::InitSwapChain()
{
	IDXGIFactory4* dxgiFactory = nullptr;
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};


	swapChainDesc.Width = Window->Bounds.Width;
	swapChainDesc.Height = Window->Bounds.Height;
	swapChainDesc.Format = BufferFormat;
	swapChainDesc.Stereo = false;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = NMR_SWAP_BUFFERS;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	ComPtr<IDXGISwapChain1> swapChain;
	DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForCoreWindow(
		CmdQueue.Get(),
		reinterpret_cast<IUnknown*>(Window.Get()),
		&swapChainDesc,
		nullptr,
		&swapChain));

	DX::ThrowIfFailed(swapChain.As(&SwapChain));


	auto RTVCPUHandle = RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < NMR_SWAP_BUFFERS; ++i)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffers[i]));
		Device->CreateRenderTargetView(SwapChainBuffers[i].Get(), nullptr, RTVCPUHandle);
		RTVCPUHandle.ptr += RTVDescriptorHandleIncrementSize;
	}
}

void FGameMain::InitShaders()
{
	Shaders["standartVS"] = DX::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	Shaders["opaquePS"] = DX::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
}


void FGameMain::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range1;
	range1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER parameter1;
	parameter1.InitAsDescriptorTable(1, &range1, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_DESCRIPTOR_RANGE range2;
	range2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	
	CD3DX12_ROOT_PARAMETER parameter2;
	parameter2.InitAsDescriptorTable(1, &range2, D3D12_SHADER_VISIBILITY_VERTEX);

	auto parameters = { parameter1, parameter2 };
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, parameters.begin(), 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
	ComPtr<ID3DBlob> rootSignatureBlobError = nullptr;
	DX::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &rootSignatureBlobError));
	if (rootSignatureBlobError != nullptr)
	{}
	
	DX::ThrowIfFailed(Device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void FGameMain::BuildPipelineStateObject()
{
	const D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	InputLayout.pInputElementDescs = inputElements;
	InputLayout.NumElements = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	psoDesc.InputLayout = InputLayout;
	
	psoDesc.pRootSignature = RootSignature.Get();
	psoDesc.VS = { Shaders["standartVS"]->GetBufferPointer(), Shaders["standartVS"]->GetBufferSize() };
	psoDesc.PS = { Shaders["opaquePS"]->GetBufferPointer(), Shaders["opaquePS"]->GetBufferSize() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = BufferFormat;
	psoDesc.DSVFormat = DepthStencilFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	DX::ThrowIfFailed(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO)));
}

void WoodenEngine::FGameMain::Update(float dtime)
{
	for (auto Object : Objects)
	{
		if (Object->IsUpdating())
		{
			Object->Update(dtime);
		}
	}

	// Shift frame to the next
	iCurrFrameResource = (iCurrFrameResource + 1) % NMR_SWAP_BUFFERS;
	CurrFrameResource = FramesResource[iCurrFrameResource].get();

	// Wait until this frame will be rendered with old const buffer
	WaitForGPU(CurrFrameResource->Fence);

	UpdateObjectsConstBuffer();
	UpdateFrameConstBuffer();
}


void WoodenEngine::FGameMain::UpdateObjectsConstBuffer()
{
	auto ObjectsBuffer = CurrFrameResource->ObjectsDataBuffer.get();
	for (auto Object : Objects)
	{
		if (Object->IsRenderable() && Object->GetNumDirtyConstBuffers() > 0)
		{
			const auto WorldMatrix = Object->GetWorldMatrix();

			SObjectData ObjectShaderData;
			XMStoreFloat4x4(&ObjectShaderData.WorldMatrix, XMMatrixTranspose(WorldMatrix));

			const auto Color = Object->GetColor();
			XMStoreFloat4(&ObjectShaderData.Color, XMLoadFloat4(&Color));

			ObjectsBuffer->CopyData(Object->GetConstBufferIndex(), ObjectShaderData);

			Object->SetNumDirtyConstBuffers(Object->GetNumDirtyConstBuffers() - 1);
		}
	}
}

void WoodenEngine::FGameMain::UpdateFrameConstBuffer()
{
	SFrameData ConstFrameData = {};

	const auto& ViewMatrix = Camera->GetViewMatrix();

	XMStoreFloat4x4(&ConstFrameData.ViewMatrix, XMMatrixTranspose(ViewMatrix));

	auto ProjMatrix = XMMatrixPerspectiveFovLH(XM_PI / 4.0f, Window->Bounds.Width / Window->Bounds.Height, 1.0, 1000.0f);
	XMStoreFloat4x4(&ConstFrameData.ProjMatrix, XMMatrixTranspose(ProjMatrix));

	auto ViewProj = XMMatrixMultiply(ViewMatrix, ProjMatrix);
	XMStoreFloat4x4(&ConstFrameData.ViewProjMatrix, XMMatrixTranspose(ViewProj));

	CurrFrameResource->FrameDataBuffer->CopyData(0, ConstFrameData);
}

void WoodenEngine::FGameMain::MouseMoved(const float dx, const float dy) noexcept
{
	for (auto Object : Objects)
	{
		if (Object->IsEnabledInputEvents())
		{
			Object->InputMouseMoved(dx, dy);
		}
	}
}

void WoodenEngine::FGameMain::Render()
{
	auto CmdListAllocator = CurrFrameResource->CmdListAllocator;

	DX::ThrowIfFailed(CmdListAllocator->Reset());

	DX::ThrowIfFailed(CmdList->Reset(CmdListAllocator.Get(), PSO.Get()));
	
	CmdList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
	);

	CmdList->SetGraphicsRootSignature(RootSignature.Get());

	ID3D12DescriptorHeap* cbvDescriptorHeaps[] = { CBVDescriptorHeap.Get() };
	CmdList->SetDescriptorHeaps(_countof(cbvDescriptorHeaps), cbvDescriptorHeaps);

	// Set frame const buffer as argumet to shader
	auto CBVGPUHandle = CurrentCBVGPUHandle();
	CBVGPUHandle.ptr += (Objects.size()*NMR_SWAP_BUFFERS + iCurrFrameResource)*CBVDescriptorHandleIncrementSize;
	CmdList->SetGraphicsRootDescriptorTable(1, CBVGPUHandle);

	CmdList->RSSetViewports(1, &ScreenViewport);
	CmdList->RSSetScissorRects(1, &ScissorRect);

	CmdList->ClearRenderTargetView(CurrentBackBufferView(), Colors::White, 0, nullptr);
	CmdList->ClearDepthStencilView(DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	CmdList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	CmdList->IASetVertexBuffers(0, 1, &GameResources->GetVertexBufferView());
	CmdList->IASetIndexBuffer(&GameResources->GetIndexBufferView());
	CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto Object : Objects)
	{
		if (!Object->IsRenderable())
		{
			continue;
		}

		const auto& MeshName = Object->GetMeshName();
		const auto MeshData = GameResources->GetMeshData(MeshName);
		
		auto CBVGPUHandle = CurrentCBVGPUHandle();
		CBVGPUHandle.ptr += (Objects.size()*iCurrFrameResource + Object->GetConstBufferIndex())*CBVDescriptorHandleIncrementSize;
		CmdList->SetGraphicsRootDescriptorTable(0, CBVGPUHandle);

		CmdList->DrawIndexedInstanced(
			MeshData.Indices.size(), 1, MeshData
			.IndexBegin, MeshData.VertexBegin, 0);
	}

	CmdList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)
	);

	DX::ThrowIfFailed(CmdList->Close());
	ID3D12CommandList* cmdLists[] = { CmdList.Get() };
	CmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	DX::ThrowIfFailed(SwapChain->Present(1, 0));

	iCurrBackBuffer = (iCurrBackBuffer + 1) % NMR_SWAP_BUFFERS;
	
	++FenceValue;
	CurrFrameResource->Fence = FenceValue;
	CmdQueue->Signal(Fence.Get(), FenceValue);
}


void FGameMain::SignalAndWaitForGPU()
{
	++FenceValue;

	DX::ThrowIfFailed(CmdQueue->Signal(Fence.Get(), FenceValue));
	DX::ThrowIfFailed(Fence->SetEventOnCompletion(FenceValue, fenceEvent));

	WaitForSingleObjectEx(fenceEvent, INFINITE, false);
}

void FGameMain::WaitForGPU(const uint64 NewFence)
{
	if (NewFence != 0 && Fence->GetCompletedValue() < NewFence)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		DX::ThrowIfFailed(Fence->SetEventOnCompletion(NewFence, EventHandle));
		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}