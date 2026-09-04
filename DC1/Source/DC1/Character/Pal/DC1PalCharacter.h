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

DECLARE_MULTICAST_DELEGATE(FDC1OnPalDied);

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

	// 진영 변경. 포획으로 길들일 때 쓴다. 서버 전용
	UFUNCTION(BlueprintCallable, Category = "Pal|Team")
	void								SetTeam(EDC1Team NewTeam);

	const UDC1PalData*					GetPalData() const { return PalData; }
	virtual UAbilitySystemComponent*	GetAbilitySystemComponent() const override;

	void								SetOrientToMovement(bool Enable);
	virtual void						GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

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

protected:
	virtual void						PostInitializeComponents() override;

	virtual void						OnTeamChanged();

	UFUNCTION()
	void								OnRep_Team();

	void								ApplyPalData();

	// 어빌리티 취소 + AI 정지 + 지각 소스 해제. 사망과 포획이 공유한다
	void								StopAndUnregister();

	void								HandleDamaged(const struct FOnAttributeChangeData& Data);

	// 래그돌, AI 정지, 퍼셉션 해제
	virtual void						OnDeath() override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1PalAttributeSet> PalAttributeSet = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pal", meta = (Categories = "Pal.Species"))
	FGameplayTag SpeciesTag;

	UPROPERTY(Transient) //디스크에 저장X
	TObjectPtr<const UDC1PalData> PalData = { nullptr };

	UPROPERTY(Transient)
	FGuid InstanceID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Team, Category = "Pal|Team")
	EDC1Team Team = { EDC1Team::Wild };
};
