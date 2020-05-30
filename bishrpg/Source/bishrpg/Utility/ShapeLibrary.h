// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShapeLibrary.generated.h"

USTRUCT(BlueprintType)
struct BISHRPG_API FShapeAABB
{
	GENERATED_BODY()

	FBox Box;
	//FVector Begin;
	//FVector End;
};

USTRUCT(BlueprintType)
struct BISHRPG_API FShapeSphere
{
	GENERATED_BODY()

	FSphere   Sphere;
	//FVector Center;
	//float   R;
};

/**
 * 
 */
UCLASS()
class BISHRPG_API UShapeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/*!	AABBを生成
	*/
	UFUNCTION(BlueprintPure, Category = "AABB")
	static void MakeAABBWithBeginEnd(FShapeAABB& aabb, const FVector& begin, const FVector& end);

	/*!	AABBを生成
	*/
	UFUNCTION(BlueprintPure, Category = "AABB")
	static void MakeAABBWithCenterSize(FShapeAABB& aabb, const FVector& center, const FVector& size);

	/*!	球を生成
	*/
	UFUNCTION(BlueprintPure, Category = "Sphere")
	static void MakeSphere(FShapeSphere& sphere, const FVector& center, float radius);

	/*!	球の位置とサイズを取得
	*/
	UFUNCTION(BlueprintPure, Category = "Sphere")
	static void GetSpherePosAndRadius(FVector& center, float& radius, const FShapeSphere& sphere);

	/*!	球からAABB範囲を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "Shape")
	static void GetSphereAABB(FShapeAABB& aabb, const FShapeSphere& sphere);

	/*!	AABBの範囲を取得
	*/
	UFUNCTION(BlueprintPure, Category = "AABB")
	static void GetAABBRange(FVector& begin, FVector& end, const FShapeAABB& aabb);

	/*!	AABBの中心位置とサイズを取得
	*/
	UFUNCTION(BlueprintPure, Category = "AABB")
	static void GetAABBCenterPosAndRange(FVector& center, FVector& size, const FShapeAABB& aabb);

	/*!	SphereとAABBの衝突判定
	*/
	UFUNCTION(BlueprintCallable, Category = "Collision")
	static bool IntersectsSphereAndAABB(const FShapeSphere& sphere, const FShapeAABB& aabb);

	/*!	AABBとAABBの衝突判定
	*/
	UFUNCTION(BlueprintCallable, Category = "Collision")
	static bool IntersectsAABBAndAABB(const FShapeAABB& aabb1, const FShapeAABB& aabb2);

	/*!	SphereとSphereの衝突判定
	*/
	UFUNCTION(BlueprintCallable, Category = "Collision")
	static bool IntersectsSphereAndSphere(const FShapeSphere& sphere1, const FShapeSphere& sphere2);
};
