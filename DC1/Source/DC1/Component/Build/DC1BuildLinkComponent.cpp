// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Build/DC1BuildLinkComponent.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Define/DC1BuildSupportHelper.h"
#include "Tags/Build/DC1GameplayTag_Build.h"

// Sets default values for this component's properties
UDC1BuildLinkComponent::UDC1BuildLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UDC1BuildLinkComponent::BeginPlay()
{
	Super::BeginPlay();

	const ADC1BuildActor* Owner = GetOwner<ADC1BuildActor>();
	if (true == ::IsValid(Owner) && false == Owner->IsPreview())
	{
		LinkStructure();
	}
}

void UDC1BuildLinkComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnlinkStructure();

	Super::EndPlay(EndPlayReason);
}

void UDC1BuildLinkComponent::RefreshPreviewLinks()
{
	DC1BuildSupportHelper::BuildLinks(GetOwner<ADC1BuildActor>(), Links);
}

void UDC1BuildLinkComponent::LinkStructure()
{
	DC1BuildSupportHelper::BuildLinks(GetOwner<ADC1BuildActor>(), Links);

	for (FDC1BuildStructureLink& Link : Links)
	{
		UDC1BuildLinkComponent* OtherLink = Link.Other.Get();
		if (false == ::IsValid(OtherLink))
		{
			continue;
		}

		OtherLink->Links.Emplace(this, Link.OtherSnapType, Link.MySnapType);
	}
}

void UDC1BuildLinkComponent::UnlinkStructure()
{
	for (const FDC1BuildStructureLink& Link : Links)
	{
		UDC1BuildLinkComponent* Other = Link.Other.Get();
		if (false == ::IsValid(Other))
		{
			continue;
		}

		Other->Links.RemoveAll([this](const FDC1BuildStructureLink& InLink) {
			return InLink.Other.Get() == this;
			});
	}

	Links.Reset();
}