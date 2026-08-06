// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/UI/DC1UILoadingData.h"

UMaterialInterface* UDC1UILoadingData::GetRandomSpinnerMaterial() const
{
    if (true == SpinnerMaterials.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Empty UI Loading Data"));
        
        return nullptr;
    }

    int32 RandomNum = FMath::RandRange(0, SpinnerMaterials.Num() - 1);
    UMaterialInterface* RadmonMaterialInterface = SpinnerMaterials[RandomNum];

    return RadmonMaterialInterface;
}
