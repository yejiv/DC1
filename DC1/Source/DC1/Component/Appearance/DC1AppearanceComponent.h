// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Appearance/DC1AppearanceTypes.h"
#include "DC1AppearanceComponent.generated.h"

class USkeletalMeshComponent;

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

	// Body 메시 교체(방어구) 후 피부색만 다시 칠하는 함수
	// 폰 초기화 순서상 SetParts보다 먼저 불릴 수 있어 캐싱된 메시 대신 직접 받는다
	void ApplyBodyColor(USkeletalMeshComponent* InBodyMesh);

	UFUNCTION(Server, Reliable)
	void Server_ChangeAppearance(const FDC1AppearanceData& InData);

public:
	void ImportSaveData(const FDC1AppearanceData& In);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void UpdateMesh(USkeletalMesh* NewMesh, USkeletalMeshComponent* CachedMesh);

private:
	UFUNCTION()
	void OnRep_CurrentAppearance();

public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAppearance)
	FDC1AppearanceData									CurrentAppearance = {};

private:
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent>	CachedBodyMesh = { nullptr };

	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent>	CachedHairMesh = { nullptr };

	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent>	CachedHeadMesh = { nullptr };

};
