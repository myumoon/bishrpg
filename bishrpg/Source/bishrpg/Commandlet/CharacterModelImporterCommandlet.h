// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CharacterModelImporterCommandlet.generated.h"

/**
 * 
 */
UCLASS()
class BISHRPG_API UCharacterModelImporterCommandlet : public UCommandlet
{
	//GENERATED_BODY()
	GENERATED_UCLASS_BODY()
public:
	virtual int32 Main(const FString& CmdLineParams) override;
};
