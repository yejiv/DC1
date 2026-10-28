// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Teleport/DC1TeleportViewModel.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Teleport/DC1TeleportComponent.h"
#include "Subsystem/DC1TeleportSubsystem.h"
#include "Subsystem/DC1UIManagerSubsystem.h"

#include "Define/DC1Macro.h"

void UDC1TeleportViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	ClearSource();

	if (nullptr == InPlayerState)
		return;

	TeleportComponent = InPlayerState->GetTeleportComponent();
	TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>();

	if (nullptr == TeleportSubsystem)
		return;
	TeleportSubsystem->OnPointsChanged.AddUObject(this, &ThisClass::HandlePointsChanged);
}

void UDC1TeleportViewModel::ClearSource()
{
	if (nullptr != TeleportComponent)
	{
		TeleportComponent.Reset();
	}

	if (nullptr != TeleportSubsystem)
	{
		TeleportSubsystem->OnPointsChanged.RemoveAll(this);
		TeleportSubsystem.Reset();
	}
}

void UDC1TeleportViewModel::SetListMode(EDC1TeleportListMode InMode, AActor* InSourcePoint)
{
	ClearListMode();

	// 텔레포트는 출발 지점이 있어야 한다
	if (EDC1TeleportListMode::Teleport == InMode && nullptr == InSourcePoint)
		return;

	ListMode = InMode;
	SourcePoint = InSourcePoint;
	RebuildEntries();
}

void UDC1TeleportViewModel::ClearListMode()
{
	SourcePoint.Reset();
	ListMode = EDC1TeleportListMode::None;
}

void UDC1TeleportViewModel::RequestTeleport(int32 InIndex)
{
	if (EDC1TeleportListMode::Teleport == ListMode && nullptr == SourcePoint)
	{
		LOG_WARNING(TEXT("[Teleport] Source is not Valid"));
		return;
	}

	if (false == Entries.IsValidIndex(InIndex))
	{
		LOG_WARNING(TEXT("[Teleport] Index is not Valid"));
		return;
	}

	AActor* Dest = Entries[InIndex].Point.Get();
	if (nullptr == Dest)
	{
		LOG_WARNING(TEXT("[Teleport] Dest is not Valid"));
		return;
	}

	if (nullptr == TeleportComponent)
		return;

	// 화면이 닫히면서 모드 / 출발 지점이 정리되므로 요청 시점 값을 들고 간다
	const EDC1TeleportListMode RequestMode = ListMode;
	TWeakObjectPtr<AActor> WeakSource = SourcePoint;
	TWeakObjectPtr<AActor> WeakDest = Dest;

	if (UDC1UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
	{
		UIManager->FadeIn(1.f, EDC1FadeType::Spinner, FSimpleDelegate::CreateWeakLambda(this, [this, RequestMode, WeakSource, WeakDest, UIManager]()
			{
				// 요청을 못 보내면 서버 응답(Client_OnTeleportFinished)도 없으니 여기서 FadeOut
				if (false == WeakDest.IsValid() || nullptr == TeleportComponent)
				{
					UIManager->FadeOut(1.f);
					return;
				}

				// Respawn
				if (EDC1TeleportListMode::Respawn == RequestMode)
				{
					TeleportComponent->RequestRespawn(WeakDest.Get());
				}
				// Teleport
				else
				{
					if (false == WeakSource.IsValid())
					{
						UIManager->FadeOut(1.f);
						return;
					}
					// 결과는 Client_OnTeleportFinished로 돌아온다 ( 성공 / 실패 모두 FadeOut )
					TeleportComponent->Server_RequestTeleport(WeakSource.Get(), WeakDest.Get());
				}
			}));
	}
}

void UDC1TeleportViewModel::RebuildEntries()
{
	if (nullptr == TeleportSubsystem || nullptr == TeleportComponent)
		return;

	// 열리지 않은 상태 / 텔레포트인데 출발 지점이 없으면 목록을 만들지 않는다
	if (EDC1TeleportListMode::None == ListMode)
		return;

	if (EDC1TeleportListMode::Teleport == ListMode && nullptr == SourcePoint)
		return;

	const APlayerState* PS = TeleportComponent->GetOwner<APlayerState>();
	if (nullptr == PS)
		return;

	Entries.Reset();

	TArray<AActor*> Points;
	TeleportSubsystem->GetUsablePoints(PS, Points);

	Entries.Reserve(Points.Num());

	for (const auto& Point : Points)
	{
		// Source Actor 제외 ( 부활은 Source가 없어 전부 포함 )
		if (nullptr != SourcePoint && Point == SourcePoint.Get())
			continue;

		FDC1TeleportEntry Entry = {};
		const IDC1TeleportPointInterface* Interface = Cast<const IDC1TeleportPointInterface>(Point);
		if (nullptr == Interface)
			continue;

		Entry.Point = Point;
		Entry.Name = Interface->GetPointName();
		Entry.Type = Interface->GetPointType();

		Entries.Add(Entry);
	}

	OnEntriesChanged.Broadcast();
}

void UDC1TeleportViewModel::HandlePointsChanged()
{
	RebuildEntries();
}
