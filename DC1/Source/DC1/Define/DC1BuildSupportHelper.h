#pragma once

#include "CoreMinimal.h"

struct FDC1BuildStructureLink;
class ADC1BuildActor;

namespace DC1BuildSupportHelper
{
	// 설치 시점 링크 생성
	void	BuildLinks(const ADC1BuildActor* InBuild, TArray<FDC1BuildStructureLink>& OutLinks);

	// 해당 건축물이 지지를 받고 있는가
	bool	HasSupport(ADC1BuildActor* InBuild);

	// 해당 건축물에 연결되어있는 건축물들 가져오기 - 파괴 전에 호출
	void	GatherNeighbors(const ADC1BuildActor* InBuild, TArray<ADC1BuildActor*>& OutNeighbors);
}