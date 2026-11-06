// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Pal/DC1PalRideComponent.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1Player.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UDC1PalRideComponent::UDC1PalRideComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1PalRideComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDC1PalRideComponent, RideState);
	DOREPLIFETIME(UDC1PalRideComponent, RideDirection);
}

void UDC1PalRideComponent::SetRideState(EDC1RideState InState, EDC1RideDirection InDirection)
{
	AActor* Owner = GetOwner();
	if (nullptr == Owner || false == Owner->HasAuthority())
	{
		return;
	}

	RideState = InState;
	RideDirection = InDirection;

	Owner->ForceNetUpdate();

	// 서버에선 OnRep 이 안 온다
	OnRep_RideState();
}

APawn* UDC1PalRideComponent::GetRider() const
{
	const AActor* Owner = GetOwner();
	if (nullptr == Owner)
	{
		return nullptr;
	}

	// 붙은 액터는 후보만. 탄 팰이 나인 플레이어가 탄 사람
	TArray<AActor*> AttachedActors;
	Owner->GetAttachedActors(AttachedActors);

	for (AActor* Attached : AttachedActors)
	{
		ADC1Player* Rider = Cast<ADC1Player>(Attached);
		if (nullptr != Rider && Owner == Rider->GetRiddenPawn())
		{
			return Rider;
		}
	}

	return nullptr;
}

EDC1RideDirection UDC1PalRideComponent::FindRideDirection(const FVector& InLocation) const
{
	const FVector Local = GetOwner()->GetActorTransform().InverseTransformPosition(InLocation);
	const float Angle = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));

	if (FMath::Abs(Angle) <= 45.f)
	{
		return EDC1RideDirection::Front;
	}

	if (FMath::Abs(Angle) >= 135.f)
	{
		return EDC1RideDirection::Rear;
	}

	return (Angle > 0.f) ? EDC1RideDirection::Right : EDC1RideDirection::Left;
}

FVector UDC1PalRideComponent::GetRideSideLocation() const
{
	const ADC1PalCharacter* Pal = GetPal();
	const UDC1PalData* PalData = GetPalData();
	const FVector* Offset = (nullptr != PalData) ? PalData->RideMountOffsets.Find(RideDirection) : nullptr;
	if (nullptr == Offset)
	{
		return GetOwner()->GetActorLocation();
	}

	// 수평은 소켓에서 팰 방향으로 띄운다. 소켓 회전은 뼈마다 비틀려 있어 쓰지 않는다
	FVector Location = Pal->GetMesh()->GetSocketLocation(PalData->RideSocketName) + Pal->GetActorRotation().RotateVector(*Offset);

	// 높이는 팰 발밑 기준. 탄 사람 액터는 캡슐 중심이라 메쉬 오프셋만큼 올려야 메쉬 루트(발)가 땅에 닿는다
	const ACharacter* Rider = Cast<ACharacter>(GetRider());
	const float MeshOffsetZ = (nullptr != Rider) ? Rider->GetMesh()->GetRelativeLocation().Z : 0.f;
	Location.Z = Pal->GetActorLocation().Z - Pal->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - MeshOffsetZ + Offset->Z;

	return Location;
}

void UDC1PalRideComponent::BeginRideMove(float InDuration)
{
	RideMoveStartTime = GetWorld()->GetTimeSeconds();
	RideMoveDuration = InDuration;

	GetWorld()->GetTimerManager().SetTimer(RideMoveTimer, this, &ThisClass::TickRideMove, 0.01f, true);
}

void UDC1PalRideComponent::EndRideMove()
{
	GetWorld()->GetTimerManager().ClearTimer(RideMoveTimer);

	PlaceRider(1.f);
}

bool UDC1PalRideComponent::IsRideActionLocked() const
{
	const ADC1PalCharacter* Pal = GetPal();
	const UAbilitySystemComponent* ASC = (nullptr != Pal) ? Pal->GetAbilitySystemComponent() : nullptr;

	// 올라타는 중엔 조작 막음
	return (EDC1RideState::Riding != RideState)
		|| (true == GetWorld()->GetTimerManager().IsTimerActive(RideSkillTurnTimer))
		|| (nullptr != ASC && true == ASC->HasMatchingGameplayTag(DC1PalTags::State_Pal_Attacking));
}

void UDC1PalRideComponent::RequestRideSkill(int32 InSkillIndex)
{
	const ADC1PalCharacter* Pal = GetPal();
	const UDC1PalData* PalData = GetPalData();
	if (true == IsRideActionLocked() || nullptr == Pal || nullptr == PalData || false == PalData->SkillTags.IsValidIndex(InSkillIndex))
	{
		return;
	}

	// 쿨타임 중
	const FGameplayTag SkillTag = PalData->SkillTags[InSkillIndex];
	const FGameplayTag CooldownTag = DC1PalTags::FindSkillCooldownTag(SkillTag);
	const UAbilitySystemComponent* ASC = Pal->GetAbilitySystemComponent();
	if (true == CooldownTag.IsValid() && nullptr != ASC && true == ASC->HasMatchingGameplayTag(CooldownTag))
	{
		return;
	}

	PendingRideSkillTag = SkillTag;
	RideSkillTurnFrom = Pal->GetActorRotation();
	RideSkillTurnTo = FRotator(0.f, Pal->GetControlRotation().Yaw, 0.f);
	RideSkillTurnStartTime = GetWorld()->GetTimeSeconds();

	GetWorld()->GetTimerManager().SetTimer(RideSkillTurnTimer, this, &ThisClass::TickRideSkillTurn, 0.01f, true);
}

void UDC1PalRideComponent::OnRep_RideState()
{
	GetWorld()->GetTimerManager().ClearTimer(RideMoveTimer);

	OnRideStateChanged.Broadcast();

	ACharacter* Rider = Cast<ACharacter>(GetRider());
	const UDC1PalData* PalData = GetPalData();
	if (nullptr == Rider || nullptr == PalData)
	{
		return;
	}

	// 다 올랐으면 앉는 자리에 고정. 붙어 있으니 이후엔 팰을 따라간다
	if (EDC1RideState::Riding == RideState)
	{
		PlaceRider(1.f);
	}

	UAnimMontage* Montage = nullptr;

	if (EDC1RideState::Mounting == RideState)
	{
		// 노티파이 구간 전까지 옆 자리에 둔다
		PlaceRider(0.f);

		const TSoftObjectPtr<UAnimMontage>* Found = PalData->RideMountMontages.Find(RideDirection);
		Montage = (nullptr != Found) ? Found->LoadSynchronous() : nullptr;
	}
	else if (EDC1RideState::Dismounting == RideState)
	{
		Montage = PalData->RideDismountMontage.LoadSynchronous();
	}

	// 몽타주는 각 기계에서 튼다
	if (nullptr != Montage)
	{
		Rider->PlayAnimMontage(Montage);
	}
}

void UDC1PalRideComponent::TickRideMove()
{
	const float Alpha = (0.f < RideMoveDuration) ? FMath::Clamp((GetWorld()->GetTimeSeconds() - RideMoveStartTime) / RideMoveDuration, 0.f, 1.f) : 1.f;

	PlaceRider(Alpha);

	if (Alpha >= 1.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RideMoveTimer);
	}
}

void UDC1PalRideComponent::PlaceRider(float InAlpha)
{
	const ADC1PalCharacter* Pal = GetPal();
	const UDC1PalData* PalData = GetPalData();
	APawn* Rider = GetRider();
	if (nullptr == Rider || nullptr == PalData)
	{
		return;
	}

	// 앉는 자리. 액터는 캡슐 중심이라 메쉬 오프셋만큼 올려야 메쉬 루트(앉은 골반)가 소켓에 온다
	const ACharacter* RiderCharacter = Cast<ACharacter>(Rider);
	const float MeshOffsetZ = (nullptr != RiderCharacter) ? RiderCharacter->GetMesh()->GetRelativeLocation().Z : 0.f;
	const FVector SeatLocation = Pal->GetMesh()->GetSocketLocation(PalData->RideSocketName) - FVector(0.f, 0.f, MeshOffsetZ);
	const FVector SideLocation = GetRideSideLocation();

	if (EDC1RideState::Mounting == RideState)
	{
		Rider->SetActorLocation(FMath::Lerp(SideLocation, SeatLocation, InAlpha));
	}
	else if (EDC1RideState::Dismounting == RideState)
	{
		Rider->SetActorLocation(FMath::Lerp(SeatLocation, SideLocation, InAlpha));
	}
	else if (EDC1RideState::Riding == RideState)
	{
		Rider->SetActorLocation(SeatLocation);
	}
}

void UDC1PalRideComponent::TickRideSkillTurn()
{
	ADC1PalCharacter* Pal = GetPal();
	if (nullptr == Pal)
	{
		GetWorld()->GetTimerManager().ClearTimer(RideSkillTurnTimer);
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - RideSkillTurnStartTime;
	const float Alpha = (RideSkillTurnTime > 0.f) ? FMath::Clamp(Elapsed / RideSkillTurnTime, 0.f, 1.f) : 1.f;

	// 탄 사람 클라가 돌리면 이동 복제로 서버에 간다
	Pal->SetActorRotation(FQuat::Slerp(RideSkillTurnFrom.Quaternion(), RideSkillTurnTo.Quaternion(), Alpha));

	if (Alpha < 1.f)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(RideSkillTurnTimer);
	Server_UseRideSkill(PendingRideSkillTag);
	PendingRideSkillTag = FGameplayTag::EmptyTag;
}

void UDC1PalRideComponent::Server_UseRideSkill_Implementation(const FGameplayTag& InSkillTag)
{
	ADC1PalCharacter* Pal = GetPal();
	const UDC1PalData* PalData = GetPalData();
	UDC1AbilitySystemComponent* ASC = (nullptr != Pal) ? Cast<UDC1AbilitySystemComponent>(Pal->GetAbilitySystemComponent()) : nullptr;
	if (nullptr == Pal || false == Pal->IsPlayerControlled() || nullptr == PalData || false == PalData->SkillTags.Contains(InSkillTag)
		|| nullptr == ASC || true == ASC->HasMatchingGameplayTag(DC1PalTags::State_Pal_Attacking))
	{
		return;
	}

	// 쿨타임은 어빌리티의 CheckCooldown 이 막는다
	ASC->ActivateAbilityByTag(InSkillTag);
}

ADC1PalCharacter* UDC1PalRideComponent::GetPal() const
{
	return Cast<ADC1PalCharacter>(GetOwner());
}

const UDC1PalData* UDC1PalRideComponent::GetPalData() const
{
	const ADC1PalCharacter* Pal = GetPal();
	return (nullptr != Pal) ? Pal->GetPalData() : nullptr;
}
