#include "LightDirectional.h"

namespace WoodenEngine
{
	WLightDirectional::WLightDirectional(
		XMFLOAT3 Strength,
		XMFLOAT3 Direction) :
		WLight(WLight::ELightType::Directional),
		Strength(Strength),
		Direction(Direction)
	{
	}

	void WLightDirectional::SetStrength(XMFLOAT3 Strength)
	{
		this->Strength = Strength;
	}

	void WLightDirectional::SetDirection(XMFLOAT3 Direction)
	{
		this->Direction = Direction;
	}

	SLightData WLightDirectional::GetShaderData() const noexcept
	{
		SLightData LightData;

		LightData.Direction = Direction;
		LightData.Strength = Strength;
		
		return LightData;
	}
}

