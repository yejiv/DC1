// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/DC1AnimInstance_Bow.h"

#include "Components/SkeletalMeshComponent.h"

#include "Character/Player/DC1Player.h"
#include "Item/Equip/Weapon/RangedWeapon/DC1ProjectileWeapon.h"

UDC1AnimInstance_Bow::UDC1AnimInstance_Bow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1AnimInstance_Bow::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Weapon = Cast<ADC1ProjectileWeapon>(GetOwningActor());
}

void UDC1AnimInstance_Bow::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 클라에서는 Instigator가 메시보다 늦게 도착할 수 있다. 잡힐 때까지 매 프레임 다시 본다
	if (false == IsValid(Player))
	{
		if (false == IsValid(Weapon)) return;

		Player = Cast<ADC1Player>(Weapon->GetInstigator());
		if (false == IsValid(Player)) return;

		// 캐릭터 포즈가 확정된 뒤에 읽어야 한다. 안 걸면 손 위치가 한 프레임 밀린다
		if (USkeletalMeshComponent* BowMesh = GetSkelMeshComponent())
		{
			BowMesh->AddTickPrerequisiteComponent(Player->GetMesh());
		}
	}

	Compute_StringLocation();
}

void UDC1AnimInstance_Bow::Compute_StringLocation()
{
	USkeletalMeshComponent* PlayerMesh = Player->GetMesh();
	if (false == IsValid(PlayerMesh)) return;

	UAnimInstance* PlayerAnimInstance = PlayerMesh->GetAnimInstance();
	if (false == IsValid(PlayerAnimInstance)) return;

	// 커브는 캐릭터 애님에만 있다. 활은 자기 애님을 돌리지 않으므로 여기서 꺼내온다
	StringAlpha = PlayerAnimInstance->GetCurveValue(DrawCurveName);

	// 기본 자세와의 블렌드는 Transform (Modify) Bone 노드의 Alpha가 처리한다.
	// 여기서는 손 위치를 활 메시 공간으로 옮기기만 한다
	const FVector HandLocation = PlayerMesh->GetSocketLocation(DrawHandSocketName);
	StringLocation = GetSkelMeshComponent()->GetComponentTransform().InverseTransformPosition(HandLocation);
}