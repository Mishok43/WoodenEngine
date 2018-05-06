#include "Object.h"

namespace WoodenEngine
{
	WObject::WObject(
		const std::string& MeshName,
		const XMFLOAT3& Position, 
		const XMFLOAT3& Rotation, 
		const XMFLOAT3& Scale) : 
		MeshName(MeshName), 
		Position(Position), 
		Rotation(Rotation), 
		Scale(Scale),
		bIsRenderable(true)
	{
		UpdateWorldMatrix();
	}


	void WObject::Update(float Delta)
	{

	}

	void WObject::SetPosition(const XMFLOAT3& Position) noexcept
	{
		this->Position = Position;
		UpdateWorldMatrix();
	}

	void WObject::SetPosition(const float X, const float Y, const float Z) noexcept
	{
		SetPosition({ X, Y, Z });
	}

	void WObject::SetRotation(const XMFLOAT3& Rotation) noexcept
	{
		this->Rotation = Rotation;
		UpdateWorldMatrix();
	}

	void WObject::SetRotation(const float X, const float Y, const float Z) noexcept
	{
		SetRotation({ X, Y, Z });
	}

	void WObject::SetScale(const XMFLOAT3& Scale) noexcept
	{
		this->Scale = Scale;
		UpdateWorldMatrix();
	}

	void WObject::SetScale(const float X, const float Y, const float Z) noexcept
	{
		SetScale({ X, Y, Z });
	}

	void WObject::SetColor(XMFLOAT4 Color) noexcept
	{
		this->Color = Color;
	}

	void WObject::UpdateWorldMatrix() noexcept 
	{
		WorldMatrix = 
			DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&Scale))*
			DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation))*
			DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&Position));
	}

	void WObject::InputMouseMoved(const float dx, const float dy) noexcept
	{
		
	}

	void WObject::SetEnableInputEvents(const bool EnableInput) noexcept
	{
		bIsEnabledInputEvents = EnableInput;
	}

	void WObject::SetNumDirtyConstBuffers(const uint8 NumDirtyConstBuffers) noexcept
	{
		this->NumDirtyConstBuffers = NumDirtyConstBuffers;
	}

	void WObject::SetIsUpdating(const bool IsUpdating) noexcept
	{
		bIsUpdating = IsUpdating;
	}

	void WObject::SetIsRenderable(const bool Renderable) noexcept
	{
		bIsRenderable = Renderable;
	}

	void WObject::SetConstBufferIndex(const uint64 Index) noexcept
	{
		ConstBufferIndex = Index;
	}

	uint64 WObject::GetConstBufferIndex() const
	{
		assert(ConstBufferIndex != UINT64_MAX); // ConstBufferIndex must be set
		return ConstBufferIndex;
	}

	uint8 WObject::GetNumDirtyConstBuffers() const noexcept
	{
		return NumDirtyConstBuffers;
	}

	const std::string& WObject::GetMeshName() const
	{
		assert(MeshName.size() != 0);
		return MeshName;
	}

	const XMMATRIX& WObject::GetWorldMatrix() const noexcept
	{
		return WorldMatrix;
	}

	bool WObject::IsUpdating() const noexcept
	{
		return bIsUpdating;
	}

	bool WObject::IsRenderable() const noexcept
	{
		return bIsRenderable;
	}

	bool WObject::IsEnabledInputEvents() const noexcept
	{
		return bIsEnabledInputEvents;
	}

	const XMFLOAT4& WObject::GetColor() const noexcept
	{
		return Color;
	}
}