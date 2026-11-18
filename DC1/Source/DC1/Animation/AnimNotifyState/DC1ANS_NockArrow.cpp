// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotifyState/DC1ANS_NockArrow.h"

#include "Character/Player/DC1Player.h"
#include "Component/DC1EquipComponent.h"
#include "Item/Equip/Weapon/RangedWeapon/DC1ProjectileWeapon.h"

#include "Components/SkeletalMeshComponent.h"

UDC1ANS_NockArrow::UDC1ANS_NockArrow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(120, 200, 90);
#endif
}

void UDC1ANS_NockArrow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// End 없이 파괴된 메쉬 정리
	for (TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<ADC1ProjectileWeapon>>::TIterator It = NockedByMesh.CreateIterator(); It; ++It)
	{
		if (false == It->Key.IsValid())
		{
			It.RemoveCurrent();
		}
	}

	ADC1ProjectileWeapon* Weapon = FindProjectileWeapon(MeshComp);
	if (nullptr == Weapon)
	{
		return;
	}

	Weapon->SetArrowNocked(true);
	NockedByMesh.Add(MeshComp, Weapon);
}

void UDC1ANS_NockArrow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 당기는 중 무기를 바꿔도 켰던 무기를 끈다
	TWeakObjectPtr<ADC1ProjectileWeapon> Weapon;
	if (true == NockedByMesh.RemoveAndCopyValue(MeshComp, Weapon) && true == Weapon.IsValid())
	{
		Weapon->SetArrowNocked(false);
	}
}

FString UDC1ANS_NockArrow::GetNotifyName_Implementation() const
{
	return TEXT("Nock Arrow");
}

ADC1ProjectileWeapon* UDC1ANS_NockArrow::FindProjectileWeapon(USkeletalMeshComponent* MeshComp)
{
	if (false == ::IsValid(MeshComp))
	{
		return nullptr;
	}

	// 몽타주 에디터 프리뷰 액터는 플레이어가 아니라 여기서 걸러진다
	ADC1Player* Player = Cast<ADC1Player>(MeshComp->GetOwner());
	if (false == ::IsValid(Player))
	{
		return nullptr;
	}

	UDC1EquipComponent* EquipComp = Player->GetEquipComponent();
	if (nullptr == EquipComp)
	{
		return nullptr;
	}

	return Cast<ADC1ProjectileWeapon>(EquipComp->GetCurrentWeapon());
}
