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
		XMFLOAT4 CameraPosition = {};
		CameraPosition.x = XMScalarCos(ZAngle)*XMScalarSin(XAngle)*ViewRadius;
		CameraPosition.y = XMScalarSin(ZAngle)*XMScalarSin(XAngle)*ViewRadius;
		CameraPosition.z = XMScalarCos(XAngle)*ViewRadius;
		CameraPosition.w = 1.0f;

		XMFLOAT4 CameraFocus = { 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 UpDirecation = { 0.0f, 1.0f, 0.0f, 0.0f };

		ViewMatrix = XMMatrixLookAtLH(XMLoadFloat4(&CameraPosition), XMLoadFloat4(&CameraFocus), XMLoadFloat4(&UpDirecation));

		SetPosition(CameraPosition.x, CameraPosition.y, CameraPosition.z);
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