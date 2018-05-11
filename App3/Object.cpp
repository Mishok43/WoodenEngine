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
		LifeTime += 0.16;
		NumDirtyConstBuffers = NMR_SWAP_BUFFERS;
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

	void WObject::SetMaterial(const FMaterialData* Material)
	{
		if (Material == nullptr)
		{
			throw std::invalid_argument("Material must be not nullptr");
		}

		this->Material = Material;
	}

	void WObject::SetIsUpdating(const bool IsUpdating) noexcept
	{
		bIsUpdating = IsUpdating;
	}

	void WObject::SetIsRenderable(const bool Renderable) noexcept
	{
		bIsRenderable = Renderable;
	}

	void WObject::SetMaterialTrasnform(const XMFLOAT4X4& MaterialTransform) noexcept
	{
		this->MaterialTransform = MaterialTransform;
	}

	void WObject::SetConstBufferIndex(const uint64 Index) noexcept
	{
		iConstBuffer = Index;
	}

	uint64 WObject::GetConstBufferIndex() const
	{
		assert(iConstBuffer != UINT64_MAX); // ConstBufferIndex must be set
		return iConstBuffer;
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

	const FMaterialData* WObject::GetMaterial() const noexcept
	{
		return Material;
	}

	const XMFLOAT4X4& WObject::GetMaterialTransform() const noexcept
	{
		return MaterialTransform;
	}

	const XMMATRIX& WObject::GetWorldMatrix() const noexcept
	{
		return WorldMatrix;
	}

	XMFLOAT3 WObject::GetWorldPosition() const noexcept
	{
		return Position;
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

	float WObject::GetLifeTime() const noexcept
	{
		return LifeTime;
	}

}