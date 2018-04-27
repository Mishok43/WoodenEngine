#pragma once
#include <DirectXMath.h>

namespace DirectXEngine
{
	class MathHelper
	{
		public:
		
			/** @brief A method returns identity not packed matrix 4x4 float
			  * @return Identity matrix 4x4 float (DirectX::XMFLOAT4X4)
			  */
			static DirectX::XMFLOAT4X4 Identity4x4() noexcept
			{
				static DirectX::XMFLOAT4X4 I(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				);
				
				return I;
			}

			static DirectX::XMFLOAT3 Identity3() noexcept
			{
				static DirectX::XMFLOAT3 I(1.0f, 1.0f, 1.0f);

				return I;
			}
	};
}