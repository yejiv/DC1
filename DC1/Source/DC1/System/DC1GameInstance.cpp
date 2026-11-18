// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DC1GameInstance.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

UDC1GameInstance::UDC1GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1GameInstance::Init()
{
	Super::Init();

	UDC1AssetManager::Get().LoadAssetsToLabel(
		DC1Labels::Label_Preload,
		FAsyncLabelLoadCompletedDelegate::CreateUObject(this, &ThisClass::OnPreloadComplete));
}

void UDC1GameInstance::Shutdown()
{
	Super::Shutdown();
}

void UDC1GameInstance::OnPreloadComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("UDC1GameInstance :: Preload Assets Complete"));
}
