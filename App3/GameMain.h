#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>


#include "EngineSettings.h"
#include "pch.h"
#include "ShaderStructures.h"
#include "FrameResource.h"
#include "GameResource.h"

// Renders Direct3D content on the screen.
namespace WoodenEngine
{
	using namespace DirectX;

	class WObject;
	class WCamera;

	class FGameMain
 	{
	public:
		FGameMain();
		
		FGameMain(FGameMain&& GameMain) = delete;
		FGameMain(const FGameMain& GameMain) = delete;
		FGameMain& operator=(const FGameMain& GameMain) = delete;

		~FGameMain();

		/** @brief Called every frame before rendering. 
		  * It contains rendering logic
		  * @return (void)
		  */
		void Update(float dtime);

		/** @brief Called when mouse was moved 
		  * @param dx Delta X (const float)
		  * @param dy Delta Y (const float)
		  * @return (void)
		  */
		void InputMouseMoved(const float dx, const float dy) noexcept;

		/** @brief Called when a key was pressed
		  * @param key Key (char)
		  * @return (void)
		  */
		void InputKeyPressed(char key);

		/** @brief Called when a key was pressed
		  * @param key Key (char)
		  * @return (void)
		  */
		void InputKeyReleased(char key);

		/** @brief Renders game view. Is called every frame
		  * @return (void)
		  */
		void Render();

		/** @brief Renders list of objects
		  * @param RenderableObjects List of renderable objects(const std::vector<WObject * > &)
		  * @param CMDList Current command list for sending commands(ComPtr<ID3D12GraphicsCommandList>)
		  * @return (void)
		  */
		void RenderObjects(
			const std::vector<WObject*>& RenderableObjects,
			ComPtr<ID3D12GraphicsCommandList> CMDList
		);

		/** @brief Init dx12 device and other components
		  * @param outputWindow (Windows::UI::Core::CoreWindow ^)
		  * @return (bool)
		  */
		bool Initialize(Windows::UI::Core::CoreWindow^ outputWindow);
	private:
		enum class ERenderLayer: uint8
		{
			Opaque = 0,
			Transparent,
			AlphaTested,
			Count
		};

		ID3D12Resource* CurrentBackBuffer() const;
		
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	

		// Initialize device, fetch property data
		void InitDevice();

		// Initialize graphics direct command list and command queue
		void InitCmdQueue();
		
		void InitGameResources();

		void AddObjects();

		void AddMaterials();

		void AddTextures();

		// Initialize rtv heaps
		void InitDescriptorHeap();

		void InitSwapChain();

		void InitDepthStencilBuffer();

		void InitConstBuffersViews();

		void InitTexturesViews();

		void InitViewport();

		void AnimateWaterMaterial();

		// Create RTV for every frame
		void BuildFrameResources();

		void BuildRootSignature();
		
		// Compile pixel and vertex hlsl shaders
		void InitShaders();

		void BuildPipelineStateObject();

		// Wait for pending GPU work to complete with signaling
		void SignalAndWaitForGPU();

		void WaitForGPU(const uint64 NewFence);

		void UpdateObjectsConstBuffer();
		
		void UpdateMaterialsConstBuffer();

		void UpdateFrameConstBuffer();

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers() const;

		WCamera* Camera;

		// DX12 Device
		ComPtr<ID3D12Device> Device;

		ComPtr<ID3D12GraphicsCommandList> CMDList;
		ComPtr<ID3D12CommandAllocator> CmdAllocatorDefault;
		ComPtr<ID3D12CommandQueue> CmdQueue;
		
		ComPtr<IDXGISwapChain3> SwapChain;

		uint8 iCurrBackBuffer = 0;
		uint8 iCurrFrameResource = 0;
		FFrameResource* CurrFrameResource;

		std::vector<std::unique_ptr<WObject>> Objects;
		std::vector<WObject*> RenderableObjects[(uint8)ERenderLayer::Count];

		std::unique_ptr<FGameResource> GameResources;
		std::unique_ptr<FFrameResource> FramesResource[NMR_SWAP_BUFFERS];

		SFrameData ConstFrameData;

		ComPtr<ID3D12Resource> SwapChainBuffers[NMR_SWAP_BUFFERS];
		ComPtr<ID3D12Resource> DepthStencilBuffer;

		ComPtr<ID3D12RootSignature> RootSignature;

		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> PipelineStates;
		
		uint16 RTVDescriptorHandleIncrementSize = 0;
		uint16 DSVDescriptorHandleIncrementSize = 0;
		uint16 CBVSRVDescriptorHandleIncrementSize = 0;

		ComPtr<ID3D12DescriptorHeap> DSVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> RTVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> CBVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> SRVDescriptorHeap;

		D3D12_VIEWPORT ScreenViewport;
		D3D12_RECT ScissorRect;

		D3D12_INPUT_LAYOUT_DESC InputLayout;

		std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;

		// GPU and CPU synchronization
		ComPtr<ID3D12Fence> Fence;
		UINT64 FenceValue = 0;
		HANDLE fenceEvent;

		// Cached reference to the output window
		Platform::Agile<Windows::UI::Core::CoreWindow> Window;

		float GameTime;
	};
}