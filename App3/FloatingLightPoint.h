#pragma once
#include "LightPoint.h"


namespace WoodenEngine
{

	class WFloatingLightPoint : public WLightPoint
	{
	public:
		WFloatingLightPoint(
			XMFLOAT3 Strength,
			XMFLOAT3 Direction,
			XMFLOAT3 Position,
			float FalloffStart,
			float FalloffEnd
		);

		WFloatingLightPoint() = default;

		/** @brief Updates the its position
		  * @param Delta (float)
		  * @return (void)
		  */
		virtual void Update(float Delta) override;

		/** @brief Sets trajectory of floating
		* @param PointStart Start position of floating (XMFLOAT3)
		* @param PointEnd End position of floating (XMFLOAT3)
		* @param FloatingTime Time of floating (float)
		* @param Repeating (bool)
		* @return (void)
		*/
		void SetTrajectory(
			XMFLOAT3 PointStart,
			XMFLOAT3 PointEnd,
			float FloatingTime,
			bool Repeating) noexcept;

	private:
		
		// Point start
		XMVECTOR PointStart;

		// Point end
		XMVECTOR PointEnd;

		float FloatingTime;

		float CurFloatingTime;

		bool bRepeatingFloating;

		bool bIsFloating;
	};


}