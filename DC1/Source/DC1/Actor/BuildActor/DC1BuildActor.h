// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Enum/DC1Enums.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1BuildActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UDC1BuildCheckComponent;
class ADC1Character;

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
	EDC1BuildState					GetBuildState() const { return BuildState; }
	const bool						GetCanBuilding() const { return CanBuilding; }
	const bool						GetIsBuildableSurface() const { return IsBuildableSurface; }

	EDC1BuildProbeDirection			GetProbeDirection() const;
	FTransform						ComputePlaceTransform(const FHitResult& InHit, float InYaw) const;
	const bool						IsInInteractRange(const APawn* InPawn) const;

protected:
	virtual void	BeginPlay() override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void	Tick(float DeltaTime) override;

public:
	virtual bool	UpdatePreview();
	
	void			InitBuild(const FDC1BuildActorData& InBuildActorData, EDC1BuildState InState);

	void			AddWorker(const ADC1Character* InWorker);
	void			RemoveWorker(const ADC1Character* InWorker);
	void			BuildDestroy();

protected:
	UFUNCTION()
	void			OnRep_BuildState();

	UFUNCTION()
	void			OnRep_BuildProgress();

	void			SetState(EDC1BuildState InNewState);
	void			ApplyState();

private:
	void			CreateTranslucentMIDs();
	void			BuildCompleted();

	float			ComputeTotalWorkSpeed();

	void			SetMaterialTranslucentColor(const FLinearColor& InColor);
	void			SetMaterialBuildProgress();
	void			SetMaterialFlash();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent>		BuildMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent>			InteractCollider = { nullptr };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDC1BuildCheckComponent>		BuildCheckComp = { nullptr };

	// 이 건물 위에 다른 건물을 지을 수 있는가 ex) 바닥 = true
	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	bool									IsBuildableSurface = { false };

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"), Category = "Build|Interact")
	float									InteractDistance = { 200.f };

	// Preview, Constrction 상태에서 사용할 Materail
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	TObjectPtr<UMaterialInterface>			TranslucentMaterial = { nullptr };

	// Completed 시점 Flash 지속시간
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	float									CompleteFlashDuration = { 0.4f };

	// 머테리얼 슬롯별 진행도 구간 ( ex) Body = 0 ~ 0.5, Head = 0.5 ~ 1 )
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	TArray<FVector2D>						MaterialProgressRanges;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor							PreviewValidColor = { FLinearColor(0.f, 0.5f, 0.f, 1.f)};

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor							PreviewInvalidColor = { FLinearColor::Red };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor							ConstructionColor = { FLinearColor::Blue };


private:
	UPROPERTY(Replicated)
	FDC1BuildActorData		BuildActorData;

	// 테스트용 EditDefaultsOnly
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_BuildState)
	EDC1BuildState			BuildState = { EDC1BuildState::Default };

	UPROPERTY(ReplicatedUsing = OnRep_BuildProgress)
	float					BuildProgress = { 0.f };

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>>	BuildMIDs;

private:
	TSet<TWeakObjectPtr<const ADC1Character>> Workers;
	bool					CanBuilding = { false };
	float					CurrentWork = { 0.f };
	float					FlashAmount = { 0.f };
};
