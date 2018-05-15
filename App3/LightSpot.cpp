#include "LightSpot.h"

namespace WoodenEngine
{
	WLightSpot::WLightSpot(
		XMFLOAT3 Strength,
		XMFLOAT3 Direction,
		XMFLOAT3 Position,
		float FalloffStart,
		float FalloffEnd,
		float SpotPower) :
		WLight(WLight::ELightType::Spot),
		Strength(Strength),
		Direction(Direction),
		FalloffStart(FalloffStart),
		FalloffEnd(FalloffEnd),
		SpotPower(SpotPower)
	{
		SetPosition(Position);
	}

	void WLightSpot::SetStrength(XMFLOAT3 Strength)
	{
		this->Strength = Strength;
	}

	void WLightSpot::SetDirection(XMFLOAT3 Direction)
	{
		this->Direction = Direction;
	}

	void WLightSpot::SetFalloff(float FalloffStart, float FalloffEnd)
	{
		this->FalloffStart = FalloffStart;
		this->FalloffEnd = FalloffEnd;
	}

	void WLightSpot::SetSpotPower(float SpotPower)
	{
		this->SpotPower = SpotPower;
	}

	SLightData WLightSpot::GetShaderData() const noexcept
	{
		SLightData LightData;
		LightData.Direction = Direction;
		LightData.Position = GetWorldPosition();
		LightData.FalloffStart = FalloffStart;
		LightData.FalloffEnd = FalloffEnd;
		LightData.Strength = Strength;
		LightData.SpotPower = SpotPower;

		return LightData;
	}
}
