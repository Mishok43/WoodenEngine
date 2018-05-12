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
			const float WindowHeight);

		virtual void Update(float Delta) override;

		void WalkForward(float Speed);

		void WalkRight(float Speed);

		void LookUp(float Angle);

		void LookRight(float Angle);

		/** @brief Moves camera view
		  * @param dx (const float)
		  * @param dy (const float)
		  * @return (void)
		  */
		virtual void InputMouseMoved(const float dx, const float dy) noexcept override;

		/** @brief Start moving forward/right
		  * @param key (char)
		  * @return (void)
		  */
		virtual void InputKeyPressed(char key) noexcept override;

		/** @brief Stop moving forward/right
		  * @param key (char)
		  * @return (void)
		  */
		virtual void InputKeyReleased(char key) noexcept override;

		/** @brief Returns view matrix for rendering
		  * @return (const DirectX::XMMATRIX&)
		  */
		const XMMATRIX& GetViewMatrix() const noexcept;
	private:
		// Crunch
		float WindowWidth;
		float WindowHeight;

		XMFLOAT3 Up = { 0.0f, 1.0f, 0.0f };
		XMFLOAT3 Right = { 1.0f, 0.0f, 0.0f };
		XMFLOAT3 Forward = { 0.0f, 0.0f, 1.0f };

		// View matrix for rendering
		XMMATRIX View;

		bool bMoveForward;
		bool bMoveBack;
		bool bMoveRight;
		bool bMoveLeft;

		float LookUpAngle = 0;
		float LookRightAngle = 0;


		/** @brief Recompute view matrix based on XAngle, ZAngle, ViewRadius for rendering
		  * @return (void)
		  */
		void UpdateViewMatrix() noexcept;
	};
}