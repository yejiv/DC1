// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1BuildActor.generated.h"

struct FDC1BuildSnapSocket;

class UStaticMeshComponent;
class UBoxComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UDC1BuildCheckComponent;

// 머테리얼에서 사용할 파라미터 이름
namespace DC1BuildMaterialParams
{
	// Translucent Material
	static const FName TranslucentColor = FName("TranslucentColor");
	static const FName BuildProgress = FName("BuildProgress");
	static const FName BaseColorTexture = FName("BaseColorTexture");
	static const FName UseTexture = FName("UseTexture");
	static const FName FlashAmount = FName("FlashAmount");

	// 원본 Material ( BaseColorTexture 추출용 )
	static const FName SourceBaseColorParamName = FName("BaseColor");
}

USTRUCT()
struct FDC1BuildActorData
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag				BuildTag = { FGameplayTag::EmptyTag };
	
	UPROPERTY()
	float						RequiredWork = { 0.f };
	
	UPROPERTY()
	TArray<FDC1ItemCost>		BuildCosts;
};

UCLASS()
class DC1_API ADC1BuildActor : public AActor
{
	GENERATED_BODY()

public:
	ADC1BuildActor();

public:
	const FDC1BuildActorData&		GetBuildActorData() const { return BuildActorData; }
	const FGameplayTag&				GetBuildTag() const { return BuildActorData.BuildTag; }
	const float						GetRequiredWork() const { return BuildActorData.RequiredWork; }
	const TArray<FDC1ItemCost>&		GetBuildCosts() const { return BuildActorData.BuildCosts; }
	const EDC1BuildState			GetBuildState() const { return BuildState; }
	const bool						GetCanBuilding() const { return CanBuilding; }
	const bool						GetIsBuildableSurface() const { return IsBuildableSurface; }

	const EDC1BuildProbeDirection	GetProbeDirection() const;
	const FVector					GetCheckBoxCenter() const;
	const FTransform				ComputePlaceTransform(const FHitResult& InHit, float InYaw) const;
	const float						GetExtentCenterToDirection(const FVector& InWorldDirection, const FQuat& InQuat) const;

protected:
	virtual void					BeginPlay() override;
	virtual void					GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual bool					UpdatePreview(bool InIsInRange);
	virtual bool					SelectSnapPointByRay(const FVector& InRayStart, const FVector& InRayEnd, const FCollisionQueryParams& InParams, 
													FDC1BuildSnapSocket& OutSnapSocket) const;
	virtual bool					SelectSnapPointByPoint(const FVector& InWorldLocation, FDC1BuildSnapSocket& OutSnapSocket) const;
	virtual void					InitBuild(const FDC1BuildActorData& InBuildActorData);

	void							InitPreview(const FDC1BuildActorData& InBuildActorData);
	virtual void					BuildDestroy();

protected:
	UFUNCTION()
	void							OnRep_BuildState();

	void							SetState(EDC1BuildState InNewState);
	void							CreateTranslucentMIDs();
	void							SetMaterialTranslucentColor(const FLinearColor& InColor);

	virtual void					ApplyState();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent>		BuildMesh = { nullptr };

	UPROPERTY(Replicated)
	FDC1BuildActorData						BuildActorData;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_BuildState)
	EDC1BuildState							BuildState = { EDC1BuildState::Default };

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>>	BuildMIDs;

private:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDC1BuildCheckComponent>		BuildCheckComp = { nullptr };

	// 이 건물 위에 다른 건물을 지을 수 있는가 ex) 바닥 = true
	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	bool									IsBuildableSurface = { false };

	// Preview, Constrction 상태에서 사용할 Materail
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	TObjectPtr<UMaterialInterface>			TranslucentMaterial = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor							PreviewValidColor = { FLinearColor(0.f, 0.5f, 0.f, 1.f) };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor							PreviewInvalidColor = { FLinearColor::Red };


private:
	bool					CanBuilding = { false };
};
