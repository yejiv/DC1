// Fill out your copyright notice in the Description page of Project Settings.


#include "Save/DC1SaveSubsystem.h"

#include "EngineUtils.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "Item/Pickup/DC1PickupItem.h"

#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Define/DC1Macro.h"

#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Interface/DC1SaveInterface.h"
#include "GameMode/DC1GameState.h"

#include "Save/DC1PlayerSaveGame.h"
#include "Save/DC1WorldSaveGame.h"
#include "Save/DC1CustomizationSaveGame.h"
#include "Character/Player/DC1PlayerState.h"

void UDC1SaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDC1SaveSubsystem::Deinitialize()
{
	FlushAll();

	Super::Deinitialize();
}

void UDC1SaveSubsystem::StartAutoSave()
{
	UWorld* World = GetGameInstance()->GetWorld();
	if (nullptr == World || NM_Client == World->GetNetMode())
		return;

	GetGameInstance()->GetTimerManager().SetTimer(
		AutoSaveTimerHandle, this, &ThisClass::HandleAutoSave, AutoSaveInterval, true);
}

void UDC1SaveSubsystem::StopAutoSave()
{
	GetGameInstance()->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
}

bool UDC1SaveSubsystem::SaveAll()
{
	// 서버만 가능
	UWorld* World = GetGameInstance()->GetWorld();
	if (nullptr == World || NM_Client == World->GetNetMode())
		return false;

	// All Player Data Save
	AGameStateBase* GameState = World->GetGameState();
	if (nullptr == GameState)
		return false;

	bool bAllRequested = true;

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		ADC1PlayerState* PS = Cast<ADC1PlayerState>(PlayerState);
		if (false == IsValid(PS))
			continue;

		bAllRequested &= SavePlayerData(PS);
	}

	// World Save
	bAllRequested &= SaveWorldData();

	return bAllRequested;
}

void UDC1SaveSubsystem::FlushAll()
{
	SavePipe.WaitUntilEmpty();

	for (TPair<FString, TArray<uint8>>& Pair : PendingBytes)
	{
		SavePipe.Launch(TEXT("DC1SaveWrite"),
			[Slot = Pair.Key, Bytes = MoveTemp(Pair.Value)]()
			{
				const bool bSuccess = UGameplayStatics::SaveDataToSlot(Bytes, Slot, 0);
				LOG(TEXT("[Save] Slot = %s Result = %d"), *Slot, bSuccess);
			});
	}
	PendingBytes.Reset();

	SavePipe.WaitUntilEmpty();
}

bool UDC1SaveSubsystem::SaveWorldData()
{
	// 서버만 저장하도록
	UWorld* World = GetGameInstance()->GetWorld();
	if (false == IsValid(World))
		return false;
	if (NM_Client == World->GetNetMode())
		return false;

	// Load 실패 시, Save 진행하지 않음
	if (false == IsWorldLoadedSuccess)
		return false;

	UDC1WorldSaveGame* SaveGame = Cast<UDC1WorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UDC1WorldSaveGame::StaticClass()));
	if (false == IsValid(SaveGame))
		return false;

	// 1. 저장할 액터 수집
	CollectBuildActor(World, SaveGame);
	CollectPickupActor(World, SaveGame);

	// 3. SaveGameToMemory → LaunchWrite(WorldFileName)
	SaveGame->SetSaveVersion(CurrentSaveVersion);
	return SaveGameToMemoryAndLaunch(WorldFileName, SaveGame);
}

bool UDC1SaveSubsystem::LoadWorldData()
{
	UWorld* World = GetGameInstance()->GetWorld();
	if (false == IsValid(World))
		return false;
	if (NM_Client == World->GetNetMode())
		return false;

	if (false == UGameplayStatics::DoesSaveGameExist(WorldFileName, 0))
	{
		// 신규 월드
		LOG(TEXT("[Load] New World"));
		return true;
	}

	// 1. LoadGameFromSlot(WorldFileName)
	UDC1WorldSaveGame* SaveGame = Cast<UDC1WorldSaveGame>(UGameplayStatics::LoadGameFromSlot(WorldFileName, 0));
	if (false == IsValid(SaveGame))
	{
		LOG_ERROR(TEXT("[Load] World Load Failed"));
		return false;
	}

	TArray<ADC1BuildActor*> Spawned;
	SpawnFromRecords<ADC1BuildActor>(World, SaveGame->BuildActors, &Spawned);
	SpawnFromRecords<ADC1PickupItem>(World, SaveGame->PickupActors);

	Spawned.Sort([](const ADC1BuildActor& Src, const ADC1BuildActor& Dst) {
		return (true == Src.IsA<ADC1PalBoxActor>()) && (false == Dst.IsA<ADC1PalBoxActor>());
		});

	for (ADC1BuildActor* Actor : Spawned)
	{
		Actor->OnPlaced(nullptr);
	}

	return true;
}

bool UDC1SaveSubsystem::SavePlayerData(ADC1PlayerState* PS)
{
	if (false == IsValid(PS) || false == PS->HasAuthority())
		return false;

	// 신규 or 세이브파일 로드한 플레이어만 Save 진행 (데이터 유실 방지)
	if (false == PS->IsSuccessLoad())
	{
		LOG_WARNING(TEXT("[Save] Skip - Not Loaded"));
		return false;
	}

	UDC1PlayerSaveGame* SaveGame = Cast<UDC1PlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UDC1PlayerSaveGame::StaticClass()));
	if (false == IsValid(SaveGame))
	{
		LOG_ERROR(TEXT("Create SaveGame is Failed"))
		return false;
	}

	CollectRecords(PS, SaveGame->Components);

	FString SlotName = MakeSlotName(PS);
	if (SlotName.IsEmpty())
	{
		LOG_ERROR(TEXT("SlotName is Empty"))
		return false;
	}
	SaveGame->SetSaveVersion(CurrentSaveVersion);
	SaveGame->SetOwnerNetId(SlotName);

	return SaveGameToMemoryAndLaunch(SlotName, SaveGame);
}

bool UDC1SaveSubsystem::LoadPlayerData(ADC1PlayerState* PS)
{
	if (false == IsValid(PS) || false == PS->HasAuthority())
		return false;

	FString SlotName = MakeSlotName(PS);
	if (SlotName.IsEmpty())
	{
		return false;
	}

	if (false == UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		// 신규 플레이어
		LOG(TEXT("[Load] New Player"));
		return true;
	}

	UDC1PlayerSaveGame* SaveGame = Cast<UDC1PlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (true == IsValid(SaveGame))
	{
		ApplyRecords(PS, SaveGame->Components);
	}
	else
	{
		return false;
	}

	// SaveVersion에 따른 분기 처리 (추후)

	return true;
}

bool UDC1SaveSubsystem::SaveCustomizeData(const FDC1AppearanceData& InData)
{
	UDC1CustomizationSaveGame* SaveGame = Cast<UDC1CustomizationSaveGame>(UGameplayStatics::CreateSaveGameObject(UDC1CustomizationSaveGame::StaticClass()));
	if (false == IsValid(SaveGame))
	{
		LOG_ERROR(TEXT("Create SaveGame is Failed"))
			return false;
	}

	SaveGame->Appearance = InData;

	return UGameplayStatics::SaveGameToSlot(SaveGame, CustomizeFileName, 0);
}

bool UDC1SaveSubsystem::LoadCustomizeData(FDC1AppearanceData* OutData)
{
	if (nullptr == OutData)
		return false;

	if (false == HasCustomizeLocalData())
	{
		return false;
	}

	UDC1CustomizationSaveGame* SaveGame = Cast<UDC1CustomizationSaveGame>(UGameplayStatics::LoadGameFromSlot(CustomizeFileName, 0));

	if (true == IsValid(SaveGame))
	{
		*OutData = SaveGame->Appearance;
	}

	return true;
}

void UDC1SaveSubsystem::HandleAutoSave()
{
	SaveAll();
}

void UDC1SaveSubsystem::GatherSaveTargets(AActor* InActor, TArray<UObject*>& Out) const
{
	if (nullptr != Cast<IDC1SaveInterface>(InActor))
		Out.Add(InActor);

	TArray<UActorComponent*> Components;
	InActor->GetComponents(Components);

	for (UActorComponent* Comp : Components)
	{
		if (nullptr != Cast<IDC1SaveInterface>(Comp))
			Out.Add(Comp);
	}
}

void UDC1SaveSubsystem::CollectRecords(AActor* InActor, TArray<FDC1ObjectSaveRecord>& Out)
{
	TArray<UObject*> Targets;
	GatherSaveTargets(InActor, Targets);

	for (UObject* Target : Targets)
	{
		Cast<IDC1SaveInterface>(Target)->OnPreSave();

		FDC1ObjectSaveRecord& Record = Out.AddDefaulted_GetRef();
		Record.Key = MakeKey(InActor, Target);
		WriteObject(Target, Record.Bytes);
	}
}

void UDC1SaveSubsystem::ApplyRecords(AActor* InActor, const TArray<FDC1ObjectSaveRecord>& In)
{
	TArray<UObject*> Targets;
	GatherSaveTargets(InActor, Targets);

	TArray<UObject*> Loaded;
	for (const FDC1ObjectSaveRecord& Record : In)
	{
		UObject** Found = Targets.FindByPredicate([&](const UObject* T)
			{
				return MakeKey(InActor, T) == Record.Key;
			});

		if (nullptr == Found)
		{
			LOG_WARNING(TEXT("[Load] No Target for Key = %s"), *(Record.Key.ToString()));
			continue;
		}

		ReadObject(*Found, Record.Bytes);
		Loaded.Add(*Found);
	}

	for (UObject* Target : Loaded)
	{
		Cast<IDC1SaveInterface>(Target)->OnPostLoad();
	}
}

void UDC1SaveSubsystem::WriteObject(UObject* InObj, TArray<uint8>& OutBytes) const
{
	FMemoryWriter Writer(OutBytes, true);
	FObjectAndNameAsStringProxyArchive Ar(Writer, false);

	// UPROPERTY(SaveGame)만 통과
	Ar.ArIsSaveGame = true;
	Ar.ArNoDelta = true;

	InObj->SerializeScriptProperties(Ar);
}

void UDC1SaveSubsystem::ReadObject(UObject* InObj, const TArray<uint8>& InBytes) const
{
	FMemoryReader Reader(InBytes, true);
	FObjectAndNameAsStringProxyArchive Ar(Reader, true);

	// UPROPERTY(SaveGame)만 통과
	Ar.ArIsSaveGame = true;

	InObj->SerializeScriptProperties(Ar);
}

void UDC1SaveSubsystem::LaunchWrite(const FString& SlotName, TArray<uint8>&& InBytes)
{
	InFlightSlots.Add(SlotName);

	TWeakObjectPtr<ThisClass> WeakThis(this);

	// 백그라운드에서 비동기로 Save 진행하는 Pipe(일종의 작업큐?)
	SavePipe.Launch(TEXT("DC1SaveWrite"),
		[WeakThis, Slot = SlotName, Bytes = MoveTemp(InBytes)]()
		{
			const bool bSuccess = UGameplayStatics::SaveDataToSlot(Bytes, Slot, 0);

			// Launch 종료 후 작업은 GameThread로 다시 넘김.
			AsyncTask(ENamedThreads::GameThread, [WeakThis, Slot, bSuccess]()
				{
					if (UDC1SaveSubsystem* Self = WeakThis.Get())
					{
						Self->OnWriteFinished(Slot, bSuccess);
					}
				}
			);
		});
}

void UDC1SaveSubsystem::OnWriteFinished(const FString& SlotName, bool bSuccess)
{
	InFlightSlots.Remove(SlotName);
	LOG(TEXT("[Save] Slot = %s Result = %d"), *SlotName, bSuccess);

	TArray<uint8> Next;
	if (PendingBytes.RemoveAndCopyValue(SlotName, Next))
	{
		LaunchWrite(SlotName, MoveTemp(Next));
	}
}

bool UDC1SaveSubsystem::SaveGameToMemoryAndLaunch(const FString& SlotName, USaveGame* SaveGame)
{
	TArray<uint8> SaveGameBytes;
	if (false == UGameplayStatics::SaveGameToMemory(SaveGame, SaveGameBytes))
	{
		LOG_ERROR(TEXT("[Save] SaveGameToMemory Failed"));
		return false;
	}

	// 이미 Save중인 슬롯이면 잔여 작업 컨테이너에 저장
	if (true == InFlightSlots.Contains(SlotName))
	{
		PendingBytes.Add(SlotName, SaveGameBytes);
		return true;
	}

	LaunchWrite(SlotName, MoveTemp(SaveGameBytes));

	return true;
}

void UDC1SaveSubsystem::CollectBuildActor(UWorld* World, UDC1WorldSaveGame* SaveGame)
{
	for (TActorIterator<ADC1BuildActor> It(World); It; ++It)
	{
		ADC1BuildActor* BuildActor = *It;

		// 레벨에 미리 배치된 액터는 저장 X
		if (false == BuildActor->GetBuildActorData().BuildTag.IsValid())
			continue;

		// 이미 완성되거나 건설중인 액터만 저장
		if (false == BuildActor->IsCompleted() && false == BuildActor->IsUnderConstruction())
			continue;

		// 액터별 ActorClass / Transform / CollectRecords
		CollectActorRecord(BuildActor, SaveGame->BuildActors);
	}
}

void UDC1SaveSubsystem::CollectPickupActor(UWorld* World, UDC1WorldSaveGame* SaveGame)
{
	for (TActorIterator<ADC1PickupItem> It(World); It; ++It)
	{
		ADC1PickupItem* PickupItem = *It;

		// 액터별 ActorClass / Transform / CollectRecords
		CollectActorRecord(PickupItem, SaveGame->PickupActors);
	}
}

void UDC1SaveSubsystem::CollectActorRecord(AActor* InActor, TArray<FDC1ActorSaveRecord>& OutRecords)
{
	FDC1ActorSaveRecord& ActorRecord = OutRecords.AddDefaulted_GetRef();
	ActorRecord.ActorClass = InActor->GetClass();
	ActorRecord.Transform = InActor->GetActorTransform();
	CollectRecords(InActor, ActorRecord.Records);
}

template<typename T>
void UDC1SaveSubsystem::SpawnFromRecords(UWorld* World, const TArray<FDC1ActorSaveRecord>& InActorRecord, TArray<T*>* OutSpawned)
{
	for (const FDC1ActorSaveRecord& Record : InActorRecord)
	{
		UClass* ActorClass = Record.ActorClass.TryLoadClass<T>();
		if (nullptr == ActorClass)
			continue;

		// 2. 액터별 스폰 → ApplyRecords
		T* Actor = World->SpawnActorDeferred<T>(
			ActorClass, Record.Transform,
			nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		if (false == IsValid(Actor))
			continue;

		ApplyRecords(Actor, Record.Records);
		Actor->FinishSpawning(Record.Transform);

		if(nullptr != OutSpawned)
			OutSpawned->Add(Actor);
	}
}

bool UDC1SaveSubsystem::HasCustomizeLocalData()
{
	if (false == UGameplayStatics::DoesSaveGameExist(CustomizeFileName, 0))
		return false;

	return true;
}

FName UDC1SaveSubsystem::MakeKey(const AActor* InActor, const UObject* InTarget) const
{
	// Actor 자신이면 Self, Component면 컴포넌트 이름
	static const FName SelfKey = TEXT("Self");
	return (InTarget == InActor) ? SelfKey : InTarget->GetFName();
}

FString UDC1SaveSubsystem::MakeSlotName(ADC1PlayerState* PS)
{
	// Null Subsystem 예외 처리
	const FUniqueNetIdRepl& NetId = PS->GetUniqueId();
	if (false == NetId.IsValid() || NetId.GetType() == NULL_SUBSYSTEM)
	{
		LOG_WARNING(TEXT("[Save Warning] Subsystem is Null"));
		return FString();
	}

	FString ID = TEXT("Player_");
	ID.Append(NetId.ToString());

	return ID;
}
