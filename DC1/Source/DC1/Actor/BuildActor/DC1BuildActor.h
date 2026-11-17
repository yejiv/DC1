// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "Define/DC1Structs.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Interface/DC1SaveInterface.h"
#include "DC1BuildActor.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnBuildDestroyFinished, ADC1BuildActor*);

class UStaticMeshComponent;
class UBoxComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UDC1BuildCheckComponent;
class UDC1BuildSnapComponent;
class UDC1BuildLinkComponent;

class ADC1DestroyActor;
class APlayerState;
class UGeometryCollection;
class UNiagaraSystem;
class USoundBase;

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

	UPROPERTY(SaveGame)
	FGameplayTag				BuildTag = { FGameplayTag::EmptyTag };

	UPROPERTY(SaveGame)
	float						RequiredWork = { 0.f };

	UPROPERTY(SaveGame)
	TArray<FDC1ItemCost>		BuildCosts;

	UPROPERTY(SaveGame)
	FGameplayTag				WorkTypeTag = { FGameplayTag::EmptyTag };
};

UCLASS()
class DC1_API ADC1BuildActor : public AActor, public IDC1SaveInterface
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
	const bool						IsCompleted() const { return EDC1BuildState::Completed == BuildState; }
	const bool						IsUnderConstruction() const { return EDC1BuildState::Construction == BuildState; }
	const bool						IsInDestroySequence() const { return EDC1BuildState::Destroying == BuildState; }

	UDC1BuildLinkComponent*	GetLinkComponent() const { return BuildLinkComp; }

	const bool						GetHasPillar() const { return HasPillar; }

	USoundBase*						GetPlaceSound() const { return PlaceSound; }
	USoundBase*						GetDismantleSound() const { return DismantleSound; }

	const bool						GetIsBaseRequired() const { return IsBaseRequired; }
	APlayerState*					GetDestroyInstigator() const { return DestroyInstigator.Get(); }
	void								SetDestroyInstigator(APlayerState* InInstigator);

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
	// BuildCheck 박스 중 가장 큰 XY 반지름
	const float						GetMaxCheckRadius2D() const;

protected:
	virtual void					BeginPlay() override;
	virtual void					EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void					GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void					OnConstruction(const FTransform& Transform) override;

public:
	virtual void					InitBuild(const FDC1BuildActorData& InBuildActorData);
	virtual void					BuildDestroy();

	// 건물별 추가 설치 조건
	virtual bool					CanPlaceAt(const APlayerState* InBuilder) const;
	// 배치 확정(서버)
	virtual void					OnPlaced(const APlayerState* InBuilder);

	// 조준 시 해체 모드 표시 ( 로컬 )
	void							SetDismantleHighlight(bool InIsHighlighted);

	// None == 설치가능, 그 외 설치불가 이유 return
	EDC1BuildBlockReason			UpdatePreview(bool InIsInRange, const APlayerState* InBuilder);
	bool							TraceCheckBox(const FVector& InRayStart, const FVector& InRayEnd, const FCollisionQueryParams& InParams, float InInflate, FVector& OutImpactPoint) const;

	void							InitPreview(const FDC1BuildActorData& InBuildActorData);

	void							BeginDestroySequence();


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

	EDC1BuildBlockReason			EvaluateBlockReason(bool InIsInRange, const APlayerState* InBuilder);

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent>				BuildMesh = { nullptr };

	UPROPERTY(Replicated, SaveGame)
	FDC1BuildActorData								BuildActorData;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_BuildState, SaveGame)
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

	// 거점 범위 안에만 건설이 가능한가 + 거점 파괴 시 같이 파괴
	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	bool											IsBaseRequired = { false };

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
	TSubclassOf<ADC1DestroyActor>				DestroyClass = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destroy")
	TObjectPtr<UGeometryCollection>					DestroyGeometryCollection = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destroy")
	TObjectPtr<UNiagaraSystem>						DestroyDustFX = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destroy", meta = (ClampMin = "0.2"))
	float											DestroySequenceDuration = { 0.2f };

	// 배치 확정 시 재생 ( 서버 Server_PlaceBuild 에서 설치한 플레이어 경유 Multicast )
	UPROPERTY(EditDefaultsOnly, Category = "Build|Sound")
	TObjectPtr<USoundBase>							PlaceSound = { nullptr };

	// 플레이어가 철거할 때 재생 ( 재질별 ). 비어 있으면 BuildPlacementComponent 의 DismantleSound
	UPROPERTY(EditDefaultsOnly, Category = "Build|Sound")
	TObjectPtr<USoundBase>							DismantleSound = { nullptr };

private:
	// Destroy를 요청한 플레이어
	TWeakObjectPtr<APlayerState>					DestroyInstigator;

	bool											CanBuilding = { false };
	FTimerHandle									DestroySequenceTimer;
	bool											IsDestroying = { false };
	bool											HasPillar = { false };
};
