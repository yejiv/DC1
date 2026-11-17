// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/PlayerPreview/DC1PreviewSubsystem.h"

#include "Actor/Preview/DC1PlayerPreviewActor.h"
#include "Actor/Preview/DC1PalPreviewActor.h"
#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "Data/DA/UI/DC1UIResourceData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UTextureRenderTarget2D* UDC1PreviewSubsystem::GetRenderTarget(EDC1PreviewType InPreviewType)
{
	const UDC1UIResourceData* ResourceData = UDC1AssetManager::GetAssetByTag<UDC1UIResourceData>(DC1AssetTags::Asset_UI_Resource_Common);

	switch (InPreviewType)
	{
	case EDC1PreviewType::Player:
		return InitRenderTarget(PlayerRenderTarget, ResourceData ? ResourceData->PlayerPreviewRenderTargetSize : FIntPoint(512, 768));

	case EDC1PreviewType::Pal:
		return InitRenderTarget(PalRenderTarget, ResourceData ? ResourceData->PalPreviewRenderTargetSize : FIntPoint(800, 768));

	default:
		return nullptr;
	}
}

UTextureRenderTarget2D* UDC1PreviewSubsystem::InitRenderTarget(TObjectPtr<UTextureRenderTarget2D>& InRenderTarget, const FIntPoint& InSize)
{
	if (false == ::IsValid(InRenderTarget))
	{
		InRenderTarget = NewObject<UTextureRenderTarget2D>(this);
		InRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
		// SceneColorHDR의 알파는 투명도의 반대값이므로 빈 곳은 알파 1로 비움
		InRenderTarget->ClearColor = FLinearColor(0.f, 0.f, 0.f, 1.f);
		InRenderTarget->InitAutoFormat(InSize.X, InSize.Y);
		InRenderTarget->UpdateResourceImmediate(true);
	}

	return InRenderTarget;
}

void UDC1PreviewSubsystem::Deinitialize()
{
	EndPreview(EDC1PreviewType::Player);
	EndPreview(EDC1PreviewType::Pal);

	if (::IsValid(PlayerPreviewActor))
	{
		PlayerPreviewActor->Destroy();
	}
	if (::IsValid(PalPreviewActor))
	{
		PalPreviewActor->Destroy();
	}

	PlayerPreviewActor = nullptr;
	PalPreviewActor = nullptr;

	Super::Deinitialize();
}

void UDC1PreviewSubsystem::BeginPlayerPreview()
{
	ADC1Player* Player = GetLocalPlayerPawn();
	if (nullptr == Player)
	{
		return;
	}

	ADC1PreviewActor* Actor = BeginPreview(EDC1PreviewType::Player);
	if (nullptr == Actor)
	{
		return;
	}

	// 열 때 한 번 복사하고, 열려 있는 동안 바뀌는 방어구는 OnArmorChanged로 다시 복사
	RefreshPlayerPreview(Player);
	BindEquipComponent(Player);

	// 갱신 전 상태가 찍히지 않도록 갱신 후에 캡처 시작
	Actor->SetCaptureEnabled(true);
}

void UDC1PreviewSubsystem::BeginPalPreview(const FGameplayTag& PalID)
{
	if (false == PalID.IsValid())
		return;

	ADC1PreviewActor* Actor = BeginPreview(EDC1PreviewType::Pal);
	if (nullptr == Actor)
	{
		return;
	}

	RefreshPalPreview(PalID);

	// 갱신 전 상태가 찍히지 않도록 갱신 후에 캡처 시작
	Actor->SetCaptureEnabled(true);
}

void UDC1PreviewSubsystem::EndPreview(EDC1PreviewType InType)
{
	bIsPreviewing = false;

	if (EDC1PreviewType::Player == InType && ::IsValid(PlayerPreviewActor))
	{
		UnbindEquipComponent();
		PlayerPreviewActor->SetCaptureEnabled(false);
	}

	if (EDC1PreviewType::Pal == InType && ::IsValid(PalPreviewActor))
	{
		PalPreviewActor->SetCaptureEnabled(false);
	}
}

ADC1PreviewActor* UDC1PreviewSubsystem::BeginPreview(EDC1PreviewType InType)
{
	const UDC1UIResourceData* ResourceData = UDC1AssetManager::GetAssetByTag<UDC1UIResourceData>(DC1AssetTags::Asset_UI_Resource_Common);

	// 액터 클래스가 비어 있으면 C++ 기본 클래스
	ADC1PreviewActor* Actor = { nullptr };
	switch (InType)
	{
	case EDC1PreviewType::Player:
	{
		TSubclassOf<ADC1PlayerPreviewActor> ActorClass = ADC1PlayerPreviewActor::StaticClass();
		if (nullptr != ResourceData && nullptr != ResourceData->PlayerPreviewActorClass)
		{
			ActorClass = ResourceData->PlayerPreviewActorClass;
		}

		Actor = FindorSpawnPreviewActor(PlayerPreviewActor, ActorClass, InType);
		break;
	}
	case EDC1PreviewType::Pal:
	{
		TSubclassOf<ADC1PalPreviewActor> ActorClass = ADC1PalPreviewActor::StaticClass();
		if (nullptr != ResourceData && nullptr != ResourceData->PalPreviewActorClass)
		{
			ActorClass = ResourceData->PalPreviewActorClass;
		}

		Actor = FindorSpawnPreviewActor(PalPreviewActor, ActorClass, InType);
		break;
	}
	default:
		break;
	}

	if (nullptr == Actor)
	{
		return nullptr;
	}

	bIsPreviewing = true;
	ActiveType = InType;

	return Actor;
}

ADC1PreviewActor* UDC1PreviewSubsystem::GetPreviewActor(EDC1PreviewType InType)
{
	switch (InType)
	{
	case EDC1PreviewType::Player:
		return PlayerPreviewActor;
	case EDC1PreviewType::Pal:
		return PalPreviewActor;
	default:
		return nullptr;
	}
}

ADC1Player* UDC1PreviewSubsystem::GetLocalPlayerPawn() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (nullptr == LocalPlayer)
	{
		return nullptr;
	}

	const APlayerController* PC = LocalPlayer->GetPlayerController(LocalPlayer->GetWorld());
	return ::IsValid(PC) ? Cast<ADC1Player>(PC->GetPawn()) : nullptr;
}

void UDC1PreviewSubsystem::RefreshPlayerPreview(ADC1Player* InPlayer)
{
	if (false == ::IsValid(PlayerPreviewActor) || false == ::IsValid(InPlayer))
	{
		return;
	}

	PlayerPreviewActor->CopyFromPlayer(InPlayer);
	PlayerPreviewActor->CopyAttachments(InPlayer);
}

void UDC1PreviewSubsystem::RefreshPalPreview(const FGameplayTag& InPalID)
{
	if (false == ::IsValid(PalPreviewActor))
	{
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	const UDC1PalData* PalData = (nullptr != Registry) ? Registry->FindPalDataByTag(InPalID).LoadSynchronous() : nullptr;
	if (nullptr == PalData)
	{
		return;
	}

	PalPreviewActor->SetPalPreview(PalData->PreviewSettings);
}

void UDC1PreviewSubsystem::HandleWeaponChanged()
{
	UWorld* World = GetLocalPlayer() ? GetLocalPlayer()->GetWorld() : nullptr;
	if (nullptr == World)
	{
		return;
	}

	// 무기 교체와 숨김 상태 복제가 같은 프레임에 끝나지 않을 수 있어 다음 틱에 복사
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (true == bIsPreviewing && EDC1PreviewType::Player == ActiveType && ::IsValid(PlayerPreviewActor))
		{
			PlayerPreviewActor->CopyAttachments(GetLocalPlayerPawn());
		}
	}));
}

void UDC1PreviewSubsystem::HandleArmorChanged()
{
	// 방어구 OnRep에서 메시 교체가 끝난 뒤 불리므로 바로 복사
	// 부착물 부모 매핑이 CopyFromPlayer마다 초기화되므로 부착물도 함께 다시 복사
	if (true == bIsPreviewing && EDC1PreviewType::Player == ActiveType)
	{
		RefreshPlayerPreview(GetLocalPlayerPawn());
	}
}

void UDC1PreviewSubsystem::BindEquipComponent(ADC1Player* InPlayer)
{
	UnbindEquipComponent();

	const ADC1PlayerState* PS = ::IsValid(InPlayer) ? InPlayer->GetPlayerState<ADC1PlayerState>() : nullptr;
	UDC1EquipComponent* Equip = ::IsValid(PS) ? PS->GetEquipComponent() : nullptr;
	if (nullptr == Equip)
	{
		return;
	}

	Equip->OnWeaponChanged.AddUObject(this, &ThisClass::HandleWeaponChanged);
	Equip->OnArmorChanged.AddUObject(this, &ThisClass::HandleArmorChanged);
	BoundEquipComponent = Equip;
}

void UDC1PreviewSubsystem::UnbindEquipComponent()
{
	if (UDC1EquipComponent* Equip = BoundEquipComponent.Get())
	{
		Equip->OnWeaponChanged.RemoveAll(this);
		Equip->OnArmorChanged.RemoveAll(this);
	}

	BoundEquipComponent.Reset();
}

template<typename T>
T* UDC1PreviewSubsystem::FindorSpawnPreviewActor(TObjectPtr<T>& InOutActor, TSubclassOf<T> InClass, EDC1PreviewType InType)
{
	UWorld* World = GetLocalPlayer() ? GetLocalPlayer()->GetWorld() : nullptr;
	if (nullptr == World)
	{
		return nullptr;
	}

	// 맵을 이동하면 이전 월드의 액터는 파괴되므로 현재 월드 소속인지 확인
	if (::IsValid(InOutActor) && World == InOutActor->GetWorld())
	{
		return InOutActor;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	InOutActor = World->SpawnActor<T>(InClass, FVector(static_cast<int32>(InType) * PreviewSpacing, 0.f, PreviewLocationZ), FRotator::ZeroRotator, SpawnParams);
	if (::IsValid(InOutActor))
	{
		InOutActor->SetRenderTarget(GetRenderTarget(InType));
	}

	return InOutActor;
}
