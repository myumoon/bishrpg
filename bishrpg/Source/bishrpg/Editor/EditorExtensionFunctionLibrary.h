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
		@returns	コピー済みのオプション数を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "EditorOnly")
	static int32 SaveSimulationChanges(AActor* sourceActor);

	/**	サムネイルを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "EditorOnly")
	static UTexture2D* FindCachedThumbnailByObject(UObject* object);

	/**	サムネイルを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "EditorOnly")
	static UTexture2D* FindCachedThumbnailByName(const FString& name);
};
