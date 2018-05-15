#pragma once

#include "Light.h"



namespace WoodenEngine
{
	/*!
	 * \class WLightDirection
	 *
	 * \brief Direction light source which is not renderable by default
	 *
	 * \author devmi
	 * \date May 2018
	 */
	class WLightDirectional : public WLight
	{
	public:
		WLightDirectional(
			XMFLOAT3 Strength,
			XMFLOAT3 Direction);

		/** @brief Sets light's color/strength
		  * @param Strength Color(XMFLOAT3)
		  * @return (void)
		  */
		void SetStrength(XMFLOAT3 Strength);

		/** @brief Sets direction of light raycing
		  * @param Direction Normalized direction of light raycing (XMFLOAT3)
		  * @return (void)
		  */
		void SetDirection(XMFLOAT3 Direction);

		virtual SLightData GetShaderData() const noexcept override;
	private:
		// Light's color/strength
		XMFLOAT3 Strength;

		// Direction of light raycing
		XMFLOAT3 Direction;
	};
}

