// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "Tags/DC1GamePlayTags.h"
#include "DC1CrosshairViewModel.generated.h"

class UAbilitySystemComponent;

struct FGameplayEventData;

// 조준점 노출 여부. 조건이 실제로 뒤집힌 순간에만 온다
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCrosshairVisibilityChanged, bool /*bVisible*/);

// 발사 순간 1회. Strength는 무기별 반동 세기(FGameplayEventData::EventMagnitude)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCrosshairFired, float /*Strength*/);

/**
 * 조준점 HUD용 ViewModel.
 * 소스는 PlayerState의 ASC 하나이고, 거기서 두 가지를 번역해 위젯에 넘긴다.
 *   - 상태: State.ZoomIn + State.Weapon.Bow 태그 카운트 -> 노출 여부
 *   - 이벤트: Event.Player.Fired 게임플레이 이벤트 -> 반동 연출 신호 (ASC::GenericGameplayEventCallbacks)
 *
 * 노출 조건식은 EvaluateVisibility 한 곳에만 둔다. 위젯은 ASC도 태그도 모른다.
 */
UCLASS()
class DC1_API UDC1CrosshairViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	FOnCrosshairVisibilityChanged	OnVisibilityChanged;
	FOnCrosshairFired					OnFired;

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	// 위젯이 BindSource보다 늦게 붙는 경우가 있으므로, 구독과 별개로 현재 값을 읽어갈 수 있어야 한다
	bool IsVisible() const;

private:
	// 태그마다 따로 불리므로, 콜백에서는 조건식 전체를 다시 평가한다
	void HandleStateTagChanged(const FGameplayTag InTag, int32 InNewCount);

	// SendGameplayEventToActor는 리플리케이트되지 않는다. 이 콜백은 이벤트가 발생한 머신에서만 돈다
	void HandleFiredEvent(const FGameplayEventData* InPayload);

private:
	// State.ZoomIn && State.Weapon.Bow
	bool EvaluateVisibility() const;

	// 값이 실제로 바뀐 경우에만 브로드캐스트한다. 두 태그가 같은 프레임에 붙어도 한 번만 나간다
	void RefreshVisibility();

	void RegisterStateHandle(const FGameplayTag& InTag);

private:
	TWeakObjectPtr<UAbilitySystemComponent>	AbilitySystemComponent = { nullptr };

	// 태그 이벤트는 RemoveAll이 없다. 핸들을 들고 있어야만 뗄 수 있다
	TMap<FGameplayTag, FDelegateHandle>			StateTagHandles;

	// GenericGameplayEventCallbacks는 정확히 일치하는 태그만 받는다.
	// 해제 헬퍼가 따로 없으므로 맵에서 직접 Remove(Handle) 해야 한다
	FDelegateHandle										FiredEventHandle;
	FGameplayTag											FiredEventTag = { DC1PlayerTags::Event_Player_Fired };

	bool														bVisible = { false };
};
