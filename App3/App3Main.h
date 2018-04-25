#pragma once

#include <unordered_map>
#include "pch.h"

// Renders Direct3D content on the screen.
namespace App3
{
	// number frame buffers for swap chain
	static const UINT NMR_SWAP_BUFFERS = 2;

	using Microsoft::WRL::ComPtr;

	class App3Main
	{
	public:
		App3Main();
		App3Main(const App3Main& app) = delete;
		App3Main& operator=(const App3Main& app) = delete;

		virtual ~App3Main();

		void CreateRenderers();
		void Update();
		void MouseMoved(const float dx, const float dy) noexcept;

		bool Render();

		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

		bool Initialize(Windows::UI::Core::CoreWindow^ outputWindow);
	private:
		ID3D12Resource* CurrentBackBuffer() const noexcept;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const noexcept;
		D3D12_GPU_DESCRIPTOR_HANDLE CurrentCBVGPUHandle() const noexcept;

		// initialize device, fetch property data
		void InitializeDevice();

		// initialize command allocator, graphics direct command list and command queue
		void InitializeCmdQueue();

		// initialize rtv heaps
		void InitializeDescriptorHeaps();

		void InitializeSwapChain();

		// create RTV for every frame
		void CreateFrameResources();

		void BuildRootSignature();
		
		void Rotate();

		// compile pixel and vertex hlsl shaders
		void CompileShaders();

		void BuildPipelineStateObject();

		// wait for pending GPU work to complete
		void WaitForGPU();

		void UpdateConstBuffers();

		float cameraTheta = DirectX::XM_PI/3.0f;
		float cameraPhi = DirectX::XM_PI/2.0f;
		float cameraRadius = 5.0f;

		int boxIndexCount;

		ComPtr<ID3D12Device> device;

		ComPtr<ID3D12CommandAllocator> cmdAllocator;
		ComPtr<ID3D12GraphicsCommandList> cmdList;
		ComPtr<ID3D12CommandQueue> cmdQueue;

		ComPtr<IDXGISwapChain1> deviceSwapChain;

		UINT currentBackBuffer = 0;

		ComPtr<ID3D12Resource> swapChainBuffers[NMR_SWAP_BUFFERS];
		ComPtr<ID3D12Resource> depthStencilBuffer;
		ComPtr<ID3D12Resource> constBuffer;
		BYTE* constBufferMappedData;

		ComPtr<ID3D12RootSignature> rootSignature;

		ComPtr<ID3D12PipelineState> pso;

		UINT rtvDescriptorHandleIncrementSize = 0;
		UINT dsvDescriptorHandleIncrementSize = 0;
		UINT cbvDescriptorHandleIncrementSize = 0;

		ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> cbvDescriptorHeap;

		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		
		ComPtr<ID3D12Resource> vertexBufferUpload;
		ComPtr<ID3D12Resource> indexBufferUpload;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		D3D12_VIEWPORT screenViewport;
		D3D12_RECT scissorRect;

		D3D12_INPUT_LAYOUT_DESC inputLayout;

		std::unordered_map<std::string, ComPtr<ID3DBlob>> shaders;

		// GPU and CPU synchronization
		ComPtr<ID3D12Fence> fence;
		UINT64 fenceValue = 0;
		HANDLE fenceEvent;
		
		// Chached reference to the output window
		Platform::Agile<Windows::UI::Core::CoreWindow> window;

		DXGI_FORMAT bufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D32_FLOAT;
	};
}