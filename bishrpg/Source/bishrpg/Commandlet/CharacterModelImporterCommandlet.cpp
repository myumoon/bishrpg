// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "CharacterModelImporterCommandlet.h"
#include <cctype>
#include "UObject/ScriptInterface.h"
#include "Engine/Texture.h"
#include "Engine/SkeletalMesh.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "Factories/ReimportFbxSkeletalMeshFactory.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialFunctionInstanceFactory.h"
//#include "Factories/MaterialParameterCollectionFactoryNew.h"
#include "Materials/Material.h"
//#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetImportTask.h"
#include "ObjectTools.h"
#include "ComponentReregisterContext.h"
#include "UObject/SoftObjectPath.h"
#include "EditorAssetLibrary.h"
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
		UMaterialInterface* mat  = MakeMaterialInstance(tex, params.partsName.GetValue(), params.fileName.GetValue());
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

	const FName assetName = *FString::Format(TEXT("/Game/Characters/{0}/Meshes/{1}.{1}"), {partsName, destFileName});
	const auto  meshAsset = AssetRegistry->GetAssetByObjectPath(assetName);
	if(auto foundAsset = Cast<USkeletalMesh>(meshAsset.GetAsset())) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing fbx : AssetDeleted {0}"), {assetName.ToString()}));
		// 再インポート時に設定が上書きされないので一度消す
		UEditorAssetLibrary::DeleteAsset(assetName.ToString());
		//foundAsset->MarkPackageDirty();
		//AssetRegistry->AssetDeleted(foundAsset);
		//AssetRegistry->PackageDeleted(meshAsset.GetPackage());
	}
	
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
	
	auto* fbxFactory             = NewObject<UFbxFactory>(UFbxFactory::StaticClass(), FName("Factory"), RF_Public | RF_Standalone);
	auto* skeletalMeshImportData = NewObject<UFbxSkeletalMeshImportData>(UFbxSkeletalMeshImportData::StaticClass(), FName("FbxSkeletalMeshImportData"), RF_Public | RF_Standalone);
	skeletalMeshImportData->VertexColorImportOption      = EVertexColorImportOption::Type::Ignore;
	skeletalMeshImportData->bUpdateSkeletonReferencePose = false;
	skeletalMeshImportData->bUseT0AsRefPose              = false;
	skeletalMeshImportData->bPreserveSmoothingGroups     = true;
	skeletalMeshImportData->bImportMeshesInBoneHierarchy = true;
	skeletalMeshImportData->bImportMorphTargets          = false;
	skeletalMeshImportData->bComputeWeightedNormals      = true;
	skeletalMeshImportData->bConvertScene                = true;
	skeletalMeshImportData->bConvertSceneUnit            = false;
	
	auto* importUIOption = NewObject<UFbxImportUI>(UFbxImportUI::StaticClass(), FName("FbxImportUI"), RF_Public | RF_Standalone);
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

	UAssetImportTask* importTask = NewObject<UAssetImportTask>(UAssetImportTask::StaticClass(), FName("AssetFbxImportTask"), RF_Public | RF_Standalone);
	importTask->bAutomated       = true;
	importTask->bReplaceExisting = true;
	importTask->bSave            = true;
	importTask->DestinationPath  = assetImportingDir;
	importTask->DestinationName  = assetImportingName;
	importTask->Filename         = importContentFbxPath;
	importTask->Options          = importUIOption;
	importTask->Factory          = fbxFactory;

	//const FString packageName = FString::Format(TEXT("/Game/Characters/{0}/Meshes/{1}"), {partsName, assetImportingName});
	//UPackage* pakage = CreatePackage(nullptr, *packageName);

	const FString importText = FString::Format(
		TEXT("vars:\n - assetImportingName({0})\n - packageName({1})\n - DestinationPath({2})\n - DestinationName({3})\n - FileName({4})\n - importContentFbx({5})"), 
		{
		assetImportingName,
		//packageName,
		TEXT("None"),
		importTask->DestinationPath,
		importTask->DestinationName,
		importTask->Filename,
		importContentFbxPath
		});
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *importText);

	if constexpr(false) {
		//bool canceled = false;
		//auto* createdMesh = fbxFactory->FactoryCreateFile(USkeletalMesh::StaticClass(), pakage, *assetImportingName, RF_Public | RF_Standalone, importContentFbxPath, TEXT("Warn"), nullptr, canceled);
		//AssetRegistry->AssetCreated(createdMesh);
	}
	else {
		TArray<UAssetImportTask*> tasks = {importTask};
		AssetTools->ImportAssetTasks(tasks);
	}
	
	FSoftObjectPath       importedMeshPath(assetImoprtingPath);
	UObject*              loadedObj = importedMeshPath.TryLoad();
	return Cast<USkeletalMesh>(loadedObj);
	//auto* importedMesh = Cast<USkeletalMesh>(AssetRegistry->GetAssetByObjectPath(*assetImoprtingPath).GetAsset());
	//return importedMesh;
}

UTexture* UCharacterModelImporterCommandlet::ImportTexture(const FString& texPath, const FString& partsName, const FString& destFileName)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing texture from {0}"), {texPath}));

	auto* texFactory = NewObject<UTextureFactory>(UTextureFactory::StaticClass(), FName("TextureFactory"), RF_Public | RF_Standalone);
	texFactory->bCreateMaterial     = false;
	texFactory->bDeferCompression   = true;
	texFactory->CompressionSettings = TextureCompressionSettings::TC_Default;
	texFactory->NoAlpha             = true;
	texFactory->bTwoSided           = true;
	texFactory->NoCompression       = false;
	texFactory->MipGenSettings      = TextureMipGenSettings::TMGS_FromTextureGroup;

	const FString assetImportingDir = FString::Format(TEXT("/Game/Characters/{0}/Textures/"), {*partsName});
	const FString assetImportingName = ObjectTools::SanitizeObjectName(destFileName);
	const FString assetImoprtingPath = assetImportingDir + assetImportingName;

	UAssetImportTask* importTask = NewObject<UAssetImportTask>(UAssetImportTask::StaticClass(), FName("AssetTextureImportTask"), RF_Public | RF_Standalone);
	importTask->bAutomated = true;
	importTask->bReplaceExisting = true;
	importTask->bSave = true;
	importTask->DestinationPath = assetImportingDir;
	importTask->DestinationName = assetImportingName;
	importTask->Filename        = texPath;
	importTask->Options         = nullptr;
	importTask->Factory         = texFactory;
	
	TArray<UAssetImportTask*> tasks = {importTask};
	AssetTools->ImportAssetTasks(tasks);

	//auto* importedTex = Cast<UTexture>(AssetRegistry->GetAssetByObjectPath(*assetImoprtingPath).GetAsset());
	//return importedTex;
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Imported texture {0}"), {assetImoprtingPath}));
	return Cast<UTexture>(FSoftObjectPath(assetImoprtingPath).TryLoad());
}

UMaterialInterface* UCharacterModelImporterCommandlet::MakeMaterialInstance(UTexture* tex, const FString& partsName, const FString& destFileName)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Making material instance with {0}"), {tex->GetFName().ToString()}));
	
	const FString srcMaterialPath  = TEXT("/Game/Characters/Materials/CharacterMatInstBase");
	const FString destMaterialPath = FString::Format(TEXT("/Game/Characters/Lower/Materials/{0}"), {*destFileName});

	// 複製元が存在しなかったらエラー
	if(!UEditorAssetLibrary::DoesAssetExist(srcMaterialPath)) {
		UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("%s"), *FString::Format(TEXT("Duplicate error. Not exists src material instance : {0}"), {srcMaterialPath}));
		return nullptr;
	}

	if(!UEditorAssetLibrary::DoesAssetExist(destMaterialPath)) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Duplicate material {0} -> {1}"), {srcMaterialPath, destMaterialPath}));
		UEditorAssetLibrary::DuplicateAsset(srcMaterialPath, destMaterialPath);
		//UEditorAssetLibrary::SaveAsset(destMaterialPath);
	}

	FSoftObjectPath       editMatPath(destMaterialPath);
	UObject*              loadedObj = editMatPath.TryLoad();
	if(auto *mat = Cast<UMaterialInstanceConstant>(loadedObj)) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("UMaterialInstanceConstant cast success"));

		FMaterialParameterInfo param("MainTexParam");
		mat->SetTextureParameterValueEditorOnly(param, tex);

		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Save MatInst to {0}"), {editMatPath.ToString()}));
		UEditorAssetLibrary::SaveAsset(editMatPath.ToString(), false);

		return mat;
	}

	return nullptr;

	#if 0
	//const FString destMaterialPath = FString::Format(TEXT("/Game/Charadcters/Lower/Materials/Test"), {TEXT("")});
	
	FSoftObjectPath       originMatPath(srcMaterialPath);
	UObject*              loadedObj = originMatPath.TryLoad();

	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Duplicate from {0}({1}) to {2} {3}"), {srcMaterialPath, loadedObj ?TEXT("loaded"):TEXT("null"), destMaterialPath, destFileName}));
	

	//UPackage*     package = CreatePackage(nullptr, *destMaterialPath);
	auto* createdObject   = AssetTools->DuplicateAsset(destFileName, destMaterialPath, loadedObj);
	//AssetRegistry->AssetCreated(createdObject);

	//auto& assetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("EditorAssetLibrary");
	//AssetTools = &assetToolsModule.Get();

	if constexpr(false) {
		// load asset
		FString OutFailureReason;
		FAssetData AssetData = AssetRegistry->GetAssetByObjectPath(FName(*srcMaterialPath));
		//FAssetData AssetData = EditorScriptingUtils::FindAssetDataFromAnyPath(srcMaterialPath, OutFailureReason);
		if(!AssetData.IsValid()) {
			UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("DuplicateAsset. Invalid path. %s"), *OutFailureReason);
			return nullptr;
		}
		auto* SourceObject = EditorScriptingUtils::LoadAsset(AssetData, false, OutFailureReason);
		if(SourceObject == nullptr) {
			UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("DuplicateAsset. Failed to find the source asset. %s"), *FailureReason);
			return nullptr;
		}

		// Make sure the asset is from the ContentBrowser
		if(!EditorScriptingUtils::IsAContentBrowserAsset(SourceObject, OutFailureReason)) {
			UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("DuplicateAsset. Failed to validate the source. %s"), *OutFailureReason);
			return nullptr;
		}

		if(!EditorScriptingUtils::CheckIfInEditorAndPIE() || !InternalEditorLevelLibrary::IsAssetRegistryModuleLoading()) {
			UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("DuplicateAsset. aaaaa"));
			return nullptr;
		}
		FString DestinationObjectPath = EditorScriptingUtils::ConvertAnyPathToObjectPath(destMaterialPath, OutFailureReason);
		if(DestinationObjectPath.IsEmpty()) {
			UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("DuplicateAsset. Failed to validate the destination. %s"), *OutFailureReason);
			return nullptr;
		}
		if(!EditorScriptingUtils::IsAValidPathForCreateNewAsset(DestinationObjectPath, OutFailureReason)) {
			UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("DuplicateAsset. Failed to validate the destination. %s"), *OutFailureReason);
			return nullptr;
		}
		// DuplicateAsset does it, but failed with a Modal
		if(FPackageName::DoesPackageExist(DestinationObjectPath, nullptr, nullptr)) {
			UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("DuplicateAsset. Failed to validate the destination '%s'. There's alreay an asset at the destination."), *DestinationObjectPath);
			return nullptr;
		}

		auto* createdObject = UEditorAssetLibrary::DuplicateAsset(TEXT("/Game/Characters/Materials/CharacterMat.CharacterMat"), TEXT("/Game/Characters/Materials/test.test"));
		AssetRegistry->AssetCreated(createdObject);
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Duplicate result createdObject({0})"), {createdObject?TEXT("true"):TEXT("false")}));

		FString DestinationLongPackagePath = FPackageName::GetLongPackagePath(DestinationObjectPath);
		FString DestinationObjectName      = FPackageName::ObjectPathToObjectName(DestinationObjectPath);

		// duplicate asset
		FAssetToolsModule& Module = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		UObject* DuplicatedAsset = Module.Get().DuplicateAsset(DestinationObjectName, DestinationLongPackagePath, SourceObject);
	}
	//AssetRegistry->AssetCreated(createdObject);
	//package->FullyLoad();
	//package->SetDirtyFlag(true);

	//createdObject->PreEditChange(NULL);
	//createdObject->PostEditChange();

	auto* createdMaterial = Cast<UMaterialInstance>(createdObject);
	//FSoftObjectPath       duplicatedMatPath(destMaterialPath);
	//auto* createdMaterial = Cast<UMaterialInstance>(duplicatedMatPath.TryLoad());

	if constexpr(false) {
		//const FString packageAsset     = TEXT("/Game/Charadcters/Materials/CharacterMat.CharacterMat");
		UPackage*     package          = CreatePackage(nullptr, *destMaterialPath);

		// Create an unreal material asset
		//auto MaterialFactory = NewObject<UMaterialFactoryNew>();
		auto* materialFactory = NewObject<UMaterialFactoryNew>();
		auto* createdMat      = (UMaterialInterface*)materialFactory->FactoryCreateNew(UMaterial::StaticClass(), package, *destFileName, RF_Standalone | RF_Public, nullptr, GWarn);
	
		AssetRegistry->AssetCreated(createdObject);
		package->FullyLoad();
		package->SetDirtyFlag(true);

		// Let the material update itself if necessary
		createdMat->PreEditChange(NULL);
		createdMat->PostEditChange();
		// make sure that any static meshes, etc using this material will stop using the FMaterialResource of the original
		// material, and will use the new FMaterialResource created when we make a new UMaterial in place
		FGlobalComponentReregisterContext recreateComponents;

		//createdMaterial = createdMat;
	}

	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Created material : createdObject({0}) createdMaterial({1})"), {createdObject ? TEXT("loaded") : TEXT("null"), createdMaterial ? TEXT("loaded") : TEXT("null")}));

	return createdMaterial;
	#endif
}

void UCharacterModelImporterCommandlet::SetMaterialToMesh(USkeletalMesh* mesh, UMaterialInterface* material)
{
	if(!mesh || !material) {
		UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("SetMaterialToMesh failure : mesh(%p), material(%p)"), mesh, material);
		return;
	}

	FSkeletalMaterial addMat(material);
	addMat.MaterialSlotName = "Main";
	mesh->Materials.Reset();
	mesh->Materials.Add(addMat);

	const FString meshPath = mesh->GetPathName();
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("Save mesh %s"), *meshPath);

	UEditorAssetLibrary::SaveAsset(meshPath, false);
}