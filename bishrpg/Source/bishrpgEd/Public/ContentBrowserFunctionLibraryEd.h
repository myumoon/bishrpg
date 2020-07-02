// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetData.h"
#include "ContentBrowserFunctionLibraryEd.generated.h"

/**	コンテンツブラウザ操作
 * 
 */
UCLASS()
class BISHRPGED_API UContentBrowserFunctionLibraryEd : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**	選択されているアセットのパスを取得
	*/
	UFUNCTION(BlueprintCallable)
	static void GetSelectedAssetsEd(TArray<FString>& assetPaths);

	/**	選択されているフォルダパスを取得
	*/
	UFUNCTION(BlueprintCallable)
	static void GetSelectedDirsEd(TArray<FString>& dirPaths);
	
	/**	コンテンツブラウザ上での表示パスを指定
	*/
	UFUNCTION(BlueprintCallable)
	static void SetSelectedPathEd(const FString& folderPath, bool needsRefresh = true);

	/**	コンテンツブラウザ上での表示パスを指定
	*/
	UFUNCTION(BlueprintCallable)
	static void BishrpgEdTest();
};
