#include "Camera.h"

namespace WoodenEngine
{
	WCamera::WCamera(
		const float WindowWidth,
		const float WindowHeight)
		:
		WindowWidth(WindowWidth),
		WindowHeight(WindowHeight)
	{
		SetPosition(-6, 30, 10);
		LookRight(XM_PI);
		LookUp(XM_PIDIV4);

		bIsRenderable = false;
		bIsEnabledInputEvents = true;

		UpdateViewTransform();
	}

	void WCamera::Update(float Delta)
	{
		WObject::Update(Delta);

		auto UpdateView = false;

		auto WalkSpeed = 10.0f / 60.0f;
		if (bMoveForward)
		{
			WalkForward(WalkSpeed);
			UpdateView = true;
		}
		if (bMoveBack)
		{
			WalkForward(-WalkSpeed);
			UpdateView = true;
		}
		if (bMoveRight)
		{
			WalkRight(WalkSpeed);
			UpdateView = true;
		}
		if (bMoveLeft)
		{
			WalkRight(-WalkSpeed);
			UpdateView = true;
		}

		if (UpdateView)
		{
			UpdateViewTransform();
		}
	}

	void WCamera::UpdateViewTransform() noexcept
	{
		XMVECTOR F = XMLoadFloat3(&Forward);
		XMVECTOR U = XMLoadFloat3(&Up);
		XMVECTOR R = XMLoadFloat3(&Right);
		XMVECTOR P = XMLoadFloat3(&GetWorldPosition());

		XMFLOAT4X4 TempView;

		// normalize them because of possible approximation errors
		F = XMVector3Normalize(F);
		U = XMVector3Normalize(XMVector3Cross(F, R));
		R = XMVector3Cross(U, F);

		TempView(0, 0) = Right.x;
		TempView(1, 0) = Right.y;
		TempView(2, 0) = Right.z;
		TempView(3, 0) = -1 * XMVectorGetX(XMVector3Dot(R, P));

		TempView(0, 1) = Up.x;
		TempView(1, 1) = Up.y;
		TempView(2, 1) = Up.z;
		TempView(3, 1) = -1 * XMVectorGetX(XMVector3Dot(U, P));

		TempView(0, 2) = Forward.x;
		TempView(1, 2) = Forward.y;
		TempView(2, 2) = Forward.z;
		TempView(3, 2) = -1 * XMVectorGetX(XMVector3Dot(F, P));

		TempView(0, 3) = 0;
		TempView(1, 3) = 0;
		TempView(2, 3) = 0;
		TempView(3, 3) = 1;

		View = XMLoadFloat4x4(&TempView);
	}

	void WCamera::WalkForward(float Speed)
	{
		auto SpeedVector = XMVectorReplicate(Speed);
		auto ForwardVector = XMLoadFloat3(&Forward);
		auto PositionVector = XMLoadFloat3(&GetWorldPosition());

		auto UpdatedPosition = XMFLOAT3{};
		XMStoreFloat3(&UpdatedPosition,XMVectorMultiplyAdd(SpeedVector, ForwardVector, PositionVector));

		SetPosition(UpdatedPosition);
	}

	void WCamera::WalkRight(float Speed)
	{
		auto SpeedVector = XMVectorReplicate(Speed);
		auto RightVector = XMLoadFloat3(&Right);
		auto PositionVector = XMLoadFloat3(&GetWorldPosition());

		auto UpdatedPosition = XMFLOAT3{};
		XMStoreFloat3(&UpdatedPosition, XMVectorMultiplyAdd(SpeedVector, RightVector, PositionVector));

		SetPosition(UpdatedPosition);
	}

	void WCamera::LookUp(float Angle)
	{
		auto PitchRotation = XMMatrixRotationAxis(XMLoadFloat3(&Right), Angle);

		XMStoreFloat3(&Forward, XMVector3TransformNormal(XMLoadFloat3(&Forward), PitchRotation));
		XMStoreFloat3(&Up, XMVector3TransformNormal(XMLoadFloat3(&Up), PitchRotation));
	}

	void WCamera::LookRight(float Angle)
	{
		auto YawRotation = XMMatrixRotationY(Angle);

		XMStoreFloat3(&Up, XMVector3TransformNormal(XMLoadFloat3(&Up), YawRotation));
		XMStoreFloat3(&Forward, XMVector3TransformNormal(XMLoadFloat3(&Forward), YawRotation));
		XMStoreFloat3(&Right, XMVector3TransformNormal(XMLoadFloat3(&Right), YawRotation));
	}

	void WCamera::InputMouseMoved(const float dx, const float dy) noexcept
	{
		WObject::InputMouseMoved(dx, dy);

		const auto dPhi = dx / WindowWidth*DirectX::XM_PI;
		const auto dTheta = dy / WindowHeight*DirectX::XM_PIDIV2;

		LookUp(dTheta);
		UpdateViewTransform();

		LookRight(dPhi);
		
		UpdateViewTransform();
	}

	void WCamera::InputKeyPressed(char key) noexcept
	{
		WObject::InputKeyPressed(key);

		bMoveForward = (key == 'w') ? true : bMoveForward;
		bMoveBack = (key == 's') ? true : bMoveBack;
		bMoveRight = (key == 'd') ? true : bMoveRight;
		bMoveLeft = (key == 'a') ? true : bMoveLeft;
	}

	void WCamera::InputKeyReleased(char key) noexcept
	{

		WObject::InputKeyReleased(key);

		bMoveForward = (key == 'w') ? false : bMoveForward;
		bMoveBack = (key == 's') ? false : bMoveBack;
		bMoveRight = (key == 'd') ? false : bMoveRight;
		bMoveLeft = (key == 'a') ? false : bMoveLeft;
	}

	const XMMATRIX& WCamera::GetViewMatrix() const noexcept
	{
		return View;
	}

}