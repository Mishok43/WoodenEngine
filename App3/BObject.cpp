#include "BObject.h"

namespace DirectXEngine
{
	BObject::BObject()
	{
	}

	BObject::BObject(const XMFLOAT3& Position, const XMFLOAT3& Rotation, const XMFLOAT3& Scale)
		:Position(Position), Rotation(Rotation), Scale(Scale)
	{
		UpdateWorldMatrix();
	}


	BObject::~BObject()
	{
	
	}

	void BObject::SetPosition(const XMFLOAT3& Position)
	{
		this->Position = Position;
		UpdateWorldMatrix();
	}

	void BObject::SetRotation(const XMFLOAT3& Rotation)
	{
		this->Rotation = Rotation;
		UpdateWorldMatrix();
	}

	void BObject::SetScale(const XMFLOAT3& Scale)
	{
		this->Scale = Scale;
		UpdateWorldMatrix();
	}

	void BObject::UpdateWorldMatrix() 
	{
		WorldMatrix = 
			DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&Scale))*
			DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation))*
			DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&Position));
	}
}