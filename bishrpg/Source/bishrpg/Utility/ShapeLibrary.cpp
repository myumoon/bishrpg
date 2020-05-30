// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "ShapeLibrary.h"

#include "Math/Sphere.h"
#include "Math/UnrealMathUtility.h"
#include "bishrpg.h"

// 統計情報
DECLARE_STATS_GROUP(TEXT("ShapeLibraryGroup"), STATGROUP_ShapeLibrary, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("UShapeLibrary::IntersectsSphereAndAABB"), STAT_IntersectsSphereAndAABB, STATGROUP_ShapeLibrary);
DECLARE_CYCLE_STAT(TEXT("UShapeLibrary::IntersectsAABBAndAABB"), IntersectsAABBAndAABB, STATGROUP_ShapeLibrary);
DECLARE_CYCLE_STAT(TEXT("UShapeLibrary::IntersectsSphereAndSphere"), IntersectsSphereAndSphere, STATGROUP_ShapeLibrary);

void UShapeLibrary::MakeAABBWithBeginEnd(FShapeAABB& aabb, const FVector& begin, const FVector& end)
{
	const FVector size   = { end.X - begin.X, end.Y - begin.Y, end.Z - begin.Z };
	const FVector center = { begin.X + size.X * 0.5f, begin.Y + size.Y * 0.5f, begin.Z + size.Z * 0.5f };
	MakeAABBWithCenterSize(aabb, center, size);
}

void UShapeLibrary::MakeAABBWithCenterSize(FShapeAABB& aabb, const FVector& center, const FVector& size)
{
	const FVector halfSize = size / 2.0f;
	aabb.Box = FBox::BuildAABB(center, halfSize);
	//aabb.Box.Begin.X = center.X - halfSize.X;
	//aabb.Box.Begin.Y = center.Y - halfSize.Y;
	//aabb.Box.Begin.Z = center.Z - halfSize.Z;
	//aabb.Box.End.X   = center.X + halfSize.X;
	//aabb.Box.End.Y   = center.Y + halfSize.Y;
	//aabb.Box.End.Z   = center.Z + halfSize.Z;
}

void UShapeLibrary::MakeSphere(FShapeSphere& sphere, const FVector& center, float radius)
{
	sphere.Sphere = FSphere(center, radius);
}

void UShapeLibrary::GetSpherePosAndRadius(FVector& center, float& radius, const FShapeSphere& sphere)
{
	center = sphere.Sphere.Center;
	radius = sphere.Sphere.W;
}

void UShapeLibrary::GetSphereAABB(FShapeAABB& aabb, const FShapeSphere& sphere)
{
	MakeAABBWithCenterSize(aabb, sphere.Sphere.Center, FVector(sphere.Sphere.W, sphere.Sphere.W, sphere.Sphere.W));
}

void UShapeLibrary::GetAABBRange(FVector& begin, FVector& end, const FShapeAABB& aabb)
{
	begin = aabb.Box.Min;
	end   = aabb.Box.Max;
}

void UShapeLibrary::GetAABBCenterPosAndRange(FVector& center, FVector& size, const FShapeAABB& aabb)
{
	center = aabb.Box.GetCenter();
	size   = aabb.Box.GetSize();
}

bool UShapeLibrary::IntersectsSphereAndAABB(const FShapeSphere& sphere, const FShapeAABB& aabb)
{
	SCOPE_CYCLE_COUNTER(STAT_IntersectsSphereAndAABB);

	return FMath::SphereAABBIntersection(sphere.Sphere, aabb.Box);
}

bool UShapeLibrary::IntersectsAABBAndAABB(const FShapeAABB& aabb1, const FShapeAABB& aabb2)
{
	SCOPE_CYCLE_COUNTER(IntersectsAABBAndAABB);

	return aabb1.Box.Intersect(aabb2.Box);
}

bool UShapeLibrary::IntersectsSphereAndSphere(const FShapeSphere& sphere1, const FShapeSphere& sphere2)
{
	SCOPE_CYCLE_COUNTER(IntersectsSphereAndSphere);

	return sphere1.Sphere.Intersects(sphere2.Sphere);
}
