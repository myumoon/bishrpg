// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CharacterModelImporterCommandlet.generated.h"

class USkeletalMesh;
class UTexture;
class UMaterialInterface;
class IAssetRegistry;
class IAssetTools;

/**	モデルインポートコマンドレット
 * 
 *  examples:
 *	  CharacterModelImporter -csv=csvpath
 *    CharacterModelImporter -fbx_path=*.fbx -tex_path=*.png -parts=Lower -filename=filenme
 *
 *  options:
 *    - csv      : Import from csv with a specified csv file.
 *                 Other options are ignored.
 *    - fbx_path : A fbx path to be imported.
 *    - tex_path : A texture path to be imported.
 *    - parts    : This model's category. [Head, Face, Upper, Lower, Accessory]
 *    - filename : This model's file name.
 * 
 *  csv format:
 *    fbx_path,tex_path,parts,filename
 *     :
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
	void                ShowHelp();
	bool                ParseArgs(ParsedParams* out, const FString& params) const;

	//!@{
	bool                ImportFromCsv(const FString& csvPath);
	bool                Import(const FString& fbxPath, const FString& fbxContentPath, const FString& texPath, const FString& texContentPath, const FString& matContentPath, const FString& partsName, const FString& filename);
	bool                IsReimport(const FString& contentPath) const;
	//!@}

	//@{
	USkeletalMesh*      ImportFbx(const FString& fbxPath, const FString& fbxContentPath, const FString& partsName, const FString& destFileName);
	UTexture*           ImportTexture(const FString& texPath, const FString& texContentPath, const FString& partsName, const FString& destFileName);
	UMaterialInterface* MakeMaterialInstance(UTexture* tex, const FString& matContentPath);
	void                SetMaterialToMesh(USkeletalMesh* mesh, UMaterialInterface* material);
	//@}
	//@}

private:
	IAssetRegistry*     AssetRegistry = nullptr;
	IAssetTools*        AssetTools    = nullptr;
};
