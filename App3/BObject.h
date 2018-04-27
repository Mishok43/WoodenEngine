#pragma once
#include <iostream>
#include <string>

#include "ShaderStructures.h"
#include "MathHelper.h"

namespace DirectXEngine
{
	/*!
	 * \class BObject
	 *
	 * \brief Basic visible object of the engine. It contains essential data for rendering.
	 *
	 * \author devmi
	 * \date April 2018
	 */
	class BObject
	{
		public:
			BObject() = default;
			~BObject();

			BObject(const BObject& Obj) = delete;
			BObject(BObject&& Obj) = delete;
			BObject& operator=(const BObject& Obj) = delete;
			
			/** @brief 
			  * @param Position An absolute world position (const XMFLOAT3 &)
			  * @param Rotation An absolute world rotation (const XMFLOAT3 &)
			  * @param Scale An absolute world scale (const XMFLOAT3 &)
			  * @return ()
			  */
			BObject(const XMFLOAT3& Position, const XMFLOAT3& Rotation, const XMFLOAT3& Scale);
			
			/** @brief Set absolute world position
			  * @param Position An absolute world position (const XMFLOAT3 &)
			  * @return (void)
			  */
			virtual void SetPosition(const XMFLOAT3& Position);
			
			/** @brief Set absolute world rotation
			  * @param Rotation An absolute world rotation(const XMFLOAT3 &)
			  * @return (void)
			  */
			virtual void SetRotation(const XMFLOAT3& Rotation);


			/** @brief Set absolute world scale
			  * @param Scale An absolute world scale(const XMFLOAT3 &)
			  * @return (void)
			  */
			virtual void SetScale(const XMFLOAT3& Scale);
		
		private:	
			/// Absolute matrix of transformation in the world. Used for rendering
			XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();
			/// Vector with an absolute position in the world
			XMFLOAT3 Position;

			/// Vector with an absolute rotation in the world
			XMFLOAT3 Rotation;

			/// Vector with an absolute scale in the world (default: 1.0f, 1.0f, 1.0f)
			XMFLOAT3 Scale = MathHelper::Identity3();
			
			/// A life time in seconds
			float LifeTime = 0;

			/// A current mesh data name
			std::string MeshName;
	
			/** @brief A method recomputes the world matrix considering Position, Rotation and Scale in the world
			  * @return (void)
			  */
			virtual void UpdateWorldMatrix() noexcept;
	};
}