#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("DC1"), STATGROUP_DC1, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("SnapOverlapQuery"), STAT_DC1_SnapOverlapQuery, STATGROUP_DC1);
DECLARE_CYCLE_STAT(TEXT("FindSnapPair"), STAT_DC1_FindsnapPair, STATGROUP_DC1);

DECLARE_CYCLE_STAT(TEXT("SnapBlockTest"), STAT_DC1_SnapBlockTest, STATGROUP_DC1);

DECLARE_CYCLE_STAT(TEXT("BuildCollectCascade"), STAT_DC1_BuildCollectCascade, STATGROUP_DC1);
/*
선언 후
ex)
{
	SCOPE_CYCLE_COUNTER(STAT_DC1_SnapOverlapQuery);
}

사용, 해당 코드 블럭 성능 수치화
*/