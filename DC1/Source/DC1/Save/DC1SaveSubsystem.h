// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Save/DC1SaveTypes.h"
#include "Tasks/Pipe.h"
#include "DC1SaveSubsystem.generated.h"

class ADC1PlayerState;
class UDC1PlayerSaveGame;

struct FDC1AppearanceData;

UCLASS()
class DC1_API UDC1SaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void StartAutoSave();
	void StopAutoSave();

public:
	bool SaveAll();
	// Pending(세이브 잔여) 모두 처리
	void FlushAll();

	// World Data Save [TODO]


	// Player Data Save
	bool SavePlayerData(ADC1PlayerState* PS);
	bool LoadPlayerData(ADC1PlayerState* PS);

	// Customize Data Dave
	bool SaveCustomizeData(const FDC1AppearanceData& InData);
	bool LoadCustomizeData(FDC1AppearanceData* OutData);

public:
	// [Client] Customize Local File Check
	bool HasCustomizeLocalData();

public:
	static constexpr int32 CurrentSaveVersion = { 1 };

private:
	void HandleAutoSave();

private:
	void GatherSaveTargets(AActor* InActor, TArray<UObject*>& Out) const;

	void CollectRecords(AActor* InActor, TArray<FDC1ObjectSaveRecord>& Out);
	void ApplyRecords(AActor* InActor, const TArray<FDC1ObjectSaveRecord>& In);

	void WriteObject(UObject* InObj, TArray<uint8>& OutBytes) const;
	void ReadObject(UObject* InObj, const TArray<uint8>& InBytes) const;

	void LaunchWrite(const FString& SlotName, TArray<uint8>&& InBytes);
	void OnWriteFinished(const FString& SlotName, bool bSuccess);

private:
	FName MakeKey(const AActor* InActor, const UObject* InTarget) const;
	FString MakeSlotName(ADC1PlayerState* PS);

private:
	FString CustomizeFileName = { "Customize" };

	FTimerHandle	AutoSaveTimerHandle;
	float				AutoSaveInterval = { 10.f };

private:
	// 작업 Pipe
	UE::Tasks::FPipe SavePipe { TEXT("DC1SavePipe") };

	// 현재 세이브 중인 Slot
	TSet<FString> InFlightSlots;

	// 세이브 잔여 작업
	TMap<FString, TArray<uint8>> PendingBytes;
};
