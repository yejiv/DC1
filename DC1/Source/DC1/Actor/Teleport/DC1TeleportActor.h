// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DC1InteractableInterface.h"
#include "Interface/DC1TeleportPointInterface.h"
#include "DC1TeleportActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UArrowComponent;

UCLASS()
class DC1_API ADC1TeleportActor : public AActor, public IDC1InteractableInterface, public IDC1TeleportPointInterface
{
	GENERATED_BODY()

public:
	FDC1OnInteractOptionsChanged	OnInteractOptionsChanged;

public:
	ADC1TeleportActor();

public:
	FName						GetPointId() const { return PointId; }

protected:
	virtual void					BeginPlay() override;
	virtual void					EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// IDC1InteractableInterface override
	virtual UPrimitiveComponent*				GetInteractCollider()															const override;
	virtual void										GetInteractOptions(TArray<FDC1InteractOption>& OutOptions)		const override;
	virtual FDC1OnInteractOptionsChanged&	GetOnInteractOptionsChanged() override { return OnInteractOptionsChanged; }
	virtual void										Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey)	override;

	// IDC1TeleportPointInterface override
	virtual EDC1TeleportPointType	GetPointType()										const override { return EDC1TeleportPointType::Teleport; }
	virtual FText							GetPointName()										const override;
	virtual FTransform					GetArrivalTransform()								const override;
	virtual bool							CanUse(const APlayerState* InPlayerState)	const override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	TObjectPtr<UStaticMeshComponent>	Mesh = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Teleport|Interact")
	TObjectPtr<UBoxComponent>			InteractCollider = { nullptr };

	// 도착 위치 / 방향
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	TObjectPtr<UArrowComponent>		ArrivalPoint = { nullptr };

	// 해금 전 옵션 - ex) F : 활성화
	UPROPERTY(EditDefaultsOnly, Category = "Teleport|Interact")
	TArray<FDC1InteractOption>			LockedOptions;

	// 해금 후 옵션 - ex) F : 빠른 이동
	UPROPERTY(EditDefaultsOnly, Category = "Teleport|Interact")
	TArray<FDC1InteractOption>			UnlockedOptions;

private:
	// UI 리스트에 보여질 이름
	UPROPERTY(EditAnywhere, Category = "Teleport")
	FText											PointName;

	// 해금 / 세이브 키 ( 지점마다 고유 )
	UPROPERTY(EditAnywhere, Category = "Teleport")
	FName										PointId = { NAME_None };
};
