// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MortonIndex.generated.h"


/*!	モートン番号
*/
USTRUCT(BlueprintType)
struct BISHRPG_API FMortonIndex {
	GENERATED_BODY()

	int32 Index = 0;
};

/**	モートン番号をBPで取り扱うためのライブラリ
 * 
 */
UCLASS()
class BISHRPG_API UMortonIndexFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/*!	有効かどうかを判定
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	static bool IsValid(const FMortonIndex& mortonIndex);
};
