// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Appearance/DC1AppearancePreviewActor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/Appearance/DC1AppearanceComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Appearance/DC1CustomizeViewModel.h"

ADC1AppearancePreviewActor::ADC1AppearancePreviewActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	// 장식용이라 몸으로 부딪히거나 막을 이유가 없음
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 캡슐 콜리전을 껐기 때문에 CharacterMovementComponent가 바닥을 감지 못 해
	// 중력으로 계속 낙하한다 - 움직일 일이 없는 고정 프리뷰 액터이므로 무브먼트 자체를 꺼서 고정시킨다
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCharacterMovement()->GravityScale = 0.f;

	USkeletalMeshComponent* BodyMesh = GetMesh();
	BodyMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	BodyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HeadMesh");
	HeadMesh->SetupAttachment(BodyMesh);

	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HairMesh");
	HairMesh->SetupAttachment(BodyMesh);

	AppearanceComponent = CreateDefaultSubobject<UDC1AppearanceComponent>("AppearanceComponent");
}

void ADC1AppearancePreviewActor::BeginPlay()
{
	Super::BeginPlay();

	HairMesh->SetLeaderPoseComponent(GetMesh());

	AppearanceComponent->SetParts(GetMesh(), HairMesh, HeadMesh);

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (false == ::IsValid(PC))
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (false == ::IsValid(LocalPlayer))
	{
		return;
	}

	UDC1ViewModelSubsystem* ViewModelSubsystem = LocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>();
	if (false == ::IsValid(ViewModelSubsystem))
	{
		return;
	}

	if (UDC1CustomizeViewModel* CustomizeViewModel = ViewModelSubsystem->GetViewModel<UDC1CustomizeViewModel>())
	{
		CustomizeViewModel->BindPreviewComponent(AppearanceComponent);
	}
}
