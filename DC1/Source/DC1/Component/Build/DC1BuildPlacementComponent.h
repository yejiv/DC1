// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Define/DC1Enums.h"
#include "Define/DC1Structs.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1BuildPlacementComponent.generated.h"

struct FDC1BuildActorData;
struct FInputActionValue;
struct FDC1BuildSnapInput;

class ADC1BuildActor;
class UDC1BuildSnapSolver;

DECLARE_MULTICAST_DELEGATE_TwoParams(FDC1OnBuildModeChanged, EDC1BuildMode, const FGameplayTag&);
DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnBuildBlockReasonChanged, EDC1BuildBlockReason);
DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnDismantleTargetChanged, ADC1BuildActor*);

UCLASS()
class DC1_API UDC1BuildPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDC1BuildPlacementComponent();

protected:
	virtual void		BeginPlay() override;
	virtual void		EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void		TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FDC1OnBuildModeChanged			OnBuildModeChanged;
	FDC1OnBuildBlockReasonChanged	OnBlockReasonChanged;
	FDC1OnDismantleTargetChanged	OnDismantleTargetChanged;

public:
	bool					StartPlacement(const FGameplayTag& InBuildTag);
	bool					StartDismantle();
	void					CancelBuildMode();

	EDC1BuildMode			GetBuildMode() const { return CurrentMode; }
	EDC1BuildBlockReason	GetBlockReason() const { return CurrentBlockReason; }
	const FGameplayTag&		GetCurrentBuildTag() const { return CurrentBuildTag; }
	ADC1BuildActor*			GetDismantleTarget() const { return DismantleTarget.Get(); }

private:
	UFUNCTION(Server, Reliable)
	void				Server_PlaceBuild(const FGameplayTag& InBuildTag, const FTransform& InTransform);

	UFUNCTION(Server, Reliable)
	void				Server_DismantleBuild(ADC1BuildActor* InTarget);

	void				CacheBuildRows();
	
	const FDC1BuildRow* FindBuildRow(const FGameplayTag& InBuildTag) const;
	FDC1BuildActorData	MakeBuildActorData(const FDC1BuildRow& InRow) const;
	ADC1BuildActor*		SpawnBuildActor(const FDC1BuildRow& InRow, const FTransform& InTransform, bool InIsPreview);

	void				CancelPlacement();
	void				ConfirmPlacement();

	void				CancelDismantle();
	void				ConfirmDismantle();

	bool				TraceSurface(FHitResult& OutHit) const;

	void				SetUpBuildInput();
	void				SetBuildMode(bool InEnabled);

	void				OnConfirmInput();
	void				OnBackInput();
	void				OnExitInput();
	void				OnToggleModeInput();
	void				OnRotateInput(const FInputActionValue& Value);
	void				OnSnapPressed();
	void				OnSnapReleased();

	void				ReturnToBuildMenu();
	void				RefreshBuildMode();
	void				SetBlockReason(EDC1BuildBlockReason InReason);

	static FGameplayTag	MakeBuildTabTag(const FGameplayTag& InBuildTag);

	void				UpdateDismantleTarget();

	const APawn*		GetOwnerPawn() const;
	float				GetPlaceYaw() const;
	float				GetControlYaw() const;
	FVector				GetAimPoint(const APawn* InOwnerPawn) const;
	FVector				GetPlacePoint(const APawn* InOwnerPawn) const;

	bool				GetViewRay(FVector& OutStart, FVector& OutEnd) const;

	// 솔버 호출 한 번에 필요한 값을 모은다 ( 시야 광선 트레이스를 포함하므로 필요할 때만 부를 것 )
	FDC1BuildSnapInput	MakeSnapInput() const;

	bool				IsInBuildRange(const ADC1BuildActor* InBuild, const float InMaxDistance, const float InMarginFactor = 1.f) const;

	ADC1BuildActor*		TraceBuildActorByView(FVector& OutRayStart, FVector& OutRayEnd, FHitResult& OutHit) const;

	float				ComputePillarHeightOffset() const;

	bool				IsOwnerDownOrDead() const;


private:
	// 배치 거리
	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		PlaceDistance = { 300.f };

	// 설치 최대 거리
	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		MaxDistance = { 500.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RangeUpHeight = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RangeDownHeight = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RotationStep = { 15.f };

	UPROPERTY(EditAnywhere, Category = "Build|Snap")
	float		SnapSearchRadius = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Snap")
	float		SnapAimInflate = { 30.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		MaxPillarDegree = { 60.f };

private:
	UPROPERTY()
	TObjectPtr<ADC1BuildActor>				PreviewActor = { nullptr };

	// 스냅 계산 전담 ( BeginPlay에서 생성 )
	UPROPERTY()
	TObjectPtr<UDC1BuildSnapSolver>			SnapSolver = { nullptr };

	TMap<FGameplayTag, const FDC1BuildRow>	BuildRowsByTag;

	FGameplayTag							CurrentBuildTag = { FGameplayTag::EmptyTag };

	// 돌아가기, 모드전환용
	FGameplayTag							LastBuildTag = { FGameplayTag::EmptyTag };

	EDC1BuildMode							CurrentMode = { EDC1BuildMode::Default };
	EDC1BuildBlockReason					CurrentBlockReason = { EDC1BuildBlockReason::None };

	float									CurrentYaw = { 0.f };
	float									DistanceMarginFactor = { 1.1f };

	// 이번 틱에 자동 스냅이 걸렸나 ( 회전 스텝·표면 검사 분기에 쓴다 )
	bool									IsAutoSnapped = { false };

	bool									IsDismantling = { false };
	TWeakObjectPtr<ADC1BuildActor>			DismantleTarget;
};
