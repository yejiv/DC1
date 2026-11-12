// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Save/DC1SaveTypes.h"
#include "Tasks/Pipe.h"
#include "DC1SaveSubsystem.generated.h"

class USaveGame;

class ADC1PlayerState;
class UDC1PlayerSaveGame;
class UDC1WorldSaveGame;

struct FDC1AppearanceData;

UCLASS()
class DC1_API UDC1SaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void SetWorldLoadedState(bool IsLoaded) { IsWorldLoadedSuccess = IsLoaded; }

public:
	void StartAutoSave();
	void StopAutoSave();

	FString MakeSlotName(ADC1PlayerState* PS);

public:
	// Player, World Data Save
	bool SaveAll();
	// Pending(세이브 잔여) 모두 처리
	void FlushAll();

	// World Data Save
	bool SaveWorldData();
	bool LoadWorldData();

	// Player Data Save
	bool SavePlayerData(ADC1PlayerState* PS);
	bool LoadPlayerData(ADC1PlayerState* PS);

	// Customize Data Save
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
	// SaveInterface를 구현한 Actor, Component 수집
	void GatherSaveTargets(AActor* InActor, TArray<UObject*>& Out) const;

	// Save 데이터 -> Bytes로 변환 (OnPreSave 호출)
	void CollectRecords(AActor* InActor, TArray<FDC1ObjectSaveRecord>& Out);
	// Bytes -> Actor, Component에 적용 (OnPostLoad 호출)
	void ApplyRecords(AActor* InActor, const TArray<FDC1ObjectSaveRecord>& In);

	// 객체의 SaveGame 속성 -> Bytes 직렬화
	void WriteObject(UObject* InObj, TArray<uint8>& OutBytes) const;
	// Bytes -> 객체의 SaveGame 속성에 덮어쓰기
	void ReadObject(UObject* InObj, const TArray<uint8>& InBytes) const;

	// Pipe에 비동기로 Save 작업 등록
	void LaunchWrite(const FString& SlotName, TArray<uint8>&& InBytes);
	// Save 작업 후 호출되는 함수 (GameThread에서 실행)
	// PendingBytes에 같은 슬롯에 대한 저장 작업이 추가로 들어와있다면 재실행
	void OnWriteFinished(const FString& SlotName, bool bSuccess);

	// 실질적 SaveGame -> Memory & Launch
	bool SaveGameToMemoryAndLaunch(const FString& SlotName, USaveGame* SaveGame);

private:
	// (World) BuildActor 수집
	void CollectBuildActor(UWorld* World, UDC1WorldSaveGame* SaveGame);
	// (World) PickupItem 수집
	void CollectPickupActor(UWorld* World, UDC1WorldSaveGame* SaveGame);

	// (World) Actor Record 수집
	void CollectActorRecord(AActor* InActor, TArray<FDC1ActorSaveRecord>& OutRecords);

	// Deferred 스폰 -> ApplyRecords -> FinishSpawning (BeginPlay 전에 값 적용), OutSpawned는 선택
	template<typename T>
	void SpawnFromRecords(UWorld* World, const TArray<FDC1ActorSaveRecord>& InActorRecord, TArray<T*>* OutSpawned = nullptr);

private:
	// Actor = Self, Component = 컴포넌트 이름
	FName MakeKey(const AActor* InActor, const UObject* InTarget) const;

private:
	FString CustomizeFileName = { "Customize" };
	FString WorldFileName = { "World" };

	FTimerHandle	AutoSaveTimerHandle;
	// 자동 저장 주기
	float				AutoSaveInterval = { 300.f };

private:
	// 작업 Pipe
	UE::Tasks::FPipe SavePipe { TEXT("DC1SavePipe") };

	// 현재 세이브 중인 Slot
	TSet<FString> InFlightSlots;

	// 세이브 잔여 작업
	// Slot마다 최신 바이트 1개만 남음
	TMap<FString, TArray<uint8>> PendingBytes;

private:
	// World 데이터 Load 성공여부
	// Load 실패 시, 기존 World Data가 오염되지 않게 막음
	bool				IsWorldLoadedSuccess = { false };
};
