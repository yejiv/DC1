// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Preview/DC1PreviewActor.h"
#include "DC1PlayerPreviewActor.generated.h"

class ADC1Player;
class UMeshComponent;
class USkeletalMeshComponent;

// 인벤토리 플레이어 미리보기 (로컬 전용, 플레이어를 복사한 메쉬를 전용 조명으로 촬영)
UCLASS()
class DC1_API ADC1PlayerPreviewActor : public ADC1PreviewActor
{
	GENERATED_BODY()

public:
	ADC1PlayerPreviewActor();

public:
	void CopyFromPlayer(ADC1Player* InPlayer);
	void CopyAttachments(ADC1Player* InPlayer);

private:
	void CopySkeletalMesh(USkeletalMeshComponent* InSource, USkeletalMeshComponent* InCopy);
	void CopyAttachedMesh(UMeshComponent* InSource, USceneComponent* InSourceParent, USkeletalMeshComponent* InCopyParent, FName InSocketName);
	USkeletalMeshComponent* FindCopyParent(const USceneComponent* InSourceParent) const;
	void ClearAttachments();

private:
	// 플레이어 메쉬 복사본 (Body / Head / Hair / HairEquip)
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyBody = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyHead = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyHair = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyHairEquip = { nullptr };

	// 원본 메쉬 -> 복사 메쉬 매핑 (부착 메쉬의 부모 찾기용, CopyFromPlayer마다 초기화)
	TMap<const USceneComponent*, TObjectPtr<USkeletalMeshComponent>> SourceToCopy;

	// 런타임에 복제한 부착 메쉬 (무기/도구 등)
	UPROPERTY()
	TArray<TObjectPtr<UMeshComponent>> AttachmentCopies;
};
