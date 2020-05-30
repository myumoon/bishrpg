// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "ShapeLibrary.h"

#include "Math/Sphere.h"
#include "Math/UnrealMathUtility.h"
#include "bishrpg.h"



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
	// 球の中心に近いところを探す
	//const float x = FMath::Max(aabb.Box.Min.X, FMath::Min(sphere.Sphere.Center.X, aabb.Box.Max.X));
	//const float y = FMath::Max(aabb.Box.Min.Y, FMath::Min(sphere.Sphere.Center.Y, aabb.Box.Max.Y));
	//const float z = FMath::Max(aabb.Box.Min.Z, FMath::Min(sphere.Sphere.Center.Z, aabb.Box.Max.Z));

	//const float dist2 = FMath::Pow(x - sphere.Sphere.Center.X, 2) + FMath::Pow(y - sphere.Sphere.Center.Y, 2) + FMath::Pow(z - sphere.Sphere.Center.Z, 2);
	//return (dist2 < sphere.Sphere.W * sphere.Sphere.W);
	return FMath::SphereAABBIntersection(sphere.Sphere, aabb.Box);
}

bool UShapeLibrary::IntersectsAABBAndAABB(const FShapeAABB& aabb1, const FShapeAABB& aabb2)
{
	//return (aabb1.Begin.X <= aabb2.End.X && aabb1.End.X >= aabb2.Begin.X) &&
	//	(aabb1.Begin.Y <= aabb2.End.Y && aabb1.End.Y >= aabb2.Begin.Y) &&
	//	(aabb1.Begin.Z <= aabb2.End.Z && aabb1.End.Z >= aabb2.Begin.Z);
	return aabb1.Box.Intersect(aabb2.Box);
}

bool UShapeLibrary::IntersectsSphereAndSphere(const FShapeSphere& sphere1, const FShapeSphere& sphere2)
{
	//return FMath::Pow(sphere2.Center.X - sphere1.Center.X, 2) + FMath::Pow(sphere2.Center.Y - sphere1.Center.Y, 2) + FMath::Pow(sphere2.Center.Z - sphere1.Center.Z, 2) <= FMath::Pow(sphere1.R + sphere2.R, 2);
	return sphere1.Sphere.Intersects(sphere2.Sphere);
}
