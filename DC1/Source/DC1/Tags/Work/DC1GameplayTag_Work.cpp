// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Work/DC1GameplayTag_Work.h"

// 옛 이름 Pal.Work.* → Config/DefaultGameplayTags.ini GameplayTagRedirects ( 에셋 재저장 후 제거 )
namespace DC1WorkTags
{
	UE_DEFINE_GAMEPLAY_TAG(Work_Building, "Work.Building");
	UE_DEFINE_GAMEPLAY_TAG(Work_Kindling, "Work.Kindling");
	UE_DEFINE_GAMEPLAY_TAG(Work_Watering, "Work.Watering");
	UE_DEFINE_GAMEPLAY_TAG(Work_Planting, "Work.Planting");
	UE_DEFINE_GAMEPLAY_TAG(Work_Electricity, "Work.Electricity");
	UE_DEFINE_GAMEPLAY_TAG(Work_Handiwork, "Work.Handiwork");
	UE_DEFINE_GAMEPLAY_TAG(Work_Gathering, "Work.Gathering");
	UE_DEFINE_GAMEPLAY_TAG(Work_Lumbering, "Work.Lumbering");
	UE_DEFINE_GAMEPLAY_TAG(Work_Mining, "Work.Mining");
	UE_DEFINE_GAMEPLAY_TAG(Work_Medicine, "Work.Medicine");
	UE_DEFINE_GAMEPLAY_TAG(Work_Cooling, "Work.Cooling");
	UE_DEFINE_GAMEPLAY_TAG(Work_Transporting, "Work.Transporting");
	UE_DEFINE_GAMEPLAY_TAG(Work_Farming, "Work.Farming");
}
