// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Define/DC1Enums.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1BuildPlacementComponent.generated.h"

struct FDC1BuildActorData;
struct FInputActionValue;
struct FDC1BuildSnapSocket;

class ADC1BuildActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1BuildPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDC1BuildPlacementComponent();

public:
	const float			GetMaxPlaceDistance() const;

protected:
	virtual void		BeginPlay() override;
	virtual void		EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void		TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	bool				StartPlacement(const FGameplayTag& InBuildTag);
	bool				IsInPlaceRange(const FVector& InLocation) const;
	
private:
	UFUNCTION(Server, Reliable)
	void				Server_PlaceBuild(const FGameplayTag& InBuildTag, const FTransform& InTransform);

	void				CacheBuildRows();
	
	const FDC1BuildRow* FindBuildRow(const FGameplayTag& InBuildTag) const;
	FDC1BuildActorData	MakeBuildActorData(const FDC1BuildRow& InRow) const;
	ADC1BuildActor*		SpawnBuildActor(const FDC1BuildRow& InRow, const FTransform& InTransform, bool InIsPreview);

	void				CancelPlacement();
	void				ConfirmPlacement();

	bool				TraceSurface(FHitResult& OutHit) const;

	void				SetUpBuildInput();
	void				SetBuildMode(bool InEnabled);

	void				OnRotateInput(const FInputActionValue& Value);
	void				OnSnapPressed();
	void				OnSnapReleased();
	void				ClearSnap();

	const APawn*		GetOwnerPawn() const;
	float				GetPlaceYaw() const;
	float				GetControlYaw() const;
	FVector				GetAimPoint(const APawn* InOwnerPawn) const;
	FVector				GetPlacePoint(const APawn* InOwnerPawn) const;

	bool				GetViewRay(FVector& OutStart, FVector& OutEnd) const;
	void				ChangeSnapConfirmTarget();
	
	bool				FindSnapFace(FDC1BuildSnapSocket& OutSnapSocket, const ADC1BuildActor*& OutTarget) const;
	bool				FindSnapFaceByRay(FDC1BuildSnapSocket& OutSnapSocket, const ADC1BuildActor*& OutTarget) const;
	bool				FindSnapFaceByRange(FDC1BuildSnapSocket& OutSnapSocket, const ADC1BuildActor*& OutTarget) const;
	void				FindSnapCandidates(const FVector& InAimPoint, TSet<ADC1BuildActor*>& OutCandidates) const;

private:
	// 배치 거리
	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		PlaceDistance = { 300.f };

	// 최대 배치 거리 ( Snap )
	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		MaxPlaceDistance = { 1500.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RangeUpHeight = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RangeDownHeight = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RotationStep = { 15.f };

	UPROPERTY(EditAnywhere, Category = "Build|Snap")
	float		SnapSearchRadius = { 600.f };

	
private:
	UPROPERTY()
	TObjectPtr<ADC1BuildActor>				PreviewActor = { nullptr };

	TMap<FGameplayTag, const FDC1BuildRow>	BuildRowsByTag;

	FGameplayTag							CurrentBuildTag = { FGameplayTag::EmptyTag };

	float									CurrentYaw = { 0.f };
	bool									CanBuilding = { false };
	float									DistanceMarginFactor = { 1.1f };

	bool									IsSnapping = { false };
	float									SnapBaseYaw = { 0.f };

	FVector									SnapSocketLocationWorld = { FVector::ZeroVector };
	FVector									SnapSocketNormalWorld = { FVector::ZeroVector };
	float									SnapMinDistance = { 0.f };
	float									SnapContactMargin = { 1.f };

	TWeakObjectPtr<const ADC1BuildActor>	SnapTarget;
};
