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
	bool				StartPlacement(const FGameplayTag& InBuildTag);
	bool				StartDismantle();

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
	void				OnCancelInput();
	void				OnRotateInput(const FInputActionValue& Value);
	void				OnSnapPressed();
	void				OnSnapReleased();

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

	float									CurrentYaw = { 0.f };
	bool									CanBuilding = { false };
	float									DistanceMarginFactor = { 1.1f };

	// 이번 틱에 자동 스냅이 걸렸나 ( 회전 스텝·표면 검사 분기에 쓴다 )
	bool									IsAutoSnapped = { false };

	bool									IsDismantling = { false };
	TWeakObjectPtr<ADC1BuildActor>			DismantleTarget;
};
