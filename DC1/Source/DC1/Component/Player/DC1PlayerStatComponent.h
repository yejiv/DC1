#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeSet.h"
#include "Define/DC1Enums.h"
#include "DC1PlayerStatComponent.generated.h"

struct FOnAttributeChangeData;

class UAbilitySystemComponent;

// 가용 포인트 또는 투자 횟수 변경
DECLARE_MULTICAST_DELEGATE(FDC1OnStatPointsChanged);

// 스탯 하나가 어떤 어트리뷰트를 얼마나 올리는지
USTRUCT(BlueprintType)
struct FDC1PlayerStatDefinition
{
	GENERATED_BODY()

public:
	// 패널에 표시하고 강화로 올리는 어트리뷰트 (체력·스태미나는 최대치)
	UPROPERTY(EditAnywhere)
	FGameplayAttribute DisplayAttribute;

	// 최대치형 스탯의 현재값 어트리뷰트, 그 외에는 비워둠
	UPROPERTY(EditAnywhere)
	FGameplayAttribute CurrentAttribute;

	// 포인트 1개당 증가량, 0이면 강화 불가 (방어력)
	UPROPERTY(EditAnywhere)
	float EnhanceAmount = { 0.f };
};

// 스탯별 포인트 투자 횟수 (확정된 투자와 서버로 보내는 배분 묶음에 같이 사용)
USTRUCT()
struct FDC1StatInvestment
{
	GENERATED_BODY()

public:
	int32 GetCount(EDC1PlayerStat InStat) const;
	int32 GetTotal() const;

	void AddCount(EDC1PlayerStat InStat, int32 InDelta);

	// 정의에 없는 스탯 값까지 검증해야 하므로 필드 전체를 순회
	template<typename FuncType>
	void ForEachCount(FuncType&& InFunc) const
	{
		InFunc(EDC1PlayerStat::Health, Health);
		InFunc(EDC1PlayerStat::Stamina, Stamina);
		InFunc(EDC1PlayerStat::Attack, Attack);
		InFunc(EDC1PlayerStat::WorkSpeed, WorkSpeed);
		InFunc(EDC1PlayerStat::Weight, Weight);
	}

private:
	int32* FindCount(EDC1PlayerStat InStat);

private:
	UPROPERTY()
	int32 Health = { 0 };

	UPROPERTY()
	int32 Stamina = { 0 };

	UPROPERTY()
	int32 Attack = { 0 };

	UPROPERTY()
	int32 WorkSpeed = { 0 };

	UPROPERTY()
	int32 Weight = { 0 };
};

// 레벨업 스탯 포인트로 플레이어 스탯을 강화 (서버 권한)
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1PlayerStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FDC1OnStatPointsChanged OnStatPointsChanged;

public:
	UDC1PlayerStatComponent();

public:
	const FDC1PlayerStatDefinition* GetStatDefinition(EDC1PlayerStat InStat) const { return StatDefinitions.Find(InStat); }
	TArray<EDC1PlayerStat>	GetStats() const;

	int32	GetAvailableStatPoints() const;
	int32	GetInvestedCount(EDC1PlayerStat InStat) const { return InvestedStats.GetCount(InStat); }
	float	GetEnhanceAmount(EDC1PlayerStat InStat) const;
	bool	IsEnhanceableStat(EDC1PlayerStat InStat) const;
	bool	CanEnhanceStats(const FDC1StatInvestment& InAllocation) const;

public:
	void	RequestEnhanceStats(const FDC1StatInvestment& InAllocation);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void	Server_EnhanceStats(const FDC1StatInvestment& InAllocation);

	UFUNCTION()
	void	OnRep_InvestedStats();

	// 강화(+횟수)와 이후 초기화(-투자 횟수)가 같이 쓰는 어트리뷰트 반영 함수
	void	ApplyStatInvestment(EDC1PlayerStat InStat, int32 InDeltaCount);

	void	HandleLevelChanged(const FOnAttributeChangeData& InData);

	UAbilitySystemComponent* GetAbilitySystemComponent() const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_InvestedStats)
	FDC1StatInvestment InvestedStats;

	// 패널에 표시할 스탯과 강화 규칙
	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	TMap<EDC1PlayerStat, FDC1PlayerStatDefinition> StatDefinitions;
};
