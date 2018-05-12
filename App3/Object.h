#pragma once
#include <iostream>
#include <string>

#include "ShaderStructures.h"
#include "MathHelper.h"
#include "EngineSettings.h"

namespace WoodenEngine
{
	struct FMaterialData;

	/*!
	 * \class BObject
	 *
	 * \brief Basic visible object of the engine. It contains essential data for rendering.
	 *
	 * \author devmi
	 * \date April 2018
	 */
	class WObject
	{
		public:
			WObject()=default;
			virtual ~WObject()=default;

			WObject(const WObject& Obj) = delete;
			WObject(WObject&& Obj) = delete;
			WObject& operator=(const WObject& Obj) = delete;
			
			/** @brief 
			  * @param MeshName Name of static mesh (const std::string &)
			  * @param Position Absolute world position (const XMFLOAT3 &)
			  * @param Rotation Absolute world rotation (const XMFLOAT3 &)
			  * @param Scale Absolute world scale (const XMFLOAT3 &)
			  * @return ()
			  */
			WObject(
				const std::string& MeshName, 
				const XMFLOAT3& Position = { 0.0f, 0.0f, 0.0f },
				const XMFLOAT3& Rotation = { 0.0f, 0.0f, 0.0f },
				const XMFLOAT3& Scale = { 1.0f, 1.0f, 1.0f });
			
			/** @brief Update function for executing object's login
			  * @warning bIsUpdating must be true!
			  * @param Delta Delta time(float)
			  * @return (void)
			  */
			virtual void Update(float Delta);

			/** @brief Enable/Disable tracking input events (such as mouse moving)
			  * @param EnableInput (const bool)
			  * @return (void)
			  */
			void SetEnableInputEvents(const bool EnableInput) noexcept;

			/** @brief Called when mouse was moved
			* @param dx Delta X (const float)
			* @param dy Delta Y (const float)
			* @return (void)
			*/
			virtual void InputMouseMoved(const float dx, const float dy) noexcept;

			/** @brief Called when a key was pressed
			  * @param key Pressed key (char)
			  * @return (void)
			  */
			virtual void InputKeyPressed(char key) noexcept;

			/** @brief Called when a key was released
			  * @param key Released key (char)
			  * @return (void)
			  */
			virtual void InputKeyReleased(char key) noexcept;

			/** @brief Sets object's index in shader const buffer
			  * @param Index Object's index in shader const buffer(const uint64)
			  * @return (void)
			  */
			void SetConstBufferIndex(const uint64 Index) noexcept;

			/** @brief Set number not updated const buffers (same as frames)
			  * @param NumDirtyConstBuffers Numbfer not updated const buffers/frames (const uint8)
			  * @return (void)
			  */
			void SetNumDirtyConstBuffers(const uint8 NumDirtyConstBuffers) noexcept;

			/** @brief Sets current material
			  * @param Material (FMaterialData *)
			  * @return (void)
			  */
			void SetMaterial(const FMaterialData* Material);

			/** @brief Sets absolute world position
			  * @param Position An absolute world position (const XMFLOAT3 &)
			  * @return (void)
			  */
			virtual void SetPosition(const XMFLOAT3& Position) noexcept;
			
			/** @brief Sets absolute world position
			  * @param X Absolute x-axis coordinate (const float)
			  * @param Y Absolute y-axis coordinate (const float)
			  * @param Z Absolute z-axis coordinate (const float)
			  * @return (void)
			  */
			void SetPosition(const float X, const float Y, const float Z) noexcept;

			/** @brief Sets absolute world rotation
			  * @param Rotation An absolute world rotation(const XMFLOAT3 &)
			  * @return (void)
			  */
			virtual void SetRotation(const XMFLOAT3& Rotation) noexcept;

			/** @brief Sets absolute world rotation
			* @param X Absolute rotation around x-axis (const float)
			* @param Y Absolute rotation around y-axis (const float)
			* @param Z Absolute rotation around z-axis (const float)
			* @return (void)
			*/
			void SetRotation(const float X, const float Y, const float Z) noexcept;

			/** @brief Sets absolute world scale
			  * @param Scale An absolute world scale(const XMFLOAT3 &)
			  * @return (void)
			  */
			virtual void SetScale(const XMFLOAT3& Scale) noexcept;
		
			/** @brief Sets absolute world scale
			* @param X Absolute x-axis scale (const float)
			* @param Y Absolute y-axis scale (const float)
			* @param Z Absolute z-axis scale (const float)
			* @return (void)
			*/
			void SetScale(const float X, const float Y, const float Z) noexcept;

			/** @brief Set default shader color
			  * @param Color (XMFLOAT4)
			  * @return (void)
			  */
			void SetColor(XMFLOAT4 Color) noexcept;

			/** @brief Set flag object is supposed to be rendered to the screen
			  * @param Renderable (const bool)
			  * @return (void)
			  */
			void SetIsRenderable(const bool Renderable) noexcept;

			/** @brief Set water factor
			  * @param WaterFactor (int)
			  * @return (void)
			  */
			void SetWaterFactor(int WaterFactor) noexcept;

			/** @brief Sets transform matrix for object's tex coordinates
			  * @param TextureTransform TexCoordinates transform matrix (const XMFLOAT4x4 &)
			  * @return (void)
			  */
			void SetTextureTransform(const XMFLOAT4X4& TextureTransform) noexcept;

			/** @brief Set flag update method is calling
			  * @return (void)
			  */
			void SetIsUpdating(const bool IsUpdating) noexcept;

			/** @brief Sets visibility
			  * @param IsVisible Visibility (const bool)
			  * @return (void)
			  */
			void SetIsVisible(const bool IsVisible) noexcept;

			/** @brief Returns object's world matrix for rendering
			  * @return World matrix (DirectX::XMMATRIX)
			  */
			const XMMATRIX& GetWorldMatrix() const noexcept;

			/** @brief Returns object's world absolute position
			  * @return World absolution position (DirectX::XMFLOAT3)
			  */
			XMFLOAT3 GetWorldPosition() const noexcept;

			/** @brief Returns index in const buffer
			  * @return Get index in const buffer (default::uint16)
			  */
			uint64 GetConstBufferIndex() const;

			/** @brief Returns number of still not updated const buffer (same as frames)
			  * @return (default::uint8)
			  */
			uint8 GetNumDirtyConstBuffers() const noexcept;

			/** @brief Returns name of static mesh data. If it's empty, method throws exception
			  * @return Name of mesh data(const std::string&)
			  */
			const std::string& GetMeshName() const;

			/** @brief Returns pointer to material data or nullptr 
			  * @return (WoodenEngine::FMaterialData*)
			  */
			const FMaterialData* GetMaterial() const noexcept;

			/** @brief Returns texture's coordinates transform matrix
			  * @return texture's coordinates transform matrix (const DirectX::XMFLOAT4X4&)
			  */
			const XMFLOAT4X4& GetTextureTransform() const noexcept;

			/** @brief Returns flag if object is supposed to be rendered to the screen
			  * @return true - renderable/false - not(bool)
			  */
			bool IsRenderable() const noexcept;

			/** @brief Returns flag if update method is calling
			  * @return (bool)
			  */
			bool IsUpdating() const noexcept;

			/** @brief Returns flag if enaling input tracking
			  * @return (bool)
			  */
			bool IsEnabledInputEvents() const noexcept;

			/** @brief Returns default shader color parameter
			  * @return (const DirectX::XMFLOAT4&)
			  */
			const XMFLOAT4& GetColor() const noexcept;

			/** @brief Returns water factor
			  * @return water factor (int)
			  */
			int GetWaterFactor() const noexcept;

			/** @brief Returns object's life time 
			  * @return Life time (float)
			  */
			float GetLifeTime() const noexcept;

			/** @brief Returns visibility
			  * @return visibility (bool)
			  */
			bool IsVisible() const noexcept;
		protected:
			// Flags if objects is hidden/visible or not
			bool bIsVisible = true;

			// Flags if update method is executing or not
			bool bIsUpdating = true;

			// Flags is object supposed to be rendered
			bool bIsRenderable = false;

			// Enabling tracking input events (such as mouse moving)
			bool bIsEnabledInputEvents = false;
		private:	
			// Absolute matrix of transformation in the world. Used for rendering
			XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();
			
			// Vector with an absolute position in the world
			XMFLOAT3 Position;

			// Vector with an absolute rotation in the world
			XMFLOAT3 Rotation;

			// Default shader color parameter
			XMFLOAT4 Color;

			// Vector with an absolute scale in the world (default: 1.0f, 1.0f, 1.0f)
			XMFLOAT3 Scale = MathHelper::Identity3();
			
			// Texture coordinates transform matrix
			XMFLOAT4X4 TextureTransform = MathHelper::Identity4x4();

			// Life time in seconds
			float LifeTime = 0;

			// Current mesh data name
			std::string MeshName;
	
			// Index of object in const buffer
			uint64 iConstBuffer = UINT64_MAX;

			// Current material
			const FMaterialData* Material = nullptr;

			// Number not updated const buffers
			uint8 NumDirtyConstBuffers = NMR_SWAP_BUFFERS;

			int WaterFactor = 1;

			/** @brief Recomputes the world matrix considering Position, Rotation and Scale in the world
			  * @return (void)
			  */
			void UpdateWorldMatrix() noexcept;
	};
}