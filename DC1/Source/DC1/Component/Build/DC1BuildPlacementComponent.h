// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Enum/DC1Enums.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1BuildPlacementComponent.generated.h"

struct FDC1BuildActorData;
struct FInputActionValue;

class ADC1BuildActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
	
private:
	UFUNCTION(Server, Reliable)
	void				Server_PlaceBuild(const FGameplayTag& InBuildTag, const FTransform& InTransform);

	void				CacheBuildRows();
	
	const FDC1BuildRow* FindBuildRow(const FGameplayTag& InBuildTag) const;
	FDC1BuildActorData	MakeBuildActorData(const FDC1BuildRow& InRow) const;
	ADC1BuildActor*		SpawnBuildActor(const FDC1BuildRow& InRow, EDC1BuildState InState, const FTransform& InTransform);

	void				CancelPlacement();
	void				ConfirmPlacement();

	bool				TraceSurface(FHitResult& OutHit) const;

	void				SetUpBuildInput();
	void				OnRotateInput(const FInputActionValue& Value);
	void				SetBuildMode(bool InEnabled);

	const APawn*		GetOwnerPawn() const;
	float				GetPlaceYaw() const;
	float				GetControlYaw() const;

private:
	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		PlaceDistance = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RangeUpHeight = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RangeDownHeight = { 300.f };

	UPROPERTY(EditAnywhere, Category = "Build|Placement")
	float		RotationStep = { 15.f };

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
};
