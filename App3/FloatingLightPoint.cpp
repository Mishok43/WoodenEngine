#include <algorithm>
#include <iostream>

#include "FloatingLightPoint.h"


namespace WoodenEngine
{

	WFloatingLightPoint::WFloatingLightPoint(
		XMFLOAT3 Strength,
		XMFLOAT3 Direction,
		XMFLOAT3 Position,
		float FalloffStart,
		float FalloffEnd) :
		WLightPoint(
			Strength, Direction, Position,
			FalloffStart, FalloffEnd)
	{
		bIsUpdating = true;
	}

	void WFloatingLightPoint::Update(float Delta)
	{
		WLightPoint::Update(Delta);

		if (!bIsFloating)
		{
			return;
		}
	
		CurFloatingTime = min(CurFloatingTime+Delta, FloatingTime);

		float Factor = CurFloatingTime / FloatingTime;

		auto UpdatedPositionPacked = (1 - Factor)*PointStart + Factor * PointEnd;

		XMFLOAT3 UpdatedPosition;
		XMStoreFloat3(&UpdatedPosition, std::move(UpdatedPositionPacked));
		SetPosition(UpdatedPosition);

		SetNumDirtyConstBuffers(NMR_SWAP_BUFFERS);

		if (Factor == 1.0f)
		{
			CurFloatingTime = 0;

			if (bRepeatingFloating)
			{
				std::swap(PointStart, PointEnd);
			}
			else
			{
				bIsFloating = false;
			}
		}
	}

	void WFloatingLightPoint::SetTrajectory(
		XMFLOAT3 PointStart,
		XMFLOAT3 PointEnd,
		float FloatingTime,
		bool Repeating) noexcept
	{
		this->PointStart = XMLoadFloat3(&PointStart);
		this->PointEnd = XMLoadFloat3(&PointEnd);
		this->FloatingTime = FloatingTime;
		this->bRepeatingFloating = Repeating;

		bIsFloating = true;
		CurFloatingTime = 0;
	}

}

