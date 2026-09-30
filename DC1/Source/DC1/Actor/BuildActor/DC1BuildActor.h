// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "Define/DC1Structs.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1BuildActor.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnBuildDestroyFinished, ADC1BuildActor*);

class UStaticMeshComponent;
class UBoxComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UDC1BuildCheckComponent;
class UDC1BuildSnapComponent;
class UDC1BuildLinkComponent;

class ADC1BuildDestroyActor;
class UGeometryCollection;
class UNiagaraSystem;

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
	FDC1OnBuildDestroyFinished		OnDestroyFinished;

public:
	const FDC1BuildActorData&		GetBuildActorData() const { return BuildActorData; }
	const bool						GetCanBuilding() const { return CanBuilding; }
	const bool						GetIsBuildableSurface() const { return IsBuildableSurface; }
	const EDC1BuildSupportRule		GetSupportRule() const { return SupportRule; }
	const int32						GetRoofSupportSpan() const { return RoofSupportSpan; }
	
	const bool						IsPreview() const { return EDC1BuildState::Preview == BuildState; }

	UDC1BuildLinkComponent*			GetLinkComponent() const { return BuildLinkComp; }

	const bool						GetHasPillar() const { return HasPillar; }

	const EDC1BuildProbeDirection	GetProbeDirection() const;
	const FVector					GetCheckBoxCenter() const;
	const FTransform				ComputePlaceTransform(const FHitResult& InHit, float InYaw) const;
	const float						GetExtentCenterToDirection(const FVector& InWorldDirection, const FQuat& InQuat) const;

	const bool						GetIsAutoSnap() const;
	const float						GetSnappedRotationStep() const;

	const bool						GetBuildSocketsToWorld(TArray<FDC1BuildSnapSocket>& OutSockets) const;
	void							SetCheckSurface(bool InIsCheckSurface);

	const bool						GetCheckBoxWorld(FDC1BuildBox& OutBox) const;
	const bool						IsBlockedAt(const FTransform& InActorTransform, const TArray<FDC1BuildBox>& InBlockers) const;
	
	const bool						GatherSurfaceSupporters(TArray<ADC1BuildActor*>& OutSupporters, bool& OutIsOnGround) const;

protected:
	virtual void					BeginPlay() override;
	virtual void					EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void					GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void					OnConstruction(const FTransform& Transform) override;

public:
	virtual void					InitBuild(const FDC1BuildActorData& InBuildActorData);

	// 조준 시 해체 모드 표시 ( 로컬 )
	void							SetDismantleHighlight(bool InIsHighlighted);

	bool							UpdatePreview(bool InIsInRange);
	bool							TraceCheckBox(const FVector& InRayStart, const FVector& InRayEnd, const FCollisionQueryParams& InParams, float InInflate, FVector& OutImpactPoint) const;

	void							InitPreview(const FDC1BuildActorData& InBuildActorData);

	void							BeginDestroySequence();

	virtual void					BuildDestroy();

protected:
	UFUNCTION()
	void							OnRep_BuildState();

	void							SetState(EDC1BuildState InNewState);
	void							CreateTranslucentMIDs();
	void							SetMaterialTranslucentColor(const FLinearColor& InColor);

	virtual void					ApplyState();
	
	void							PlayDestroyFX();

private:
	void							FinishDestroySequence();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent>				BuildMesh = { nullptr };

	UPROPERTY(Replicated)
	FDC1BuildActorData								BuildActorData;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_BuildState)
	EDC1BuildState									BuildState = { EDC1BuildState::Default };

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>>	BuildMIDs;

private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDC1BuildCheckComponent>				BuildCheckComp = { nullptr };

	UPROPERTY()
	TArray<TObjectPtr<UDC1BuildCheckComponent>>		SubBuildCheckComps;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDC1BuildSnapComponent>				BuildSnapComp = { nullptr };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDC1BuildLinkComponent>				BuildLinkComp = { nullptr };

	// 이 건물 위에 다른 건물을 지을 수 있는가 ex) 바닥 = true
	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	bool											IsBuildableSurface = { false };

	// Preview, Constrction 상태에서 사용할 Materail
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	TObjectPtr<UMaterialInterface>					TranslucentMaterial = { nullptr };

	// 해체 모드 조준 시 사용할 Material
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	TObjectPtr<UMaterialInterface>					DismantleOverlayMaterial = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor									PreviewValidColor = { FLinearColor(0.f, 0.5f, 0.f, 1.f) };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor									PreviewInvalidColor = { FLinearColor::Red };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Support")
	EDC1BuildSupportRule							SupportRule = { EDC1BuildSupportRule::Surface };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Support", meta = (ClampMin = "1", EditCondition = "SupportRule == EDC1BuildSupportRule::RoofSpan"))
	int32											RoofSupportSpan = { 2 };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destroy")
	TSubclassOf<ADC1BuildDestroyActor>				DestroyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destroy")
	TObjectPtr<UGeometryCollection>					DestroyGeometryCollection = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destroy")
	TObjectPtr<UNiagaraSystem>						DestroyDustFX = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destroy", meta = (ClampMin = "0.2"))
	float											DestroySequenceDuration = { 0.2f };

private:
	bool											CanBuilding = { false };
	FTimerHandle									DestroySequenceTimer;
	bool											IsDestroying = { false };
	bool											HasPillar = { false };
};
