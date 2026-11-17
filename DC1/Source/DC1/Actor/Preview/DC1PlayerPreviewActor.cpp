// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Preview/DC1PlayerPreviewActor.h"

#include "Character/Player/DC1Player.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

ADC1PlayerPreviewActor::ADC1PlayerPreviewActor()
{
	// 복사 메쉬 - 원본 플레이어와 같은 계층 (Body 아래 Head / Hair)
	CopyBody = CreateDefaultSubobject<USkeletalMeshComponent>("CopyBody");
	CopyBody->SetupAttachment(CharacterPivot);

	CopyHead = CreateDefaultSubobject<USkeletalMeshComponent>("CopyHead");
	CopyHead->SetupAttachment(CopyBody);

	CopyHair = CreateDefaultSubobject<USkeletalMeshComponent>("CopyHair");
	CopyHair->SetupAttachment(CopyBody);

	CopyHairEquip = CreateDefaultSubobject<USkeletalMeshComponent>("CopyHairEquip");
	CopyHairEquip->SetupAttachment(CopyBody);

	for (USkeletalMeshComponent* Copy : { CopyBody.Get(), CopyHead.Get(), CopyHair.Get(), CopyHairEquip.Get() })
	{
		Copy->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		ApplyPreviewLighting(Copy);
	}
}

void ADC1PlayerPreviewActor::CopyFromPlayer(ADC1Player* InPlayer)
{
	if (false == ::IsValid(InPlayer))
	{
		return;
	}

	SourceToCopy.Reset();

	// 몸통은 캡슐 기준 오프셋까지 복사, 나머지는 몸통 기준 오프셋
	CopySkeletalMesh(InPlayer->GetMesh(), CopyBody);
	CopySkeletalMesh(InPlayer->HeadMesh, CopyHead);
	CopySkeletalMesh(InPlayer->HairMesh, CopyHair);
	CopySkeletalMesh(InPlayer->HairEquipMesh, CopyHairEquip);

	// 이 액터의 모든 컴포넌트만 촬영
	SetShowOnlyActor(this);
}

void ADC1PlayerPreviewActor::CopyAttachments(ADC1Player* InPlayer)
{
	ClearAttachments();

	if (false == ::IsValid(InPlayer))
	{
		return;
	}

	TArray<AActor*> AttachedActors;
	InPlayer->GetAttachedActors(AttachedActors, true, true);

	for (AActor* Attached : AttachedActors)
	{
		// 숨긴 액터(안 든 무기 등)는 월드에서도 안 보이므로 제외
		if (false == ::IsValid(Attached) || true == Attached->IsHidden() || nullptr == Attached->GetRootComponent())
		{
			continue;
		}

		USceneComponent* SourceParent = Attached->GetRootComponent()->GetAttachParent();
		USkeletalMeshComponent* CopyParent = FindCopyParent(SourceParent);
		const FName SocketName = Attached->GetRootComponent()->GetAttachSocketName();

		TArray<UMeshComponent*> MeshComponents;
		Attached->GetComponents<UMeshComponent>(MeshComponents);
		for (UMeshComponent* Mesh : MeshComponents)
		{
			if (true == ::IsValid(Mesh) && true == Mesh->IsVisible())
			{
				CopyAttachedMesh(Mesh, SourceParent, CopyParent, SocketName);
			}
		}
	}
}

void ADC1PlayerPreviewActor::CopySkeletalMesh(USkeletalMeshComponent* InSource, USkeletalMeshComponent* InCopy)
{
	if (false == ::IsValid(InSource) || false == ::IsValid(InCopy))
	{
		return;
	}

	InCopy->SetSkeletalMeshAsset(InSource->GetSkeletalMeshAsset());
	// Hairequip 붙이기용
	InCopy->AttachToComponent(InCopy->GetAttachParent(), FAttachmentTransformRules::KeepRelativeTransform, InSource->GetAttachSocketName());
	InCopy->SetRelativeTransform(InSource->GetRelativeTransform());

	// 외형 색상은 원본의 동적 머티리얼에 들어 있으므로 그대로 공유
	for (int32 Index = 0; Index < InSource->GetNumMaterials(); ++Index)
	{
		InCopy->SetMaterial(Index, InSource->GetMaterial(Index));
	}

	// 원본 뼈대를 그대로 따라가므로 애니메이션 계산 없이 같은 자세가 됨
	InCopy->SetLeaderPoseComponent(InSource, true);
	// 메시가 비었거나 원본이 숨겨져 있으면(투구 착용 시 머리카락) 복사본도 숨김
	InCopy->SetVisibility(nullptr != InSource->GetSkeletalMeshAsset() && true == InSource->GetVisibleFlag());

	SourceToCopy.Add(InSource, InCopy);
}

void ADC1PlayerPreviewActor::CopyAttachedMesh(UMeshComponent* InSource, USceneComponent* InSourceParent, USkeletalMeshComponent* InCopyParent, FName InSocketName)
{
	if (false == ::IsValid(InSourceParent) || false == ::IsValid(InCopyParent))
	{
		return;
	}

	UMeshComponent* Copy = nullptr;

	if (const UStaticMeshComponent* SourceStatic = Cast<UStaticMeshComponent>(InSource))
	{
		UStaticMeshComponent* CopyStatic = NewObject<UStaticMeshComponent>(this);
		CopyStatic->SetStaticMesh(SourceStatic->GetStaticMesh());
		Copy = CopyStatic;
	}
	else if (USkeletalMeshComponent* SourceSkeletal = Cast<USkeletalMeshComponent>(InSource))
	{
		USkeletalMeshComponent* CopySkeletal = NewObject<USkeletalMeshComponent>(this);
		CopySkeletal->SetSkeletalMeshAsset(SourceSkeletal->GetSkeletalMeshAsset());
		CopySkeletal->SetLeaderPoseComponent(SourceSkeletal, true);
		Copy = CopySkeletal;
	}

	if (nullptr == Copy)
	{
		return;
	}

	for (int32 Index = 0; Index < InSource->GetNumMaterials(); ++Index)
	{
		Copy->SetMaterial(Index, InSource->GetMaterial(Index));
	}

	Copy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ApplyPreviewLighting(Copy);

	// 소켓 기준 상대 위치는 손이 움직여도 변하지 않으므로 한 번만 계산
	const FTransform SocketTransform = InSourceParent->GetSocketTransform(InSocketName);
	Copy->SetupAttachment(InCopyParent, InSocketName);
	Copy->RegisterComponent();
	Copy->SetRelativeTransform(InSource->GetComponentTransform().GetRelativeTransform(SocketTransform));

	AttachmentCopies.Add(Copy);
}

USkeletalMeshComponent* ADC1PlayerPreviewActor::FindCopyParent(const USceneComponent* InSourceParent) const
{
	if (const TObjectPtr<USkeletalMeshComponent>* Found = SourceToCopy.Find(InSourceParent))
	{
		return *Found;
	}

	return CopyBody;
}

void ADC1PlayerPreviewActor::ClearAttachments()
{
	for (UMeshComponent* Copy : AttachmentCopies)
	{
		if (::IsValid(Copy))
		{
			Copy->DestroyComponent();
		}
	}

	AttachmentCopies.Reset();
}
