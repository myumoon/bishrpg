// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetData.h"
#include "ContentBrowserFunctionLibrary.generated.h"

/**	コンテンツブラウザ操作
 * 
 */
UCLASS()
class BISHRPG_API UContentBrowserFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**	選択されているアセットのパスを取得
	*/
	UFUNCTION(BlueprintCallable)
	static void GetSelectedAssets(TArray<FString>& assetPaths);

	/**	選択されているフォルダパスを取得
	*/
	UFUNCTION(BlueprintCallable)
	static void GetSelectedDirs(TArray<FString>& dirPaths);
	
	/**	コンテンツブラウザ上での表示パスを指定
	*/
	UFUNCTION(BlueprintCallable)
	static void SetSelectedPath(const FString& folderPath, bool needsRefresh = true);
};
