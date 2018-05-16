#include "LightPoint.h"


namespace WoodenEngine
{
	WLightPoint::WLightPoint(
		XMFLOAT3 Strength,
		XMFLOAT3 Position,
		float FalloffStart,
		float FalloffEnd):
		WLight(WLight::ELightType::Point),
		Strength(Strength),
		FalloffStart(FalloffStart),
		FalloffEnd(FalloffEnd)
	{
		SetPosition(Position);
	}

	void WLightPoint::SetStrength(XMFLOAT3 Strength)
	{
		this->Strength = Strength;
	}

	void WLightPoint::SetFalloff(float FalloffStart, float FalloffEnd)
	{
		this->FalloffStart = FalloffStart;
		this->FalloffEnd = FalloffEnd;
	}

	SLightData WLightPoint::GetShaderData() const noexcept
	{
		SLightData LightData;
		LightData.Position = GetWorldPosition();
		LightData.FalloffStart = FalloffStart;
		LightData.FalloffEnd = FalloffEnd;
		LightData.Strength = Strength;

		return LightData;
	}
}
