// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EditorExtensionFunctionLibraryEd.generated.h"

/**
 * 
 */
UCLASS()
class BISHRPGED_API UEditorExtensionFunctionLibraryEd : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/**	エディタ実行中のプロパティを保存
		@returns	コピー済みのオプション数を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "EditorOnly")
	static int32 SaveSimulationChangesEd(AActor* sourceActor);

	/**	サムネイルを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "EditorOnly")
	static UTexture2D* FindCachedThumbnailByObjectEd(UObject* object);

	/**	サムネイルを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "EditorOnly")
	static UTexture2D* FindCachedThumbnailByNameEd(const FString& name);
};
