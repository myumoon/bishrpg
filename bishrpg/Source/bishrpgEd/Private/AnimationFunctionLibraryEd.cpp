// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "AnimationFunctionLibraryEd.h"
#include "Animation/AnimSequence.h"
#include "AssetData.h"
#include "EditorAnimUtils.h"

void UAnimationFunctionLibraryEd::RetargetAnimationsEd(USkeleton* newSkeleton, const TArray<UAnimSequence*> assetsToRetarget, bool remapReferencedAssets, bool convertSpaces, bool duplicate, const FRenameRuleEd& duplicationRenameRule)
{
	if(!newSkeleton || assetsToRetarget.Num() == 0) {
		return;
	}

	TArray<TWeakObjectPtr<UObject>> objectToRetarget;
	for(UAnimSequence* animSequence : assetsToRetarget)
	{
		objectToRetarget.Empty();
		objectToRetarget.Add(animSequence);

		if(duplicate) {
			EditorAnimUtils::FNameDuplicationRule duplicationRule;
			duplicationRule.Prefix		= duplicationRenameRule.Prefix;
			duplicationRule.Suffix		= duplicationRenameRule.Suffix;
			duplicationRule.ReplaceFrom	= duplicationRenameRule.ReplaceFrom;
			duplicationRule.ReplaceTo	= duplicationRenameRule.ReplaceTo;
			duplicationRule.FolderPath	= duplicationRenameRule.FolderPath;
			EditorAnimUtils::RetargetAnimations(animSequence->GetSkeleton(), newSkeleton, objectToRetarget, remapReferencedAssets, &duplicationRule, convertSpaces);
		}
		else {
			EditorAnimUtils::RetargetAnimations(animSequence->GetSkeleton(), newSkeleton, objectToRetarget, remapReferencedAssets, nullptr, convertSpaces);
		}
	}
}

void UAnimationFunctionLibraryEd::RetargetAnimationEd(USkeleton* newSkeleton, UAnimSequence* assetToRetarget, bool remapReferencedAssets, bool convertSpaces, bool duplicate, const FRenameRuleEd& duplicateRenameRule)
{
	TArray<UAnimSequence*> assetsToRetarget;
	assetsToRetarget.Add(assetToRetarget);
	RetargetAnimationsEd(newSkeleton, assetsToRetarget, remapReferencedAssets, convertSpaces, duplicate, duplicateRenameRule);
}
