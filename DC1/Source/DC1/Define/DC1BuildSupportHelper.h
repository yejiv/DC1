#pragma once

#include "CoreMinimal.h"

class ADC1BuildActor;

// 지지 판정 - 연결 저장 X , 소켓 Probe 질의로 판정
namespace DC1BuildSupportHelper
{
	// 설치 판정 ( 클라 프리뷰 / 서버 재검사 )
	bool HasSupport(ADC1BuildActor* InBuild);

	// 연쇄 파괴 판정 ( 서버 ) - InOrigin 포함, 파괴될 전체 집합 / 집합만 Out, 직접 파괴 X
	void CollectCascade(ADC1BuildActor* InOrigin, TSet<ADC1BuildActor*>& OutDoomed);
}
