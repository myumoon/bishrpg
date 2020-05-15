// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "EditorExtensionFunctionLibrary.h"

#include "bishrpg.h"

void UEditorExtensionFunctionLibrary::SaveSimulationChanges(AActor* sourceActor)
{
#if WITH_EDITOR
	AActor* editorWorldActor = EditorUtilities::GetEditorWorldCounterpartActor(sourceActor);
	if(editorWorldActor != NULL) {
		const auto copyOptions = (EditorUtilities::ECopyOptions::Type)(
			EditorUtilities::ECopyOptions::CallPostEditChangeProperty |
			EditorUtilities::ECopyOptions::CallPostEditMove |
			EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties |
			EditorUtilities::ECopyOptions::FilterBlueprintReadOnly);
		const int32 CopiedPropertyCount = EditorUtilities::CopyActorProperties(sourceActor, editorWorldActor, copyOptions);
	}
#endif
}
