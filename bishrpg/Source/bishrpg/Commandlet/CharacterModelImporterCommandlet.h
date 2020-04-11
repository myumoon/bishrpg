// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CharacterModelImporterCommandlet.generated.h"

class USkeletalMesh;
class UTexture;
class UMaterial;
class IAssetRegistry;
class IAssetTools;

/**
 * 
 */
UCLASS()
class BISHRPG_API UCharacterModelImporterCommandlet : public UCommandlet
{
	//GENERATED_BODY()
	GENERATED_UCLASS_BODY()
public:
	/*!	インポート
		
	*/
	virtual int32 Main(const FString& CmdLineParams) override;

private:
	struct ParsedParams;
	void               ShowHelp();
	bool               ParseArgs(ParsedParams* out, const FString& params) const;
	USkeletalMesh*     ImportFbx(const FString& fbxPath, const FString& partsName, const FString& destFileName);
	UTexture*          ImportTexture(const FString& texPath, const FString& partsName, const FString& destFileName);
	UMaterial*         MakeMaterialInstance(UTexture* tex, const FString& partsName, const FString& destFileName);
	void               SetMaterialToMesh(USkeletalMesh* mesh, UMaterial* material);

private:
	IAssetRegistry*    AssetRegistry = nullptr;
	IAssetTools*       AssetTools    = nullptr;
};
