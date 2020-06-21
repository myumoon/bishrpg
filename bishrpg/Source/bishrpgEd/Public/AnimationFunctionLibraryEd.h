// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AnimationFunctionLibraryEd.generated.h"


/**	アセットリネームルール

	EditorAnimUtils::FNameDuplicationRuleをエディタから扱えるようにしたもの。
*/
USTRUCT(BlueprintType)
struct FRenameRuleEd {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString Prefix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString Suffix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString ReplaceFrom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString ReplaceTo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString FolderPath;
};

/**
 * 
 */
UCLASS()
class BISHRPGED_API UAnimationFunctionLibraryEd : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

	/**	アニメーションをリターゲット

		@param	newSkeleton				リターゲット後のスケルトン
		@param	assetsToRetarget		リターゲット対象
		@param	remapReferencedAssets	true の場合、assetsToRetarget のアセットによって参照されたアセットをリターゲットします。false の場合、参照はクリアされます。
		@param	convertSpaces			新しいターゲットにマッチするようにアニメーションのコンポーネント空間で変換を行います。
		@param	duplicate				アセットを複製します。 trueの場合はduplicateRenameRuleのルールに従って複製されます。
		@param	duplicationRenameRule	複製時のリネームルール
	*/	
	UFUNCTION(BlueprintCallable)
	static void RetargetAnimationsEd(USkeleton* newSkeleton, const TArray<UAnimSequence*> assetsToRetarget, bool remapReferencedAssets, bool convertSpaces, bool duplicate, const FRenameRuleEd& duplicationRenameRule);

	/**	アニメーションをリターゲット

		@param	newSkeleton				リターゲット後のスケルトン
		@param	assetToRetarget			リターゲット対象
		@param	remapReferencedAssets	true の場合、assetsToRetarget のアセットによって参照されたアセットをリターゲットします。false の場合、参照はクリアされます。
		@param	convertSpaces			新しいターゲットにマッチするようにアニメーションのコンポーネント空間で変換を行います。
		@param	duplicate				アセットを複製します。 trueの場合はduplicateRenameRuleのルールに従って複製されます。
		@param	duplicationRenameRule	複製時のリネームルール
	*/
	UFUNCTION(BlueprintCallable)
	static void RetargetAnimationEd(USkeleton* newSkeleton, UAnimSequence* assetsToRetarget, bool remapReferencedAssets, bool convertSpaces, bool duplicate, const FRenameRuleEd& duplicationRenameRule);
};
