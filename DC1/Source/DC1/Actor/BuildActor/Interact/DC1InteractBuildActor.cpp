// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerController.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Tags/State/DC1GameplayTag_State.h"
#include "Tags/Player/DC1GameplayTag_Player.h"

#include "Net/UnrealNetwork.h"

ADC1InteractBuildActor::ADC1InteractBuildActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	InteractCollider = CreateDefaultSubobject<UBoxComponent>("InteractCollider");
	InteractCollider->SetupAttachment(BuildMesh);
	InteractCollider->SetGenerateOverlapEvents(true);

	ConstructionOptions.Emplace(EDC1InteractKey::F, NSLOCTEXT("Interact", "Build", "건설"), EDC1InteractInputType::HoldAbility, DC1PlayerTags::Ability_Player_BuildWork);
	ConstructionOptions.Emplace(EDC1InteractKey::C, NSLOCTEXT("Interact", "Cancel", "취소"), EDC1InteractInputType::HoldConfirm, FGameplayTag::EmptyTag);
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

	// 건설 / 생산 처리
	if (true == Workers.IsEmpty())
	{
		SetActorTickEnabled(false);
		return;
	}

	if (false == HasAuthority())
	{
		return;
	}

	const float TotalWorkSpeed = ComputeTotalWorkSpeed();

	if (TotalWorkSpeed > 0.f)
	{
		OnWorkTick(TotalWorkSpeed * DeltaTime);
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

void ADC1InteractBuildActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	OutOptions.Reset();

	switch (BuildState)
	{
	case EDC1BuildState::Construction :
		OutOptions = ConstructionOptions;
		break;
	case EDC1BuildState::Completed :
		OutOptions = CompletedOptions;
		break;
	default:
		return;
	}
}

void ADC1InteractBuildActor::Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	if (false == ::IsValid(InInteractOwner))
	{
		return;
	}

	if (true == IsUnderConstruction() && InKey == EDC1InteractKey::C)
	{
		ADC1PlayerController* PC = InInteractOwner->GetController<ADC1PlayerController>();
		if (true == ::IsValid(PC))
		{
			UDC1InteractComponent* InteractComp = PC->GetInteractComponent();
			if (true == ::IsValid(InteractComp))
			{
				InteractComp->RequestCancelBuild(this);
			}
		}

		return;
	}

	if (BuildState == EDC1BuildState::Completed)
	{
		OnCompletedInteract(InInteractOwner, InKey);
	}
}

void ADC1InteractBuildActor::InitBuild(const FDC1BuildActorData& InBuildActorData)
{
	BuildActorData = InBuildActorData;

	BuildState = InBuildActorData.RequiredWork > 0.f ? EDC1BuildState::Construction : EDC1BuildState::Completed;
}

void ADC1InteractBuildActor::BuildDestroy()
{
	CancelWorkersAbility();
	ClearWorkers();

	Super::BuildDestroy();
}

void ADC1InteractBuildActor::ApplyState()
{
	Super::ApplyState();

	if (BuildState == EDC1BuildState::Default || false == ::IsValid(InteractCollider))
	{
		return;
	}

	ECollisionEnabled::Type EnabledType = (BuildState == EDC1BuildState::Preview || BuildState == EDC1BuildState::Destroying) ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly;

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

	OnInteractOptionsChanged.Broadcast();
}

bool ADC1InteractBuildActor::CanAcceptWorker() const
{
	return IsUnderConstruction();
}

void ADC1InteractBuildActor::OnWorkTick(float InWork)
{
	if (BuildState != EDC1BuildState::Construction)
	{
		return;
	}


	if (BuildActorData.RequiredWork > 0.f && CurrentWork < BuildActorData.RequiredWork)
	{
		CurrentWork += InWork;
		
		BuildProgress = (CurrentWork / BuildActorData.RequiredWork);

		if (BuildProgress >= 1.f)
		{
			BuildProgress = 1.f;

			SetState(EDC1BuildState::Completed);
		}

		SetMaterialBuildProgress();
	}
}

void ADC1InteractBuildActor::AddWorker(const ADC1Character* InWorker)
{
	if (false == ::IsValid(InWorker) || false == HasAuthority() || false == CanAcceptWorker())
	{
		return;
	}

	Workers.Add(InWorker);

	SetActorTickEnabled(true);
}

void ADC1InteractBuildActor::RemoveWorker(const ADC1Character* InWorker)
{
	if (false == ::IsValid(InWorker) || false == HasAuthority())
	{
		return;
	}

	Workers.Remove(InWorker);
}

void ADC1InteractBuildActor::ClearWorkers()
{
	Workers.Empty();
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
}

void ADC1InteractBuildActor::CancelWorkersAbility()
{
	FGameplayTagContainer WorkTags;
	WorkTags.AddTag(DC1StateTags::State_Build_Working);

	// 서버 환경에서 RemoveWorker로 인한 동기화 문제 방지
	TSet<TWeakObjectPtr<const ADC1Character>> CachedWorker = Workers;

	for (TWeakObjectPtr<const ADC1Character> Worker : CachedWorker)
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

		BuildMIDs[i]->SetScalarParameterValue(DC1BuildMaterialParams::BuildProgress, BuildProgress);
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
