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

void UDC1TeleportViewModel::SetSourcePoint(AActor* InSourcePoint)
{
	ClearSourcePoint();

	if (nullptr == InSourcePoint)
		return;

	SourcePoint = InSourcePoint;
	RebuildEntries();
}

void UDC1TeleportViewModel::ClearSourcePoint()
{
	SourcePoint.Reset();
}

void UDC1TeleportViewModel::RequestTeleport(int32 InIndex)
{
	if (nullptr == SourcePoint)
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

	TWeakObjectPtr<AActor> WeakSource = SourcePoint;
	TWeakObjectPtr<AActor> WeakDest = Dest;

	if (UDC1UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
	{
		UIManager->FadeIn(1.f, EDC1FadeType::Spinner, FSimpleDelegate::CreateWeakLambda(this, [this, WeakSource, WeakDest, UIManager]()
			{
				// 요청을 못 보내면 서버 응답(Client_OnTeleportFinished)도 없으니 여기서 FadeOut
				if (false == WeakSource.IsValid() || false == WeakDest.IsValid() || nullptr == TeleportComponent)
				{
					UIManager->FadeOut(1.f);
					return;
				}

				// 결과는 Client_OnTeleportFinished로 돌아온다 ( 성공 / 실패 모두 FadeOut )
				TeleportComponent->Server_RequestTeleport(WeakSource.Get(), WeakDest.Get());
			}));
	}
}

void UDC1TeleportViewModel::RebuildEntries()
{
	if (nullptr == TeleportSubsystem || nullptr == TeleportComponent)
		return;

	if (nullptr == SourcePoint)
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
		// Source Actor 제외
		if (Point == SourcePoint.Get())
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
