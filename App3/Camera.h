#pragma once

#include "Object.h"

namespace WoodenEngine
{
    /*!
	 * \class WCamera
	 *
	 * \brief Unrenderable camera class responsible for moving camera and computing view matrix
	 *
	 * \author devmi
	 * \date May 2018
	 */
	class WCamera : public WObject
	{
	public:
		WCamera(
			const float WindowWidth,
			const float WindowHeight,
			const float DefaultXAngle=XM_PIDIV4,
			const float DefaultYAngle= 2.55f,
			const float DefaultRadius=25.0f);

		virtual void Update(float Delta) override;

		/** @brief Moves camera view
		  * @param dx (const float)
		  * @param dy (const float)
		  * @return (void)
		  */
		virtual void InputMouseMoved(const float dx, const float dy) noexcept override;

		/** @brief Returns view matrix for rendering
		  * @return (const DirectX::XMMATRIX&)
		  */
		const XMMATRIX& GetViewMatrix() const noexcept;
	private:
		// Crunch
		float WindowWidth;
		float WindowHeight;

		float XAngle = XM_PIDIV4;
		
		float YAngle = 2.55f;
		
		float ViewRadius = 25.0f;

		// View matrix for rendering
		XMMATRIX ViewMatrix;
		
		/** @brief Recompute view matrix based on XAngle, ZAngle, ViewRadius for rendering
		  * @return (void)
		  */
		void UpdateViewMatrix() noexcept;
	};
}