// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EditorExtensionFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class BISHRPG_API UEditorExtensionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/**	エディタ実行中のプロパティを保存
	*/
	UFUNCTION(BlueprintCallable, Category = "EditorOnly")
	static void SaveSimulationChanges(AActor* sourceActor);
};
