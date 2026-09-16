// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "UI/World/DC1PalHealthWidget.h"

#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionSystem.h"

ADC1PalCharacter::ADC1PalCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//AIControllerClass = ADC1PalAIController::StaticClass();
	//AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	PalAttributeSet = CreateDefaultSubobject<UDC1PalAttributeSet>(TEXT("PalAttributeSet"));

	// 머리 위 HP바. 위젯 클래스와 높이는 BP_Pal_* 에서 종마다 지정한다
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
	HealthWidgetComponent->SetupAttachment(RootComponent);
	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);	// 거리와 무관하게 크기 고정
	HealthWidgetComponent->SetDrawAtDesiredSize(true);
	HealthWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	HealthWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthWidgetComponent->SetTickWhenOffscreen(false);			// 화면 밖이면 돌지 않는다

	// 회전쓰지 않음
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	SetOrientToMovement(false); // 기본은 AIController의 focus가 회전을 구동
	Movement->RotationRate = FRotator(0.f, 360.f, 0.f);
	Movement->MaxWalkSpeed = 400.f;
	Movement->bUseRVOAvoidance = true;

	Movement->GetNavMovementProperties()->bUseAccelerationForPaths = true;
}

void ADC1PalCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = GetPawnViewLocation();
	OutRotation = GetActorRotation();
}

void ADC1PalCharacter::SetOrientToMovement(bool Enable)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = Enable;
		Movement->bUseControllerDesiredRotation = (false == Enable);
	}
}

void ADC1PalCharacter::PostInitializeComponents()
{
	ApplyPalData(); //PalData 먼저로드
	Super::PostInitializeComponents();
}

void ADC1PalCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (false == IsValid(HealthWidgetComponent))
	{
		return;
	}

	// 위젯은 아직 안 만들어져 있을 수 있다. 데디케이티드 서버에서는 만들어지지 않는다
	HealthWidgetComponent->InitWidget();

	// 위젯이 자기 주인을 모르므로 팰이 직접 넣어준다.
	// GetOwningPlayerPawn 은 로컬 플레이어의 폰을 돌려주므로 쓸 수 없다
	if (UDC1PalHealthWidget* HealthWidget = Cast<UDC1PalHealthWidget>(HealthWidgetComponent->GetUserWidgetObject()))
	{
		HealthWidget->InitForPal(this);
	}
	else
	{
		// 위젯이 없으면(데디케이티드 서버 등) 가시성 검사도 의미가 없다
		return;
	}

	HealthWidgetComponent->SetVisibility(false);

	GetWorldTimerManager().SetTimer(HealthWidgetVisibilityTimer, this,
		&ThisClass::UpdateHealthWidgetVisibility, HealthWidgetCheckInterval, true);
}

void ADC1PalCharacter::UpdateHealthWidgetVisibility()
{
	if (false == IsValid(HealthWidgetComponent))
	{
		return;
	}

	bool ShouldShow = (GetHealthRatio() < 1.f);

	if (false == ShouldShow)
	{
		if (const APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			const float DistanceSq = FVector::DistSquared(GetActorLocation(), LocalPawn->GetActorLocation());
			ShouldShow = (DistanceSq <= (HealthWidgetVisibleDistance * HealthWidgetVisibleDistance));
		}
	}

	if (HealthWidgetComponent->IsVisible() != ShouldShow)
	{
		HealthWidgetComponent->SetVisibility(ShouldShow);
	}
}

void ADC1PalCharacter::ApplyPalData()
{
	if (false == SpeciesTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: SpeciesTag is not set."), *GetName());
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: Can't find PalRegistryData. Check PDA_AssetData."), *GetName());
		return;
	}

	// 나중에 스폰 전에 비동기로 미리 올려두는 쪽으로 바꾸기.
	PalData = Registry->FindPalDataByTag(SpeciesTag).LoadSynchronous();
	if (false == IsValid(PalData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: Failed to load PalData [%s]."), *GetName(), *SpeciesTag.ToString());
		return;
	}

	// Locomotion
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = PalData->RunSpeed;
		Movement->RotationRate = FRotator(0.f, PalData->RotationRateYaw, 0.f);
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->InitAbilitySystem(this, this, PalData->AbilityDataTag);
		BindHealthWatcher();

		// Health는 PostGameplayEffectExecute 가 SetHealth 로 바꾸므로 Damage 를 구독
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetDamageAttribute()).AddUObject(this, &ThisClass::HandleDamaged);

		// HP바용. BindHealthWatcher 가 RemoveAll(this) 를 하므로 반드시 그 뒤에 온다
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChangedForUI);
	}

	if (IsValid(PalAttributeSet))
	{
		PalAttributeSet->InitMaxHealth(PalData->MaxHealth);
		PalAttributeSet->InitHealth(PalData->MaxHealth);
		PalAttributeSet->InitAttackPower(PalData->AttackPower);

		const int32 FinalLevel = (true == bOverrideLevel)
			? LevelOverride
			: FMath::RandRange(PalData->MinLevel, FMath::Max(PalData->MinLevel, PalData->MaxLevel));

		PalAttributeSet->InitLevel(FinalLevel);
		PalInstance.Level = FinalLevel;
	}
}

void ADC1PalCharacter::HandleDamaged(const FOnAttributeChangeData& Data)
{
	// Damage 메타 어트리뷰트가 채워지는 순간만 본다. 곧바로 0으로 되돌리는 변경은 무시
	if (false == HasAuthority() || Data.NewValue <= 0.f || nullptr == Data.GEModData)
	{
		return;
	}

	AActor* Causer = Data.GEModData->EffectSpec.GetContext().GetEffectCauser();

	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->ProvokeBy(Causer);
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->ActivateAbilityByTag(DC1PalTags::Ability_Pal_Hit);
	}
}

void ADC1PalCharacter::HandleHealthChangedForUI(const FOnAttributeChangeData& Data)
{
	if (false == IsValid(PalAttributeSet))
	{
		return;
	}

	OnPalHealthChanged.Broadcast(Data.NewValue, PalAttributeSet->GetMaxHealth());
}

void ADC1PalCharacter::StopAndUnregister()
{
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAbilities();
	}

	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->StopBehavior();
	}

	// 남들이 계속 감지해 쫓지 않도록
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->UnregisterSource(*this);
	}
}

int32 ADC1PalCharacter::GetLevel() const
{
	if (false == IsValid(PalAttributeSet))
	{
		return 1;
	}

	return FMath::Max(1, FMath::RoundToInt(PalAttributeSet->GetLevel()));
}

float ADC1PalCharacter::GetHealthRatio() const
{
	if (false == IsValid(PalAttributeSet))
	{
		return 0.f;
	}

	const float Max = PalAttributeSet->GetMaxHealth();
	if (Max <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(PalAttributeSet->GetHealth() / Max, 0.f, 1.f);
}

float ADC1PalCharacter::GetCaptureRate() const
{
	return (nullptr != PalData) ? PalData->CaptureRate : 0.f;
}

FDC1PalInstance ADC1PalCharacter::Fold()
{
	// 새로 만들지 않고 들고 있던 것에서 시작한다.
	// 그래야 닉네임·경험치·패시브처럼 액터가 건드리지 않는 값이 보존된다
	FDC1PalInstance Instance = PalInstance;

	Instance.SpeciesTag = SpeciesTag;

	// 소환됐던 개체면 원래 ID를 유지
	Instance.PalID = InstanceID.IsValid() ? InstanceID : FGuid::NewGuid();

	if (IsValid(PalAttributeSet))
	{
		Instance.CurrentHealth = PalAttributeSet->GetHealth();
		Instance.Level = GetLevel();
	}

	StopAndUnregister();
	Destroy();

	return Instance;
}

void ADC1PalCharacter::OnCaptureFailed(AActor* InThrower)
{
	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->ProvokeBy(InThrower);
	}
}

void ADC1PalCharacter::BeginCapture()
{
	// 파괴하지 않음. 실패 시 되돌려야 함
	StopAndUnregister();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ADC1PalCharacter::CancelCapture()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	// StopAndUnregister가 뺀 지각 복구
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->RegisterSource(*this);
	}

	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->TryStartBehavior();
	}

	UE_LOG(LogTemp, Log, TEXT("[Pal] %s escaped the sphere."), *GetName());
}

ADC1PalCharacter* ADC1PalCharacter::SpawnFromInstanceData(UWorld* InWorld, const FDC1PalInstance& InInstance, const FTransform& InTransform)
{
	if (nullptr == InWorld || false == InInstance.IsValidPal())
	{
		return nullptr;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Spawn: Can't find PalRegistryData. Check PDA_AssetData."));
		return nullptr;
	}

	const UDC1PalData* SpeciesData = Registry->FindPalDataByTag(InInstance.SpeciesTag).LoadSynchronous();
	if (false == IsValid(SpeciesData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Spawn: Failed to load PalData [%s]."), *InInstance.SpeciesTag.ToString());
		return nullptr;
	}

	UClass* PalClass = SpeciesData->PalClass.LoadSynchronous();
	if (nullptr == PalClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Spawn: No PalClass for [%s]. Check DA_Pal_*."), *InInstance.SpeciesTag.ToString());
		return nullptr;
	}


	ADC1PalCharacter* Pal = InWorld->SpawnActorDeferred<ADC1PalCharacter>(PalClass, InTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (false == IsValid(Pal))
	{
		return nullptr;
	}

	Pal->SpeciesTag = InInstance.SpeciesTag;
	Pal->InstanceID = InInstance.PalID;
	Pal->Team = EDC1Team::Player;

	// 원본을 통째로 보관
	Pal->PalInstance = InInstance;
	Pal->bOverrideLevel = true;
	Pal->LevelOverride = FMath::Max(1, InInstance.Level);

	Pal->FinishSpawning(InTransform);

	if (IsValid(Pal->PalAttributeSet))
	{
		Pal->PalAttributeSet->SetHealth(InInstance.CurrentHealth);
	}

	return Pal;
}

void ADC1PalCharacter::OnDeath()
{
	StopAndUnregister();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));

		// 발에 치이게
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetSimulatePhysics(true);

		MeshComp->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	}

	UE_LOG(LogTemp, Log, TEXT("[Pal] %s died."), *GetName());

	OnPalDied.Broadcast();
}

UAbilitySystemComponent* ADC1PalCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADC1PalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADC1PalCharacter, Team);
}

ETeamAttitude::Type ADC1PalCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	return DC1Team::GetAttitude(Team, DC1Team::ResolveTeam(Other));
}

FGenericTeamId ADC1PalCharacter::GetGenericTeamId() const
{
	return DC1Team::ToGenericTeamId(Team);
}

void ADC1PalCharacter::SetTeam(EDC1Team NewTeam)
{
	if (false == HasAuthority())
	{
		return;
	}

	if (Team == NewTeam)
	{
		return;
	}

	Team = NewTeam;
	OnTeamChanged();
}

void ADC1PalCharacter::OnRep_Team()
{
	OnTeamChanged();
}

void ADC1PalCharacter::OnTeamChanged()
{
	// EnemyPal 차단
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName((EDC1Team::Player == Team) ? TEXT("AllyPal") : TEXT("EnemyPal"));
	}

	// 적대 판정은 sight query 생성 시점에 한 번만 해서 진영이 바뀌면 양쪽을 다 갱신!

	// 남들이 나를 다시 판정
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->UnregisterSource(*this);
		PerceptionSystem->RegisterSource(*this);
	}

	// 내가 남들을 다시 판정 + 진영에 맞는 행동 트리로 갈아탄다
	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->HandleTeamChanged();
	}

	UE_LOG(LogTemp, Log, TEXT("[Pal] %s team changed to %s."), *GetName(), *UEnum::GetValueAsString(Team));
}
