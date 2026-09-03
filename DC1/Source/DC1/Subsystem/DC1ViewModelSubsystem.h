// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1ViewModelSubsystem.generated.h"

class ADC1PlayerState;

/**
 * ViewModel의 생성과 조회, 소스 배선을 담당하는 LocalPlayer 스코프 서브시스템.
 * VM은 요청 시점에 생성되므로 새 VM을 추가할 때 이 파일을 수정할 필요가 없다.
 */
UCLASS()
class DC1_API UDC1ViewModelSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/** 없으면 생성해서 반환. 위젯과 PlayerController 모두 이 함수로 VM에 접근한다 */
	template<typename T>
	T* GetViewModel()
	{
		static_assert(TIsDerivedFrom<T, UDC1ViewModelBase>::Value, "T must derive from UDC1ViewModelBase");
		return Cast<T>(FindOrCreateViewModel(T::StaticClass()));
	}

	/** PlayerState 확정 시점에 호출. 생성되어 있는 모든 VM에 소스를 물린다 */
	void BindAll(ADC1PlayerState* InPlayerState);

	/** 소스가 파괴되기 전에 호출. 모든 VM의 구독을 끊는다 */
	void ClearAll();

private:
	UDC1ViewModelBase* FindOrCreateViewModel(TSubclassOf<UDC1ViewModelBase> InViewModelClass);

private:
	UPROPERTY()
	TMap<TSubclassOf<UDC1ViewModelBase>, TObjectPtr<UDC1ViewModelBase>> ViewModels;

	// BindAll 이후에 생성된 VM도 즉시 바인딩하기 위해 캐싱한다
	TWeakObjectPtr<ADC1PlayerState> CachedPlayerState = { nullptr };
};