// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "Interface/DC1TeleportPointInterface.h"
#include "DC1TeleportViewModel.generated.h"

class UDC1TeleportComponent;
class UDC1TeleportSubsystem;

// 텔레포트 목록 한 줄 표시값
struct FDC1TeleportEntry
{
	FText									Name;
	EDC1TeleportPointType			Type = { EDC1TeleportPointType::Teleport };
	TWeakObjectPtr<AActor>		Point = { nullptr };
};

DECLARE_MULTICAST_DELEGATE(FOnTeleportEntriesChanged);

/**
 * 텔레포트 화면용 ViewModel. 출발 지점 기준 이동 가능한 지점 목록을 든다
 */
UCLASS()
class DC1_API UDC1TeleportViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	// 화면 열기 전 출발 지점 설정 ( 목록에서 제외 / 서버 검증용 )
	void							SetSourcePoint(AActor* InSourcePoint);
	void							ClearSourcePoint();

	const TArray<FDC1TeleportEntry>&	GetEntries() const { return Entries; }

	// 목록 InIndex 지점으로 이동 요청
	void							RequestTeleport(int32 InIndex);

public:
	FOnTeleportEntriesChanged		OnEntriesChanged;

private:
	void							RebuildEntries();
	void							HandlePointsChanged();

private:
	TWeakObjectPtr<UDC1TeleportComponent>	TeleportComponent = { nullptr };
	TWeakObjectPtr<UDC1TeleportSubsystem>	TeleportSubsystem = { nullptr };

	TWeakObjectPtr<AActor>					SourcePoint = { nullptr };

	TArray<FDC1TeleportEntry>				Entries;
};
