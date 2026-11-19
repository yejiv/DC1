// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DC1DestroyActor.generated.h"

class UGeometryCollection;
class UGeometryCollectionComponent;

/*
직접 상속받아 사용하는 클래스가 아닌 파괴되는 시점에 교체할 클래스
*/

UCLASS()
class DC1_API ADC1DestroyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADC1DestroyActor();

protected:
	virtual void	BeginPlay() override;

public:
	void			InitDestroy(const UGeometryCollection* InCollection);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent>	DestroyComp = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Destroy")
	float										DestroyLifeTime = { 4.f };

	// Crumble 직후 조각을 중심에서 바깥으로 미는 속도(cm/s). 0이면 밀지 않음
	UPROPERTY(EditDefaultsOnly, Category = "Destroy")
	float										BreakImpulseStrength = { 300.f };
};
