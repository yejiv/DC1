// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Preview/DC1PreviewActor.h"

#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

ADC1PreviewActor::ADC1PreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	CharacterPivot = CreateDefaultSubobject<USceneComponent>("CharacterPivot");
	CharacterPivot->SetupAttachment(Root);

	// 전용 조명 - 정면 위 키 라이트 + 반대편 필 라이트, 채널 2만 비춤
	KeyLight = CreateDefaultSubobject<UPointLightComponent>("KeyLight");
	KeyLight->SetupAttachment(Root);
	KeyLight->LightingChannels.bChannel0 = false;
	KeyLight->LightingChannels.bChannel2 = true;

	FillLight = CreateDefaultSubobject<UPointLightComponent>("FillLight");
	FillLight->SetupAttachment(Root);
	FillLight->LightingChannels.bChannel0 = false;
	FillLight->LightingChannels.bChannel2 = true;

	// 정면에서 캐릭터를 바라봄, 자기 컴포넌트만 촬영
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("CaptureComponent");
	CaptureComponent->SetupAttachment(Root);
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;

	// 월드 환경광/하늘/안개가 섞이지 않도록 촬영 화면에서만 끔 (이름 기반이라 없는 플래그는 무시됨)
	TArray<FEngineShowFlagsSetting> ShowFlagSettings;
	for (const TCHAR* FlagName : { TEXT("Atmosphere"), TEXT("Fog"), TEXT("VolumetricFog"), TEXT("Cloud"), TEXT("SkyLighting"),
		TEXT("DirectionalLights"), TEXT("GlobalIllumination"), TEXT("LumenGlobalIllumination"), TEXT("LumenReflections"), TEXT("AmbientOcclusion") })
	{
		FEngineShowFlagsSetting Setting;
		Setting.ShowFlagName = FlagName;
		Setting.Enabled = false;
		ShowFlagSettings.Add(Setting);
	}
	CaptureComponent->SetShowFlagSettings(ShowFlagSettings);
}

void ADC1PreviewActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyPreviewSettings();
}

#if WITH_EDITOR
void ADC1PreviewActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// PIE 중 스폰된 액터는 OnConstruction이 다시 돌지 않으므로 직접 반영
	ApplyPreviewSettings();
}
#endif

void ADC1PreviewActor::ApplyPreviewSettings()
{
	// CDO/BP 템플릿의 컴포넌트를 건드리면 재인스턴싱 중 크래시가 날 수 있어 실제 인스턴스만 처리
	if (true == IsTemplate())
	{
		return;
	}

	CharacterPivot->SetRelativeLocationAndRotation(CharacterOffset, CharacterRotation);

	KeyLight->SetRelativeLocation(KeyLightOffset);
	KeyLight->SetIntensity(KeyLightIntensity);
	KeyLight->SetLightColor(KeyLightColor);

	FillLight->SetRelativeLocation(FillLightOffset);
	FillLight->SetIntensity(FillLightIntensity);
	FillLight->SetLightColor(FillLightColor);

	ApplyPreviewCameraSetting();
}

void ADC1PreviewActor::SetCameraSetting(float InDistance, float InHeight, float InRight, float InYaw, float InPitch, float InFov)
{
	CameraDistance = InDistance;
	CameraHeight = InHeight;
	CameraRight = InRight;
	CameraYaw = InYaw;
	CameraPitch = InPitch;
	CameraFOV = InFov;

	ApplyPreviewCameraSetting();
}

void ADC1PreviewActor::ApplyPreviewCameraSetting()
{
	// 카메라 오른쪽 = -Y
	const FVector CameraLocation = FRotator(0.f, CameraYaw, 0.f).RotateVector(FVector(CameraDistance, -CameraRight, CameraHeight));
	CaptureComponent->SetRelativeLocationAndRotation(CameraLocation, FRotator(-CameraPitch, 180.f + CameraYaw, 0.f));
	CaptureComponent->FOVAngle = CameraFOV;
}

void ADC1PreviewActor::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	CaptureComponent->TextureTarget = InRenderTarget;
}

void ADC1PreviewActor::SetCaptureEnabled(bool bInEnabled)
{
	CaptureComponent->bCaptureEveryFrame = bInEnabled;
}

void ADC1PreviewActor::ApplyPreviewLighting(UPrimitiveComponent* InComponent)
{
	// 월드 조명(채널 0)은 받지 않고 전용 조명(채널 2)만 받음
	InComponent->SetLightingChannels(false, false, true);
	InComponent->SetCastShadow(false);
}

void ADC1PreviewActor::SetShowOnlyActor(ADC1PreviewActor* InPreviewActor)
{
	CaptureComponent->ShowOnlyActors.Reset();
	CaptureComponent->ShowOnlyActors.Add(InPreviewActor);
}
