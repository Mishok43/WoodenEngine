#pragma once

#include "Light.h"

namespace WoodenEngine
{
	using namespace DirectX;

	/*!
	 * \class WLightPoint
	 *
	 * \brief Point light source which is not renderable by default
	 *
	 * \author devmi
	 * \date May 2018
	 */
	class WLightPoint : public WLight
	{
	public:
		WLightPoint(
			XMFLOAT3 Strength,
			XMFLOAT3 Direction,
			XMFLOAT3 Position,
			float FalloffStart,
			float FalloffEnd
		);

		WLightPoint() = default;

		/** @brief Sets light's color/strength
		  * @param Strength Color (XMFLOAT3)
		  * @return (void)
		  */
		void SetStrength(XMFLOAT3 Strength);

		/** @brief Sets light's direction
		  * @param Direction (XMFLOAT3)
		  * @return (void)
		  */
		void SetDirection(XMFLOAT3 Direction);

		/** @brief Sets attenuation
		  * @param Start Distance from source (it) for starting attenuation (float)
		  * @param End Distance from source (it) for ending attenuation (float)
		  * @return (void)
		  */
		void SetFalloff(float FalloffStart, float FalloffEnd);

		virtual SLightData GetShaderData() const noexcept override;
	private:
		// Light color/strength
		XMFLOAT3 Strength;

		// Direction of light raycing
		XMFLOAT3 Direction;

		// Distance from source (it) for starting attenuation
		float FalloffStart = 0.0f;

		// Distance from source (it) for ending attenuation 
		float FalloffEnd = 0.0f;

	};
}