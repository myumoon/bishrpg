// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "EditorExtensionFunctionLibraryEd.h"

#include "Editor.h"
#include "ImageUtils.h"
#include "ObjectTools.h"
#include "bishrpgEd.h"

int32 UEditorExtensionFunctionLibraryEd::SaveSimulationChangesEd(AActor* sourceActor)
{
#if WITH_EDITOR
	AActor* editorWorldActor = EditorUtilities::GetEditorWorldCounterpartActor(sourceActor);
	if(editorWorldActor != NULL) {
	#if 0
		const auto copyOptions = (EditorUtilities::ECopyOptions::Type)(
			EditorUtilities::ECopyOptions::CallPostEditChangeProperty |
			EditorUtilities::ECopyOptions::CallPostEditMove |
			EditorUtilities::ECopyOptions::PropagateChangesToArchetypeInstances);
	#else
		const auto copyOptions = (EditorUtilities::ECopyOptions::Type)(
			EditorUtilities::ECopyOptions::CallPostEditChangeProperty |
			EditorUtilities::ECopyOptions::CallPostEditMove |
			EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties |
			EditorUtilities::ECopyOptions::FilterBlueprintReadOnly);
	#endif
		const int32 CopiedPropertyCount = EditorUtilities::CopyActorProperties(sourceActor, editorWorldActor, copyOptions);
		return CopiedPropertyCount;
	}
#endif
	return 0;
}

UTexture2D* UEditorExtensionFunctionLibraryEd::FindCachedThumbnailByObjectEd(UObject* object)
{
#if WITH_EDITOR
	EDITOR_LOG("FindCachedThumbnailByObject")
	const FObjectThumbnail* thumbnailObj = ThumbnailTools::GetThumbnailForObject(object);
	if(!thumbnailObj) {
		EDITOR_ASSERT(thumbnailObj);
		return nullptr;
	}
	auto* texture = FImageUtils::ImportBufferAsTexture2D(thumbnailObj->GetUncompressedImageData());
	EDITOR_ASSERT(texture);
	return texture;
#else
	return nullptr;
#endif
}

UTexture2D* UEditorExtensionFunctionLibraryEd::FindCachedThumbnailByNameEd(const FString& name)
{
#if WITH_EDITOR
	EDITOR_LOG_FMT("FindCachedThumbnailByObject({0})", *name);
	const FObjectThumbnail* thumbnailObj = ThumbnailTools::FindCachedThumbnail(name);
	if(!thumbnailObj) {
		EDITOR_ASSERT(thumbnailObj);
		return nullptr;
	}
	auto* texture = FImageUtils::ImportBufferAsTexture2D(thumbnailObj->GetUncompressedImageData());
	EDITOR_ASSERT(texture);
	return texture;
#else
	return nullptr;
#endif
}
