#pragma once
#include <iostream>
#include <string>

#include "ShaderStructures.h"
#include "MathHelper.h"

namespace WoodenEngine
{
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
			WObject();
			~WObject();

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
			
			void Update(float Delta);

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
		private:	
			// Absolute matrix of transformation in the world. Used for rendering
			XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();
			
			// Vector with an absolute position in the world
			XMFLOAT3 Position;

			// Vector with an absolute rotation in the world
			XMFLOAT3 Rotation;

			// Vector with an absolute scale in the world (default: 1.0f, 1.0f, 1.0f)
			XMFLOAT3 Scale = MathHelper::Identity3();
			
			// Life time in seconds
			float LifeTime = 0;

			// Current mesh data name
			std::string MeshName;
	
			/** @brief Recomputes the world matrix considering Position, Rotation and Scale in the world
			  * @return (void)
			  */
			virtual void UpdateWorldMatrix() noexcept;
	};
}