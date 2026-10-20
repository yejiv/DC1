// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/DC1BuildActor.h"
#include "Interface/DC1InteractableInterface.h"
#include "DC1InteractBuildActor.generated.h"

class ADC1Character;
class ADC1PalCharacter;
class UWidgetComponent;
class UDC1BuildInfoWidget;

// 건물 정보 패널(World UI) 상태 ( 로컬 )
UENUM()
enum class EDC1BuildInfoState : uint8
{
	Hidden = 0,				// 표시 X
	Construction = 1,		// 건설 중 + 포커스
	ProductionNoItem = 2,	// 생산 - 대기 중인 아이템 없음
	ProductionCompact = 3,	// 생산 - 범위 안 ( 간단 표시 )
	ProductionDetail = 4,	// 생산 - 포커스 ( 세부 표시 )
};

DECLARE_MULTICAST_DELEGATE(FDC1OnBuildCompleted);
DECLARE_MULTICAST_DELEGATE(FDC1OnWorkProgressChanged);
DECLARE_MULTICAST_DELEGATE(FDC1OnPalWorkersChanged);

UCLASS()
class DC1_API ADC1InteractBuildActor : public ADC1BuildActor, public IDC1InteractableInterface
{
	GENERATED_BODY()

public:
	// 건설 완료
	FDC1OnBuildCompleted			OnBuildCompleted;
	// 건설 중, 건설 완료 상태에 따른 상호작용 표시 변경
	FDC1OnInteractOptionsChanged	OnInteractOptionsChanged;
	// 작업속도 변화 및 작업 진행도에 따른 진행도 표시 변경
	FDC1OnWorkProgressChanged		OnWorkProgressChanged;
	// 배정된 팰 목록 변경
	FDC1OnPalWorkersChanged			OnPalWorkersChanged;

public:
	ADC1InteractBuildActor();


public:
	const bool						IsUnderConstruction() const { return EDC1BuildState::Construction == BuildState; }
	float							GetBuildProgress() const { return BuildProgress; }
	float							GetCurrentWorkSpeed() const { return CurrentWorkSpeed; }
	EDC1BuildInfoState				GetInfoState() const { return InfoState; }
	float							GetRemainingWork() const;
	const TArray<TWeakObjectPtr<ADC1PalCharacter>>& GetPalWorkers() const { return PalWorkers; }
	bool							IsPalWorkerAssigned(const ADC1PalCharacter* InPal) const;

	virtual FGameplayTag			GetWorkTypeTag() const;

public:
	virtual void					Tick(float DeltaTime) override;

protected:
	virtual void					GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// IDC1InteractableInterface override
	virtual UPrimitiveComponent*	GetInteractCollider() const override;
	virtual void					GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;
	virtual FDC1OnInteractOptionsChanged& GetOnInteractOptionsChanged() override { return OnInteractOptionsChanged; }
	virtual void					Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override final;
	virtual void					OnLocalFocusChanged(bool InIsFocused) override;

public:
	virtual void					InitBuild(const FDC1BuildActorData& InBuildActorData) override;
	virtual void					BuildDestroy() override;

	void							AddWorker(const ADC1Character* InWorker);
	void							RemoveWorker(const ADC1Character* InWorker);

	bool							AssignPalWorker(ADC1PalCharacter* InPal);
	void							UnassignPalWorker(ADC1PalCharacter* InPal);

protected:
	virtual void					ApplyState() override;
	// Completed 상태에서의 상호작용
	virtual void					OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) {}
	// 작업이 가능한 상태인지 ex) IsConstruction
	virtual bool					CanAcceptWorker(const ADC1Character* InWorker) const;
	// 이번 Tick 작업량 소비 ( WorkSpeed 합 * DeltaTime ) - 서버
	virtual void					OnWorkTick(float InWork);
	// 클래스에 따른 InfoState 분기
	virtual EDC1BuildInfoState		ComputeInfoState() const;
	// InfoState에 따른 WidgetClass
	virtual TSubclassOf<UDC1BuildInfoWidget> GetInfoWidgetClass(EDC1BuildInfoState InState) const;

	// 팰이 지금 작업할 수 있는 상태인지
	virtual bool					CanPalWork() const;
	// 팰이 작업할 자리가 남아있는지
	virtual bool					HasPalWorkerSlot() const { return true; }

	void							BuildCompleted();

	// 작업자 집합 비우기 ( 서버 - 진행 정지만 Ability 종료는 클라 강제 해제 )
	void							ClearWorkers();
	void							ClearPalWorkers();

	void							UpdateInfoState();

	
	UFUNCTION()
	void							OnRep_BuildProgress();

	UFUNCTION()
	void							OnRep_CurrentWorkSpeed();

	UFUNCTION()
	void							OnRep_PalWorkers();

private:

	void							CancelWorkersAbility();
	float							ComputeTotalWorkSpeed();

	void							SetMaterialBuildProgress();
	void							SetMaterialFlash();

	// 서버 호출
	void							SetCurrentWorkSpeed(float InWorkSpeed);

	// InfoWidget Bind
	UDC1BuildInfoWidget*			AcquireInfoWidget();
	// InfoWidget Unbind
	void							ReleaseInfoWidget();

	// 팀 / 적성 / 거점 위치
	bool							CanAssignPal(const ADC1PalCharacter* InPal) const;
	// 서버 직접 / 클라 OnRep 수렴점
	void							ApplyPalWorkers();

	// Debug
	void							DebugAssignNearbyPals();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent>				InteractCollider = { nullptr };

	// 건설 중 옵션 F 건설(HoldAbility), C 취소(HoldConfirm)
	UPROPERTY(EditDefaultsOnly, Category = "Build|Interact")
	TArray<FDC1InteractOption>				ConstructionOptions;

	// Completed 후 옵션 - ex) F : 상자 열기, V : 빠른 이동, C : 제작 취소
	UPROPERTY(EditDefaultsOnly, Category = "Build|Interact")
	TArray<FDC1InteractOption>				CompletedOptions;

	// 정보 패널 - Screen Space, Prompt보다 뒤 레이어에 위치. 위젯 클래스는 UpdateInfoState로 지정 ( 건설 중, 생산(ProductionBuildActor) )
	UPROPERTY(EditDefaultsOnly, Category = "Build|UI")
	TObjectPtr<UWidgetComponent>			InfoWidgetComponent = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|UI")
	TSubclassOf<UDC1BuildInfoWidget>		ConstructionInfoWidgetClass = { nullptr };

	UPROPERTY(ReplicatedUsing = OnRep_PalWorkers)
	TArray<TWeakObjectPtr<ADC1PalCharacter>>	PalWorkers;

protected:
	bool									IsLocalFocused = { false };

private:
	// Completed 시점 Flash 지속시간
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	float									CompleteFlashDuration = { 0.4f };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor							ConstructionColor = { FLinearColor::Blue };

	// 건설 진행률
	UPROPERTY(ReplicatedUsing = OnRep_BuildProgress)
	float									BuildProgress = { 0.f };

	// Workers 작업 속도 합
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWorkSpeed)
	float									CurrentWorkSpeed = { 0.f };


	UPROPERTY(EditDefaultsOnly, Category = "Build|Debug", meta = (ClampMin = "0"))
	float									DebugAssignRadius = { 0.f };

private:
	TSet<TWeakObjectPtr<const ADC1Character>>	Workers;
	float									CurrentWork = { 0.f };

	float									FlashAmount = { 0.f };

	EDC1BuildInfoState						InfoState = { EDC1BuildInfoState::Hidden };
};
