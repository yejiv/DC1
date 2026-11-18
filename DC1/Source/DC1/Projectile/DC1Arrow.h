// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/DC1Projectile.h"
#include "DC1Arrow.generated.h"

class ACharacter;

UCLASS()
class DC1_API ADC1Arrow : public ADC1Projectile
{
	GENERATED_BODY()

public:
	ADC1Arrow();

public:
	virtual void OnRep_ReplicatedMovement() override;

protected:
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	void StickToCharacterMesh(ACharacter* InCharacter, const FHitResult& InHit);

private:
	// 박힌 뒤 남아있는 시간
	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
	float		StuckLifeSpan = { 3.f };

	// 캡슐 표면 기준 앞뒤로 몸 표면을 찾는 거리
	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
	float		MeshTraceDistance = { 100.f };

	// 몸 표면에서 더 박히는 깊이
	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
	float		PenetrationDepth = { 10.f };

	// 로컬에서 본에 박은 뒤엔 서버 위치 복제를 무시
	bool		bStuckLocally = { false };
};
