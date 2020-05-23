// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class BISHRPG_API UCameraFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/*!	マウスでのカメラ回転
	*/
	UFUNCTION(BlueprintCallable, Category = "Camera")
	static bool CalcCameraRotationFromAnchor(FRotator& Result, const FVector& AnchorVector, const FVector2D& PointerUV, float FovXInDegree, float AspectRatio);
};
