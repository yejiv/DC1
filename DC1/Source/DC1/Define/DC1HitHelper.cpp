#include "Define/DC1HitHelper.h"

#include "Components/PrimitiveComponent.h"
#include "GameplayEffect.h"

bool DC1HitHelper::FindHitLocation(const FGameplayEffectSpec& InSpec, const AActor* InTarget, UPrimitiveComponent* InSurface, FVector& OutPoint, FVector& OutNormal)
{
	const FGameplayEffectContextHandle Context = InSpec.GetContext();

	// 히트를 싣는 공격이 대상을 맞힌 히트면 그 자리
	const FHitResult* HitResult = Context.GetHitResult();
	if (nullptr != HitResult && InTarget == HitResult->GetActor() && false == HitResult->bStartPenetrating)
	{
		OutPoint = HitResult->ImpactPoint;
		OutNormal = HitResult->ImpactNormal;
		return true;
	}

	if (false == ::IsValid(InSurface))
	{
		return false;
	}

	const AActor* Causer = Context.GetEffectCauser();
	if (false == ::IsValid(Causer))
	{
		Causer = Context.GetInstigator();
	}

	if (false == ::IsValid(Causer))
	{
		return false;
	}

	// 대상 콜리전만 트레이스 - 공격자를 향한 표면
	// GetClosestPointOnCollision은 볼록 도형만 지원해 복합 콜리전 노드에서 실패 -> LineTraceComponent로 변경
	FHitResult SurfaceHit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(DC1HitLocation), true);

	// 공격자 높이에서 대상 피벗 축으로 수평 -> 나무처럼 높은 대상 위치 맞춤
	// 범위 중심이 아니라 피벗 - 가지가 한쪽으로 쏠린 나무는 범위 중심이 줄기 밖이라 빗나간다
	// 높이는 대상 범위 안으로 - 공격자가 더 높거나 낮아도 표면을 지남
	const FBoxSphereBounds& Bounds = InSurface->Bounds;
	const FVector CauserLocation = Causer->GetActorLocation();
	const FVector Pivot = InSurface->GetComponentLocation();
	const float TraceZ = FMath::Clamp(CauserLocation.Z, Bounds.Origin.Z - Bounds.BoxExtent.Z, Bounds.Origin.Z + Bounds.BoxExtent.Z);
	const FVector Start(CauserLocation.X, CauserLocation.Y, TraceZ);
	const FVector End(Pivot.X, Pivot.Y, TraceZ);

	// 수평으로 빗나가면 ( 중심축이 비어 있는 모양 ) 대상 중심으로 한 번 더
	if (false == InSurface->LineTraceComponent(SurfaceHit, Start, End, Params)
		&& false == InSurface->LineTraceComponent(SurfaceHit, CauserLocation, Bounds.Origin, Params))
	{
		return false;
	}

	OutPoint = SurfaceHit.ImpactPoint;
	OutNormal = SurfaceHit.ImpactNormal;
	return true;
}
