#include "Light.h"


namespace WoodenEngine
{
	WLight::WLight(ELightType LightType):
		Type(LightType)
	{
		bIsRenderable = false;
		bIsUpdating = false;
	}

	WLight::ELightType WLight::GetType() const noexcept
	{
		return Type;
	}
}




