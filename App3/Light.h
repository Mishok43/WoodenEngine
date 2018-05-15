#pragma once

#include "Object.h"
#include "ShaderStructures.h"

namespace WoodenEngine
{

	using namespace DirectX;

	/*!
	 * \class WLight
	 *
	 * \brief Abstract general class for lights sources. Is not renderable by default
	 *
	 * \author devmi
	 * \date May 2018
	 */
	class WLight : public WObject
	{
	public:
		enum class ELightType : uint8
		{
			Directional = 0,
			Point,
			Spot,
			Count
		};

		WLight(ELightType LightType);


		/** @brief Returns general lights shader data
		  * @return (WoodenEngine::SLightData)
		  */
		virtual SLightData GetShaderData() const noexcept = 0;

		/** @brief Returns light source type
		  * @return (WoodenEngine::WLight::ELightType)
		  */
		ELightType GetType() const noexcept;
	private:
		// Light source type
		ELightType Type;

	};

}

