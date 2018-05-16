#include "pch.h"
#include <array>

#include "MeshData.h"
#include "GameMain.h"
#include "Object.h"
#include "Camera.h"
#include "Common/DirectXHelper.h"
#include "LightPoint.h"
#include "LightDirectional.h"
#include "LightSpot.h"
#include "FloatingLightPoint.h"
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

		InitDescriptorHeaps();

		InitSwapChain();
		InitDepthStencilBuffer();
		InitViewport();

		BuildFrameResources();
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
			FramesResource[iFrame] = std::make_unique<FFrameResource>(
				Device, NumRenderableObjectsConstBuffers, GameResources->GetNumMaterials());
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
		D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DepthStencilFormat, Window->Bounds.Width, Window->Bounds.Height, 1, 1);
		
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

		Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &depthStencilViewDesc,
			DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
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
	}

	void FGameMain::InitGameResources()
	{
		GameResources = std::make_unique<FGameResource>(Device);

		AddTextures();
		AddMaterials();
		AddObjects();
		AddLights();
	}
	
	void FGameMain::AddTextures()
	{
		auto BasePath = static_cast<std::wstring>(L"Assets\\Textures\\");

		GameResources->LoadTexture(BasePath + L"WireFence.dds", "wirefence", CMDList);
		GameResources->LoadTexture(BasePath + L"WoodCrate01.dds", "crate", CMDList);
		GameResources->LoadTexture(BasePath + L"water1.dds", "water", CMDList);
		GameResources->LoadTexture(BasePath + L"grass.dds", "grass", CMDList);
		GameResources->LoadTexture(BasePath + L"ice.dds", "glass", CMDList);
		GameResources->LoadTexture(BasePath + L"white1x1.dds", "white1x1", CMDList);
	}


	void FGameMain::AddMaterials()
	{
		uint64 iConstBuffer = 0;

		auto GrassMaterial = std::make_unique<FMaterialData>("grass");
		GrassMaterial->iConstBuffer = iConstBuffer;
		GrassMaterial->FresnelR0 = { 0.01f, 0.01f, 0.01f };
		GrassMaterial->Roughness = 0.800f;
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

		auto dinoMaterial = std::make_unique<FMaterialData>("dino");
		dinoMaterial->iConstBuffer = iConstBuffer;
		dinoMaterial->FresnelR0 = { 0.05f, 0.05f, 0.05f };
		dinoMaterial->Roughness = 0.4f;
		dinoMaterial->DiffuseTexture = GameResources->GetTextureData("white1x1");
		GameResources->AddMaterial(std::move(dinoMaterial));
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

		auto GlassMaterial = std::make_unique<FMaterialData>("glass");
		GlassMaterial->iConstBuffer = iConstBuffer;
		GlassMaterial->FresnelR0 = { 0.1f, 0.1f, 0.1f };
		GlassMaterial->Roughness = 0.7f;
		GlassMaterial->DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 0.4f };
		GlassMaterial->DiffuseTexture = GameResources->GetTextureData("glass");
		GameResources->AddMaterial(std::move(GlassMaterial));
		++iConstBuffer;

		auto ShadowMaterial = std::make_unique<FMaterialData>("shadow");
		ShadowMaterial->iConstBuffer = iConstBuffer;
		ShadowMaterial->FresnelR0 = { 0.001f, 0.001f, 0.001f};
		ShadowMaterial->DiffuseAlbedo = { 0.0f, 0.0f, 0.0f, 0.5f };
		ShadowMaterial->Roughness = 1.0f;
		ShadowMaterial->DiffuseTexture = GameResources->GetTextureData("white1x1");
		GameResources->AddMaterial(std::move(ShadowMaterial));
		++iConstBuffer;
	}

	void FGameMain::AddObjects()
	{
		// Build and load static meshes
		auto MeshGenerator = std::make_unique<FMeshGenerator>();
		
		auto BoxMesh = MeshGenerator->CreateBox(1.0f, 1.0f, 1.0f);
		
		auto LandscapeMesh = MeshGenerator->CreateLandscapeGrid(40.0f, 40.0f, 80, 80);
		LandscapeMesh->Name = "landscape";

		auto MirrorMesh = MeshGenerator->CreateBox(6.0f, 6.0f, 0.5f, 4);
		MirrorMesh->Name = "Mirror";

		auto PlaneMesh = MeshGenerator->CreateGrid(23.0f, 23.0f, 30, 30);
		auto SphereMesh = MeshGenerator->CreateSphere(1.0f, 15.0f, 15.0f);

		auto MeshParser = std::make_unique<FMeshParser>();
		//auto dinoMesh = MeshParser->ParseTxtData("Assets\\Models\\dino.txt");
		//dinoMesh->Name = "dino";

		auto DinoMesh = MeshParser->ParseObjFile("Assets\\Models\\robot.obj");
		DinoMesh->Name = "robot";

		const auto BoxSubmeshName = BoxMesh->Name;
		const auto SphereSubmeshName = SphereMesh->Name;
		//const auto dinoSubmeshName = dinoMesh->Name;
		const auto PlaneSubmeshName = PlaneMesh->Name;
		const auto LandscapeSubmeshName = LandscapeMesh->Name;
		const auto MirrorSubmeshName = MirrorMesh->Name;
		const auto RobotSubmeshName = DinoMesh->Name;

		const std::string& GeoMeshName = "geo";
		std::vector<std::unique_ptr<FMeshRawData>> GeometricSubmeshes;
		GeometricSubmeshes.push_back(std::move(BoxMesh));
		GeometricSubmeshes.push_back(std::move(SphereMesh));
		GameResources->LoadStaticMesh(std::move(GeometricSubmeshes), GeoMeshName, CMDList);

		const std::string& EnviromentMeshName = "env";
		std::vector<std::unique_ptr<FMeshRawData>> EnviromentSubmeshes;
		EnviromentSubmeshes.push_back(std::move(PlaneMesh));
		EnviromentSubmeshes.push_back(std::move(LandscapeMesh));
		EnviromentSubmeshes.push_back(std::move(MirrorMesh));
		GameResources->LoadStaticMesh(std::move(EnviromentSubmeshes), EnviromentMeshName, CMDList);

		const std::string& DinoMeshName = "dino";
		std::vector<std::unique_ptr<FMeshRawData>> DinoSubmeshes;
		DinoSubmeshes.push_back(std::move(DinoMesh));
		GameResources->LoadStaticMesh(std::move(DinoSubmeshes), DinoMeshName, CMDList);

		uint8 iConstBuffer = 0;

		auto LandscapeObject = std::make_unique<WObject>(EnviromentMeshName, LandscapeSubmeshName);
		XMFLOAT4X4 LandscapeTextureTransform;
		XMStoreFloat4x4(&LandscapeTextureTransform, XMMatrixScaling(6.0f, 6.0f, 1.0f));
		LandscapeObject->SetTextureTransform(std::move(LandscapeTextureTransform));
		LandscapeObject->SetPosition(0, -2, 0);
		LandscapeObject->SetWaterFactor(0);
		LandscapeObject->SetMaterial(GameResources->GetMaterialData("grass"));
		
		AddObjectToScene(ERenderLayer::Opaque, LandscapeObject.get());
		Objects.push_back(std::move(LandscapeObject));

		auto WaterObject = std::make_unique<WObject>(EnviromentMeshName, PlaneSubmeshName);
		XMFLOAT4X4 TextureTransform;
		XMStoreFloat4x4(&TextureTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
		WaterObject->SetTextureTransform(std::move(TextureTransform));
		WaterObject->SetPosition(0, 0, 0);
		WaterObject->SetMaterial(GameResources->GetMaterialData("water"));

		AddObjectToScene(ERenderLayer::Transparent, WaterObject.get());
		Objects.push_back(std::move(WaterObject));

		// Create objects
		auto BoxObject = std::make_unique<WObject>(GeoMeshName, BoxSubmeshName);
		BoxObject->SetPosition(1.5f, 0.2f, 0.0f);
		BoxObject->SetWaterFactor(1.0);
		BoxObject->SetMaterial(GameResources->GetMaterialData("wirefence"));

		AddObjectToScene(ERenderLayer::AlphaTested, BoxObject.get());
		Objects.push_back(std::move(BoxObject));

		auto SphereObject = std::make_unique<WObject>(GeoMeshName, SphereSubmeshName);
		SphereObject->SetPosition(-2.5f, -0.2f, 2.0f);
		SphereObject->SetWaterFactor(-1.0);
		SphereObject->SetMaterial(GameResources->GetMaterialData("crate"));

		AddObjectToScene(ERenderLayer::Opaque, SphereObject.get());
		Objects.push_back(std::move(SphereObject));
	
		auto PlatformObject = std::make_unique<WObject>(GeoMeshName, BoxSubmeshName);
		PlatformObject->SetPosition(-25.0f, 8.0f, -15.0f);
		PlatformObject->SetScale(20.0f, 1.0f, 20.0f);
		PlatformObject->SetWaterFactor(0);
		PlatformObject->SetMaterial(GameResources->GetMaterialData("grass"));

		auto ShadowPlaneNormal = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		auto ShadowPlaneDisplacement = XMVectorGetX(
			XMVector3Dot(XMLoadFloat3(&PlatformObject->GetWorldPosition()), ShadowPlaneNormal));
		ShadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, -ShadowPlaneDisplacement);

		AddObjectToScene(ERenderLayer::Opaque, PlatformObject.get());
		Objects.push_back(std::move(PlatformObject));

		auto MirrorObject = std::make_unique<WObject>(EnviromentMeshName, MirrorSubmeshName);
		MirrorObject->SetPosition(-25.0f, 10.5f, -15.0f);
		MirrorObject->SetWaterFactor(0);
		MirrorObject->SetMaterial(GameResources->GetMaterialData("glass"));

		auto MirrorPlaneDirection = XMVectorSet(-0.0f, -0.0f, -1.0f, 1.0f);
		auto MirrorDisplacement = XMVectorGetX(
			XMVector3Dot(XMLoadFloat3(&MirrorObject->GetWorldPosition()), MirrorPlaneDirection));
		MirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, MirrorDisplacement);

		AddObjectToScene(ERenderLayer::Mirrors, MirrorObject.get());
		Objects.push_back(std::move(MirrorObject));

		auto DinoObject = std::make_unique<WObject>(DinoMeshName, RobotSubmeshName);
		DinoObject->SetPosition(-25.0f, 8.5f, -23.0f);
		DinoObject->SetRotation(0, XM_PI, 0);
		DinoObject->SetScale(0.5f, 0.5f, 0.5f);
		DinoObject->SetWaterFactor(0);
		DinoObject->SetMaterial(GameResources->GetMaterialData("dino"));

		auto DinoReflectedObject = std::make_unique<WObject>(*DinoObject);

		auto ReflectTransform = XMMatrixReflect(MirrorPlane);
		auto DinoWorldTransform = DinoObject->GetWorldTransform();
		DinoReflectedObject->SetWorldTransform(DinoWorldTransform*ReflectTransform);

		auto DinoShadowObject = std::make_unique<WObject>(*DinoObject);
		DinoShadowObject->SetMaterial(GameResources->GetMaterialData("shadow"));

		this->DinoObject = DinoObject.get();
		this->DinoShadowObject = DinoShadowObject.get();

		AddObjectToScene(ERenderLayer::Shadow, DinoShadowObject.get());
		Objects.push_back(std::move(DinoShadowObject));

		AddObjectToScene(ERenderLayer::CastShadow, DinoObject.get());
		Objects.push_back(std::move(DinoObject));

		AddObjectToScene(ERenderLayer::Reflected, DinoReflectedObject.get());
		Objects.push_back(std::move(DinoReflectedObject));

		auto CameraObject = std::make_unique<WCamera>(Window->Bounds.Width, Window->Bounds.Height);
		Camera = CameraObject.get();

		Objects.push_back(std::move(CameraObject));
	}

	void FGameMain::AddLights()
	{
		auto LeftFrontLight = std::make_unique<WLightDirectional>(
			XMFLOAT3(0.6f, 0.6f, 0.6f), XMFLOAT3(0.57735f, -0.57735f, 0.57735f));
		LightsDirectional.push_back(LeftFrontLight.get());
		Objects.push_back(std::move(LeftFrontLight));

		auto RightFrontLight = std::make_unique<WLightDirectional>(
			XMFLOAT3(0.3f, 0.3f, 0.3f), XMFLOAT3(-0.57735f, 0.57735f, 0.57735f));
		LightsDirectional.push_back(RightFrontLight.get());
		Objects.push_back(std::move(RightFrontLight));

		auto BackLight = std::make_unique<WLightDirectional>(
			XMFLOAT3(0.15f, 0.15f, 0.15f), XMFLOAT3(0.0f, -0.707f, -0.707f));
		LightsDirectional.push_back(BackLight.get());
		Objects.push_back(std::move(BackLight));


		auto DinoLight = std::make_unique<WFloatingLightPoint>(
			XMFLOAT3(0.8f, 0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),
			XMFLOAT3( 0.0f, 0.0f, 0.0f), 10.0f, 20.0f 
		);

		auto DinoPosition = DinoObject->GetWorldPosition();


		DinoLight->SetTrajectory(
			{ DinoPosition.x - 6.0f, DinoPosition.y + 8.0f, DinoPosition.z - 3.5f },
			{ DinoPosition.x + 8.0f, DinoPosition.y + 8.0f, DinoPosition.z - 3.5f },
			5.0f, true
		);

		DinoLight->SetIsRenderable(true);
		DinoLight->SetMesh("geo", "sphere");
		DinoLight->SetMaterial(GameResources->GetMaterialData("dino"));
		DinoLight->SetScale(0.2f, 0.2f, 0.2f);

		CastShadowLight = DinoLight.get();

		AddObjectToScene(ERenderLayer::Opaque, DinoLight.get());
		LightsPoint.push_back(DinoLight.get());
		Objects.push_back(std::move(DinoLight));
	}

	void FGameMain::InitDescriptorHeaps()
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
			"LIGHTING", "1",
			NULL, NULL
		};

		const D3D_SHADER_MACRO AlphaTestShaderDefines[] =
		{
			"FOG", "1",
			"ALPHA_TEST", "1",
			"LIGHTING", "1",
			NULL, NULL
		};

		const D3D_SHADER_MACRO ShadowShaderDefines[] = {
			"FOG", "1",
			NULL, NULL
		};

		Shaders["standartVS"] = DX::CompileShader(L"Shaders\\Shader.hlsl", nullptr, "VS", "vs_5_0");
		Shaders["opaquePS"] = DX::CompileShader(L"Shaders\\Shader.hlsl", OpaqueShaderDefines, "PS", "ps_5_0");
		Shaders["alphatestPS"] = DX::CompileShader(L"Shaders\\Shader.hlsl", AlphaTestShaderDefines, "PS", "ps_5_0");
		Shaders["shadowPS"] = DX::CompileShader(L"Shaders\\Shader.hlsl", ShadowShaderDefines, "PS", "ps_5_0");
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
		
		// Create pipeline state object with based on alpha blending for transparent objects

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

		// Creates pipelines state objects for marking drawing mirros to stencil buffer

		// Marks all projected pixels in stencil buffers as stencil reference
		D3D12_DEPTH_STENCIL_DESC MarkMirrorsDepthStencilDesc;
		MarkMirrorsDepthStencilDesc.DepthEnable = true;
		MarkMirrorsDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		MarkMirrorsDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

		MarkMirrorsDepthStencilDesc.StencilEnable = true;
		MarkMirrorsDepthStencilDesc.StencilReadMask = 0xFF;
		MarkMirrorsDepthStencilDesc.StencilWriteMask = 0xFF;

		MarkMirrorsDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		MarkMirrorsDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		MarkMirrorsDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		MarkMirrorsDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;

		MarkMirrorsDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		MarkMirrorsDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		MarkMirrorsDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		MarkMirrorsDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC MarkMirrorsPSODesc = OpaquePSODesc;
		MarkMirrorsPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
		MarkMirrorsPSODesc.DepthStencilState = MarkMirrorsDepthStencilDesc;

		DX::ThrowIfFailed(Device->CreateGraphicsPipelineState(
			&MarkMirrorsPSODesc,
			IID_PPV_ARGS(&PipelineStates["markmirrors"])
		));

		// Create pipeline state object for rendering reflected from mirrors objects
		D3D12_DEPTH_STENCIL_DESC ReflectionsDepthStencilDesc;
		ReflectionsDepthStencilDesc.DepthEnable = true;
		ReflectionsDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		ReflectionsDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		ReflectionsDepthStencilDesc.StencilEnable = true;
		ReflectionsDepthStencilDesc.StencilReadMask = 0xFF;
		ReflectionsDepthStencilDesc.StencilWriteMask = 0x00;

		ReflectionsDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		ReflectionsDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		ReflectionsDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		ReflectionsDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

		ReflectionsDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		ReflectionsDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		ReflectionsDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		ReflectionsDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC ReflectionsPSODesc = OpaquePSODesc;
		ReflectionsPSODesc.RasterizerState.FrontCounterClockwise ^= 1;
		ReflectionsPSODesc.DepthStencilState = ReflectionsDepthStencilDesc;

		DX::ThrowIfFailed(Device->CreateGraphicsPipelineState(
			&ReflectionsPSODesc,
			IID_PPV_ARGS(&PipelineStates["reflections"])
		));

		D3D12_DEPTH_STENCIL_DESC ShadowDepthStencilDesc;
		ShadowDepthStencilDesc.DepthEnable = true;
		ShadowDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		ShadowDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		ShadowDepthStencilDesc.StencilEnable = true;
		ShadowDepthStencilDesc.StencilReadMask = 0xFF;
		ShadowDepthStencilDesc.StencilWriteMask = 0xFF;

		ShadowDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		ShadowDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
		ShadowDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		ShadowDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

		ShadowDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		ShadowDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
		ShadowDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		ShadowDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;


		D3D12_GRAPHICS_PIPELINE_STATE_DESC ShadowPSODesc = TransparentPSODesc;
		ShadowPSODesc.DepthStencilState = ShadowDepthStencilDesc;
		ShadowPSODesc.PS = {
			Shaders["shadowPS"]->GetBufferPointer(),
			Shaders["shadowPS"]->GetBufferSize()
		};
		DX::ThrowIfFailed(Device->CreateGraphicsPipelineState(
			&ShadowPSODesc,
			IID_PPV_ARGS(&PipelineStates["shadow"])
		));

	}

	void WoodenEngine::FGameMain::Update(float dtime)
	{
		dtime = 0.016f;

		for(auto iObject=0; iObject < Objects.size(); ++iObject)
		{
			auto Object = Objects[iObject].get();
			if (Object->IsUpdating())
			{
				Object->Update(dtime);
			}
		}

		AnimateWaterMaterial();
		UpdateShadowTransform();

		GameTime += dtime;

		// Shift frame to the next
		iCurrFrameResource = (iCurrFrameResource + 1) % NMR_SWAP_BUFFERS;
		CurrFrameResource = FramesResource[iCurrFrameResource].get();

		// Wait until this frame will be rendered with old const buffer
		WaitForGPU(CurrFrameResource->Fence);

		UpdateObjectsConstBuffer();
		UpdateReflectedFrameConstBuffer();
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

	void WoodenEngine::FGameMain::UpdateShadowTransform()
	{
		auto WorldTransform = DinoObject->GetWorldTransform();
		auto LightPosition = CastShadowLight->GetWorldPosition();
		auto LightPositionH = XMFLOAT4(LightPosition.x, LightPosition.y, LightPosition.z, 1.0f);

		auto ShadowTransform = XMMatrixShadow(ShadowPlane, XMLoadFloat4(&LightPositionH));

		auto LiftUpTransform = XMMatrixTranslation(0.0f,  0.5f+0.001f, 0.0f);
		DinoShadowObject->SetWorldTransform(WorldTransform*ShadowTransform*LiftUpTransform);

		DinoShadowObject->SetNumDirtyConstBuffers(NMR_SWAP_BUFFERS);
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

				auto WorldMatrix = Object->GetWorldTransform();
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
		FrameConstData = {};

		const auto& ViewMatrix = Camera->GetViewMatrix();

		XMStoreFloat4x4(&FrameConstData.ViewMatrix, XMMatrixTranspose(ViewMatrix));

		auto ProjMatrix = XMMatrixPerspectiveFovLH(XM_PI / 4.0f, Window->Bounds.Width / Window->Bounds.Height, 1.0, 1000.0f);
		XMStoreFloat4x4(&FrameConstData.ProjMatrix, XMMatrixTranspose(ProjMatrix));

		auto ViewProj = XMMatrixMultiply(ViewMatrix, ProjMatrix);
		XMStoreFloat4x4(&FrameConstData.ViewProjMatrix, XMMatrixTranspose(ViewProj));

		FrameConstData.CameraPosition = Camera->GetWorldPosition();
		FrameConstData.GameTime = GameTime;

		uint8 iLight = 0;
		for (auto i = 0; i < LightsDirectional.size(); ++i, ++iLight)
		{
			FrameConstData.Lights[iLight] = LightsDirectional[i]->GetShaderData();
		}

		for (auto i = 0; i < LightsPoint.size(); ++i, ++iLight)
		{
			FrameConstData.Lights[iLight] = LightsPoint[i]->GetShaderData();
		}

		for (auto i = 0; i < LightsSpot.size(); ++i, ++iLight)
		{
			FrameConstData.Lights[iLight] = LightsSpot[i]->GetShaderData();
		}

		CurrFrameResource->FrameDataBuffer->CopyData(0, FrameConstData);
	}

	void WoodenEngine::FGameMain::UpdateReflectedFrameConstBuffer()
	{
		auto ReflectedFrameConstBuffer = FrameConstData;

		XMMATRIX ReflectionMatrix = XMMatrixReflect(MirrorPlane);

		uint8 iLight = 0;
		for (auto i = 0; i < LightsDirectional.size(); ++i, ++iLight)
		{
			auto& LightData = ReflectedFrameConstBuffer.Lights[iLight];
			
			auto Direction = XMLoadFloat3(&LightData.Direction);
			XMStoreFloat3(&LightData.Direction, XMVector3Transform(Direction, ReflectionMatrix));
		}

		for (auto i = 0; i < LightsPoint.size(); ++i, ++iLight)
		{
			auto& LightData = ReflectedFrameConstBuffer.Lights[iLight];

			auto Position = XMLoadFloat3(&LightData.Position);
			XMStoreFloat3(&LightData.Position, XMVector3Transform(Position, ReflectionMatrix));
		}

		for (auto i = 0; i < LightsSpot.size(); ++i, ++iLight)
		{
			ReflectedFrameConstBuffer.Lights[iLight] = LightsSpot[i]->GetShaderData();
		}

		CurrFrameResource->FrameDataBuffer->CopyData(1, ReflectedFrameConstBuffer);
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
		auto FrameConstDataResAddress = CurrFrameResource->FrameDataBuffer->Resource()->GetGPUVirtualAddress();
		CMDList->SetGraphicsRootConstantBufferView(2, FrameConstDataResAddress);

		CMDList->RSSetViewports(1, &ScreenViewport);
		CMDList->RSSetScissorRects(1, &ScissorRect);

		CMDList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&FrameConstData.FogColor, 0, nullptr);
		CMDList->ClearDepthStencilView(DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		CMDList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		RenderObjects(ERenderLayer::Opaque, CMDList);

		CMDList->OMSetStencilRef(1);
		CMDList->SetPipelineState(PipelineStates["markmirrors"].Get());
		RenderObjects(ERenderLayer::Mirrors, CMDList);

		// Set reflected frame const buffer as argument to shader
		auto ReflectedFrameConstDataResAddress = CurrFrameResource->FrameDataBuffer->Resource()->GetGPUVirtualAddress() +
			CurrFrameResource->FrameDataBuffer->GetElementByteSize();
		CMDList->SetGraphicsRootConstantBufferView(2, ReflectedFrameConstDataResAddress);

		CMDList->SetPipelineState(PipelineStates["reflections"].Get());
		RenderObjects(ERenderLayer::Reflected, CMDList);

		CMDList->OMSetStencilRef(0);
		CMDList->SetGraphicsRootConstantBufferView(2, FrameConstDataResAddress);

		CMDList->SetPipelineState(PipelineStates["alphatest"].Get());
		RenderObjects(ERenderLayer::AlphaTested, CMDList);
		
		CMDList->ClearDepthStencilView(DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),  D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);


		CMDList->SetPipelineState(PipelineStates["shadow"].Get());
		RenderObjects(ERenderLayer::Shadow, CMDList);

		CMDList->SetPipelineState(PipelineStates["opaque"].Get());
		RenderObjects(ERenderLayer::CastShadow, CMDList);

		CMDList->SetPipelineState(PipelineStates["transparent"].Get());
		RenderObjects(ERenderLayer::Transparent, CMDList);
		RenderObjects(ERenderLayer::Mirrors, CMDList);


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

	void FGameMain::AddObjectToScene(ERenderLayer RenderLayer, WObject* Object)
	{
		Object->SetConstBufferIndex(NumRenderableObjectsConstBuffers);
		RenderableObjects[(uint8)RenderLayer].push_back(Object);

		++NumRenderableObjectsConstBuffers;
	}

	void FGameMain::RenderObjects(ERenderLayer RenderLayer, ComPtr<ID3D12GraphicsCommandList> CMDList)
	{
		RenderObjects(RenderableObjects[(uint8)RenderLayer], CMDList);
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

			const auto& MeshData = GameResources->GetMeshData(Object->GetMeshName());
			const auto& SubmeshData = GameResources->GetSubmeshData(Object->GetMeshName(), Object->GetSubmeshName());

			CMDList->IASetVertexBuffers(0, 1, &MeshData.VertexBufferView);
			CMDList->IASetIndexBuffer(&MeshData.IndexBufferView);
			CMDList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

			CMDList->DrawIndexedInstanced(
				SubmeshData.NumIndices, 1, SubmeshData.IndexBegin,
				SubmeshData.VertexBegin, 0);
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