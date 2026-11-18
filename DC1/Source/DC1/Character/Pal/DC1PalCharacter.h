// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "AI/DC1TeamTypes.h"
#include "Data/Pal/DC1PalTypes.h"
#include "Interface/DC1WorkTargetInterface.h"
#include "UObject/WeakInterfacePtr.h"
#include "DC1PalCharacter.generated.h"

class ADC1PalBoxActor;
class ADC1PlayerState;
class UDC1AbilitySystemComponent;
class UDC1PalAttributeSet;
class UDC1PalData;
class UDC1PalRegistryData;
class UDC1PalRideComponent;
class UCapsuleComponent;
class UWidgetComponent;
struct FInputActionValue;
struct FGameplayEventData;

DECLARE_MULTICAST_DELEGATE(FDC1OnPalDied);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDC1OnPalHealthChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDC1OnPalSanityChanged, float /*Current*/, float /*Max*/);

/**
 *
 */
UCLASS()
class DC1_API ADC1PalCharacter : public ADC1Character, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ADC1PalCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IGenericTeamAgentInterface
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	EDC1Team							GetTeam() const { return Team; }

	// 알파(필드 보스) 개체인가. 복제된다
	bool								IsAlpha() const { return bIsAlpha; }

	const UDC1PalData*					GetPalData() const { return PalData; }
	virtual UAbilitySystemComponent*	GetAbilitySystemComponent() const override;

	void								SetOrientToMovement(bool Enable);
	virtual void						GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	// 탑승 중엔 탄 플레이어 카메라
	virtual void						CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	// AttributeSet 의 Level 은 float 이라 표시용으로 정수화해서 돌려준다
	int32								GetLevel() const;

	// 스포너·저장 데이터가 FinishSpawning 전에 호출한다.
	void								SetInitialLevel(int32 InLevel);
	void								SetInitialAlpha(bool bInIsAlpha);
	EDC1PalGender						GetGender() const { return PalInstance.Gender; }

	// 현재 레벨 누적 경험치
	float								GetExperience() const { return PalInstance.Experience; }

	// 경험치 획득. 서버 전용
	void								GainExperience(int32 InAmount);

	// 속성
	EDC1PalElement						GetElement() const;
	EDC1PalElement						GetElement2() const;

	// 파티가 이 액터를 슬롯과 대조할 때 쓴다. 야생이면 비어 있다
	const FGuid&						GetInstanceID() const { return InstanceID; }

	// 주인(내 팰의 플레이어). 야생은 널. 폰은 탑승·리스폰 때 바뀌므로 PlayerState 를 든다
	ADC1PlayerState*					GetOwnerPlayerState() const { return OwnerPlayerState; }

	// 거점 팰이면 소속 팰박스. 파트너,야생은 널. 복제된다 (클라 HP바가 거점 팰인지 본다)
	ADC1PalBoxActor*					GetBasePalBox() const { return BasePalBox.Get(); }

	// SAN 현재값 / 최대값
	float								GetSanity() const;
	float								GetMaxSanity() const;

	// 작업 대상 - 펠 ST가 AssignPalWorker 성공 후 설정 -> Work GA가 GetCurrentWorkTarget으로 읽음. 서버 전용
	void								SetWorkTarget(IDC1WorkTargetInterface* InWorkTarget) { WorkTarget = InWorkTarget; }
	virtual IDC1WorkTargetInterface*	GetCurrentWorkTarget() const override { return WorkTarget.Get(); }

#pragma region Capture
	float								GetHealthRatio() const;
	float								GetCaptureRate() const;

	FDC1PalInstance						Fold(); // 데이터화
	
	void								OnCaptureFailed(AActor* InThrower);// 포획 실패
	void								BeginCapture();// 스피어에 빨려 들어감. 결과까지 액터 유지
	void								CancelCapture();// BeginCapture 되돌리기

	// InBasePalBox 가 있으면 거점 팰로 나온다
	static ADC1PalCharacter*			SpawnFromInstanceData(UWorld* InWorld, const FDC1PalInstance& InInstance, const FTransform& InTransform, ADC1PlayerState* InOwner, ADC1PalBoxActor* InBasePalBox = nullptr);
#pragma endregion

	// 탑승 상태
	UDC1PalRideComponent*				GetRideComponent() const { return RideComponent; }

	// 죽었다. 파티가 받아서 자동 회수한다. OnDeath의 맨 끝에서 발화한다
	FDC1OnPalDied						OnPalDied;

	// 체력이 변했다. 머리 위 HP바가 구독한다
	FDC1OnPalHealthChanged				OnPalHealthChanged;

	// SAN 이 변했다. 머리 위 SAN 배지가 구독한다
	FDC1OnPalSanityChanged				OnPalSanityChanged;

protected:
	virtual void						PostInitializeComponents() override;

	virtual void						BeginPlay() override;

	// Event.Exp 수신. 거점 팰만 적용 ( 서버 )
	void								HandleExpEvent(FGameplayTag InEventTag, const FGameplayEventData* InPayload);

	virtual void						NotifyControllerChanged() override;

	// 탑승 입력. IMC_Ride 가 켜져 있는 동안 받는다
	virtual void						SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void								OnRideMove(const FInputActionValue& Value);
	void								OnRideJumpPressed(const FInputActionValue& Value);
	void								OnRideJumpReleased(const FInputActionValue& Value);
	void								OnRideSprintPressed(const FInputActionValue& Value);
	void								OnRideSprintReleased(const FInputActionValue& Value);

	// 탑승 중 스킬. InSkillIndex 는 PalData.SkillTags 순서.
	void								OnRideSkillPressed(const FInputActionValue& Value, int32 InSkillIndex);

	// PalData.SkillTags 마다 그 스킬 스펙을 준다 (서버)
	void								GrantSkillAbilities();

	virtual void						OnTeamChanged();

	void								ApplyTeamCollisionProfile();

	UFUNCTION()
	void								OnRep_Team();

	// 클라는 ApplyPalData 때 알파인지 모를 수 있어서 복제로 받은 뒤 크기를 맞춘다
	UFUNCTION()
	void								OnRep_IsAlpha();

	void								ApplyAlphaScale();

	void								ApplyPalData();

	// PassiveTags 를 GE 로 바꿔 붙인다
	void								ApplyPassives(const UDC1PalRegistryData& InRegistry);

	// 어빌리티 취소 + AI 정지 + 지각 소스 해제. 사망과 포획이 공유한다
	void								StopAndUnregister();

	void								HandleDamaged(const struct FOnAttributeChangeData& Data);

	void								HandleHealthChangedForUI(const struct FOnAttributeChangeData& Data);

	void								HandleSanityChangedForUI(const struct FOnAttributeChangeData& Data);

	// 레벨 변경 → HP바 갱신
	void								HandleLevelChangedForUI(const struct FOnAttributeChangeData& Data);

	void								UpdateHealthWidgetVisibility(); // 위젯을 껐다 켠다. 

	// 래그돌, AI 정지, 퍼셉션 해제
	virtual void						OnDeath() override;

	void								SpawnDrops();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };

	// 피격 판정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UCapsuleComponent> HitboxComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthWidgetComponent = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ClampMin = "0.0"))
	float HealthWidgetVisibleDistance = { 2000.f };

	// 가시성 검사 주기
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ClampMin = "0.05"))
	float HealthWidgetCheckInterval = { 0.2f };

	FTimerHandle HealthWidgetVisibilityTimer;

	// 사망 후 래그돌 시체가 월드에 남아 있는 시간. 서버가 만료 시 액터를 제거한다.
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Death", meta = (ClampMin = "0.0", Units = "s"))
	float CorpseLifeSpan = { 8.f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Ride")
	TObjectPtr<UDC1PalRideComponent> RideComponent = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1PalAttributeSet> PalAttributeSet = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pal", meta = (Categories = "Pal.Species"))
	FGameplayTag SpeciesTag;

	UPROPERTY(Transient) //디스크에 저장X
	TObjectPtr<const UDC1PalData> PalData = { nullptr };

	UPROPERTY(Transient)
	FGuid InstanceID;

	UPROPERTY(Transient, Replicated)
	TObjectPtr<ADC1PalBoxActor> BasePalBox = { nullptr };

	UPROPERTY(Transient, Replicated)
	TObjectPtr<ADC1PlayerState> OwnerPlayerState = { nullptr };

	TWeakInterfacePtr<IDC1WorkTargetInterface> WorkTarget;

	// 이 팰의 데이터. 소환될 때 원본을 통째로 받아두고 회수할 때 그대로 돌려준다.
	UPROPERTY(Transient)
	FDC1PalInstance PalInstance;

	// 마지막 가해자
	TWeakObjectPtr<AActor> LastDamageCauser;

	// 드랍 수평 속도
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Drop", meta = (ClampMin = "0.0"))
	float DropScatterSpeed = { 250.f };

	// 드랍 위쪽 속도
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Drop", meta = (ClampMin = "0.0"))
	float DropLaunchSpeed = { 350.f };

	// 맵에 직접 배치한 개체의 고정 레벨. 0이면 PalData의 MinLevel~MaxLevel을 사용한다.
	// BP 클래스 기본값에서는 설정하지 않고, 배치된 인스턴스에서만 편집한다.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Pal|Level", meta = (ClampMin = "0"))
	int32 InitialLevel = { 0 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Team, Category = "Pal|Team")
	EDC1Team Team = { EDC1Team::Wild };

	// 알파(필드 보스)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsAlpha, Category = "Pal|Alpha")
	bool bIsAlpha = { false };
};
