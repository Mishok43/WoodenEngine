
#include "pch.h"
#include <array>

#include "MeshData.h"
#include "GameMain.h"
#include "Object.h"
#include "Camera.h"
#include "Common/DirectXHelper.h"
#include "ShaderStructures.h"

namespace WoodenEngine
{
	using namespace Windows::Foundation;
	using namespace Windows::System::Threading;
	using namespace Concurrency;

#define _DEBUG

	FGameMain::FGameMain()
	{
	}

	FGameMain::~FGameMain()
	{
	}


	bool FGameMain::Initialize(Windows::UI::Core::CoreWindow^ outWindow)
	{
		Window = outWindow;

		InitDevice();
		InitCmdQueue();
		
		InitGameResources();

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
		auto CPUDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE{
				RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				iCurrBackBuffer, RTVDescriptorHandleIncrementSize };

		return CPUDescriptorHandle;
	}

	void FGameMain::BuildFrameResources()
	{
		for (uint8 iFrame = 0; iFrame < NMR_SWAP_BUFFERS; ++iFrame)
		{
			FramesResource[iFrame] = std::make_unique<FFrameResource>(Device, Objects.size(), GameResources->GetNumMaterials());
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
		auto CBVDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE{
			CBVDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

		// Initialize cbv for objects 
		for (uint8 iFrame = 0; iFrame < NMR_SWAP_BUFFERS; ++iFrame)
		{
			auto* ObjectsConstBuffer = FramesResource[iFrame]->ObjectsDataBuffer->Resource();
			const auto ObjectByteSize = FramesResource[iFrame]->ObjectsDataBuffer->GetElementByteSize();
			for (uint32 iObject = 0; iObject < Objects.size(); ++iObject)
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC ConstBufferViewDesc = {};
				ConstBufferViewDesc.SizeInBytes = ObjectByteSize;
				ConstBufferViewDesc.BufferLocation = ObjectsConstBuffer->GetGPUVirtualAddress() + iObject * ObjectByteSize;

				Device->CreateConstantBufferView(&ConstBufferViewDesc, CBVDescriptorHandle);

				CBVDescriptorHandle.Offset(1, CBVDescriptorHandleIncrementSize);
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

			CBVDescriptorHandle.Offset(1, CBVDescriptorHandleIncrementSize);
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

	void FGameMain::InitGameResources()
	{
		GameResources = std::make_unique<FGameResources>(Device);

		AddMaterials();
		AddObjects();
	}

	void FGameMain::AddMaterials()
	{
		auto GrassMaterial = std::make_unique<FMaterialData>("grass");
		GrassMaterial->iConstBuffer = 0;
		GrassMaterial->DiffuseAlbedo = { 0.2f, 0.6f, 0.6f, 1.0f };
		GrassMaterial->FresnelR0 = { 0.01f, 0.01f, 0.01f };
		GrassMaterial->Roughness = 0.125f;

		auto WaterMaterial = std::make_unique<FMaterialData>("water");
		WaterMaterial->iConstBuffer = 1;
		WaterMaterial->DiffuseAlbedo = { 0.0f, 0.2f, 0.6f, 1.0f };
		WaterMaterial->FresnelR0 = { 0.1f, 0.1f, 0.1f };
		WaterMaterial->Roughness = 0.0f;

		GameResources->AddMaterial(std::move(GrassMaterial));
		GameResources->AddMaterial(std::move(WaterMaterial));
	}

	void FGameMain::AddObjects()
	{
		// Build and load static meshes
		auto MeshGenerator = std::make_unique<FMeshGenerator>();
		auto BoxMesh = MeshGenerator->CreateBox(1.0f, 1.0f, 1.0f);
		auto SphereMesh = MeshGenerator->CreateSphere(1.0f, 20.0f, 20.0f);

		auto MeshParser = std::make_unique<FMeshParser>();
		auto SkullMesh = MeshParser->ParseMeshData("Assets\\Models\\skull.txt");
		SkullMesh->Name = "Skull";

		const auto BoxMeshName = BoxMesh->Name;
		const auto SphereMeshName = SphereMesh->Name;
		const auto SkullMeshName = SkullMesh->Name;

		std::vector<std::unique_ptr<FMeshData>> Meshes;
		Meshes.resize(3);
		Meshes[0] = std::move(BoxMesh);
		Meshes[1] = std::move(SphereMesh);
		Meshes[2] = std::move(SkullMesh);

		GameResources->LoadMeshes(std::move(Meshes), CmdList);

		// Create objects
		auto* BoxObject = new WObject(BoxMeshName);
		BoxObject->SetConstBufferIndex(0);
		BoxObject->SetPosition(1.5f, 0.0f, 0.0f);
		BoxObject->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		BoxObject->SetMaterial(GameResources->GetMaterialData("grass"));
		Objects.push_back(BoxObject);

		auto* SphereObject = new WObject(SphereMeshName);
		SphereObject->SetConstBufferIndex(1);
		SphereObject->SetPosition(2.5f, 2.0f, 2.0f);
		SphereObject->SetScale(1.25f, 1.25f, 1.25f);
		SphereObject->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
		SphereObject->SetMaterial(GameResources->GetMaterialData("water"));
		Objects.push_back(SphereObject);

		/*
		auto* SkullObject = new WObject(SkullMeshName);
		SkullObject->SetConstBufferIndex(2);
		SkullObject->SetPosition({ -1.5f, -1.5f, 0.0f });
		SkullObject->SetScale(0.5f, 0.5f, 0.5f);
		SkullObject->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f });
		SkullObject->SetMaterial(GameResources->GetMaterialData("grass"));
		Objects.push_back(SkullObject);
		*/

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
		cbvDescriptorHeapDesc.NumDescriptors = NMR_SWAP_BUFFERS * (Objects.size() + 1); // frame's cbv = cube_cbv(world matrix) + const_cbv (viewProjMatrix + Game Time)
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


		auto RTVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE{
			RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };
		for (int i = 0; i < NMR_SWAP_BUFFERS; ++i)
		{
			SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffers[i]));
			Device->CreateRenderTargetView(SwapChainBuffers[i].Get(), nullptr, RTVCPUHandle);
			RTVCPUHandle.Offset(1, RTVDescriptorHandleIncrementSize);
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
		parameter1.InitAsDescriptorTable(1, &range1);

		CD3DX12_ROOT_PARAMETER parameter2;
		parameter2.InitAsConstantBufferView(1);

		CD3DX12_DESCRIPTOR_RANGE range3;
		range3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

		CD3DX12_ROOT_PARAMETER parameter3;
		parameter3.InitAsDescriptorTable(1, &range3);

		auto parameters = { parameter1, parameter2, parameter3};
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(3, parameters.begin(), 0, nullptr, rootSignatureFlags);

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
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		InputLayout.pInputElementDescs = inputElements;
		InputLayout.NumElements = 2;

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
		UpdateMaterialsConstBuffer();
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

				ObjectShaderData.Color = Object->GetColor();

				ObjectsBuffer->CopyData(Object->GetConstBufferIndex(), ObjectShaderData);

				Object->SetNumDirtyConstBuffers(Object->GetNumDirtyConstBuffers() - 1);
			}
		}
	}

	void WoodenEngine::FGameMain::UpdateMaterialsConstBuffer()
	{
		auto MaterialsBuffer = CurrFrameResource->MaterialsDataBuffer.get();

		const auto& MaterialsData = GameResources->GetMaterialsData();
		for (auto MaterialsDataIter = MaterialsData.cbegin(); MaterialsDataIter != MaterialsData.cend(); MaterialsDataIter++)
		{
			auto MaterialData = MaterialsDataIter->second.get();
			if (MaterialData->NumDirtyConstBuffers > 0)
			{
				SMaterialData MaterialShaderData;
				MaterialShaderData.DiffuzeAlbedo = MaterialData->DiffuseAlbedo;
				MaterialShaderData.FresnelR0 = MaterialData->FresnelR0;
				MaterialShaderData.Roughness = MaterialData->Roughness;

				MaterialsBuffer->CopyData(MaterialData->iConstBuffer, MaterialShaderData);

				--MaterialData->NumDirtyConstBuffers;
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

		ConstFrameData.EyePosition = Camera->GetWorldPosition();
		ConstFrameData.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f};

		ConstFrameData.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		ConstFrameData.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };


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
		auto CBVGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE{
			CBVDescriptorHeap->GetGPUDescriptorHandleForHeapStart() };
		CBVGPUHandle.Offset(Objects.size()*NMR_SWAP_BUFFERS + iCurrFrameResource,
			CBVDescriptorHandleIncrementSize);
		CmdList->SetGraphicsRootDescriptorTable(2, CBVGPUHandle);

		CmdList->RSSetViewports(1, &ScreenViewport);
		CmdList->RSSetScissorRects(1, &ScissorRect);

		CmdList->ClearRenderTargetView(CurrentBackBufferView(), Colors::White, 0, nullptr);
		CmdList->ClearDepthStencilView(DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		CmdList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		CmdList->IASetVertexBuffers(0, 1, &GameResources->GetVertexBufferView());
		CmdList->IASetIndexBuffer(&GameResources->GetIndexBufferView());
		CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto* CurMaterialsResource = CurrFrameResource->MaterialsDataBuffer.get();
		for (auto Object : Objects)
		{
			if (!Object->IsRenderable())
			{
				continue;
			}

			const auto& MeshName = Object->GetMeshName();
			const auto MeshData = GameResources->GetMeshData(MeshName);

			auto ObjectCBVHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE{
				CBVDescriptorHeap->GetGPUDescriptorHandleForHeapStart() };
			ObjectCBVHandle.Offset(Objects.size()*iCurrFrameResource + Object->GetConstBufferIndex(),
				CBVDescriptorHandleIncrementSize);

			CmdList->SetGraphicsRootDescriptorTable(0, ObjectCBVHandle);

			auto MaterialsResAddress =
				CurMaterialsResource->Resource()->GetGPUVirtualAddress() +
				Object->GetMaterial()->iConstBuffer*CurMaterialsResource->GetElementByteSize();

			CmdList->SetGraphicsRootConstantBufferView(1, MaterialsResAddress);

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
}