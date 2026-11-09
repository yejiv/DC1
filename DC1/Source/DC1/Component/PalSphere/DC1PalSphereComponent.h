// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "DC1PalSphereComponent.generated.h"

class ADC1PalSphere;

DECLARE_MULTICAST_DELEGATE(FOnSphereSelectionChanged);

// 스피어 등급 한 줄. 배열 순서 = 선택 키(2) 순서
USTRUCT(BlueprintType)
struct FDC1PalSphereDef
{
	GENERATED_BODY()

	// DT_ItemData 의 ItemID. 던질 때 인벤토리에서 하나 뺀다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName						ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ADC1PalSphere>	SphereClass;
};

/**
 * 던질 팰스피어 등급. ADC1PlayerState 에 붙는다.
 * 선택은 클라 전용이고, 서버는 Q 를 누를 때 Server_SetThrowSphere 로 받은 등급만 안다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1PalSphereComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1PalSphereComponent();

public:
	// 다음 등급(2 키). SphereDefs 순서로 돌고 끝에서 감싼다. 개수 0 인 등급도 고른다 ( 클라 )
	void						CycleSphere();

	// 고른 등급의 ItemID. 표가 비었으면 None
	FName						GetSelectedSphereItemID() const;

	// Q 를 누를 때 GA 발동보다 먼저 보낸다. ASC 와 같은 PlayerState 의 Reliable RPC 라 발동보다 먼저 도착한다
	UFUNCTION(Server, Reliable)
	void						Server_SetThrowSphere(FName InItemID);

	// 서버. 이번에 던질 등급
	FName						GetThrowSphereItemID() const { return ThrowSphereItemID; }

	// 표에 없으면 nullptr
	const FDC1PalSphereDef*		FindSphereDef(FName InItemID) const;

public:
	// 등급 선택 변화 ( 클라 )
	FOnSphereSelectionChanged	OnSphereSelectionChanged;

private:
	// 스피어 등급. 순서대로 선택 키(2)가 넘어간다
	UPROPERTY(EditDefaultsOnly, Category = "PalSphere")
	TArray<FDC1PalSphereDef>	SphereDefs;

	// 고른 등급 ( SphereDefs 번호 ). 클라 입력으로만 바뀐다. 복제 없음
	int32						SelectedSphereIndex = { 0 };

	// 이번에 던질 등급. 서버 전용
	FName						ThrowSphereItemID;
};
