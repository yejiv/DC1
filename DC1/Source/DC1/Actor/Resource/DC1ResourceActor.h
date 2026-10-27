// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "Interface/DC1WorkTargetInterface.h"
#include "DC1ResourceActor.generated.h"

class UDC1AbilitySystemComponent;
class UDC1AttributeSet;
class UStaticMeshComponent;
class UGeometryCollection;
class UNiagaraSystem;
class ADC1DestroyActor;
class ADC1PalCharacter;
struct FOnAttributeChangeData;

/*
- 필드 채집 액터 ( 나무, 돌 )
- 피격 -> ASC + GE
- HP 0 -> 숨김 + 콜리전 off, 리스폰 대기
*/

UENUM(BlueprintType)
enum class EDC1ResourceScaleType : uint8
{
	Fixed = 0 UMETA(DisplayName ="Fixed"),					// HP와 무관하게 메쉬 유지
	HealthScaled = 1 UMETA(DisplayName = "HealthScaled"),	// HP 비율에 따라 메쉬 스케일
};

UENUM()
enum class EDC1ResourceActorState : uint8
{
	Active = 0,
	Depleted = 1,	// 숨김 + 콜리전 off, 리스폰 대기
};

UCLASS()
class DC1_API ADC1ResourceActor : public AActor, public IAbilitySystemInterface, public IDC1WorkTargetInterface
{
	GENERATED_BODY()

public:
	ADC1ResourceActor();

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	// IDC1WorkTargetInterface override
	virtual FGameplayTag				GetWorkTypeTag() const override;
	virtual void						AddWorker(const ADC1Character* InWorker) override {}		// 데미지는 타격으로
	virtual void						RemoveWorker(const ADC1Character* InWorker) override {}
	virtual bool						AssignPalWorker(ADC1PalCharacter* InPal) override;
	virtual void						UnassignPalWorker(ADC1PalCharacter* InPal) override;
	virtual bool						IsPalWorkerAssigned(const ADC1PalCharacter* InPal) const override;
	virtual FDC1OnPalWorkersChanged&	GetOnPalWorkersChanged() override { return OnPalWorkersChanged; }
	virtual void						ApplyWorkHit(const ADC1Character* InWorker, float InMultiplier) override;

protected:
	virtual void	PostInitializeComponents() override;
	virtual void	BeginPlay() override;
	virtual void	EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 서버 - GE 적용 직후 / 클라 - Attribute 복제 직후
	void	HandleHealthChanged(const FOnAttributeChangeData& InData);

	// 서버 - Damage 메타가 적용될 때. GE 컨텍스트( 공격자 + 히트 )는 이때만 옴 → 드랍 위치 미리 계산
	void	HandleDamageApplied(const FOnAttributeChangeData& InData);

	// 서버
	void	SetResourceState(EDC1ResourceActorState InNewState);
	void	Respawn();
	void	WakeForReplication();
	void	SleepForReplication();
	FVector	GetDropOrigin(const FOnAttributeChangeData& InData) const;
	void	AccumulateDrop(float InDamage, const FVector& InOrigin);

	UFUNCTION()
	void	OnRep_ResourceState();

	void	ApplyResourceState(bool InPlayFX);
	void	ApplyScale();
	
	void	PlayDepleteFX();

	bool	IsRecentDeplete() const;
	float	GetHealthRatio() const;

	// 서버 - 고갈 시 배정 전원 해제 → 펠 Work GA 종료
	void	ClearPalWorkers();

	// TEMP v3 - ST 연결 전 검증용 ( ST 완성 후 삭제 )
	void	DebugAssignNearbyPals();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Resource")
	TObjectPtr<USceneComponent>						SceneRoot = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Resource")
	TObjectPtr<UStaticMeshComponent>				ResourceMesh = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent>			AbilitySystemComponent = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1AttributeSet>					AttributeSet = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Resource")
	EDC1ResourceScaleType							ScaleType = { EDC1ResourceScaleType::Fixed };

	UPROPERTY(EditDefaultsOnly, Category = "Resource", meta = (ClampMin = "1"))
	float											MaxHealth = { 100.f };

	UPROPERTY(EditDefaultsOnly, Category = "Resource", meta = (ClampMin = "1"))
	float											RespawnTime = { 60.f };

	UPROPERTY(EditDefaultsOnly, Category = "Resource|Destroy")
	TSubclassOf<ADC1DestroyActor>					DestroyClass = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Resource|Destroy")
	TObjectPtr<UGeometryCollection>					DestroyGeometryCollection = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Resource|Destroy")
	TObjectPtr<UNiagaraSystem>						DestroyDustFX = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Resource|Drop", meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName											DropItemID;

	// DropDamage만큼 HP가 줄 때마다 드랍 1회
	UPROPERTY(EditDefaultsOnly, Category = "Resource|Drop", meta = (ClampMin = "1"))
	float											DropDamage = { 10.f };

	// 드랍 1회 당 떨어트릴 개수( Pickup Item 1개가 가지는 개수 ) , ex) DropCountPerDrop == 3이면 DropItem 1개 Spawn -> 획득 시 3개 획득
	UPROPERTY(EditDefaultsOnly, Category = "Resource|Drop", meta = (ClampMin = "1"))
	int32											DropCountPerDrop = { 1 };

	// 이 노드가 요구하는 작업 종류 - 나무 Work.Lumbering / 돌 Work.Mining
	UPROPERTY(EditDefaultsOnly, Category = "Resource|Work", meta = (Categories = "Work"))
	FGameplayTag									WorkTypeTag;

	// Debug TEMP  - 0이면 꺼짐
	UPROPERTY(EditAnywhere, Category = "Resource|Debug", meta = (ClampMin = "0"))
	float											DebugAssignRadius = { 0.f };

private:
	UPROPERTY(ReplicatedUsing = OnRep_ResourceState)
	EDC1ResourceActorState							ResourceState = { EDC1ResourceActorState::Active };

	// 자원이 고갈된 서버 시각 -> 늦은 클라가 연출을 다시 틀지 않게
	UPROPERTY(Replicated)
	double											DepletedServerTime = { -1.0 };

	FVector											InitialMeshScale = { FVector::OneVector };
	FTimerHandle									RespawnTimer;
	FDelegateHandle									HealthChangedHandle;
	FDelegateHandle									DamageAppliedHandle;

	// 서버만 - Damage 콜백에서 계산한 이번 타격의 드랍 위치. Health 콜백이 쓰고 비운다
	FVector											PendingDropOrigin = { FVector::ZeroVector };
	bool											HasPendingDropOrigin = { false };

	float											AccumulatedDropDamage = { 0.f };

	FDC1OnPalWorkersChanged							OnPalWorkersChanged;

	// 서버만 - 배정된 펠 ( 휴식 중에도 유지 ). 정보 패널(UI) 없어서 복제 X ( Dormancy 유지 )
	TArray<TWeakObjectPtr<ADC1PalCharacter>>		PalWorkers;

	FTimerHandle									DebugAssignTimer;	// TEMP v3
};
