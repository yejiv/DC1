// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "DC1AppearancePreviewActor.generated.h"

class USkeletalMeshComponent;
class UDC1AppearanceComponent;

/**
 * 타이틀(커스터마이징) 화면 전용 미리보기 액터.
 *
 * ADC1Character를 상속한다 - DC1AnimInstance::NativeInitializeAnimation이
 * TryGetPawnOwner()를 ADC1Character로 캐스팅해서 Owner를 채우는데, 순수 AActor는
 * TryGetPawnOwner() 자체가 null이라 ABP_Player_Head를 못 쓴다(Accessed None).
 * 이 상속으로 Head에 ABP_Player_Head를 그대로 바인딩해서 쓸 수 있다.
 * Body는 별도 컴포넌트를 만들지 않고 ADC1Character가 이미 갖고 있는 GetMesh()를 그대로 쓴다
 * (ADC1Player와 동일 패턴).
 *
 * PlayerState 없이 자기 AppearanceComponent를 직접 들고 있고,
 * BeginPlay에서 UDC1CustomizeViewModel에 스스로 등록한다 (BindPreviewComponent).
 */
UCLASS()
class DC1_API ADC1AppearancePreviewActor : public ADC1Character
{
	GENERATED_BODY()

public:
	ADC1AppearancePreviewActor(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> HeadMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> HairMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDC1AppearanceComponent> AppearanceComponent = { nullptr };
};
