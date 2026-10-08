// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Preview/DC1PlayerPreviewActor.h"

#include "Character/Player/DC1Player.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/TextureRenderTarget2D.h"

namespace
{
	// 고정 카메라 - 정면(+X)에서 원점을 바라보는 배치를 Yaw만큼 공전
	constexpr float CameraDistance = 250.f;
	constexpr float CameraHeight = 20.f;
	constexpr float CameraRight = -10.f;	// 양수면 화면 기준 오른쪽
	constexpr float CameraYaw = -20.f;
	constexpr float CameraPitch = 10.f;	// 양수면 내려다봄
	constexpr float CameraFOV = 30.f;

	// 카메라 오른쪽 = -Y
	FVector GetFixedCameraLocation()
	{
		return FRotator(0.f, CameraYaw, 0.f).RotateVector(FVector(CameraDistance, -CameraRight, CameraHeight));
	}
}

ADC1PlayerPreviewActor::ADC1PlayerPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	CharacterPivot = CreateDefaultSubobject<USceneComponent>("CharacterPivot");
	CharacterPivot->SetupAttachment(Root);

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

	// 전용 조명 - 정면 위 키 라이트 + 반대편 필 라이트, 채널 2만 비춤
	KeyLight = CreateDefaultSubobject<UPointLightComponent>("KeyLight");
	KeyLight->SetupAttachment(Root);
	KeyLight->LightingChannels.bChannel0 = false;
	KeyLight->LightingChannels.bChannel2 = true;

	FillLight = CreateDefaultSubobject<UPointLightComponent>("FillLight");
	FillLight->SetupAttachment(Root);
	FillLight->LightingChannels.bChannel0 = false;
	FillLight->LightingChannels.bChannel2 = true;

	// 정면에서 캐릭터를 바라봄, 자기 컴포넌트만 촬영
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("CaptureComponent");
	CaptureComponent->SetupAttachment(Root);
	CaptureComponent->SetRelativeLocationAndRotation(GetFixedCameraLocation(), FRotator(-CameraPitch, 180.f + CameraYaw, 0.f));
	CaptureComponent->FOVAngle = CameraFOV;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;

	// 월드 환경광/하늘/안개가 섞이지 않도록 촬영 화면에서만 끔 (이름 기반이라 없는 플래그는 무시됨)
	TArray<FEngineShowFlagsSetting> ShowFlagSettings;
	for (const TCHAR* FlagName : { TEXT("Atmosphere"), TEXT("Fog"), TEXT("VolumetricFog"), TEXT("Cloud"), TEXT("SkyLighting"),
		TEXT("DirectionalLights"), TEXT("GlobalIllumination"), TEXT("LumenGlobalIllumination"), TEXT("LumenReflections"), TEXT("AmbientOcclusion") })
	{
		FEngineShowFlagsSetting Setting;
		Setting.ShowFlagName = FlagName;
		Setting.Enabled = false;
		ShowFlagSettings.Add(Setting);
	}
	CaptureComponent->SetShowFlagSettings(ShowFlagSettings);
}

void ADC1PlayerPreviewActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyPreviewSettings();
}

#if WITH_EDITOR
void ADC1PlayerPreviewActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// PIE 중 스폰된 액터는 OnConstruction이 다시 돌지 않으므로 직접 반영
	ApplyPreviewSettings();
}
#endif

void ADC1PlayerPreviewActor::ApplyPreviewSettings()
{
	// CDO/BP 템플릿의 컴포넌트를 건드리면 재인스턴싱 중 크래시가 날 수 있어 실제 인스턴스만 처리
	if (true == IsTemplate())
	{
		return;
	}

	CharacterPivot->SetRelativeLocationAndRotation(CharacterOffset, CharacterRotation);

	KeyLight->SetRelativeLocation(KeyLightOffset);
	KeyLight->SetIntensity(KeyLightIntensity);
	KeyLight->SetLightColor(KeyLightColor);

	FillLight->SetRelativeLocation(FillLightOffset);
	FillLight->SetIntensity(FillLightIntensity);
	FillLight->SetLightColor(FillLightColor);
}

void ADC1PlayerPreviewActor::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	CaptureComponent->TextureTarget = InRenderTarget;
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
	CaptureComponent->ShowOnlyActors.Reset();
	CaptureComponent->ShowOnlyActors.Add(this);
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

void ADC1PlayerPreviewActor::SetCaptureEnabled(bool bInEnabled)
{
	CaptureComponent->bCaptureEveryFrame = bInEnabled;
}

void ADC1PlayerPreviewActor::CopySkeletalMesh(USkeletalMeshComponent* InSource, USkeletalMeshComponent* InCopy)
{
	if (false == ::IsValid(InSource) || false == ::IsValid(InCopy))
	{
		return;
	}

	InCopy->SetSkeletalMeshAsset(InSource->GetSkeletalMeshAsset());
	InCopy->SetRelativeTransform(InSource->GetRelativeTransform());

	// 외형 색상은 원본의 동적 머티리얼에 들어 있으므로 그대로 공유
	for (int32 Index = 0; Index < InSource->GetNumMaterials(); ++Index)
	{
		InCopy->SetMaterial(Index, InSource->GetMaterial(Index));
	}

	// 원본 뼈대를 그대로 따라가므로 애니메이션 계산 없이 같은 자세가 됨
	InCopy->SetLeaderPoseComponent(InSource, true);
	InCopy->SetVisibility(nullptr != InSource->GetSkeletalMeshAsset());

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

void ADC1PlayerPreviewActor::ApplyPreviewLighting(UPrimitiveComponent* InComponent)
{
	// 월드 조명(채널 0)은 받지 않고 전용 조명(채널 2)만 받음
	InComponent->SetLightingChannels(false, false, true);
	InComponent->SetCastShadow(false);
}
