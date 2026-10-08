// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/PlayerPreview/DC1PlayerPreviewSubsystem.h"

#include "Actor/Preview/DC1PlayerPreviewActor.h"
#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"
#include "Data/DA/UI/DC1UIResourceData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UTextureRenderTarget2D* UDC1PlayerPreviewSubsystem::GetRenderTarget()
{
	if (false == ::IsValid(RenderTarget))
	{
		FIntPoint Size(512, 768);
		if (const UDC1UIResourceData* ResourceData = UDC1AssetManager::GetAssetByTag<UDC1UIResourceData>(DC1AssetTags::Asset_UI_Resource_Common))
		{
			Size = ResourceData->PlayerPreviewRenderTargetSize;
		}

		RenderTarget = NewObject<UTextureRenderTarget2D>(this);
		RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
		// SceneColorHDR의 알파는 투명도의 반대값이므로 빈 곳은 알파 1로 비움
		RenderTarget->ClearColor = FLinearColor(0.f, 0.f, 0.f, 1.f);
		RenderTarget->InitAutoFormat(Size.X, Size.Y);
		RenderTarget->UpdateResourceImmediate(true);
	}

	return RenderTarget;
}

void UDC1PlayerPreviewSubsystem::Deinitialize()
{
	EndPreview();

	if (::IsValid(PreviewActor))
	{
		PreviewActor->Destroy();
	}
	PreviewActor = nullptr;

	Super::Deinitialize();
}

void UDC1PlayerPreviewSubsystem::BeginPreview()
{
	ADC1PlayerPreviewActor* Actor = FindOrSpawnPreviewActor();
	ADC1Player* Player = GetLocalPlayerPawn();
	if (nullptr == Actor || nullptr == Player)
	{
		return;
	}

	bIsPreviewing = true;

	// 외형은 커스터마이징 화면에서만 바뀌므로 열 때마다 다시 복사하면 충분
	RefreshPreview();
	BindEquipComponent(Player);
	Actor->SetCaptureEnabled(true);
}

void UDC1PlayerPreviewSubsystem::EndPreview()
{
	bIsPreviewing = false;
	UnbindEquipComponent();

	if (::IsValid(PreviewActor))
	{
		PreviewActor->SetCaptureEnabled(false);
	}
}

ADC1PlayerPreviewActor* UDC1PlayerPreviewSubsystem::FindOrSpawnPreviewActor()
{
	UWorld* World = GetLocalPlayer() ? GetLocalPlayer()->GetWorld() : nullptr;
	if (nullptr == World)
	{
		return nullptr;
	}

	// 맵을 이동하면 이전 월드의 액터는 파괴되므로 현재 월드 소속인지 확인
	if (::IsValid(PreviewActor) && World == PreviewActor->GetWorld())
	{
		return PreviewActor;
	}

	TSubclassOf<ADC1PlayerPreviewActor> ActorClass = ADC1PlayerPreviewActor::StaticClass();
	if (const UDC1UIResourceData* ResourceData = UDC1AssetManager::GetAssetByTag<UDC1UIResourceData>(DC1AssetTags::Asset_UI_Resource_Common))
	{
		if (nullptr != ResourceData->PlayerPreviewActorClass)
		{
			ActorClass = ResourceData->PlayerPreviewActorClass;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	PreviewActor = World->SpawnActor<ADC1PlayerPreviewActor>(ActorClass, FVector(0.f, 0.f, PreviewLocationZ), FRotator::ZeroRotator, SpawnParams);
	if (::IsValid(PreviewActor))
	{
		PreviewActor->SetRenderTarget(GetRenderTarget());
	}

	return PreviewActor;
}

ADC1Player* UDC1PlayerPreviewSubsystem::GetLocalPlayerPawn() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (nullptr == LocalPlayer)
	{
		return nullptr;
	}

	const APlayerController* PC = LocalPlayer->GetPlayerController(LocalPlayer->GetWorld());
	return ::IsValid(PC) ? Cast<ADC1Player>(PC->GetPawn()) : nullptr;
}

void UDC1PlayerPreviewSubsystem::RefreshPreview()
{
	ADC1Player* Player = GetLocalPlayerPawn();
	if (false == ::IsValid(PreviewActor) || nullptr == Player)
	{
		return;
	}

	PreviewActor->CopyFromPlayer(Player);
	PreviewActor->CopyAttachments(Player);
}

void UDC1PlayerPreviewSubsystem::HandleWeaponChanged()
{
	UWorld* World = GetLocalPlayer() ? GetLocalPlayer()->GetWorld() : nullptr;
	if (nullptr == World)
	{
		return;
	}

	// 무기 교체와 숨김 상태 복제가 같은 프레임에 끝나지 않을 수 있어 다음 틱에 복사
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (true == bIsPreviewing && ::IsValid(PreviewActor))
		{
			PreviewActor->CopyAttachments(GetLocalPlayerPawn());
		}
	}));
}

void UDC1PlayerPreviewSubsystem::BindEquipComponent(ADC1Player* InPlayer)
{
	UnbindEquipComponent();

	const ADC1PlayerState* PS = ::IsValid(InPlayer) ? InPlayer->GetPlayerState<ADC1PlayerState>() : nullptr;
	UDC1EquipComponent* Equip = ::IsValid(PS) ? PS->GetEquipComponent() : nullptr;
	if (nullptr == Equip)
	{
		return;
	}

	Equip->OnWeaponChanged.AddUObject(this, &ThisClass::HandleWeaponChanged);
	BoundEquipComponent = Equip;
}

void UDC1PlayerPreviewSubsystem::UnbindEquipComponent()
{
	if (UDC1EquipComponent* Equip = BoundEquipComponent.Get())
	{
		Equip->OnWeaponChanged.RemoveAll(this);
	}

	BoundEquipComponent.Reset();
}
