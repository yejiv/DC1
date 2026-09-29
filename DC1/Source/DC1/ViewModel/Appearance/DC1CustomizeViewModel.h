// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "Data/Appearance/DC1AppearanceTypes.h"
#include "DC1CustomizeViewModel.generated.h"

class UDC1AppearanceComponent;
class UDC1SaveSubsystem;

/**
 * 커스터마이징 화면용 ViewModel.
 * 편집 중인 FDC1AppearanceData는 여기가 유일한 소유자다 (PlayerState 없이도 동작해야 함 - 타이틀 화면).
 * 확정 시에만 로컬 저장 + (PlayerState가 있다면) 서버로 반영한다.
 */
UCLASS()
class DC1_API UDC1CustomizeViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	// 로컬 세이브가 있으면 그 값으로, 없으면 기본값으로 편집 세션을 시작한다
	void BeginEdit();

	// 타이틀 화면(PlayerState 없음)에서 미리보기 액터가 자기 컴포넌트를 직접 등록할 때 호출
	void BindPreviewComponent(UDC1AppearanceComponent* InPreviewComponent);

	const FDC1AppearanceData& GetPendingAppearance() const { return PendingAppearance; }

	void SetHair(const FGameplayTag& InTag);
	void SetHead(const FGameplayTag& InTag);
	void SetEye(const FGameplayTag& InTag);
	void SetHairColor(const FLinearColor& InColor);
	void SetSkinColor(const FLinearColor& InColor);
	void SetEyeColor(const FLinearColor& InColor);

	// 확정: 로컬 저장 + (연결돼 있다면) 서버로 반영
	bool ConfirmAppearance();

private:
	UDC1SaveSubsystem* GetSaveSubsystem();

	// 지금 바인딩된 프리뷰 액터 컴포넌트에 PendingAppearance를 그대로 반영
	void ApplyPreview();

private:
	FDC1AppearanceData										PendingAppearance = {};

	// 시드 소스이자 확정(RPC) 대상. BindSource(PlayerState)로만 세팅됨 - 타이틀 화면엔 없음
	TWeakObjectPtr<UDC1AppearanceComponent>		PlayerAppearanceComponent = { nullptr };

	// 편집 중 실시간 렌더 대상. 타이틀/게임 중 상관없이 항상 그 자리 프리뷰 액터의 컴포넌트
	TWeakObjectPtr<UDC1AppearanceComponent>		PreviewComponent = { nullptr };
};
