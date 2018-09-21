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

	struct FBillboardVertex;
	class WObject;
	class WCamera;
	class WLightDirectional;
	class WLightPoint;
	class WLightSpot;
	class FFilterBlur;

	/*!
	 * \class FGameMain
	 *
	 * \brief The main game class is responsible for rendering and executing logic 
	 *
	 * \author devmi
	 * \date May 2018
	 */
	class FGameMain
 	{
	public:

		/*!
		* \enum ERenderLayer
		*
		* \brief Types of rendrable objects
		*
		* \author devmi
		* \date May 2018
		*/
		enum class ERenderLayer : uint8
		{
			Opaque = 0,
			Transparent,
			AlphaTested,
			Mirrors,
			Reflected,
			CastShadow,
			Shadow,
			Billboard,
			Geosphere,
			Count
		};


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

		/** @brief Adds object to scene for rendering
		  * @param RenderLayer Render layer of object(ERenderLayer)
		  * @param Object (WObject *)
		  * @return (void)
		  */
		void AddObjectToScene(ERenderLayer RenderLayer, WObject* Object);

		/** @brief Renders list of objects
		  * @param RenderableObjects List of renderable objects(const std::vector<WObject * > &)
		  * @param CMDList Current command list for sending commands(ComPtr<ID3D12GraphicsCommandList>)
		  * @return (void)
		  */
		void RenderObjects(
			const std::vector<WObject*>& RenderableObjects,
			ComPtr<ID3D12GraphicsCommandList> CMDList
		);

		/** @brief Renders list of renderable objects of specific render layer
		  * @param RenderLayer Render layer (ERenderLayer)
		  * @param CMDList Current command list for sending commands (ComPtr<ID3D12GraphicsCommandList>)
		  * @return (void)
		  */
		void RenderObjects(
			ERenderLayer RenderLayer,
			ComPtr<ID3D12GraphicsCommandList> CMDList
		);


		/** @brief Init dx12 device and other components
		  * @param outputWindow (Windows::UI::Core::CoreWindow ^)
		  * @return (bool)
		  */
		bool Initialize(Windows::UI::Core::CoreWindow^ outputWindow);
	private:
		/** @brief Returns current back buffer
		  * @return (ID3D12Resource*)
		  */
		ID3D12Resource* CurrentBackBuffer() const;
		
		/** @brief Returns current back buffer view
		  * @return (D3D12_CPU_DESCRIPTOR_HANDLE)
		  */
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	

		/** @brief Initializes directx device
		  * @return (void)
		  */
		void InitDevice();

		
		/** @brief Initializes cmd queue, lists and allocator
		  * @return (void)
		  */
		void InitCmdQueue();
		
		/** @brief Initializes game resources
		  * @return (void)
		  */
		void InitGameResources();

		/** @brief Adds renderable objects
		  * @return (void)
		  */
		void AddObjects();

		/** @brief Adds lights 
		  * @return (void)
		  */
		void AddLights();

		/** @brief Loads materials
		  * @return (void)
		  */
		void AddMaterials();

		/** @brief Loads textures
		  * @return (void)
		  */
		void AddTextures();
		
		/** @brief Initializes descriptor heaps
		  * @return (void)
		  */
		void InitDescriptorHeaps();

		/** @brief Initializes device swap chain
		  * @return (void)
		  */
		void InitSwapChain();

		/** @brief Initializes depth stencil buffer
		  * @return (void)
		  */
		void InitDepthStencilBuffer();

		/** @brief Initializes loaded textures views
		  * @return (void)
		  */
		void InitTexturesViews();

		/** @brief Initializes viewport settins and scissor rectangle
		  * @return (void)
		  */
		void InitViewport();

		/** @brief Initializes rendering filters
		  * @return (void)
		  */
		void InitFilters();

		/** @brief Animates water materials. Shifts water's textures coordinates
		  * @return (void)
		  */
		void AnimateWaterMaterial();

		
		/** @brief Builds frame resources 
		  * objects + frame consts + materials resource buffers per every frame 
		  * @return (void)
		  */
		void BuildFrameResources();

		/** @brief Builds root signature
		  * @return (void)
		  */
		void BuildRootSignature();
		
		
		/** @brief Compiles pixel and vertex shaders
		  * @return (void)
		  */
		void InitShaders();

		/** @brief Builds pipeline state objects for different render layers
		  * @return (void)
		  */
		void BuildPipelineStateObject();

		
		/** @brief Signals and waits for gpu until it reaches the current fence
		  * @return (void)
		  */
		void SignalAndWaitForGPU();

		/** @brief Waits for gpu until it reaches the new fence
		  * If the cmd queue has reached yet -> it just returns without waiting
		  * @param NewFence Fence to reach (const uint64)
		  * @return (void)
		  */
		void WaitForGPU(const uint64 NewFence);

		/** @brief Updates const buffers of renderable objects
		  * @return (void)
		  */
		void UpdateObjectsConstBuffer();


		/** @brief Updates reflected frame's const buffers
		  * @return (void)
		  */
		void UpdateReflectedFrameConstBuffer();

		/** @brief Updates const buffers of materials
		  * @return (void)
		  */
		void UpdateMaterialsConstBuffer();

		/** @brief Updates const buffer of current frame
		  * @return (void)
		  */
		void UpdateFrameConstBuffer();

		/** @brief Updates shadow's transform
		  * @return (void)
		  */
		void UpdateShadowTransform();

		/** @brief Updates reflection's transform
		  * @return (void)
		  */
		void UpdateReflectionTransform();

		/** @brief Updates demo logic
		* @return (void)
		*/
		void UpdateDemoLogic();

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers() const;

		// Current view camera
		WCamera* Camera;

		XMVECTOR MirrorPlane;
		XMVECTOR ShadowPlane;

		const WLightPoint* CastShadowLight;

		WObject* DinoReflectedObject;
		WObject* DinoShadowObject;
		WObject* DinoObject;

		// DX12 Device
		ComPtr<ID3D12Device> Device;

		ComPtr<ID3D12GraphicsCommandList> CMDList;
		ComPtr<ID3D12CommandAllocator> CmdAllocatorDefault;
		ComPtr<ID3D12CommandQueue> CmdQueue;
		
		ComPtr<IDXGISwapChain3> SwapChain;

		// Index of current back buffer
		uint8 iCurrBackBuffer = 0;

		// Index of current frame resource
		uint8 iCurrFrameResource = 0;

		// Current frame resource

		FFrameResource* CurrFrameResource;
		
		// Array with all existing objects (renderable and not renderable)
		std::vector<std::unique_ptr<WObject>> Objects;

		std::vector<WLightDirectional*> LightsDirectional;
		std::vector<WLightPoint*> LightsPoint;
		std::vector<WLightSpot*> LightsSpot;

		// Array with renderable objects. It's divided to several render layers. 
		// See ERenderLayer
		std::vector<WObject*> RenderableObjects[(uint8)ERenderLayer::Count];

		// Number const buffers for renderable objects
		uint8 NumRenderableObjectsConstBuffers = 0;

		std::unique_ptr<FFilterBlur> FilterBlur;

		std::unique_ptr<FGameResource> GameResources;
		std::unique_ptr<FFrameResource> FramesResource[NMR_SWAP_BUFFERS];

		SFrameData FrameConstData;

		ComPtr<ID3D12Resource> SwapChainBuffers[NMR_SWAP_BUFFERS];
		ComPtr<ID3D12Resource> DepthStencilBuffer;

		std::unordered_map<std::string, ComPtr<ID3D12RootSignature>> RootSignatures;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> PipelineStates;
		
		uint16 RTVDescriptorHandleIncrementSize = 0;
		uint16 DSVDescriptorHandleIncrementSize = 0;
		uint16 CBVSRVDescriptorHandleIncrementSize = 0;

		ComPtr<ID3D12DescriptorHeap> DSVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> RTVDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap> SRVDescriptorHeap;

		D3D12_VIEWPORT ScreenViewport;
		D3D12_RECT ScissorRect;

		D3D12_INPUT_LAYOUT_DESC InputLayout;

		std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;

		// GPU and CPU synchronization
		ComPtr<ID3D12Fence> Fence;
		UINT64 FenceValue = 0;
		HANDLE fenceEvent;
		
		uint32 MsaaQualityLevels;

		// Cached reference to the output window
		Platform::Agile<Windows::UI::Core::CoreWindow> Window;

		float GameTime;
	};
}