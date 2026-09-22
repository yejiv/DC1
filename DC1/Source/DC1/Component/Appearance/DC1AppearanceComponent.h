// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DC1AppearanceComponent.generated.h"

class USkeletalMeshComponent;

struct FDC1AppearanceData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1AppearanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1AppearanceComponent();

public:
	void SetParts(USkeletalMeshComponent* InBodyMesh, USkeletalMeshComponent* InHairMesh, USkeletalMeshComponent* InHeadMesh);

public:
	void ApplyAppearance(const FDC1AppearanceData& Data);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void UpdateMesh(USkeletalMesh* NewMesh, USkeletalMeshComponent* CachedMesh);

private:
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent>	CachedBodyMesh = { nullptr };

	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent>	CachedHairMesh = { nullptr };

	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent>	CachedHeadMesh = { nullptr };
};
