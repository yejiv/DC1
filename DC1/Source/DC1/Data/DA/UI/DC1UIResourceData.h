// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1UIResourceData.generated.h"

class UTexture2D;
class UMaterialInterface;
class ADC1PlayerPreviewActor;

/**
 * 여러 화면이 같이 쓰는 UI 리소스(속성 아이콘 등). 태그로 꺼낸다.
 * 하드 참조라 이 DA가 로드되면 텍스처도 전부 올라온다. Preload 라벨로 등록해 게임 내내 붙잡아 둔다.
 * 작은 공용 리소스만 둔다. 종/아이템처럼 늘어나는 대량 리소스는 별도 DA에 soft로
 */
UCLASS()
class DC1_API UDC1UIResourceData : public UDataAsset
{
	GENERATED_BODY()

public:
	UTexture2D* FindTextureByTag(const FGameplayTag& ResourceTag) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (Categories = "UI.Icon"))
	TMap<FGameplayTag, TObjectPtr<UTexture2D>>	TextureEntries;

	// 인벤토리 플레이어 미리보기 액터 클래스 (비어 있으면 C++ 기본 클래스)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|PlayerPreview")
	TSubclassOf<ADC1PlayerPreviewActor>			PlayerPreviewActorClass;

	// 미리보기 렌더 타깃을 표시할 UI 머티리얼 (텍스처 파라미터 "RenderTarget")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|PlayerPreview")
	TObjectPtr<UMaterialInterface>				PlayerPreviewMaterial;

	// 최초 생성 시에만 적용 (변경은 PIE 재시작 후 반영)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|PlayerPreview", meta = (ClampMin = "64", ClampMax = "2048"))
	FIntPoint									PlayerPreviewRenderTargetSize = { 512, 768 };
};