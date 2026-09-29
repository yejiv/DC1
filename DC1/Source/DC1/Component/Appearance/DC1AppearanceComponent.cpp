// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Appearance/DC1AppearanceComponent.h"

#include "Net/UnrealNetwork.h"

#include "Components/SkeletalMeshComponent.h"

#include "Data/DA/Appearance/DC1AppearanceRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

UDC1AppearanceComponent::UDC1AppearanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UDC1AppearanceComponent::SetParts(USkeletalMeshComponent* InBodyMesh, USkeletalMeshComponent* InHairMesh, USkeletalMeshComponent* InHeadMesh)
{
	CachedBodyMesh = InBodyMesh;
	CachedHairMesh = InHairMesh;
	CachedHeadMesh = InHeadMesh;
}

void UDC1AppearanceComponent::ApplyAppearance(const FDC1AppearanceData& Data)
{
	const UDC1AppearanceRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1AppearanceRegistryData>(DC1AssetTags::Asset_Appearance_Registry);
	if (false == IsValid(Registry))
	{
		return;
	}

	// Apply Body
	if (true == CachedBodyMesh.IsValid())
	{
		CachedBodyMesh->SetColorParameterValueOnMaterials(Registry->SkinColorPropertyName, Data.SkinColor);
	}

	// Apply Hair
	if (true == CachedHairMesh.IsValid())
	{
		if (const FDC1AppearancePartDef* HairAsset = Registry->GetHairData(Data.HairTag))
		{
			USkeletalMesh* NewHairMesh = HairAsset->SkeletalMesh.LoadSynchronous();
			UpdateMesh(NewHairMesh, CachedHairMesh.Get());
			CachedHairMesh->SetColorParameterValueOnMaterials(Registry->HairColorPropertyName, Data.HairColor);
		}
	}

	// Apply Head
	if (true == CachedHeadMesh.IsValid())
	{
		if (const FDC1AppearancePartDef* HeadAsset = Registry->GetHeadData(Data.HeadTag))
		{
			USkeletalMesh* NewHeadMesh = HeadAsset->SkeletalMesh.LoadSynchronous();
			UpdateMesh(NewHeadMesh, CachedHeadMesh.Get());
			CachedHeadMesh->SetColorParameterValueOnMaterials(Registry->SkinColorPropertyName, Data.SkinColor);
			CachedHeadMesh->SetColorParameterValueOnMaterials(Registry->HairColorPropertyName, Data.HairColor);
		}
	}
}

void UDC1AppearanceComponent::Server_ChangeAppearance_Implementation(const FDC1AppearanceData& InData)
{
	CurrentAppearance = InData;
	OnRep_CurrentAppearance();
}

void UDC1AppearanceComponent::ImportSaveData(const FDC1AppearanceData& In)
{
	// 외형은 서버가 파일로 들고 있지 않는다. 로컬(클라)에서 읽은 값을 그대로 서버에 올린다
	// (권한 체크 없이 RPC를 태움: 서버에서 부르면 즉시 로컬 실행, 클라에서 부르면 네트워크로 나간다)
	Server_ChangeAppearance(In);
}

void UDC1AppearanceComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDC1AppearanceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	CachedBodyMesh.Reset();
	CachedHairMesh.Reset();
	CachedHeadMesh.Reset();
}

void UDC1AppearanceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDC1AppearanceComponent, CurrentAppearance);
}

void UDC1AppearanceComponent::UpdateMesh(USkeletalMesh* NewMesh, USkeletalMeshComponent* CachedMesh)
{
	if (false == IsValid(NewMesh) || false == IsValid(CachedMesh))
	{
		return;
	}

	if (CachedMesh->GetSkeletalMeshAsset() != NewMesh)
	{
		CachedMesh->EmptyOverrideMaterials();
		CachedMesh->SetSkeletalMesh(NewMesh);
	}
}

void UDC1AppearanceComponent::OnRep_CurrentAppearance()
{
	ApplyAppearance(CurrentAppearance);
}
