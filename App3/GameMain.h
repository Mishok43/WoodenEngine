#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>

#include "EngineSettings.h"
#include "pch.h"


// Renders Direct3D content on the screen.
namespace WoodenEngine
{
	using namespace DirectX;

	class FFrameResource;
	class FGameResources;
	class WObject;
	class WCamera;

	class FGameMain
 	{
	public:
		FGameMain();
		
		FGameMain(FGameMain&& GameMain) = delete;
		FGameMain(const FGameMain& GameMain) = delete;
		FGameMain& operator=(const FGameMain& GameMain) = delete;

		virtual ~FGameMain();

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
		void MouseMoved(const float dx, const float dy) noexcept;

		/** @brief Called every frame
		  * @return (void)
		  */
		void Render();

		/** @brief Init dx12 device and other components
		  * @param outputWindow (Windows::UI::Core::CoreWindow ^)
		  * @return (bool)
		  */
		bool Initialize(Windows::UI::Core::CoreWindow^ outputWindow);
	private:
		ID3D12Resource* CurrentBackBuffer() const;
		
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		
		D3D12_GPU_DESCRIPTOR_HANDLE CurrentCBVGPUHandle() const;

		// Initialize device, fetch property data
		void InitDevice();

		// Initialize graphics direct command list and command queue
		void InitCmdQueue();
		
		void AddObjects();

		// Initialize rtv heaps
		void InitDescriptorHeap();

		void InitSwapChain();

		void InitDepthStencilBuffer();

		void InitConstBuffersViews();

		void InitViewport();

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

		void UpdateFrameConstBuffer();

		WCamera* Camera;

		// DX12 Device
		ComPtr<ID3D12Device> Device;

		ComPtr<ID3D12GraphicsCommandList> CmdList;
		ComPtr<ID3D12CommandAllocator> CmdAllocatorDefault;
		ComPtr<ID3D12CommandQueue> CmdQueue;
		
		ComPtr<IDXGISwapChain3> SwapChain;

		uint8 iCurrBackBuffer = 0;
		uint8 iCurrFrameResource = 0;
		FFrameResource* CurrFrameResource;

		std::vector<WObject*>  Objects; 

		std::unique_ptr<FGameResources> GameResources;
		std::unique_ptr<FFrameResource> FramesResource[NMR_SWAP_BUFFERS];

		ComPtr<ID3D12Resource> SwapChainBuffers[NMR_SWAP_BUFFERS];
		ComPtr<ID3D12Resource> DepthStencilBuffer;

		ComPtr<ID3D12RootSignature> RootSignature;

		ComPtr<ID3D12PipelineState> PSO;

		uint16 RTVDescriptorHandleIncrementSize = 0;
		uint16 DSVDescriptorHandleIncrementSize = 0;
		uint16 CBVDescriptorHandleIncrementSize = 0;

		ComPtr<ID3D12DescriptorHeap> DSVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> RTVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> CBVDescriptorHeap;

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
	};
}