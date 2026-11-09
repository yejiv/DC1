// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "Define/DC1Enums.h"
#include "DC1PalData.generated.h"

class UAnimMontage;
class UAnimSequence;
class UTexture2D;
class USkeletalMesh;

class ADC1PalProjectile;
class ADC1PalCharacter;

/**
 *
 */
USTRUCT(BlueprintType)
struct FDC1PalMontageSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	TArray<TSoftObjectPtr<UAnimMontage>>	Montages;

	TSoftObjectPtr<UAnimMontage> PickRandomMontage() const
	{
		return (0 < Montages.Num()) ? Montages[FMath::RandRange(0, Montages.Num() - 1)] : nullptr;
	}
};

// 팰 미리보기 설정 (메쉬 / Idle 애니 / 카메라). 카메라 값은 PreviewActor의 Camera 변수와 같은 의미
USTRUCT(BlueprintType)
struct FDC1PalPreviewSettings
{
	GENERATED_BODY()

	// 프리뷰용 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Mesh")
	TSoftObjectPtr<USkeletalMesh>		Mesh = { nullptr };

	// 원본 메쉬를 따라갈 수 없으므로 루프 재생할 Idle 애니
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Anim")
	TSoftObjectPtr<UAnimSequence>	IdleAnim = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Camera", meta = (ClampMin = "10"))
	float							CameraDistance = { 50.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Camera")
	float							CameraHeight = { 0.f };

	// 양수면 화면 기준 오른쪽
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Camera")
	float							CameraRight = { 0.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Camera")
	float							CameraYaw = { 0.f };

	// 양수면 내려다봄
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Camera")
	float							CameraPitch = { 0.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview|Camera", meta = (ClampMin = "5", ClampMax = "170"))
	float							CameraFOV = { 90.f };
};

// 드랍
USTRUCT(BlueprintType)
struct FDC1PalDropDef
{
	GENERATED_BODY()

	// DT_ItemData 의 ItemID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName							ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32							MinCount = { 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32							MaxCount = { 1 };

	// 0~1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float							Chance = { 1.f };
};

UCLASS()
class DC1_API UDC1PalData : public UDataAsset
{
	GENERATED_BODY()

public:
#pragma region Species
	// 이 종을 소환할 때 쓸 폰. 데이터에서 액터로 되돌릴 떄 필요
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Species")
	TSoftClassPtr<ADC1PalCharacter>	PalClass;

	// 속성. 상성 계산과 UI 색/아이콘 구분에 쓴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Species")
	EDC1PalElement					Element = { EDC1PalElement::Neutral };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Species")
	EDC1PalElement					Element2 = { EDC1PalElement::Neutral };
#pragma endregion

#pragma region UI
	// 파티 HUD 등에서 쓰는 종족 아이콘. 개체가 아니라 종 단위 값이라 여기에 둔다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D>		Icon;

	// 화면에 띄울 종 이름. 비면 Nickname 폴백도 없으니 종마다 채운다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText							DisplayName;
#pragma endregion

#pragma region Locomotion
	// 달리기 (원작 RunSpeed). 추격·도주·추종·작업 이동·탑승 평소 속도.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							RunSpeed = { 400.f };

	// 걷기 (원작 WalkSpeed). 배회·잠자리 이동
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							WalkSpeed = { 150.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							RotationRateYaw = { 360.f };
#pragma endregion

#pragma region Ride
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride")
	bool							bRideable = { false };

	// 스켈레톤 소켓
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	FName							RideSocketName = { TEXT("RideMarker") };

	// 탑승 질주 (원작 RideSprintSpeed). 질주 키를 누르는 동안. 평소 탑승 속도는 RunSpeed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable", ClampMin = "0.0"))
	float							RideSprintSpeed = { 700.f };

	// 최대 스태미나 (원작 Stamina). 탑승 질주가 쓴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable", ClampMin = "0.0"))
	float							Stamina = { 100.f };

	// 올라타기 시작 / 내리기 끝 위치. 팰 방향 (X 앞, Y 오른쪽). XY 는 탑승 소켓 기준, Z 는 팰 발밑 기준
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	TMap<EDC1RideDirection, FVector>	RideMountOffsets = {
		{ EDC1RideDirection::Front,	FVector(150.f, 0.f, 0.f) },
		{ EDC1RideDirection::Left,	FVector(0.f, -120.f, 0.f) },
		{ EDC1RideDirection::Right,	FVector(0.f, 120.f, 0.f) },
		{ EDC1RideDirection::Rear,	FVector(-150.f, 0.f, 0.f) },
	};

	// 탄 사람이 트는 몽타주. 옆 ↔ 등 이동 구간은 몽타주의 DC1 Ride Move 노티파이로 찍는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	TMap<EDC1RideDirection, TSoftObjectPtr<UAnimMontage>>	RideMountMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	TSoftObjectPtr<UAnimMontage>	RideDismountMontage;

	// 내리는 쪽. 탄 방향과 상관없이 항상 여기로 내린다 (자리는 RideMountOffsets)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	EDC1RideDirection				RideDismountDirection = { EDC1RideDirection::Left };

	// 탄 사람 몽타주 길이에 맞춘다. 서버는 이 시간 뒤에 탑승·하차를 끝낸다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable", ClampMin = "0.01"))
	float							RideMountDuration = { 1.2f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable", ClampMin = "0.01"))
	float							RideDismountDuration = { 1.f };
#pragma endregion

#pragma region Stats
	// 레벨당 성장 계수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float							HealthStat = { 100.f };

	// 야생으로 나올 때의 레벨 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
	int32							MinLevel = { 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
	int32							MaxLevel = { 1 };
#pragma endregion

#pragma region Work
	// 거점 작업 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work", meta = (ClampMin = "0.0"))
	float							WorkSpeedStat = { 10.f };

	// 작업 적성. 없는 작업은 넣지 않는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work", meta = (Categories = "Work"))
	TMap<FGameplayTag, int32>		WorkSuitability;
#pragma endregion

#pragma region Ability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Asset"))
	FGameplayTag					AbilityDataTag;

	// 종별 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Anim.Pal"))
	TMap<FGameplayTag, FDC1PalMontageSet>	Montages;
#pragma endregion

#pragma region Idle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Idle", meta = (Categories = "Anim.Pal.Idle"))
	TArray<FGameplayTag>			IdleMontageTags;
#pragma endregion

#pragma region Combat
	// AI가 쓸 스킬 후보.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Pal.Skill"))
	TArray<FGameplayTag>			SkillTags;

	// 성장 계수. 최종값은 GetAttackPowerAtLevel() 이 계산
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float							AttackStat = { 100.f };

	// 성장 계수. 최종값은 GetDefensePowerAtLevel() 이 계산
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float							DefenseStat = { 100.f };

	// 이 종이 어떤 탄을 쏘는가. 키는 스킬 태그. 메시/속도/속성은 투사체 BP가 갖는다. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Ability.Pal"))
	TMap<FGameplayTag, TSubclassOf<ADC1PalProjectile>>	Projectiles;
#pragma endregion

#pragma region Capture
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Capture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float							CaptureRate = { 0.5f };
#pragma endregion

#pragma region Drop
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TArray<FDC1PalDropDef>			Drops;
#pragma endregion

#pragma region Alpha
	// 알파 개체에만 쓰는 배율. 1 = 일반 개체와 같다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alpha", meta = (ClampMin = "0.1"))
	float							AlphaScale = { 1.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alpha", meta = (ClampMin = "0.1"))
	float							AlphaHealthRate = { 1.f };

	// 야생일 때만. 잡은 알파에는 안 건다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alpha", meta = (ClampMin = "0.0"))
	float							AlphaReceiveDamageRate = { 1.f };

	// 처치·포획 경험치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alpha", meta = (ClampMin = "0.0"))
	float							AlphaExpRate = { 1.f };

	// CaptureRate 에 곱한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alpha", meta = (ClampMin = "0.0"))
	float							AlphaCaptureRate = { 1.f };
#pragma endregion

#pragma region AI
	// 야생일 때 플레이어를 보면 어떻게 하나
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	EDC1PalTemperament				Temperament = { EDC1PalTemperament::Timid };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							SightRadius = { 1500.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							LoseSightRadius = { 2000.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							PeripheralVisionAngleDegrees = { 70.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							StimulusMaxAge = { 5.f };

	// 추격 유지 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							TargetMemoryDuration = { 3.f };

	// 시야에서 놓쳐도 따라가는 시간. 보이는 동안은 계속 갱신
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							ProvokeMemoryDuration = { 10.f };

	// 스폰 지점 기준 배회 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Wander")
	float							WanderRadius = { 800.f };
#pragma endregion

#pragma region Preview
	// UI 미리보기(팰 정보 화면 등)에서 쓰는 메쉬 / 애니 / 카메라
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Preview")
	FDC1PalPreviewSettings			PreviewSettings;
#pragma endregion

public:
	TSoftObjectPtr<UAnimMontage> FindMontageByTag(const FGameplayTag& MontageTag) const
	{
		const FDC1PalMontageSet* Found = Montages.Find(MontageTag);
		return (nullptr != Found) ? Found->PickRandomMontage() : nullptr;
	}

	TSubclassOf<ADC1PalProjectile> FindProjectileByTag(const FGameplayTag& SkillTag) const
	{
		const TSubclassOf<ADC1PalProjectile>* Found = Projectiles.Find(SkillTag);
		return (nullptr != Found) ? *Found : nullptr;
	}

	int32 GetWorkSuitabilityLevel(const FGameplayTag& InWorkType) const
	{
		if (false == InWorkType.IsValid())
		{
			return 0;
		}

		for (const TPair<FGameplayTag, int32>& Pair : WorkSuitability)
		{
			if (true == InWorkType.MatchesTag(Pair.Key))
			{
				return Pair.Value;
			}
		}

		return 0;
	}

	bool CanDoWork(const FGameplayTag& InWorkType) const
	{
		return GetWorkSuitabilityLevel(InWorkType) > 0;
	}
};
