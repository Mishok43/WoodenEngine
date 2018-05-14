#include "Object.h"

namespace WoodenEngine
{
	WObject::WObject(
		const std::string& MeshName,
		const std::string& SubmeshName,
		const XMFLOAT3& Position, 
		const XMFLOAT3& Rotation, 
		const XMFLOAT3& Scale) : 
		MeshName(MeshName), 
		SubmeshName(SubmeshName),
		Position(Position), 
		Rotation(Rotation), 
		Scale(Scale),
		bIsRenderable(true)
	{
		UpdateWorldTransform();
	}


	void WObject::Update(float Delta)
	{
		LifeTime += 0.16;
		NumDirtyConstBuffers = NMR_SWAP_BUFFERS;
	}

	void WObject::SetPosition(const XMFLOAT3& Position) noexcept
	{
		this->Position = Position;
		UpdateWorldTransform();
	}

	void WObject::SetPosition(const float X, const float Y, const float Z) noexcept
	{
		SetPosition({ X, Y, Z });
	}

	void WObject::SetRotation(const XMFLOAT3& Rotation) noexcept
	{
		this->Rotation = Rotation;
		UpdateWorldTransform();
	}

	void WObject::SetRotation(const float X, const float Y, const float Z) noexcept
	{
		SetRotation({ X, Y, Z });
	}

	void WObject::SetScale(const XMFLOAT3& Scale) noexcept
	{
		this->Scale = Scale;
		UpdateWorldTransform();
	}

	void WObject::SetScale(const float X, const float Y, const float Z) noexcept
	{
		SetScale({ X, Y, Z });
	}

	void WObject::SetColor(XMFLOAT4 Color) noexcept
	{
		this->Color = Color;
	}

	void WObject::UpdateWorldTransform() noexcept 
	{
		WorldTransform = 
			DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&Scale))*
			DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation))*
			DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&Position));
	}

	void WObject::InputMouseMoved(const float dx, const float dy) noexcept
	{
	}

	void WObject::InputKeyPressed(char key) noexcept
	{

	}

	void WObject::InputKeyReleased(char key) noexcept
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

	void WObject::SetWorldTransform(const XMMATRIX& WorldTransform) noexcept
	{
		this->WorldTransform = WorldTransform;
	}

	void WObject::SetIsUpdating(const bool IsUpdating) noexcept
	{
		bIsUpdating = IsUpdating;
	}

	void WObject::SetIsVisible(const bool IsVisible) noexcept
	{
		bIsVisible = IsVisible;
	}

	void WObject::SetIsRenderable(const bool Renderable) noexcept
	{
		bIsRenderable = Renderable;
	}

	void WObject::SetWaterFactor(int WaterFactor) noexcept
	{
		this->WaterFactor = WaterFactor;
	}

	void WObject::SetTextureTransform(const XMFLOAT4X4& TextureTransform) noexcept
	{
		this->TextureTransform = TextureTransform;
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
		if (MeshName.empty())
		{
			throw std::length_error("Mesh name is empty");
		}

		return MeshName;
	}

	const std::string& WObject::GetSubmeshName() const
	{
		if (SubmeshName.empty())
		{
			throw std::length_error("Submesh name is empty");
		}

		return SubmeshName;
	}

	const FMaterialData* WObject::GetMaterial() const noexcept
	{
		return Material;
	}

	const XMFLOAT4X4& WObject::GetTextureTransform() const noexcept
	{
		return TextureTransform;
	}

	const XMMATRIX& WObject::GetWorldTransform() const noexcept
	{
		return WorldTransform;
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

	int WObject::GetWaterFactor() const noexcept
	{
		return WaterFactor;
	}

	float WObject::GetLifeTime() const noexcept
	{
		return LifeTime;
	}

	bool WObject::IsVisible() const noexcept
	{
		return bIsVisible;
	}


}