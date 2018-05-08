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
			const float DefaultXAngle=DirectX::XM_PI/3.0f,
			const float DefaultZAngle=DirectX::XM_PIDIV2,
			const float DefaultRadius=10.0);

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

		float XAngle = DirectX::XM_PI / 3.0f;
		
		float ZAngle = DirectX::XM_PIDIV2;
		
		float ViewRadius = 10.0f;

		// View matrix for rendering
		XMMATRIX ViewMatrix;
		
		/** @brief Recompute view matrix based on XAngle, ZAngle, ViewRadius for rendering
		  * @return (void)
		  */
		void UpdateViewMatrix() noexcept;
	};
}