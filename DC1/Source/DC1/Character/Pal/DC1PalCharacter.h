// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "AI/DC1TeamTypes.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalCharacter.generated.h"

class UDC1AbilitySystemComponent;
class UDC1PalAttributeSet;
class UDC1PalData;
class UDC1PalRegistryData;
class UWidgetComponent;

DECLARE_MULTICAST_DELEGATE(FDC1OnPalDied);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDC1OnPalHealthChanged, float /*Current*/, float /*Max*/);

/**
 *
 */
UCLASS()
class DC1_API ADC1PalCharacter : public ADC1Character, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ADC1PalCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IGenericTeamAgentInterface
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	EDC1Team							GetTeam() const { return Team; }

	const UDC1PalData*					GetPalData() const { return PalData; }
	virtual UAbilitySystemComponent*	GetAbilitySystemComponent() const override;

	void								SetOrientToMovement(bool Enable);
	virtual void						GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	// AttributeSet 의 Level 은 float 이라 표시용으로 정수화해서 돌려준다
	int32								GetLevel() const;
	EDC1PalGender						GetGender() const { return PalInstance.Gender; }

	// 현재 레벨 누적 경험치
	float								GetExperience() const { return PalInstance.Experience; }

	// 경험치 획득. 서버 전용
	void								GainExperience(int32 InAmount);

	// 속성
	EDC1PalElement						GetElement() const;
	EDC1PalElement						GetElement2() const;

	// 파티가 이 액터를 슬롯과 대조할 때 쓴다. 야생이면 비어 있다
	const FGuid&						GetInstanceID() const { return InstanceID; }

#pragma region Capture
	float								GetHealthRatio() const;
	float								GetCaptureRate() const;

	FDC1PalInstance						Fold(); // 데이터화
	
	void								OnCaptureFailed(AActor* InThrower);// 포획 실패
	void								BeginCapture();// 스피어에 빨려 들어감. 결과까지 액터 유지
	void								CancelCapture();// BeginCapture 되돌리기

	static ADC1PalCharacter*			SpawnFromInstanceData(UWorld* InWorld, const FDC1PalInstance& InInstance, const FTransform& InTransform);
#pragma endregion

	// 죽었다. 파티가 받아서 자동 회수한다. OnDeath의 맨 끝에서 발화한다
	FDC1OnPalDied						OnPalDied;

	// 체력이 변했다. 머리 위 HP바가 구독한다
	FDC1OnPalHealthChanged				OnPalHealthChanged;

protected:
	virtual void						PostInitializeComponents() override;

	virtual void						BeginPlay() override;

	virtual void						OnTeamChanged();

	UFUNCTION()
	void								OnRep_Team();

	void								ApplyPalData();

	// PassiveTags 를 GE 로 바꿔 붙인다
	void								ApplyPassives(const UDC1PalRegistryData& InRegistry);

	// 어빌리티 취소 + AI 정지 + 지각 소스 해제. 사망과 포획이 공유한다
	void								StopAndUnregister();

	void								HandleDamaged(const struct FOnAttributeChangeData& Data);

	void								HandleHealthChangedForUI(const struct FOnAttributeChangeData& Data);

	// 레벨 변경 → HP바 갱신
	void								HandleLevelChangedForUI(const struct FOnAttributeChangeData& Data);

	void								UpdateHealthWidgetVisibility(); // 위젯을 껐다 켠다. 

	// 래그돌, AI 정지, 퍼셉션 해제
	virtual void						OnDeath() override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthWidgetComponent = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ClampMin = "0.0"))
	float HealthWidgetVisibleDistance = { 2000.f };

	// 가시성 검사 주기
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ClampMin = "0.05"))
	float HealthWidgetCheckInterval = { 0.2f };

	FTimerHandle HealthWidgetVisibilityTimer;

	UPROPERTY()
	TObjectPtr<UDC1PalAttributeSet> PalAttributeSet = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pal", meta = (Categories = "Pal.Species"))
	FGameplayTag SpeciesTag;

	UPROPERTY(Transient) //디스크에 저장X
	TObjectPtr<const UDC1PalData> PalData = { nullptr };

	UPROPERTY(Transient)
	FGuid InstanceID;

	// 이 팰의 데이터. 소환될 때 원본을 통째로 받아두고 회수할 때 그대로 돌려준다.
	UPROPERTY(Transient)
	FDC1PalInstance PalInstance;

	// 마지막 가해자
	TWeakObjectPtr<AActor> LastDamageCauser;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pal|Level")
	bool bOverrideLevel = { false };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pal|Level", meta = (EditCondition = "bOverrideLevel", ClampMin = "1"))
	int32 LevelOverride = { 1 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Team, Category = "Pal|Team")
	EDC1Team Team = { EDC1Team::Wild };
};
