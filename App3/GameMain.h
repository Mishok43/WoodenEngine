#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>

#include "EngineSettings.h"
#include "Object.h"
#include "FrameResource.h"
#include "GameResources.h"
#include "pch.h"

// Renders Direct3D content on the screen.
namespace WoodenEngine
{
	class FGameMain
	{
	public:
		FGameMain() = default;
		
		FGameMain(FGameMain&& GameMain) = delete;
		FGameMain(const FGameMain& GameMain) = delete;
		FGameMain& operator=(const FGameMain& GameMain) = delete;

		virtual ~FGameMain();

		/** @brief Called every frame before rendering. 
		  * It contains rendering logic
		  * @return (void)
		  */
		void Update();

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
		void InitializeDevice();

		// Initialize graphics direct command list and command queue
		void InitializeCmdQueue();

		void BuildObjects();

		// Initialize rtv heaps
		void InitializeDescriptorHeaps();

		void InitializeSwapChain();

		void InitializeDepthStencilBuffer();

		void InitializeConstBuffersViews();

		void InitializeViewport();

		// Create RTV for every frame
		void BuildFrameResources();

		void BuildRootSignature();
		
		void Rotate();

		// Compile pixel and vertex hlsl shaders
		void CompileShaders();

		void BuildPipelineStateObject();

		// Wait for pending GPU work to complete
		void WaitForGPU();

		void UpdateConstBuffers();

		float CameraTheta = DirectX::XM_PI/3.0f;
		float CameraPhi = DirectX::XM_PI/2.0f;
		float CameraRadius = 5.0f;

		// DX12 Device
		ComPtr<ID3D12Device> Device;

		ComPtr<ID3D12GraphicsCommandList> CmdList;
		ComPtr<ID3D12CommandQueue> CmdQueue;

		ComPtr<IDXGISwapChain3> SwapChain;

		uint8 iCurrentBackBuffer = 0;

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
		ComPtr<ID3D12Fence> fence;
		UINT64 fenceValue = 0;
		HANDLE fenceEvent;


		
		// Chached reference to the output window
		Platform::Agile<Windows::UI::Core::CoreWindow> Window;
	};
}