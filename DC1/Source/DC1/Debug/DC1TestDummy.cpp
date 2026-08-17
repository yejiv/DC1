// Fill out your copyright notice in the Description page of Project Settings.
// TODO : 공격 판정 테스트용 더미. 정식 타겟 나오면 이 파일 통째로 삭제

#include "Debug/DC1TestDummy.h"

#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"
#include "Component/DC1AbilitySystemComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ADC1TestDummy::ADC1TestDummy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 공격 콜라이더가 OverlapAllDynamic 이라 Pawn 오브젝트로 두면 서로 겹친다
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.f, 90.f);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionObjectType(ECC_Pawn);
	Capsule->SetCollisionResponseToAllChannels(ECR_Overlap);
	SetRootComponent(Capsule);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (true == CubeFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeFinder.Object);
	}

	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Damage 메타 어트리뷰트가 여기 있어서 기존 데미지 GE 가 그대로 먹는다.
	// 피격 로그도 UDC1PalAttributeSet::PostGameplayEffectExecute 가 찍어준다.
	AttributeSet = CreateDefaultSubobject<UDC1PalAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADC1TestDummy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADC1TestDummy::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}