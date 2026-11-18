// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DC1ViewModelBase.generated.h"

class ADC1PlayerState;

/**
 * 모든 ViewModel의 베이스 클래스.
 * 생성과 수명은 UDC1ViewModelSubsystem이 관리하고, 소스 배선은 파생 클래스가 스스로 한다.
 */
UCLASS(Abstract)
class DC1_API UDC1ViewModelBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * PlayerState가 확정된 시점에 호출된다.
	 * 파생 VM이 필요한 소스(ASC, 인벤토리 컴포넌트 등)를 PlayerState에서 직접 꺼내 구독한다.
	 * 구독 직후 현재 값을 한 번 브로드캐스트해야 위젯 생성 순서와 무관해진다.
	 */
	virtual void BindSource(ADC1PlayerState* InPlayerState) PURE_VIRTUAL(UDC1ViewModelBase::BindSource, );

	/** 소스 구독 해제. 레벨 이동 등으로 소스가 파괴되기 전에 호출된다 */
	virtual void ClearSource() PURE_VIRTUAL(UDC1ViewModelBase::ClearSource, );
};