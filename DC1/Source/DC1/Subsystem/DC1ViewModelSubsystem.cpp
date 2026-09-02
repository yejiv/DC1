// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/DC1ViewModelSubsystem.h"

#include "Character/Player/DC1PlayerState.h"

void UDC1ViewModelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDC1ViewModelSubsystem::Deinitialize()
{
	ClearAll();

	Super::Deinitialize();
}

void UDC1ViewModelSubsystem::BindAll(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	CachedPlayerState = InPlayerState;

	for (auto& Pair : ViewModels)
	{
		if(UDC1ViewModelBase* VM = Pair.Value)
			VM->BindSource(CachedPlayerState.Get());
	}
}

void UDC1ViewModelSubsystem::ClearAll()
{
	for (auto& Pair : ViewModels)
	{
		if(UDC1ViewModelBase* VM = Pair.Value)
			VM->ClearSource();
	}

	CachedPlayerState.Reset();
}

UDC1ViewModelBase* UDC1ViewModelSubsystem::FindOrCreateViewModel(TSubclassOf<UDC1ViewModelBase> InViewModelClass)
{
	// TODO : 맵에서 찾고, 없으면 NewObject로 생성해 등록
	// TODO : CachedPlayerState가 유효하면 생성 직후 BindSource까지 호출
	UDC1ViewModelBase* VM = ViewModels.FindRef(InViewModelClass);
	if (false == IsValid(VM))
	{
		VM = NewObject<UDC1ViewModelBase>(this, InViewModelClass);
		ViewModels.Add(InViewModelClass, VM);

		if (CachedPlayerState.IsValid())
		{
			VM->BindSource(CachedPlayerState.Get());
		}
	}

	return VM;
}