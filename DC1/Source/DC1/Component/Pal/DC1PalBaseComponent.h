// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalBaseComponent.generated.h"

class ADC1PalCharacter;
class ADC1PalBoxActor;
class APlayerState;

// 나와 있는 거점 팰이 지금 하는 일. 일·잠 태그는 서버에만 있어 클라 표시용으로 따로 복제한다
UENUM()
enum class EDC1BasePalActivity : uint8
{
	None,
	Working,
	Sleeping,
};

USTRUCT()
struct FDC1BasePalActivity
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid					PalID;

	UPROPERTY()
	EDC1BasePalActivity		Activity = { EDC1BasePalActivity::None };

	bool operator==(const FDC1BasePalActivity& InOther) const { return PalID == InOther.PalID && Activity == InOther.Activity; }
};

/*
	서버 공용 팰 거점 정보 - 서버 당 거점 1개
	Server RPC를 두지 말 것 -> Owner가 PalBox(PalBox는 Owner 없음, 서버 스폰 액터) -> RPC 불발
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1PalBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1PalBaseComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	TArray<FDC1PalInstance>&		GetBasePals() { return BasePals; }
	const TArray<FDC1PalInstance>&	GetBasePals() const { return BasePals; }
	int32							GetBaseCapacity() const { return BaseCapacity; }

	// 나와 있지 않거나 아무것도 안 하면 None ( 거점 정보 HUD )
	EDC1BasePalActivity				FindActivity(const FGuid& InPalID) const;
	// PalBox OnPlaced에서 호출
	void							InitBase();
	// PalBox EndPlay에서 호출
	void							ReleaseBase(APlayerState* InDestroyer);

	// 거점 칸과 월드 팰을 맞춤 -> 없는 건 꺼내고, 칸에 없는 건 접는다 ( 서버 )
	void							RefreshBasePals();

	// 거점 제거 팰을 Swap 전에 접어 체력을 칸에 기록 ( 서버 )
	void							FoldBasePal(const FGuid& InPalID);

private:
	UFUNCTION()
	void	OnRep_BasePals();

	UFUNCTION()
	void	OnRep_BasePalActivities();

private:
	// 거점 회복 타이머
	void	HealBasePals();

	// 나와 있는 거점 팰의 일·잠 태그를 복제 배열에 옮김 ( 서버, 회복 타이머마다 )
	void	RefreshActivities();

	// 접힌 팰을 같은 PalID 칸에 덮음
	void	StoreFoldedPal(const FDC1PalInstance& InFolded);

	// Owner가 GS에 등록한 거점인가 확인, GS가 없거나 다르면 nullptr
	ADC1PalBoxActor*	GetActiveBase() const;

	void	NotifyBaseChanged() const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_BasePals)
	TArray<FDC1PalInstance>		BasePals;
	UPROPERTY(ReplicatedUsing = OnRep_BasePalActivities)
	TArray<FDC1BasePalActivity>	BasePalActivities;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Base", meta = (ClampMin = "1"))
	int32						BaseCapacity = { 5 };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Base", meta = (ClampMin = "1"))
	float						BaseHealInterval = { 1.f };

	// 거점 소환 팰 (서버 전용)
	UPROPERTY()
	TArray<TObjectPtr<ADC1PalCharacter>> BaseSpawnedPals;

private:
	FTimerHandle BaseHealTimer;
};
