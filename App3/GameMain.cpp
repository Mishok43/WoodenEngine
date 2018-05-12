#include "pch.h"
#include <array>

#include "MeshData.h"
#include "GameMain.h"
#include "Object.h"
#include "Camera.h"
#include "Common/DirectXHelper.h"
#include "ShaderStructures.h"

#define _DEBUG

namespace WoodenEngine
{
	using namespace Windows::Foundation;
	using namespace Windows::System::Threading;
	using namespace Concurrency;

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
		InitTexturesViews();

		BuildRootSignature();
		InitShaders();
		BuildPipelineStateObject();

		DX::ThrowIfFailed(CMDList->Close());
		ID3D12CommandList* cmdLists[] = { CMDList.Get() };
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

				CBVDescriptorHandle.Offset(1, CBVSRVDescriptorHandleIncrementSize);
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

			CBVDescriptorHandle.Offset(1, CBVSRVDescriptorHandleIncrementSize);
		}
	}

	void FGameMain::InitTexturesViews()
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVDescriptorHandle(
			SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		const auto& TexturesData = GameResources->GetTexturesData();
		uint32 iTexture = 0;
		for (auto TexturesDataIter = TexturesData.cbegin(); TexturesDataIter != TexturesData.cend(); ++TexturesDataIter, ++iTexture)
		{
			auto TextureData = TexturesDataIter->second.get();
		
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SRVDesc.Format = TextureData->Resource->GetDesc().Format;
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MostDetailedMip = 0;
			SRVDesc.Texture2D.MipLevels = TextureData->Resource->GetDesc().MipLevels;
			SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			Device->CreateShaderResourceView(TextureData->Resource.Get(), &SRVDesc, SRVDescriptorHandle);

			TextureData->iSRVHeap = iTexture;

			SRVDescriptorHandle.Offset(CBVSRVDescriptorHandleIncrementSize);
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
		CBVSRVDescriptorHandleIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
		GameResources = std::make_unique<FGameResource>(Device);

		AddTextures();
		AddMaterials();
		AddObjects();
	}

	void FGameMain::AddTextures()
	{
		auto BasePath = static_cast<std::wstring>(L"Assets\\Textures\\");

		GameResources->LoadTexture(BasePath + L"WireFence.dds", "wirefence", CMDList);
		GameResources->LoadTexture(BasePath + L"WoodCrate01.dds", "crate", CMDList);
		GameResources->LoadTexture(BasePath + L"water1.dds", "water", CMDList);
		GameResources->LoadTexture(BasePath + L"grass.dds", "grass", CMDList);
	}


	void FGameMain::AddMaterials()
	{
		uint64 iConstBuffer = 0;

		auto GrassMaterial = std::make_unique<FMaterialData>("grass");
		GrassMaterial->iConstBuffer = iConstBuffer;
		GrassMaterial->FresnelR0 = { 0.01f, 0.01f, 0.01f };
		GrassMaterial->Roughness = 0.125f;
		GrassMaterial->DiffuseTexture = GameResources->GetTextureData("grass");
		GameResources->AddMaterial(std::move(GrassMaterial));
		++iConstBuffer;

		auto WaterMaterial = std::make_unique<FMaterialData>("water");
		WaterMaterial->iConstBuffer = iConstBuffer;
		WaterMaterial->FresnelR0 = { 0.1f, 0.1f, 0.1f };
		WaterMaterial->Roughness = 0.0f;
		WaterMaterial->DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 0.4f };
		WaterMaterial->DiffuseTexture = GameResources->GetTextureData("water");
		GameResources->AddMaterial(std::move(WaterMaterial));
		++iConstBuffer;

		auto SkullMaterial = std::make_unique<FMaterialData>("skull");
		SkullMaterial->iConstBuffer = iConstBuffer;
		SkullMaterial->FresnelR0 = { 0.05f, 0.05f, 0.05f };
		SkullMaterial->Roughness = 0.4f;
		SkullMaterial->DiffuseTexture = GameResources->GetTextureData("grass");
		GameResources->AddMaterial(std::move(SkullMaterial));
		++iConstBuffer;

		auto CrateMaterial = std::make_unique<FMaterialData>("crate");
		CrateMaterial->iConstBuffer = iConstBuffer;
		CrateMaterial->FresnelR0 = { 0.05f, 0.05f, 0.05f };
		CrateMaterial->Roughness = 0.85f;
		CrateMaterial->DiffuseTexture = GameResources->GetTextureData("crate");
		GameResources->AddMaterial(std::move(CrateMaterial));
		++iConstBuffer;

		auto WireFenceMaterial = std::make_unique<FMaterialData>("wirefence");
		WireFenceMaterial->iConstBuffer = iConstBuffer;
		WireFenceMaterial->FresnelR0 = { 0.05f, 0.05f, 0.05f };
		WireFenceMaterial->Roughness = 0.85f;
		WireFenceMaterial->DiffuseTexture = GameResources->GetTextureData("wirefence");
		GameResources->AddMaterial(std::move(WireFenceMaterial));
		++iConstBuffer;

	}

	void FGameMain::AddObjects()
	{
		// Build and load static meshes
		auto MeshGenerator = std::make_unique<FMeshGenerator>();
		
		auto BoxMesh = MeshGenerator->CreateBox(1.0f, 1.0f, 1.0f);
		
		auto LandscapeMesh = MeshGenerator->CreateLandscapeGrid(40.0f, 40.0f, 80, 80);
		LandscapeMesh->Name = "Landscape";

		auto PlaneMesh = MeshGenerator->CreateGrid(23.0f, 23.0f, 30, 30);
		auto SphereMesh = MeshGenerator->CreateSphere(1.0f, 15.0f, 15.0f);

		auto MeshParser = std::make_unique<FMeshParser>();
		auto SkullMesh = MeshParser->ParseTxtData("Assets\\Models\\skull.txt");
		SkullMesh->Name = "Skull";

		const auto BoxMeshName = BoxMesh->Name;
		const auto SphereMeshName = SphereMesh->Name;
		const auto SkullMeshName = SkullMesh->Name;
		const auto PlaneMeshName = PlaneMesh->Name;
		const auto LandscapeMeshName = LandscapeMesh->Name;

		std::vector<std::unique_ptr<FMeshData>> Meshes;
		Meshes.resize(5);
		Meshes[0] = std::move(BoxMesh);
		Meshes[1] = std::move(SphereMesh);
		Meshes[2] = std::move(SkullMesh);
		Meshes[3] = std::move(PlaneMesh);
		Meshes[4] = std::move(LandscapeMesh);

		GameResources->LoadMeshes(std::move(Meshes), CMDList);

		uint8 iConstBuffer = 0;

		auto LandscapeObject = std::make_unique<WObject>(LandscapeMeshName);
		XMFLOAT4X4 LandscapeTextureTransform;
		XMStoreFloat4x4(&LandscapeTextureTransform, XMMatrixScaling(6.0f, 6.0f, 1.0f));
		LandscapeObject->SetTextureTransform(std::move(LandscapeTextureTransform));
		LandscapeObject->SetPosition(0, -2, 0);
		LandscapeObject->SetConstBufferIndex(iConstBuffer);
		LandscapeObject->SetWaterFactor(0);
		LandscapeObject->SetMaterial(GameResources->GetMaterialData("grass"));
		++iConstBuffer;

		RenderableObjects[(uint8)ERenderLayer::Opaque].push_back(LandscapeObject.get());
		Objects.push_back(std::move(LandscapeObject));

		auto WaterObject = std::make_unique<WObject>(PlaneMeshName);
		XMFLOAT4X4 TextureTransform;
		XMStoreFloat4x4(&TextureTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
		WaterObject->SetTextureTransform(std::move(TextureTransform));
		WaterObject->SetPosition(0, 0, 0);
		WaterObject->SetConstBufferIndex(iConstBuffer);
		WaterObject->SetMaterial(GameResources->GetMaterialData("water"));
		++iConstBuffer;

		RenderableObjects[(uint8)ERenderLayer::Transparent].push_back(WaterObject.get());
		Objects.push_back(std::move(WaterObject));

		// Create objects
		auto BoxObject = std::make_unique<WObject>(BoxMeshName);
		BoxObject->SetConstBufferIndex(iConstBuffer);
		BoxObject->SetPosition(1.5f, 0.2f, 0.0f);
		BoxObject->SetWaterFactor(1.0);
		BoxObject->SetMaterial(GameResources->GetMaterialData("wirefence"));
		++iConstBuffer;

		RenderableObjects[(uint8)ERenderLayer::AlphaTested].push_back(BoxObject.get());
		Objects.push_back(std::move(BoxObject));

		auto SphereObject = std::make_unique<WObject>(SphereMeshName);
		SphereObject->SetConstBufferIndex(iConstBuffer);
		SphereObject->SetPosition(-2.5f, -0.2f, 2.0f);
		SphereObject->SetWaterFactor(-1.0);
		SphereObject->SetMaterial(GameResources->GetMaterialData("crate"));
		++iConstBuffer;

		RenderableObjects[(uint8)ERenderLayer::Opaque].push_back(SphereObject.get());
		Objects.push_back(std::move(SphereObject));
	
		auto SkullObject = std::make_unique<WObject>(SkullMeshName);
		SkullObject->SetConstBufferIndex(iConstBuffer);
		SkullObject->SetPosition({ -1.5f, -0.5f, 0.0f });
		SkullObject->SetScale(0.5f, 0.5f, 0.5f);
		SkullObject->SetWaterFactor(-1.0);
		SkullObject->SetMaterial(GameResources->GetMaterialData("skull"));
		SkullObject->SetIsVisible(false);
		++iConstBuffer;

		RenderableObjects[(uint8)ERenderLayer::Opaque].push_back(SkullObject.get());
		Objects.push_back(std::move(SkullObject));

		auto CameraObject = std::make_unique<WCamera>(Window->Bounds.Width, Window->Bounds.Height);
		Camera = CameraObject.get();

		Objects.push_back(std::move(CameraObject));
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

		D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
		srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvDescriptorHeapDesc.NumDescriptors = GameResources->GetNumTexturesData();
		DX::ThrowIfFailed(Device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&SRVDescriptorHeap)));
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
			nullptr, IID_PPV_ARGS(&CMDList)));
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
		const D3D_SHADER_MACRO OpaqueShaderDefines[] =
		{
			"FOG", "1",
			NULL, NULL
		};

		const D3D_SHADER_MACRO AlphaTestShaderDefines[] =
		{
			"FOG", "1",
			"ALPHA_TEST", "1",
			NULL, NULL
		};

		Shaders["standartVS"] = DX::CompileShader(L"Shaders\\Shader.hlsl", nullptr, "VS", "vs_5_0");
		Shaders["opaquePS"] = DX::CompileShader(L"Shaders\\Shader.hlsl", OpaqueShaderDefines, "PS", "ps_5_0");
		Shaders["alphatestPS"] = DX::CompileShader(L"Shaders\\Shader.hlsl", AlphaTestShaderDefines, "PS", "ps_5_0");
	}

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> FGameMain::GetStaticSamplers() const
	{
		// Applications usually only need a handful of samplers.  So just define them all up front
		// and keep them available as part of the root signature.  

		const CD3DX12_STATIC_SAMPLER_DESC PointWrap(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC PointClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC LinearWrap(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC LinearClamp(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC AnisotropicWrap(
			4, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
			0.0f,                             // mipLODBias
			8);                               // maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC AnisotropicClamp(
			5, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
			0.0f,                              // mipLODBias
			8);                                // maxAnisotropy

		return {
			PointWrap, PointClamp,
			LinearWrap, LinearClamp,
			AnisotropicWrap, AnisotropicClamp };
	}


	void FGameMain::BuildRootSignature()
	{
		// Initialize parameters

		// per object const buffer
		CD3DX12_ROOT_PARAMETER ObjectDataParameter;
		ObjectDataParameter.InitAsConstantBufferView(0);

		// material const buffer
		CD3DX12_ROOT_PARAMETER MaterialDataParameter;
		MaterialDataParameter.InitAsConstantBufferView(1);

		// frame const buffer
		CD3DX12_ROOT_PARAMETER FrameDataParameter;
		FrameDataParameter.InitAsConstantBufferView(2);

		// diffuse texture
		CD3DX12_DESCRIPTOR_RANGE DiffuseTexturesRange;
		DiffuseTexturesRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER diffuseTexturesDataParameter;
		diffuseTexturesDataParameter.InitAsDescriptorTable(
			1, &DiffuseTexturesRange, D3D12_SHADER_VISIBILITY_PIXEL);

		auto Parameters = { 
			ObjectDataParameter,
			MaterialDataParameter, 
			FrameDataParameter,
			diffuseTexturesDataParameter };

		// Initialize root signature
		CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init(
			4, Parameters.begin(), 6, GetStaticSamplers().data(), 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
		ComPtr<ID3DBlob> rootSignatureBlobError = nullptr;
		DX::ThrowIfFailed(D3D12SerializeRootSignature(
			&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, 
			&rootSignatureBlob, &rootSignatureBlobError));

		if (rootSignatureBlobError != nullptr)
		{
			throw std::invalid_argument("Root signature hasn't been initialized correctly");
		}

		DX::ThrowIfFailed(Device->CreateRootSignature(
			0, rootSignatureBlob->GetBufferPointer(), 
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}

	void FGameMain::BuildPipelineStateObject()
	{
		const D3D12_INPUT_ELEMENT_DESC inputElements[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		InputLayout.pInputElementDescs = inputElements;
		InputLayout.NumElements = 3;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC OpaquePSODesc = {};
		OpaquePSODesc.pRootSignature = RootSignature.Get();
		OpaquePSODesc.VS = { Shaders["standartVS"]->GetBufferPointer(), Shaders["standartVS"]->GetBufferSize() };
		OpaquePSODesc.PS = { Shaders["opaquePS"]->GetBufferPointer(), Shaders["opaquePS"]->GetBufferSize() };
		OpaquePSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		OpaquePSODesc.SampleMask = UINT_MAX;
		OpaquePSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		OpaquePSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		OpaquePSODesc.NumRenderTargets = 1;
		OpaquePSODesc.RTVFormats[0] = BufferFormat;
		OpaquePSODesc.DSVFormat = DepthStencilFormat;
		OpaquePSODesc.SampleDesc.Count = 1;
		OpaquePSODesc.SampleDesc.Quality = 0;
		OpaquePSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		OpaquePSODesc.InputLayout = InputLayout;

		DX::ThrowIfFailed(Device->CreateGraphicsPipelineState(
			&OpaquePSODesc,
			IID_PPV_ARGS(&PipelineStates["opaque"])));
		
		// Create pipeline state object with based on alpha bledning for transparent objects

		D3D12_RENDER_TARGET_BLEND_DESC BlendDesc;
		BlendDesc.BlendEnable = true;
		
		// Cd = As*Cs + (1-As)*Cd
		BlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		// Ad = As
		BlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		
		// Distable logic operators because of enabled blendop
		BlendDesc.LogicOpEnable = false;
		BlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		BlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC TransparentPSODesc = OpaquePSODesc;
		TransparentPSODesc.BlendState.RenderTarget[0] = BlendDesc;
		

		DX::ThrowIfFailed(Device->CreateGraphicsPipelineState(
			&TransparentPSODesc,
			IID_PPV_ARGS(&PipelineStates["transparent"])));

		// Create pipeline state object with alpha test. for semi-transparent objects
		D3D12_GRAPHICS_PIPELINE_STATE_DESC AlfaTestPSODesc = OpaquePSODesc;
		AlfaTestPSODesc.PS = {
			Shaders["alphatestPS"]->GetBufferPointer(),
			Shaders["alphatestPS"]->GetBufferSize() 
		};
		AlfaTestPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		DX::ThrowIfFailed(Device->CreateGraphicsPipelineState(
			&AlfaTestPSODesc,
			IID_PPV_ARGS(&PipelineStates["alphatest"])));
	}

	void WoodenEngine::FGameMain::Update(float dtime)
	{
		AnimateWaterMaterial();

		for(auto iObject=0; iObject < Objects.size(); ++iObject)
		{
			auto Object = Objects[iObject].get();
			if (Object->IsUpdating())
			{
				Object->Update(dtime);
			}
		}

		GameTime += 0.016f;

		// Shift frame to the next
		iCurrFrameResource = (iCurrFrameResource + 1) % NMR_SWAP_BUFFERS;
		CurrFrameResource = FramesResource[iCurrFrameResource].get();

		// Wait until this frame will be rendered with old const buffer
		WaitForGPU(CurrFrameResource->Fence);

		UpdateObjectsConstBuffer();
		UpdateMaterialsConstBuffer();
		UpdateFrameConstBuffer();
	}

	void FGameMain::AnimateWaterMaterial()
	{
		auto WaterMaterial = GameResources->GetMaterialData("water");

		auto& TexU = WaterMaterial->Transform(3, 0);
		auto& TexV = WaterMaterial->Transform(3, 1);


		auto Displacement = 0.03f*(0.5f*sin(GameTime / 15.0f) + 0.5f);

		TexU += Displacement;
		TexV += Displacement;

		// Clamp to [0, 1]
		if (TexU >= 1.0f)
		{
			TexU -= 1.0f;
		}

		if (TexV >= 1.0f)
		{
			TexV -= 1.0f;
		}

		WaterMaterial->Transform(3, 0) = TexU;
		WaterMaterial->Transform(3, 1) = TexV;

		WaterMaterial->NumDirtyConstBuffers = NMR_SWAP_BUFFERS;
	}

	void WoodenEngine::FGameMain::UpdateObjectsConstBuffer()
	{
		auto ObjectsBuffer = CurrFrameResource->ObjectsDataBuffer.get();
		for(auto iObject=0; iObject < Objects.size(); ++iObject)
		{
			auto Object = Objects[iObject].get();
			if (Object->IsRenderable() && Object->GetNumDirtyConstBuffers() > 0)
			{
				SObjectData ObjectShaderData;

				auto WorldMatrix = Object->GetWorldMatrix();
				XMStoreFloat4x4(&ObjectShaderData.WorldMatrix, XMMatrixTranspose(WorldMatrix));
				
				auto TextureTransform = Object->GetTextureTransform();
				XMStoreFloat4x4(&ObjectShaderData.MaterialTransform, 
					XMMatrixTranspose(XMLoadFloat4x4(&TextureTransform)));

				ObjectShaderData.Time = Object->GetLifeTime();
				
				// Crunch!!!!!!!!!!!!!!!!!!!!!!!!!!!
				ObjectShaderData.IsWater = Object->GetMaterial()->Name == "water";
				ObjectShaderData.WaterFactor = Object->GetWaterFactor();

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
				XMStoreFloat4x4(&MaterialShaderData.MaterialTransform, 
					XMMatrixTranspose(XMLoadFloat4x4(&MaterialData->Transform)));

				MaterialsBuffer->CopyData(MaterialData->iConstBuffer, MaterialShaderData);

				--MaterialData->NumDirtyConstBuffers;
			}
		}
	}

	void WoodenEngine::FGameMain::UpdateFrameConstBuffer()
	{
		ConstFrameData = {};

		const auto& ViewMatrix = Camera->GetViewMatrix();

		XMStoreFloat4x4(&ConstFrameData.ViewMatrix, XMMatrixTranspose(ViewMatrix));

		auto ProjMatrix = XMMatrixPerspectiveFovLH(XM_PI / 4.0f, Window->Bounds.Width / Window->Bounds.Height, 1.0, 1000.0f);
		XMStoreFloat4x4(&ConstFrameData.ProjMatrix, XMMatrixTranspose(ProjMatrix));

		auto ViewProj = XMMatrixMultiply(ViewMatrix, ProjMatrix);
		XMStoreFloat4x4(&ConstFrameData.ViewProjMatrix, XMMatrixTranspose(ViewProj));

		ConstFrameData.CameraPosition = Camera->GetWorldPosition();
		ConstFrameData.GameTime = GameTime;

		ConstFrameData.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		ConstFrameData.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };

		ConstFrameData.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		ConstFrameData.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };

		ConstFrameData.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		ConstFrameData.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

		ConstFrameData.Lights[3].Direction = { 0.0f, -1.0f, 0.0f};
		ConstFrameData.Lights[3].Strength = { 0.8f, 0.0f, 0.0f };
		ConstFrameData.Lights[3].Position = { 0.0f, 5.0f, 0.0f };
		ConstFrameData.Lights[3].FalloffStart = 400;
		ConstFrameData.Lights[3].FalloffEnd = 500;

		CurrFrameResource->FrameDataBuffer->CopyData(0, ConstFrameData);
	}

	void WoodenEngine::FGameMain::InputMouseMoved(const float dx, const float dy) noexcept
	{
		for (auto iObject = 0; iObject < Objects.size(); ++iObject)
		{
			auto Object = Objects[iObject].get();
			if (Object->IsEnabledInputEvents())
			{
				Object->InputMouseMoved(dx, dy);
			}
		}
	}

	void FGameMain::InputKeyPressed(char key)
	{
		for(auto iObject =0; iObject < Objects.size(); ++iObject)
		{
			auto Object = Objects[iObject].get();
			if (Object->IsEnabledInputEvents())
			{
				Object->InputKeyPressed(key);
			}
		}
	}

	void FGameMain::InputKeyReleased(char key)
	{
		for (auto iObject = 0; iObject < Objects.size(); ++iObject)
		{
			auto Object = Objects[iObject].get();
			if (Object->IsEnabledInputEvents())
			{
				Object->InputKeyReleased(key);
			}
		}
	}

	void WoodenEngine::FGameMain::Render()
	{
		auto CmdListAllocator = CurrFrameResource->CmdListAllocator;

		DX::ThrowIfFailed(CmdListAllocator->Reset());

		DX::ThrowIfFailed(CMDList->Reset(CmdListAllocator.Get(), PipelineStates["opaque"].Get()));

		CMDList->ResourceBarrier(
			1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
		);

		CMDList->SetGraphicsRootSignature(RootSignature.Get());

		ID3D12DescriptorHeap* srvDescriptorHeaps[] = {  SRVDescriptorHeap.Get()};
		CMDList->SetDescriptorHeaps(_countof(srvDescriptorHeaps), srvDescriptorHeaps);

		// Set frame const buffer as argument to shader
		auto FrameDataResAddress = CurrFrameResource->FrameDataBuffer->Resource()->GetGPUVirtualAddress();
		CMDList->SetGraphicsRootConstantBufferView(2, FrameDataResAddress);

		CMDList->RSSetViewports(1, &ScreenViewport);
		CMDList->RSSetScissorRects(1, &ScissorRect);

		CMDList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&ConstFrameData.FogColor, 0, nullptr);
		CMDList->ClearDepthStencilView(DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		CMDList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		CMDList->IASetVertexBuffers(0, 1, &GameResources->GetVertexBufferView());
		CMDList->IASetIndexBuffer(&GameResources->GetIndexBufferView());
		CMDList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		
		RenderObjects(RenderableObjects[(uint8)ERenderLayer::Opaque], CMDList);

		CMDList->SetPipelineState(PipelineStates["alphatest"].Get());
		RenderObjects(RenderableObjects[(uint8)ERenderLayer::AlphaTested], CMDList);
		
		CMDList->SetPipelineState(PipelineStates["transparent"].Get());
		RenderObjects(RenderableObjects[(uint8)ERenderLayer::Transparent], CMDList);


		CMDList->ResourceBarrier(
			1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)
		);

		DX::ThrowIfFailed(CMDList->Close());
		ID3D12CommandList* cmdLists[] = { CMDList.Get() };
		CmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		DX::ThrowIfFailed(SwapChain->Present(1, 0));

		iCurrBackBuffer = (iCurrBackBuffer + 1) % NMR_SWAP_BUFFERS;

		++FenceValue;
		CurrFrameResource->Fence = FenceValue;
		CmdQueue->Signal(Fence.Get(), FenceValue);
	}

	void FGameMain::RenderObjects(
		const std::vector<WObject*>& RenderableObjects, 
		ComPtr<ID3D12GraphicsCommandList> CMDList)
	{
		auto* CurMaterialsResource = CurrFrameResource->MaterialsDataBuffer.get();

		const auto MaterialConstBufferSize = CurMaterialsResource->GetElementByteSize();
		const auto ObjectConstBufferSize = CurrFrameResource->ObjectsDataBuffer->GetElementByteSize();

		for (auto Object : RenderableObjects)
		{
			if (!Object->IsVisible())
			{
				continue;
			}

			auto ObjectDataResAddress =
				CurrFrameResource->ObjectsDataBuffer->Resource()->GetGPUVirtualAddress() +
				Object->GetConstBufferIndex()*ObjectConstBufferSize;

			auto MaterialsResAddress =
				CurMaterialsResource->Resource()->GetGPUVirtualAddress() +
				Object->GetMaterial()->iConstBuffer*MaterialConstBufferSize;

			auto DiffuseTexSRVHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE{
				SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
			};
			DiffuseTexSRVHandle.Offset(Object->GetMaterial()->DiffuseTexture->iSRVHeap, 
				CBVSRVDescriptorHandleIncrementSize);


			CMDList->SetGraphicsRootConstantBufferView(0, ObjectDataResAddress);
			CMDList->SetGraphicsRootConstantBufferView(1, MaterialsResAddress);
			CMDList->SetGraphicsRootDescriptorTable(3, DiffuseTexSRVHandle);

			auto MeshName = Object->GetMeshName();
			auto MeshData = GameResources->GetMeshData(std::move(MeshName));

			CMDList->DrawIndexedInstanced(
				MeshData.Indices.size(), 1, MeshData
				.IndexBegin, MeshData.VertexBegin, 0);
		}
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