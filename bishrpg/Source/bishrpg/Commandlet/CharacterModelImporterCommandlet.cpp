// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "CharacterModelImporterCommandlet.h"
#include <cctype>
#include "UObject/ScriptInterface.h"
#include "Engine/Texture.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "Factories/ReimportFbxSkeletalMeshFactory.h"
#include "Materials/Material.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetImportTask.h"
#include "ObjectTools.h"
//#include "FbxImporter.h"

#include "Commandlet/Utility/FileUtil.h"

//#include "bishrpg.h"


DEFINE_LOG_CATEGORY_STATIC(CharacterModelImporterCommandlet, Log, All);

namespace {
	constexpr TCHAR* CharacterAssetDir = TEXT("/Game/Character/");
}

struct UCharacterModelImporterCommandlet::ParsedParams {
	TOptional<FString> fbxPath;
	TOptional<FString> texPath;
	TOptional<FString> partsName;
	TOptional<FString> fileName;

	bool IsValid() const 
	{
		return partsName.IsSet() && fileName.IsSet() && (fbxPath.IsSet() || texPath.IsSet());
	}
};


UCharacterModelImporterCommandlet::UCharacterModelImporterCommandlet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	LogToConsole = false;

	auto& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistry             = &assetRegistryModule.Get();

	auto& assetToolsModule    = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetTools                = &assetToolsModule.Get();

}

int32 UCharacterModelImporterCommandlet::Main(const FString& commandlineParams)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("Start UTestCmdFunctionCommandlet"));

	//auto dir = FPaths::GameDir();
	//auto destPath = FPaths::Combine(dir, TEXT("test.txt"));
	//UE_LOG(LogTestCmdFunctionCommandlet, Display, TEXT("file : %s"), *destPath);
	//FFileHelper::SaveStringToFile(TEXT("aaaaa"), *destPath);

	//UFbxSkeletalMeshImportData importData;
	//importData.bPreserveSmoothingGroups = true;
	
	ParsedParams params;
	if(!ParseArgs(&params, commandlineParams)) {
		ShowHelp();
		return 1;
	}

	USkeletalMesh* mesh = ImportFbx(params.fbxPath.GetValue(), params.partsName.GetValue(), params.fileName.GetValue());
	UTexture*      tex  = ImportTexture(params.texPath.GetValue(), params.partsName.GetValue(), params.fileName.GetValue());
	if(tex) {
		UMaterial* mat  = MakeMaterialInstance(tex, params.partsName.GetValue(), params.fileName.GetValue());
		SetMaterialToMesh(mesh, mat);
	}

	return 0;
}

void UCharacterModelImporterCommandlet::ShowHelp()
{
	const auto helpText = FString::Format(
		TEXT(
			"Import and make character model.\n" \
			"\n" \
			"Arguments:\n" \
			"  CharacterModelImporter [-fbx_path=*.fbx] [-tex_path=*.png] -parts=[\"Hair\",\"Face\",\"Upper\",\"Lower\"] -filename=string"
		), 
		{ TEXT("") }
		);
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *helpText);
}

bool UCharacterModelImporterCommandlet::ParseArgs(ParsedParams* out, const FString& params) const
{
	if(!out) {
		return false;
	}

	FString fbxPath;
	if(FParse::Value(*params, TEXT("fbx_path="), fbxPath)) {
		out->fbxPath = fbxPath;
		//UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("dir=%s"), *dir);
	}

	FString texPath;
	if(FParse::Value(*params, TEXT("tex_path="), texPath)) {
		out->texPath = texPath;
	}

	FString partsName;
	if(FParse::Value(*params, TEXT("parts="), partsName)) {
		if(0 < partsName.Len()) {
			// 先頭を大文字にする
			partsName[0] = std::toupper(partsName[0]);
		}
		out->partsName = partsName;
	}

	FString fileName;
	if(FParse::Value(*params, TEXT("filename="), fileName)) {
		out->fileName = fileName;
	}

	return out->IsValid();
}

USkeletalMesh* UCharacterModelImporterCommandlet::ImportFbx(const FString& fbxPath, const FString& partsName, const FString& destFileName)
{
	FString importContentFbxPath;
	if constexpr (false) {
		// プロジェクトファイルにfbxをコピー
		const auto fbxFileName         = FPaths::GetCleanFilename(fbxPath);
		const auto importContentFbxDir = FString::Format(TEXT("{0}/Content/Characters/{1}/Meshes"), {FPaths::GameDir(), *partsName});
		importContentFbxPath           = FString::Format(TEXT("{0}/{1}"), {importContentFbxDir, *fbxFileName });
		FileUtil::CopyFile(fbxPath, importContentFbxPath);
	}
	else {
		// コピーせずにインポート
		importContentFbxPath = fbxPath;
	}
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing fbx from {0}"), {importContentFbxPath}));
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing fbx to   {0}"), {importContentFbxPath}));
	
	auto* fbxFactory             = NewObject<UFbxFactory>(UFbxFactory::StaticClass(), FName("Factory"), RF_NoFlags);
	auto* skeletalMeshImportData = NewObject<UFbxSkeletalMeshImportData>(UFbxSkeletalMeshImportData::StaticClass(), FName("FbxSkeletalMeshImportData"), RF_NoFlags);
	skeletalMeshImportData->VertexColorImportOption      = EVertexColorImportOption::Type::Ignore;
	skeletalMeshImportData->bUpdateSkeletonReferencePose = false;
	skeletalMeshImportData->bUseT0AsRefPose              = false;
	skeletalMeshImportData->bPreserveSmoothingGroups     = true;
	skeletalMeshImportData->bImportMeshesInBoneHierarchy = true;
	skeletalMeshImportData->bImportMorphTargets          = false;
	skeletalMeshImportData->bComputeWeightedNormals      = true;
	skeletalMeshImportData->bConvertScene                = true;
	skeletalMeshImportData->bConvertSceneUnit            = false;
	
	auto* importUIOption = NewObject<UFbxImportUI>(UFbxImportUI::StaticClass(), FName("FbxImportUI"), RF_NoFlags);
	importUIOption->bIsObjImport        = false;
	importUIOption->OriginalImportType  = EFBXImportType::FBXIT_SkeletalMesh;
	importUIOption->bImportAsSkeletal   = true;
	importUIOption->bImportMesh         = true;
	importUIOption->Skeleton            = Cast<USkeleton>(AssetRegistry->GetAssetByObjectPath(TEXT("/Game/Characters/Common/CharacterCommon_Skeleton.CharacterCommon_Skeleton")).GetAsset());
	importUIOption->bCreatePhysicsAsset = false;
	importUIOption->bImportMaterials    = false;
	importUIOption->bImportAnimations   = false;	
	importUIOption->bImportRigidMesh    = false;
	importUIOption->bImportTextures     = false;
	importUIOption->SkeletalMeshImportData = skeletalMeshImportData;

	const FString assetImportingDir  = FString::Format(TEXT("/Game/Characters/{0}/Meshes/"), {*partsName});
	const FString assetImportingName = ObjectTools::SanitizeObjectName(destFileName);
	const FString assetImoprtingPath = assetImportingDir + assetImportingName;

	UAssetImportTask* importTask = NewObject<UAssetImportTask>(UAssetImportTask::StaticClass(), FName("AssetImportTask"), RF_NoFlags);
	importTask->bAutomated       = true;
	importTask->bReplaceExisting = true;
	importTask->bSave            = true;
	importTask->DestinationPath  = assetImportingDir;
	importTask->DestinationName  = assetImportingName;
	importTask->Filename         = importContentFbxPath;
	importTask->Options          = importUIOption;
	importTask->Factory          = fbxFactory;

	const FString packageName = FString::Format(TEXT("/Game/Characters/{0}/Meshes/{1}"), {partsName, assetImportingName});

	UPackage* pakage = CreatePackage(nullptr, *packageName);

	const FString importText = FString::Format(
		TEXT("vars:\n - assetImportingName({0})\n - packageName({1})\n - DestinationPath({2})\n - DestinationName({3})\n - FileName({4})\n - importContentFbx({5})"), 
		{
		assetImportingName,
		packageName,
		importTask->DestinationPath,
		importTask->DestinationName,
		importTask->Filename,
		importContentFbxPath
		});
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *importText);

	TArray<UAssetImportTask*> tasks = {importTask};
	AssetTools->ImportAssetTasks(tasks);

	auto* importedMesh = Cast<USkeletalMesh>(AssetRegistry->GetAssetByObjectPath(*assetImoprtingPath).GetAsset());
	return importedMesh;
}

UTexture* UCharacterModelImporterCommandlet::ImportTexture(const FString& texPath, const FString& partsName, const FString& destFileName)
{
	return nullptr;
}

UMaterial* UCharacterModelImporterCommandlet::MakeMaterialInstance(UTexture* tex, const FString& partsName, const FString& destFileName)
{
	return nullptr;
}

void UCharacterModelImporterCommandlet::SetMaterialToMesh(USkeletalMesh* mesh, UMaterial* material)
{
	
}