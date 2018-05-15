#pragma once

#include "Light.h"

namespace WoodenEngine
{
	/*!
	 * \class WLightSpot
	 *
	 * \brief Spot light source which is not renderable by default
	 *
	 * \author devmi
	 * \date May 2018
	 */
	class WLightSpot : public WLight
	{
	public:
		WLightSpot(
			XMFLOAT3 Strength,
			XMFLOAT3 Direction,
			XMFLOAT3 Position,
			float FalloffStart,
			float FalloffEnd,
			float SpotPower
		);

		WLightSpot() = default;

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

		/** @brief Sets spot power
		  * @param SpotPower (float)
		  * @return (void)
		  */
		void SetSpotPower(float SpotPower);

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

		// Spot power determines the angle of the spot indirectly
		float SpotPower = 1.0f;
	};
}