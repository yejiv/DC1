// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Widget/DC1DamageNumberComponent.h"

#include "Actor/Resource/DC1ResourceActor.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1PlayerState.h"
#include "Tags/DC1GamePlayTags.h"

#include "AbilitySystemComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffectExtension.h"

UDC1DamageNumberComponent::UDC1DamageNumberComponent()
{
	// 데미지가 있을 때만 Tick
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDC1DamageNumberComponent::NotifyDamage(const FGameplayEffectModCallbackData& InData, float InAmount, EDC1DamageNumberType InType)
{
	if (InAmount <= 0.f)
	{
		return;
	}

	const AActor* Target = InData.Target.GetAvatarActor();
	if (false == ::IsValid(Target))
	{
		return;
	}

	// 리소스(나무,돌) 일때 거리 확인
	if (true == IsWorkNumberHidden(InData, Target))
	{
		return;
	}

	// 데미지 + 위치 담아 Cue
	FGameplayCueParameters Params;
	Params.RawMagnitude = InAmount;
	Params.Location = GetNumberLocation(Target);

	// 대상 ASC->ExecuteGameplayCue -> 사본 ASC들이 받음 ( 서버 -> 클라 )
	const FGameplayTag CueTag = (EDC1DamageNumberType::Shield == InType) ? DC1EffectTags::GameplayCue_DamageNumber_Shield : DC1EffectTags::GameplayCue_DamageNumber_Health;
	InData.Target.ExecuteGameplayCue(CueTag, Params);
}

void UDC1DamageNumberComponent::ShowDamageNumber(const FVector& InLocation, float InAmount, EDC1DamageNumberType InType)
{
	// 먼 거리 데미지는 표시 X ( 다른 클라 데미지 )
	if (nullptr == WidgetClass || true == IsTooFarFromCamera(InLocation))
	{
		return;
	}


	UWidgetComponent* Number = AcquireNumber();
	if (nullptr == Number)
	{
		return;
	}

	Number->SetWorldLocation(InLocation);

	UDC1DamageNumberWidget* Widget = Cast<UDC1DamageNumberWidget>(Number->GetUserWidgetObject());
	if (true == ::IsValid(Widget))
	{
		Widget->SetDamage(InAmount, InType);
	}

	Number->SetVisibility(true);

	// 활성 풀에 추가
	FDC1DamageNumberEntry& Entry = ActiveNumbers.AddDefaulted_GetRef();
	Entry.Number = Number;
	Entry.ExpireTime = GetWorld()->GetTimeSeconds() + NumberLifetime;

	SetComponentTickEnabled(true);
}

void UDC1DamageNumberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 풀 반납
	const double Now = GetWorld()->GetTimeSeconds();
	while (ActiveNumbers.Num() > 0 && ActiveNumbers[0].ExpireTime <= Now)
	{
		ReleaseNumber(ActiveNumbers[0].Number);
		ActiveNumbers.RemoveAt(0);
	}

	if (ActiveNumbers.Num() <= 0)
	{
		SetComponentTickEnabled(false);
	}
}

void UDC1DamageNumberComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ActiveNumbers.Empty();
	FreeNumbers.Empty();

	if (true == ::IsValid(NumberHost))
	{
		NumberHost->Destroy();
	}
	NumberHost = nullptr;

	Super::EndPlay(EndPlayReason);
}

bool UDC1DamageNumberComponent::IsWorkNumberHidden(const FGameplayEffectModCallbackData& InData, const AActor* InTarget)
{
	// 대상이 자원이고 가해자가 팰일 때만 거리 확인
	const ADC1ResourceActor* Resource = Cast<ADC1ResourceActor>(InTarget);
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(InData.EffectSpec.GetContext().GetInstigator());
	if (nullptr == Resource || nullptr == Pal)
	{
		return false;
	}

	const ADC1PlayerState* OwnerPS = Pal->GetOwnerPlayerState();
	if (false == ::IsValid(OwnerPS))
	{
		return true;
	}

	const APawn* OwnerPawn = OwnerPS->GetPawn();
	if (false == ::IsValid(OwnerPawn))
	{
		return true;
	}

	return FVector::DistSquared(OwnerPawn->GetActorLocation(), Resource->GetActorLocation()) > FMath::Square(Resource->GetWorkNumberRange());
}

FVector UDC1DamageNumberComponent::GetNumberLocation(const AActor* InTarget)
{
	FVector Origin;
	FVector Extent;
	InTarget->GetActorBounds(true, Origin, Extent);

	return Origin;
}

bool UDC1DamageNumberComponent::IsTooFarFromCamera(const FVector& InLocation) const
{
	const APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (nullptr == PC || nullptr == PC->PlayerCameraManager)
	{
		return false;
	}

	return FVector::DistSquared(PC->PlayerCameraManager->GetCameraLocation(), InLocation) > FMath::Square(MaxShowDistance);
}

UWidgetComponent* UDC1DamageNumberComponent::AcquireNumber()
{
	if (FreeNumbers.Num() > 0)
	{
		return FreeNumbers.Pop();
	}

	// 풀 모두 사용시 재활용
	if (ActiveNumbers.Num() >= MaxNumbers)
	{
		UWidgetComponent* Oldest = ActiveNumbers[0].Number;
		ActiveNumbers.RemoveAt(0);
		return Oldest;
	}

	return CreateNumber();
}

UWidgetComponent* UDC1DamageNumberComponent::CreateNumber()
{
	AActor* Host = GetOrSpawnHost();
	if (nullptr == Host)
	{
		return nullptr;
	}

	UWidgetComponent* Number = NewObject<UWidgetComponent>(Host);
	Number->SetWidgetSpace(EWidgetSpace::Screen);
	Number->SetDrawAtDesiredSize(true);
	Number->SetPivot({ 0.5f, 1.f });
	Number->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Number->SetUsingAbsoluteLocation(true);
	Number->SetWidgetClass(WidgetClass);
	Number->SetupAttachment(Host->GetRootComponent());
	Number->RegisterComponent();
	Number->InitWidget();

	return Number;
}

void UDC1DamageNumberComponent::ReleaseNumber(UWidgetComponent* InNumber)
{
	if (false == ::IsValid(InNumber))
	{
		return;
	}

	InNumber->SetVisibility(false);
	FreeNumbers.Add(InNumber);
}

AActor* UDC1DamageNumberComponent::GetOrSpawnHost()
{
	// PC가 Hidden이라 Screen Space 위젯이 화면에 안보임
	if (true == ::IsValid(NumberHost))
	{
		return NumberHost;
	}

	NumberHost = GetWorld()->SpawnActor<AActor>();
	if (false == ::IsValid(NumberHost))
	{
		return nullptr;
	}

	USceneComponent* Root = NewObject<USceneComponent>(NumberHost);
	NumberHost->SetRootComponent(Root);
	Root->RegisterComponent();

	return NumberHost;
}
