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

	void BObject::UpdateWorldMatrix() 
	{
		WorldMatrix = 
			DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&Scale))*
			DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation))*
			DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&Position));
	}
}