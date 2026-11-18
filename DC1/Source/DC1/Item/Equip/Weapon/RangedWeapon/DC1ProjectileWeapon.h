// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "GameplayEffectTypes.h"
#include "DC1ProjectileWeapon.generated.h"

class ADC1Projectile;
class UPrimitiveComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;

UCLASS()
class DC1_API ADC1ProjectileWeapon : public ADC1Weapon
{
	GENERATED_BODY()

public:
	ADC1ProjectileWeapon();

public:
	virtual UPrimitiveComponent* GetAttackCollider() override;
	virtual bool UsesAmmo() const override { return true; }

	// 데미지는 쏘는 쪽(GA)이 발사 시점 값으로 굳혀서 넘긴다
	ADC1Projectile*		FireProjectile(const FGameplayEffectSpecHandle& DamageSpec);

	FTransform			GetMuzzleTransform() const;

	// 당기는 동안 시위에 걸린 화살 표시 ( 몽타주 NockArrow 노티파이 스테이트에서 호출 - 모든 클라 )
	void				SetArrowNocked(bool bInNocked);

public:
	// 상속받은 ItemMesh(Static)를 쓰지 않고 스켈레탈 메시를 쓴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged")
	TObjectPtr<USkeletalMeshComponent>	RangedMesh = { nullptr };

	// RangedMesh 기준 발사 지점. 손에 붙일 때 쓰는 AttackSocketName과는 별개다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged")
	FName									MuzzleSocketName;

	// 시위에 걸린 화살 ( 표시용, 발사체와 별개 ). 시위 본(string1)에 붙여서 같이 당겨진다. 메쉬 / 오프셋은 BP에서
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged")
	TObjectPtr<UStaticMeshComponent>	NockedArrowMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged")
	TSubclassOf<ADC1Projectile>		ProjectileClass;

	// 쏠 때 인벤토리에서 하나 빼는 탄 ( DT_ItemData 의 ItemID ). 비우면 탄 없이 쏜다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged", meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName									AmmoItemID;

private:
	FVector				GetAimDirection();

private:
	// 화면 정중앙 트레이스 사거리
	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	float AimTraceDistance = { 10000.f };

	// 목표가 이보다 가까우면 이 거리로 고정
	UPROPERTY(EditDefaultsOnly, Category = "Ranged")
	float MinConvergeDistance = { 500.f };
};
