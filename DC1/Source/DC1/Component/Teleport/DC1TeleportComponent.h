// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/DC1SaveInterface.h"
#include "DC1TeleportComponent.generated.h"

class USoundBase;

/**
 *
 * 텔레포트 지점 해금 목록. ADC1PlayerState에 붙는다.
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1TeleportComponent : public UActorComponent, public IDC1SaveInterface
{
	GENERATED_BODY()

public:
	UDC1TeleportComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 세이브 로드 완료 후 ( 서버 )
	virtual void OnPostLoad() override;

public:
	const TArray<FName>&	GetUnlockedPointIds() const { return UnlockedPointIds; }
	bool					IsPointUnlocked(FName InPointId) const;

	// 텔레포트 오브젝트 해금
	UFUNCTION(Server, Reliable)
	void					Server_UnlockPoint(AActor* InPoint);

	// ID로 해금. 서버 전용 ( 검증은 호출하는 쪽 책임 - Server_UnlockPoint, 거점 완성 등 )
	void					UnlockPointById(FName InPointId);

	// InSource 지점에서 InDest 지점으로 순간이동
	UFUNCTION(Server, Reliable)
	void					Server_RequestTeleport(AActor* InSource, AActor* InDest);

	// InDest 지점에서 부활 요청 ( 클라 ) - 새 Pawn 도착 시 FadeOut 하도록 표시 후 서버로 전달
	void					RequestRespawn(AActor* InDest);

	// Teleport 종료 후 처리 ( 성공 / 실패 모두 호출 -> FadeOut 보장 ), 부활은 실패 시에만
	UFUNCTION(Client, Reliable)
	void					Client_OnTeleportFinished(bool bInSuccess);

protected:
	virtual void			BeginPlay() override;
	virtual void			EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// InDest 지점에서 부활. 사용 불가 지점이면 시작 지점
	UFUNCTION(Server, Reliable)
	void					Server_RequestRespawn(AActor* InDest);

	UFUNCTION()
	void					OnRep_UnlockedPointIds();

	// 부활 FadeOut 시점 ( 새 Pawn이 PlayerState를 받은 순간 )
	UFUNCTION()
	void					HandlePawnSet(APlayerState* InPlayerState, APawn* InNewPawn, APawn* InOldPawn);

private:
	// 텔레포트 검증 + 이동 ( 서버 )
	bool					TryTeleport(AActor* InSource, AActor* InDest);

	// InDest 사용 가능 여부 판단 + 도착 Transform 반환 ( 서버 )
	bool					ResolveDest(AActor* InDest, FTransform& OutTransform) const;

	// 상호작용 허용 거리 내인지 판단
	bool					IsInnerInteractRange(AActor* InPoint) const;

private:
	// 해금된 텔레포트 지점 ID ( 세이브 키 )
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedPointIds, SaveGame)
	TArray<FName>			UnlockedPointIds;

	// 서버 검증용 지점 상호작용 허용 거리 ( 네트워크 지연을 감안해 상호작용 범위보다 넉넉하게 )
	UPROPERTY(EditDefaultsOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float					InteractAllowDistance = { 500.f };

	// 순간이동 도착음 ( 서버에서 도착 지점에 Multicast )
	UPROPERTY(EditDefaultsOnly, Category = "Teleport|Sound")
	TObjectPtr<USoundBase>	TeleportSound = { nullptr };

	// 부활 요청 후 새 Pawn 대기 중 ( 본인 클라 )
	bool					bPendingRespawnFade = { false };
};
