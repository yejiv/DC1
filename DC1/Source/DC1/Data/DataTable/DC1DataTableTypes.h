// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "DC1DataTableTypes.generated.h"

class ADC1Equip;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct FDC1LoadingTipRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = "true"))
	FText Tip;
};

USTRUCT(BlueprintType)
struct FDC1TechRow : public FTableRowBase
{
	GENERATED_BODY()

	// DataTable에서 특정 테크를 찾기 위한 Key
	// 해당 테크 Tag - ex) Build_Crafting_CraftAndRepair_WorkBench00
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag TechTag;

	// 활성화 및 배치될 레벨 ( 플레이어 레벨 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredLevel;

	// 테크 이름 - ex) 원시적인 작업대
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	// 분류 이름 - ex) 건축물, 아이템
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText CategoryName;

	// Icon Image
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	// 해금 비용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UnlockCost;

	// 설명 ( 툴팁 전용, 비어있을 시 BuildData의 Description 표시 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
	FText Description;
};

USTRUCT(BlueprintType)
struct FDC1ItemCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	int32 Count;
};

USTRUCT(BlueprintType)
struct FDC1BuildRow : public FTableRowBase
{
	GENERATED_BODY()

	// 해금 TechTag - 1:N 관계 ( ex) TechTag = Build_Building_Wood 인 경우 해금되는 Build는 Build_Building_Wood_Floor, Build_Building_Wood_Wall 등등 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag RequiredTechTag;

	//	4단계 고정 - ex) Build_Crafting_CraftAndRepair_WorkBench00
	//	Page = Build_Crafing 
	//	Row = Build_Crafting_CraftAndRepair
	//	Slot = Build_Crafting_CraftAndRepair_WorkBench00
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag BuildTag;

	// Row 내부 정렬 순서
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SortOrder;

	// Build 비용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDC1ItemCost> BuildCosts;

	// Build 시간 ( 작업량 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RequiredWork;

	// 건축물 이름 ( 툴팁 전용 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;
	// 설명 ( 툴팁 전용 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
	FText Description;
	// 건축물 요구 적성
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Work"))
	FGameplayTag WorkTypeTag;

	// Icon Image
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	// Build Actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> BuildActorClass;
};

USTRUCT(BlueprintType)
struct FDC1ItemRow : public FTableRowBase
{
	GENERATED_BODY()

	// DataTable에서 특정 테크를 찾기 위한 Key
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CategoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText	ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDC1ItemRarity ItemRarity = EDC1ItemRarity::Common;

	// 작업대 RankMax 이하만 제작 가능 - 레어도와 별개 ( 메가 스피어 = Common / Rank 2 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 Rank = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStack = 9999;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
	FText Description = FText::FromString(TEXT("설명 텍스트"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ItemIcon;

	// PickupItem이 사용할 Mesh ( World에 Spawn 되었을 때 Mesh, 비워두면 기본 메쉬 사용(주머니) ) 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> WorldMesh;
};

// 귀중품 전용 데이터 (DataTable_ValuableItem에 있으면 귀중품 - 상자 보관/버리기 불가, 무게 제외, 스택 제한 없음)
USTRUCT(BlueprintType)
struct FDC1ValuableItemRow : public FDC1ItemRow
{
	GENERATED_BODY()

	// 먹이 주머니가 여는 음식 칸 수 (0이면 주머니 아님, 여러 개면 가장 큰 값만 적용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "5"))
	int32 FoodPouchSlotCount = 0;
};

// 장비 아이템 전용 데이터 (FDC1ItemRow와 ItemID로 연결, 행이 있어야 장착 가능)
USTRUCT(BlueprintType)
struct FDC1EquipItemRow : public FTableRowBase
{
	GENERATED_BODY()

	// FDC1ItemRow::ItemID와 같은 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	// 장착 가능한 장비 칸
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDC1EquipSlotType EquipSlotType = EDC1EquipSlotType::Weapon;

	// 장착 시 스폰할 액터 ( 현재는 무기 칸만 사용 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<ADC1Equip> EquipClass;

	// 장착 시 교체할 메시 ( 방어구 칸만 사용 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> EquipMesh;

	// HeadEquip 전용, 어느 뼈에 붙을것인지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachBoneName;
	// HeadEquip 전용, Attach Offset
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform AttachOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 AttackPower = 0;
	// 벌목 데미지 배율 ( 도끼 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float LumberingDamageRate = 1.f;
	// 채굴 데미지 배율 ( 곡괭이 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float MiningDamageRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 DefensePower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 Health = 0;

	// 활공 속도 ( 글라이더 칸만 사용 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float Speed = 0.f;
};

// 음식 아이템 전용 데이터 ( FDC1ItemRow와 ItemID로 연결, 행이 있어야하고 + Item타입이 Food여야 먹을 수 있음 )
USTRUCT(BlueprintType)
struct FDC1FoodItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetFoodItemIDOptions"))
	FName ItemID;

	// 포만도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float SatietyAmount = 0.f;
};

// 레시피 재료 - 건축 비용( FDC1ItemCost )과 분리해 에디터 표시를 레시피에만 적용
USTRUCT(BlueprintType)
struct FDC1RecipeItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 Count = 1;

#if WITH_EDITORONLY_DATA
	// 배열 항목 제목 ( 목재 ×30 )
	UPROPERTY(VisibleAnywhere, Category = "Editor")
	FString EditorLabel;
#endif
};

// 작업대 제작 레시피 - 제작 가능한 작업대는 완성품의 CategoryTag / Rank로 판정
USTRUCT(BlueprintType)
struct FDC1ItemRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// FTableRowBase 가상 함수 - 행을 편집하거나 임포트할 때 엔진이 호출
	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override;

private:
	void RefreshEditorLabels();

public:
	// 행 이름이 아니라 이 값으로 식별
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RecipeID;

#if WITH_EDITORONLY_DATA
	// 테이블에서 보는 요약 ( 메가 스피어 ×1 ← 팰지움 파편 ×1, ... )
	UPROPERTY(VisibleAnywhere, Category = "Editor")
	FString EditorSummary;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName ProductItemID;

	// 1회 제작 시 나오는 개수 ( 화살 = 10 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 ProductCount = 1;

	// 1회 제작에 필요한 재료
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (TitleProperty = "EditorLabel"))
	TArray<FDC1RecipeItem> RequiredItems;

	// 1회 제작 작업량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.01"))
	float RequiredWork = 1.f;

	// 비어 있으면 해금 조건 없음
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag RequiredTechTag;

	// None이 아니면 이 설계도를 플레이어 인벤토리에 가지고 있어야 표시 ( 소모 X )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetSchematicItemIDOptions"))
	FName RequiredSchematicItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SortOrder = 0;
};