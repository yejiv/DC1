// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Enum/DC1Enums.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1BuildActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UDC1BuildCheckComponent;

namespace DC1BuildMaterialParams
{
	static const FName TranslucentColor = FName("TranslucentColor");
	static const FName BuildProgress = FName("BuildProgress");
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

protected:
	virtual void	BeginPlay() override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void	Tick(float DeltaTime) override;

public:
	void			InitBuild(const FDC1BuildActorData& InBuildActorData, EDC1BuildState InState);
	virtual bool	UpdatePreview();
	void			AddBuildProgress(float InWork);
	void			BuildDestroy();

protected:
	UFUNCTION()
	void			OnRep_BuildState();

	UFUNCTION()
	void			OnRep_BuildProgress();

	void			SetState(EDC1BuildState InNewState);
	void			ApplyState();

private:
	void			SetMaterialTranslucentColor(const FLinearColor& InColor);
	void			SetMaterialBuildProgress();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent>		BuildMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USphereComponent>			InteractCollider = { nullptr };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDC1BuildCheckComponent>		BuildCheckComp = { nullptr };

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
	bool					CanBuilding = { false };
	float					CurrentWork = { 0.f };
	float					WorkSpeed = { 0.f };
};
