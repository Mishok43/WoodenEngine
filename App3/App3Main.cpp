
#include "pch.h"

#include "App3Main.h"

#include "Common\DirectXHelper.h"
#include "ShaderStructures.h"

#define _DEBUG

using namespace DirectXEngine;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;


App3Main::App3Main()
{
}
App3Main::~App3Main()
{
}


void App3Main::OnWindowSizeChanged()
{}

bool App3Main::Initialize(Windows::UI::Core::CoreWindow^ outWindow)
{
	window = outWindow;

	InitializeDevice();
	InitializeCmdQueue();
	InitializeDescriptorHeaps();
	InitializeSwapChain();

	CreateFrameResources();
	BuildRootSignature();
	CompileShaders();
	BuildPipelineStateObject();
	WaitForGPU();

	DX::ThrowIfFailed(cmdList->Close());
	ID3D12CommandList* cmdLists[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	WaitForGPU();


	return true;
}


ID3D12Resource* DirectXEngine::App3Main::CurrentBackBuffer() const 
{

	return swapChainBuffers[currentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXEngine::App3Main::CurrentBackBufferView() const
{
	auto curBackBufferCPUHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	curBackBufferCPUHandle.ptr += currentBackBuffer * rtvDescriptorHandleIncrementSize;
	return curBackBufferCPUHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXEngine::App3Main::CurrentCBVGPUHandle() const 
{
	auto currentCBVDescriptorHeapGPUHandle = cbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	currentCBVDescriptorHeapGPUHandle.ptr += 0* cbvDescriptorHandleIncrementSize;
	return currentCBVDescriptorHeapGPUHandle;
}

void App3Main::CreateFrameResources()
{

	currentBackBuffer = deviceSwapChain->GetCurrentBackBufferIndex();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < NMR_SWAP_BUFFERS; ++i)
	{
		deviceSwapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffers[i]));
		device->CreateRenderTargetView(swapChainBuffers[i].Get(), nullptr, rtvDescriptor);
		rtvDescriptor.Offset(rtvDescriptorHandleIncrementSize);
	}

	D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(depthStencilFormat, window->Bounds.Width, window->Bounds.Height, 1, 1);
	depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	CD3DX12_CLEAR_VALUE depthOptimizedClearValue(depthStencilFormat, 1.0f, 0);

	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, 
		&depthResourceDesc, 
		D3D12_RESOURCE_STATE_DEPTH_WRITE, 
		&depthOptimizedClearValue, 
		IID_PPV_ARGS(&depthStencilBuffer)));

	DX::SetName(depthStencilBuffer.Get(), L"DepthStencilBuffer");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilViewDesc.Format = depthStencilFormat;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	screenViewport.Width = window->Bounds.Width;
	screenViewport.Height = window->Bounds.Height;
	screenViewport.TopLeftX = 0;
	screenViewport.TopLeftY = 0;
	screenViewport.MinDepth = 0.0f;
	screenViewport.MaxDepth = 1.0f;

	scissorRect = { 0, 0, static_cast<long>(screenViewport.Width), static_cast<long>(screenViewport.Height) };
	
	VertexData cubeVertices[] = {
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Red) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Green) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Blue) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Black) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Yellow) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Purple) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) }
	};

	const auto vertexBufferSize = sizeof(cubeVertices);

	
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	));

	DX::SetName(vertexBuffer.Get(), L"VertexBuffer");

	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUpload)
	));

	DX::SetName(vertexBufferUpload.Get(), L"VertexBufferUpload");


	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	DX::ThrowIfFailed(UpdateSubresources(cmdList.Get(), vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	
	vertexBufferView.BufferLocation = vertexBufferUpload->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vertexBufferSize;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	
	uint16_t cubeIndices[] =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	boxIndexCount = _countof(cubeIndices);

	const auto indexBufferSize = sizeof(cubeIndices);

	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);


	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)
	));
	

	DX::SetName(indexBuffer.Get(), L"IndexBuffer");


	
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)
	));
	
	DX::SetName(indexBufferUpload.Get(), L"IndexBufferUpload");


	D3D12_SUBRESOURCE_DATA indexBufferSubresourceData;
	indexBufferSubresourceData.pData = reinterpret_cast<BYTE*>(&cubeIndices);
	indexBufferSubresourceData.RowPitch = indexBufferSize;
	indexBufferSubresourceData.SlicePitch = indexBufferSize;

	DX::ThrowIfFailed(UpdateSubresources(
		cmdList.Get(), 
		indexBuffer.Get(), 
		indexBufferUpload.Get(), 
		0, 0, 1, &indexBufferSubresourceData));


	CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	cmdList->ResourceBarrier(1, &indexBufferResourceBarrier);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = indexBufferSize;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	const auto constBufferFrameSize = 256;

	auto constBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constBufferFrameSize*NMR_SWAP_BUFFERS);
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&constBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffer)
	));

	DX::SetName(constBuffer.Get(), L"ConstBuffer");


	auto cbvDescriptorHeapCPUHandle = cbvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto constBufferGPUAddress = constBuffer->GetGPUVirtualAddress();
	for (int i = 0; i < NMR_SWAP_BUFFERS; ++i)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvViewDesc;
		cbvViewDesc.SizeInBytes = constBufferFrameSize;
		cbvViewDesc.BufferLocation = constBufferGPUAddress;

		device->CreateConstantBufferView(&cbvViewDesc, cbvDescriptorHeapCPUHandle);
		constBufferGPUAddress += constBufferFrameSize;
		cbvDescriptorHeapCPUHandle.ptr += cbvDescriptorHandleIncrementSize;
	}

	constBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&constBufferMappedData));

	ZeroMemory(constBufferMappedData, NMR_SWAP_BUFFERS*constBufferFrameSize);
}

void App3Main::InitializeDevice()
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

	DX::ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	rtvDescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbvDescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DX::ThrowIfFailed(device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleData = {};
	multisampleData.Format = bufferFormat;
	multisampleData.SampleCount = 4;
	multisampleData.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	DX::ThrowIfFailed(device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		(void*)&multisampleData,
		sizeof(multisampleData)));

}

void App3Main::InitializeDescriptorHeaps()
{

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = NMR_SWAP_BUFFERS;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.NodeMask = 0;

	DX::ThrowIfFailed(device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorHeapDesc.NodeMask = 0;
	dsvDescriptorHeapDesc.NumDescriptors = 1;

	DX::ThrowIfFailed(device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC cbvDescriptorHeapDesc = {};
	cbvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvDescriptorHeapDesc.NodeMask = 0;
	cbvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvDescriptorHeapDesc.NumDescriptors = NMR_SWAP_BUFFERS; // frame's cbv = cube_cbv(world matrix) + const_cbv (viewProjMatrix + Game Time)
	DX::ThrowIfFailed(device->CreateDescriptorHeap(&cbvDescriptorHeapDesc, IID_PPV_ARGS(&cbvDescriptorHeap)));
	
}

void App3Main::InitializeCmdQueue()
{
	auto cmdQueueDesc = D3D12_COMMAND_QUEUE_DESC{};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	auto createCmdQueue = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(cmdQueue.GetAddressOf()));
	DX::ThrowIfFailed(createCmdQueue);

	auto createCmdAllocatorRes = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
	DX::ThrowIfFailed(createCmdAllocatorRes);

	auto createCmdListRes = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));
	DX::ThrowIfFailed(createCmdListRes);

}

void DirectXEngine::App3Main::InitializeSwapChain()
{
	IDXGIFactory4* dxgiFactory = nullptr;
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	deviceSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = window->Bounds.Width;
	swapChainDesc.Height = window->Bounds.Height;
	swapChainDesc.Format = bufferFormat;
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
		cmdQueue.Get(),
		reinterpret_cast<IUnknown*>(window.Get()),
		&swapChainDesc,
		nullptr,
		&swapChain));

	DX::ThrowIfFailed(swapChain.As(&deviceSwapChain));

}

void App3Main::CompileShaders()
{
	shaders["standartVS"] = DX::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	shaders["opaquePS"] = DX::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
}


void App3Main::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range1;
	range1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER parameter1;
	parameter1.InitAsDescriptorTable(1, &range1, D3D12_SHADER_VISIBILITY_VERTEX);


	auto parameters = { parameter1 };
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, parameters.begin(), 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
	ComPtr<ID3DBlob> rootSignatureBlobError = nullptr;
	DX::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &rootSignatureBlobError));
	if (rootSignatureBlobError != nullptr)
	{}
	
	DX::ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void DirectXEngine::App3Main::Rotate()
{
	cameraPhi += DirectX::XM_PI / 10000.0;
}

void App3Main::BuildPipelineStateObject()
{
	const D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	inputLayout.pInputElementDescs = inputElements;
	inputLayout.NumElements = 2;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	psoDesc.InputLayout = inputLayout;
	
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { shaders["standartVS"]->GetBufferPointer(), shaders["standartVS"]->GetBufferSize() };
	psoDesc.PS = { shaders["opaquePS"]->GetBufferPointer(), shaders["opaquePS"]->GetBufferSize() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = bufferFormat;
	psoDesc.DSVFormat = depthStencilFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void DirectXEngine::App3Main::Update()
{

	UpdateConstBuffers();
	WaitForGPU();
}

void DirectXEngine::App3Main::MouseMoved(const float dx, const float dy) noexcept
{
	const auto dPhi = dx / window->Bounds.Width*DirectX::XM_PI;
	const auto dTheta = dy / window->Bounds.Height*DirectX::XM_PIDIV2;

	cameraPhi += dPhi;

	cameraTheta += dTheta;
	
}


void DirectXEngine::App3Main::UpdateConstBuffers()
{
	ConstData constData;

	// Update the view matrix
	XMFLOAT4 cameraPosition;
	cameraPosition.x = XMScalarCos(cameraPhi)*XMScalarSin(cameraTheta)*cameraRadius;
	cameraPosition.y = XMScalarSin(cameraPhi)*XMScalarSin(cameraTheta)*cameraRadius;
	cameraPosition.z = XMScalarCos(cameraTheta)*cameraRadius;
	cameraPosition.w = 1.0f;

	XMFLOAT4 cameraFocus = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 upDirecation = { 0.0f, 1.0f, 0.0f, 0.0f };

	const auto viewMatrix = XMMatrixLookAtLH(XMLoadFloat4(&cameraPosition), XMLoadFloat4(&cameraFocus), XMLoadFloat4(&upDirecation));

	XMStoreFloat4x4(&constData.view_matrix, viewMatrix);
	
	// Update the perspective matrix
	const auto width = window->Bounds.Width;
	const auto height = window->Bounds.Height;

	const auto projMatrix = XMMatrixPerspectiveFovLH(XM_PI / 4.0f, window->Bounds.Width / window->Bounds.Height, 1.0, 1000.0f);
	XMStoreFloat4x4(&constData.proj_matrix, projMatrix);

	const auto viewProjMatrix = viewMatrix * projMatrix;
	XMStoreFloat4x4(&constData.view_proj_matrix, viewProjMatrix);

	const auto worldMatrix = XMMatrixScaling(1, 1, 1);
	const auto worldViewProj = worldMatrix * viewMatrix*projMatrix;
	XMStoreFloat4x4(&constData.world_view_proj_matrix, XMMatrixTranspose(worldViewProj));

	auto* constBufferDestination = constBufferMappedData + currentBackBuffer*256;
	memcpy(constBufferDestination, &constData, sizeof(constData));
}

bool DirectXEngine::App3Main::Render()
{
	DX::ThrowIfFailed(cmdAllocator->Reset());

	DX::ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), pso.Get()));
	
	cmdList->SetGraphicsRootSignature(rootSignature.Get());


	ID3D12DescriptorHeap* cbvDescriptorHeaps[] = { cbvDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(cbvDescriptorHeaps), cbvDescriptorHeaps);


	cmdList->SetGraphicsRootDescriptorTable(0, CurrentCBVGPUHandle());


	cmdList->RSSetViewports(1, &screenViewport);
	cmdList->RSSetScissorRects(1, &scissorRect);



	cmdList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
	);



	cmdList->ClearRenderTargetView(CurrentBackBufferView(), Colors::White, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmdList->OMSetRenderTargets(1, &CurrentBackBufferView(), false, &dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());






	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
	cmdList->IASetIndexBuffer(&indexBufferView);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	cmdList->DrawIndexedInstanced(boxIndexCount, 1, 0, 0, 0);

	cmdList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)
	);

	DX::ThrowIfFailed(cmdList->Close());
	ID3D12CommandList* cmdLists[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);


	DX::ThrowIfFailed(deviceSwapChain->Present(1, 0));

	currentBackBuffer = (currentBackBuffer + 1) % NMR_SWAP_BUFFERS;
	
	WaitForGPU();


	return true;
}


void App3Main::WaitForGPU()
{
	DX::ThrowIfFailed(cmdQueue->Signal(fence.Get(), fenceValue));
	DX::ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));

	WaitForSingleObjectEx(fenceEvent, INFINITE, false);

	++fenceValue;

	/*
	// Advance the fence value to mark commands up to this fence point.
	fenceValue++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	DX::ThrowIfFailed(cmdQueue->Signal(fence.Get(), fenceValue));

	// Wait until the GPU has completed commands up to this fence point.
	if (fence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		DX::ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}*/
}


// Notifies the app that it is being suspended.
void App3Main::OnSuspending()
{
	// TODO: Replace this with your app's suspending logic.

	// Process lifetime management may terminate suspended apps at any time, so it is
	// good practice to save any state that will allow the app to restart where it left off.

	// If your application uses video memory allocations that are easy to re-create,
	// consider releasing that memory to make it available to other applications.
}

// Notifes the app that it is no longer suspended.
void App3Main::OnResuming()
{
	// TODO: Replace this with your app's resuming logic.
}

// Notifies renderers that device resources need to be released.
void App3Main::OnDeviceRemoved()
{
	// TODO: Save any necessary application or renderer state and release the renderer
	// and its resources which are no longer valid.
}
