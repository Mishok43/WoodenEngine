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

		/** @brief Moves camera forward by Speed regarding the camera's forward vector
		  * @param Speed Displacement (it can be negative for back walking) (float)
		  * @return (void)
		  */
		void WalkForward(float Speed);

		/** @brief Moves camera right by Speed regarding the camera's right vector
		  * @param Speed Displacement (it can be negative for left walking) (float)
		  * @return (void)
		  */
		void WalkRight(float Speed);

		/** @brief Lifts up the camera, reducing Pitch angle, regarding the camera's right vector
		  * @param Angle Pitch angle (float)
		  * @return (void)
		  */
		void LookUp(float Angle);

		/** @brief Turns right the camera regarding the world's up vector {0, 1, 0}
		  * @param Angle (float)
		  * @return (void)
		  */
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

		// If true camera moves forward per every update call
		bool bMoveForward;

		// If true camera moves back per every update call
		bool bMoveBack;

		// If true camera moves right per every update call
		bool bMoveRight;

		// If true camera moves left per every update call
		bool bMoveLeft;

		/** @brief Recomputes view matrix based on XAngle, ZAngle, ViewRadius for rendering
		  * @return (void)
		  */
		void UpdateViewMatrix() noexcept;
	};
}