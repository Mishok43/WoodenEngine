#include "Camera.h"

namespace WoodenEngine
{
	WCamera::WCamera(
		const float WindowWidth,
		const float WindowHeight,
		const float DefaultXAngle/* =DirectX::XM_PI/3.0f */,
		const float DefaultZAngle/* =DirectX::XM_PIDIV2 */,
		const float DefaultRadius/* =10.0 */)
		:
		WindowWidth(WindowWidth),
		WindowHeight(WindowHeight),
		XAngle(DefaultXAngle),
		ZAngle(DefaultZAngle),
		ViewRadius(DefaultRadius)
	{
		bIsRenderable = false;
		bIsEnabledInputEvents = true;

		UpdateViewMatrix();
	}

	void WCamera::Update(float Delta)
	{
		WObject::Update(Delta);

		// To do camera logic
	}


	void WCamera::UpdateViewMatrix() noexcept
	{
		XMFLOAT4 cameraPosition = {};
		cameraPosition.x = XMScalarCos(ZAngle)*XMScalarSin(XAngle)*ViewRadius;
		cameraPosition.y = XMScalarSin(ZAngle)*XMScalarSin(XAngle)*ViewRadius;
		cameraPosition.z = XMScalarCos(XAngle)*ViewRadius;
		cameraPosition.w = 1.0f;

		XMFLOAT4 cameraFocus = { 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 upDirecation = { 0.0f, 1.0f, 0.0f, 0.0f };

		ViewMatrix = XMMatrixLookAtLH(XMLoadFloat4(&cameraPosition), XMLoadFloat4(&cameraFocus), XMLoadFloat4(&upDirecation));
	}

	void WCamera::InputMouseMoved(const float dx, const float dy) noexcept
	{
		WObject::InputMouseMoved(dx, dy);

		const auto dPhi = dx / WindowWidth*DirectX::XM_PI;
		const auto dTheta = dy / WindowHeight*DirectX::XM_PIDIV2;

		ZAngle += dPhi;
		XAngle += dTheta;

		UpdateViewMatrix();
	}

	const XMMATRIX& WCamera::GetViewMatrix() const noexcept
	{
		return ViewMatrix;
	}

}