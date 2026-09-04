// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Subsystem/DC1UIManagerSubsystem.h"

#include "Character/DC1Character.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Tags/State/DC1GameplayTag_State.h"

#include "Net/UnrealNetwork.h"

ADC1InteractBuildActor::ADC1InteractBuildActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;


	InteractCollider = CreateDefaultSubobject<UBoxComponent>("InteractCollider");
	InteractCollider->SetupAttachment(BuildMesh);
	InteractCollider->SetGenerateOverlapEvents(true);
}

void ADC1InteractBuildActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Completed 처리
	if (FlashAmount > 0.f)
	{
		FlashAmount -= (DeltaTime / CompleteFlashDuration);

		if (FlashAmount <= 0.f)
		{
			FlashAmount = 0.f;
			BuildCompleted();
		}
		else
		{
			SetMaterialFlash();
		}

		return;
	}

	// Construction 처리
	if (true == Workers.IsEmpty() || false == HasAuthority())
	{
		return;
	}

	const float TotalWorkSpeed = ComputeTotalWorkSpeed();

	if (TotalWorkSpeed > 0.f && BuildActorData.RequiredWork > 0.f && CurrentWork < BuildActorData.RequiredWork)
	{
		CurrentWork += (TotalWorkSpeed * DeltaTime);

		BuildProgress = (CurrentWork / BuildActorData.RequiredWork);

		if (BuildProgress >= 1.f)
		{
			BuildProgress = 1.f;

			SetState(EDC1BuildState::Completed);
		}

		SetMaterialBuildProgress();
	}
}

void ADC1InteractBuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BuildProgress);
}

UPrimitiveComponent* ADC1InteractBuildActor::GetInteractCollider() const
{
	if (false == ::IsValid(InteractCollider))
	{
		return nullptr;
	}

	return InteractCollider;
}

FGameplayTag ADC1InteractBuildActor::GetInteractAbilityTag() const
{
	if (BuildState == EDC1BuildState::Construction)
	{
		return InteractAbilityTag;
	}

	return FGameplayTag::EmptyTag;
}

void ADC1InteractBuildActor::OnInteractableFocused()
{
	if (false == OnFocusedUITag.IsValid())
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	// TODO Push World UI
	//UIManager->PushScreen(OnFocusedUITag);
}

void ADC1InteractBuildActor::OnInteractableUnfocused()
{
	if (false == OnFocusedUITag.IsValid())
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	// TODO Pop World UI
}

void ADC1InteractBuildActor::InitBuild(const FDC1BuildActorData& InBuildActorData)
{
	BuildActorData = InBuildActorData;

	BuildState = InBuildActorData.RequiredWork > 0.f ? EDC1BuildState::Construction : EDC1BuildState::Completed;
}

void ADC1InteractBuildActor::ApplyState()
{
	Super::ApplyState();

	if (BuildState == EDC1BuildState::Default || false == ::IsValid(InteractCollider))
	{
		return;
	}

	ECollisionEnabled::Type EnabledType = BuildState == EDC1BuildState::Preview ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly;

	InteractCollider->SetCollisionEnabled(EnabledType);

	if (BuildState == EDC1BuildState::Construction)
	{
		BuildMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CreateTranslucentMIDs();
		SetMaterialTranslucentColor(ConstructionColor);
		SetMaterialBuildProgress();
		SetActorTickEnabled(true);
	}
	else if (BuildState == EDC1BuildState::Completed)
	{
		// Construction을 거쳐온 경우에만 MID가 남아 있음 - Flash 후 원본 머티리얼로 복귀
		if (false == BuildMIDs.IsEmpty())
		{
			FlashAmount = 1.f;
			SetActorTickEnabled(true);
		}

		OnBuildCompleted.Broadcast();

		Workers.Empty();
	}
}

void ADC1InteractBuildActor::BuildDestroy()
{
	Super::BuildDestroy();

	CancelWorkersAbility();

	Workers.Empty();
}

void ADC1InteractBuildActor::AddWorker(const ADC1Character* InWorker)
{
	if (false == ::IsValid(InWorker) || false == HasAuthority() || BuildState != EDC1BuildState::Construction)
	{
		return;
	}

	Workers.Add(InWorker);
}

void ADC1InteractBuildActor::RemoveWorker(const ADC1Character* InWorker)
{
	if (nullptr == InWorker || false == HasAuthority())
	{
		return;
	}

	Workers.Remove(InWorker);
}

void ADC1InteractBuildActor::OnRep_BuildProgress()
{
	SetMaterialBuildProgress();
}

void ADC1InteractBuildActor::BuildCompleted()
{
	if (true == ::IsValid(BuildMesh))
	{
		BuildMesh->EmptyOverrideMaterials();
	}

	BuildMIDs.Reset();

	SetActorTickEnabled(false);
}

void ADC1InteractBuildActor::CancelWorkersAbility()
{
	FGameplayTagContainer WorkTags;
	WorkTags.AddTag(DC1StateTags::State_Build_Working);

	for (TWeakObjectPtr<const ADC1Character> Worker : Workers)
	{
		const ADC1Character* WorkCharacter = Worker.Get();
		if (false == ::IsValid(WorkCharacter))
		{
			continue;
		}

		UAbilitySystemComponent* ASC = WorkCharacter->GetAbilitySystemComponent();
		if (false == ::IsValid(ASC))
		{
			continue;
		}

		ASC->CancelAbilities(&WorkTags);
	}
}

float ADC1InteractBuildActor::ComputeTotalWorkSpeed()
{
	float TotalWorkSpeed = 0.f;

	for (TSet<TWeakObjectPtr<const ADC1Character>>::TIterator Iter = Workers.CreateIterator(); Iter; ++Iter)
	{
		const ADC1Character* Worker = Iter->Get();
		if (false == ::IsValid(Worker))
		{
			Iter.RemoveCurrent();
			continue;
		}

		const UAbilitySystemComponent* ASC = Worker->GetAbilitySystemComponent();
		if (false == ::IsValid(ASC) || ASC->HasMatchingGameplayTag(DC1StateTags::State_Dead))
		{
			continue;
		}

		TotalWorkSpeed += ASC->GetNumericAttribute(UDC1AttributeSet::GetWorkSpeedAttribute());
	}

	return TotalWorkSpeed;
}

void ADC1InteractBuildActor::SetMaterialBuildProgress()
{
	int32 NumMIDs = BuildMIDs.Num();

	for (int32 i = 0; i < NumMIDs; ++i)
	{
		if (false == ::IsValid(BuildMIDs[i]))
		{
			continue;
		}

		if (false == MaterialProgressRanges.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("Material None Progress Range : %d"), i);
			continue;
		}

		const FVector2D& Range = MaterialProgressRanges[i];
		const float RangeSize = (Range.Y - Range.X);

		float MaterialProgress = 0.f;

		if (RangeSize > KINDA_SMALL_NUMBER)
		{
			MaterialProgress = FMath::Clamp(((BuildProgress - Range.X) / RangeSize), 0.f, 1.f);
		}

		BuildMIDs[i]->SetScalarParameterValue(DC1BuildMaterialParams::BuildProgress, MaterialProgress);
	}
}

void ADC1InteractBuildActor::SetMaterialFlash()
{
	for (UMaterialInstanceDynamic* MID : BuildMIDs)
	{
		if (true == ::IsValid(MID))
		{
			MID->SetScalarParameterValue(DC1BuildMaterialParams::FlashAmount, FlashAmount);
		}
	}
}
