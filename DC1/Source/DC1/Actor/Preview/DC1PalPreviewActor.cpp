// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Preview/DC1PalPreviewActor.h"

#include "Data/DA/Pal/DC1PalData.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"

ADC1PalPreviewActor::ADC1PalPreviewActor()
{
	PalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("PalMesh");
	PalMesh->SetupAttachment(CharacterPivot);
	PalMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	ApplyPreviewLighting(PalMesh);
}

void ADC1PalPreviewActor::SetPalPreview(const FDC1PalPreviewSettings& InSettings)
{
	PalMesh->SetSkeletalMeshAsset(InSettings.Mesh.LoadSynchronous());
	PalMesh->PlayAnimation(InSettings.IdleAnim.LoadSynchronous(), true);

	SetShowOnlyActor(this);
}

void ADC1PalPreviewActor::SetAnimationEnabled(bool bInEnabled)
{
	PalMesh->SetComponentTickEnabled(bInEnabled);
}

void ADC1PalPreviewActor::SetCaptureEnabled(bool bInEnabled)
{
	Super::SetCaptureEnabled(bInEnabled);

	SetAnimationEnabled(bInEnabled);
}
