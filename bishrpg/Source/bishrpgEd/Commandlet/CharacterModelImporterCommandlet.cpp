// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "CharacterModelImporterCommandlet.h"
#include <cctype>
#include "Templates/UnrealTemplate.h"
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
//#include "Factories/TextureFactory.h"
#include "Factories/ReimportTextureFactory.h"
#include "Factories/TrueTypeFontFactory.h"
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
#include "HAL/PlatformFilemanager.h"
#include "HAL/FileManager.h"

#include "Commandlet/Utility/FileUtil.h"

#include "bishrpgEd.h"

DEFINE_LOG_CATEGORY_STATIC(CharacterModelImporterCommandlet, Log, All);

namespace {
	// 文字列を区切り文字単位で分割する
	TArray<FString> SplitString(const FString& origin, const FString& splitStr)
	{
		FString workStr = origin;
		TArray<FString> splitString;
		splitString.Reserve(32);

		FString left, right;
		while(workStr.Split(splitStr, &left, &right)) {
			workStr = right;
			splitString.Add(left);
			//UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("split=%s"), *left);
		}
		if(!right.IsEmpty()) {
			splitString.Add(right);
			//UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("split=%s"), *right);
		}
		return MoveTemp(splitString);
	}
}


namespace {
	constexpr const TCHAR* CharacterAssetDir = TEXT("/Game/Character/");
}

struct UCharacterModelImporterCommandlet::ParsedParams {
	TOptional<FString>         csvFile;
	TOptional<FString>         fbxPath;
	TOptional<FString>         texPath;
	TOptional<FString>         matPath;
	TOptional<FString>         partsName;
	TOptional<FString>         fileName;

	bool ImportingFromCsvFile() const
	{
		return csvFile.IsSet();
	}
	bool IsFileImport() const 
	{
		return partsName.IsSet() && fileName.IsSet() && (fbxPath.IsSet() || (texPath.IsSet() && matPath.IsSet()));
	}

	bool HasValidOption() const
	{
		return ImportingFromCsvFile() || IsFileImport();
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

	ParsedParams params;
	if(!ParseArgs(&params, commandlineParams)) {
		ShowHelp();
		return 1;
	}

	if(params.ImportingFromCsvFile()) {
		ImportFromCsv(params.csvFile.GetValue());
	}
	if(params.IsFileImport()) {
		Import(params.fbxPath.GetValue(), params.fbxPath.GetValue(), params.texPath.GetValue(), params.texPath.GetValue(), params.matPath.GetValue(), params.partsName.GetValue(), params.fileName.GetValue());
	}

	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("Done UTestCmdFunctionCommandlet"));

	return 0;
}

void UCharacterModelImporterCommandlet::ShowHelp()
{
	const auto helpText = FString::Format(
		TEXT(
			"Import and make character model.\n" \
			"\n" \
			"Arguments:\n" \
			"  CharacterModelImporter [-csv=*.csv] [-fbx_path=*.fbx] [-tex_path=*.png] -parts=[\"Hair\",\"Face\",\"Upper\",\"Lower\"] -filename=string"
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

	// テキストファイルの中身を一括変換
	FString csvFilePath;
	if(FParse::Value(*params, TEXT("csv="), csvFilePath)) {
		out->csvFile = csvFilePath;
	}

	FString fbxPath;
	if(FParse::Value(*params, TEXT("fbx_path="), fbxPath)) {
		out->fbxPath = fbxPath;
	}

	FString texPath;
	if(FParse::Value(*params, TEXT("tex_path="), texPath)) {
		out->texPath = texPath;
	}

	FString matPath;
	if(FParse::Value(*params, TEXT("mat_path="), matPath)) {
		out->matPath = matPath;
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

	return out->HasValidOption();
}

bool UCharacterModelImporterCommandlet::ImportFromCsv(const FString& csvPath)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing from text file : {0}"), {csvPath}));

	TArray<FString> resultLines;
	if(!FFileHelper::LoadFileToStringArray(resultLines, *csvPath)) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Not found {0}"), {csvPath}));
		return false;
	}

	constexpr int32 SplitCols = 4;
	TArray<FString> splittedLine;

	bool result = true;
	for(const FString& line : resultLines) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Line : {0}"), {line}));
		splittedLine.Reset();
		const int32 splittedCount = line.ParseIntoArray(splittedLine, TEXT(","));
		if(splittedCount < SplitCols) {
			continue;
		}

		// fbx_path,texture_path,parts,outname
		const FString& fbxSrcPath     = splittedLine[0];
		const FString& fbxContentPath = splittedLine[1];
		const FString& texSrcPath     = splittedLine[2];
		const FString& texContentPath = splittedLine[3];
		const FString& matContentPath = splittedLine[4];
		const FString& partsName      = splittedLine[5];
		const FString& outName        = splittedLine[6];
		result &= Import(fbxSrcPath, fbxContentPath, texSrcPath, texContentPath, matContentPath, partsName, outName);
	}

	return result;
}


bool UCharacterModelImporterCommandlet::Import(const FString& fbxPath, const FString& fbxContentPath, const FString& texPath, const FString& texContentPath, const FString& matContentPath, const FString& partsName, const FString& filename)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("--- Import Start %s"), *fbxPath);

	USkeletalMesh* mesh = ImportFbx(fbxPath, fbxContentPath, partsName, filename);
	if(!mesh) {
		return false;
	}
	UTexture*      tex = ImportTexture(texPath, texContentPath, partsName, filename);
	if(!tex) {
		return false;
	}
	UMaterialInterface* mat = MakeMaterialInstance(tex, matContentPath);
	if(!mat) {
		return false;
	}
	SetMaterialToMesh(mesh, mat);

	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("--- Finish Start %s"), *fbxPath);

	return true;
}

USkeletalMesh* UCharacterModelImporterCommandlet::ImportFbx(const FString& fbxPath, const FString& fbxContentPath, const FString& partsName, const FString& destFileName)
{	
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing fbx from {0}"), {fbxPath}));
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing fbx to   {0}"), {fbxContentPath}));

	UFbxFactory* fbxFactory = nullptr;
	UFbxSkeletalMeshImportData* skeletalMeshImportData = nullptr;
	UFbxImportUI* importUIOption = nullptr;

	if(IsReimport(fbxContentPath)) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Reimport"), {TEXT("")}));

		auto* reimportTarget = FSoftObjectPath(fbxContentPath).TryLoad();
		const bool success = FReimportManager::Instance()->Reimport(reimportTarget, false, false, fbxPath, nullptr, INDEX_NONE, false);
		return Cast<USkeletalMesh>(reimportTarget);
	}

	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Use Import Factory"), {TEXT("")}));
	fbxFactory = NewObject<UFbxFactory>(UFbxFactory::StaticClass(), FName("FbxFactory"), RF_Public | RF_Standalone);

	skeletalMeshImportData = NewObject<UFbxSkeletalMeshImportData>(UFbxSkeletalMeshImportData::StaticClass(), FName("FbxSkeletalMeshImportData"), RF_Public | RF_Standalone);
	skeletalMeshImportData->VertexColorImportOption = EVertexColorImportOption::Type::Ignore;
	skeletalMeshImportData->bUpdateSkeletonReferencePose = false;
	skeletalMeshImportData->bUseT0AsRefPose = false;
	skeletalMeshImportData->bPreserveSmoothingGroups = true;
	skeletalMeshImportData->bImportMeshesInBoneHierarchy = true;
	skeletalMeshImportData->bImportMorphTargets = false;
	skeletalMeshImportData->bComputeWeightedNormals = true;
	skeletalMeshImportData->bConvertScene = true;
	skeletalMeshImportData->bConvertSceneUnit = false;


	importUIOption = NewObject<UFbxImportUI>(UFbxImportUI::StaticClass(), FName("FbxImportUI"), RF_Public | RF_Standalone);
	importUIOption->bIsObjImport = false;
	importUIOption->OriginalImportType = EFBXImportType::FBXIT_SkeletalMesh;
	importUIOption->bImportAsSkeletal = true;
	importUIOption->bImportMesh = true;
	importUIOption->Skeleton = Cast<USkeleton>(AssetRegistry->GetAssetByObjectPath(TEXT("/Game/Characters/Common/CharacterCommon_Skeleton.CharacterCommon_Skeleton")).GetAsset());
	importUIOption->bCreatePhysicsAsset = false;
	importUIOption->PhysicsAsset = nullptr;
	importUIOption->bImportMaterials = false;
	importUIOption->bImportAnimations = false;
	importUIOption->bImportRigidMesh = false;
	importUIOption->bImportTextures = false;
	importUIOption->SkeletalMeshImportData = skeletalMeshImportData;

	FString asserDir, assetName, extension;
	FPaths::Split(fbxContentPath, asserDir, assetName, extension);

	UAssetImportTask* importTask = NewObject<UAssetImportTask>(UAssetImportTask::StaticClass(), FName("AssetFbxImportTask"), RF_Public | RF_Standalone);
	importTask->bAutomated       = true;
	importTask->bReplaceExisting = true;
	importTask->bSave            = true;
	importTask->DestinationPath  = asserDir;
	importTask->DestinationName  = assetName;
	importTask->Filename         = fbxPath;
	importTask->Options          = importUIOption;
	importTask->Factory          = fbxFactory;

	TArray<UAssetImportTask*> tasks = {importTask};
	AssetTools->ImportAssetTasks(tasks);

	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Imported fbx {0}"), {fbxContentPath}));
	
	FSoftObjectPath       importedMeshPath(fbxContentPath);
	UObject*              loadedObj = importedMeshPath.TryLoad();
	return Cast<USkeletalMesh>(loadedObj);
}

UTexture* UCharacterModelImporterCommandlet::ImportTexture(const FString& texPath, const FString& texContentPath, const FString& partsName, const FString& destFileName)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Importing texture from {0}"), {texPath}));

	auto setTextureSettings = [texContentPath](UTexture* loadedTexture) {
		if(!loadedTexture) {
			return;
		}
		loadedTexture->PowerOfTwoMode = ETexturePowerOfTwoSetting::Type::PadToPowerOfTwo;
		loadedTexture->SRGB           = true;
		loadedTexture->Filter         = TextureFilter::TF_Nearest;
		UEditorAssetLibrary::SaveAsset(texContentPath);
	};

	if(IsReimport(texContentPath)) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Reimport"), {TEXT("")}));

		// リンクエラーになるのでUReimportTextureFactoryは使わない
		//auto* reimportTexFactory = NewObject<UReimportTextureFactory>(/*UReimportTextureFactory::StaticClass()*/this, FName("ReimportTextureFactory"), RF_NoFlags);
		//reimportTexFactory->pOriginalTex = Cast<UTexture>(FSoftObjectPath(texContentPath).TryLoad());
		//texFactory                       = reimportTexFactory;

		auto* reimportTarget = FSoftObjectPath(texContentPath).TryLoad();
		const bool success = FReimportManager::Instance()->Reimport(reimportTarget, false, false, texPath, nullptr, INDEX_NONE, false);
		auto* tex = Cast<UTexture>(reimportTarget);
		setTextureSettings(tex);
		return tex;
	}


	UTextureFactory* texFactory = NewObject<UTextureFactory>(UTextureFactory::StaticClass(), FName("TextureFactory"), RF_Public | RF_Standalone);
	texFactory->bCreateMaterial = false;
	texFactory->bDeferCompression = true;
	texFactory->CompressionSettings = TextureCompressionSettings::TC_Default;
	texFactory->NoAlpha = true;
	texFactory->bTwoSided = false;
	texFactory->NoCompression = false;
	texFactory->MipGenSettings = TextureMipGenSettings::TMGS_FromTextureGroup;
	texFactory->Blending = EBlendMode::BLEND_Opaque;
	texFactory->bUsingExistingSettings = false;

	FString asserDir, assetName, extension;
	FPaths::Split(texContentPath, asserDir, assetName, extension);

	UAssetImportTask* importTask = NewObject<UAssetImportTask>(UAssetImportTask::StaticClass(), FName("AssetTextureImportTask"), RF_Public | RF_Standalone);
	importTask->bAutomated       = true;
	importTask->bReplaceExisting = true;
	importTask->bSave            = true;
	importTask->DestinationPath  = asserDir;
	importTask->DestinationName  = assetName;
	importTask->Filename         = texPath;
	importTask->Options          = nullptr;
	importTask->Factory          = texFactory;
	
	TArray<UAssetImportTask*> tasks = {importTask};
	AssetTools->ImportAssetTasks(tasks);

	auto* loadedTexture = Cast<UTexture>(FSoftObjectPath(texContentPath).TryLoad());
	setTextureSettings(loadedTexture);

	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Imported texture {0}"), {texContentPath}));
	return loadedTexture;
}

UMaterialInterface* UCharacterModelImporterCommandlet::MakeMaterialInstance(UTexture* tex, const FString& destPath)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Making material instance with {0}"), {tex->GetFName().ToString()}));
	
	const FString srcMaterialPath  = TEXT("/Game/Characters/Materials/CharacterMatInstBase");

	// 複製元が存在しなかったらエラー
	if(!UEditorAssetLibrary::DoesAssetExist(srcMaterialPath)) {
		UE_LOG(CharacterModelImporterCommandlet, Error, TEXT("%s"), *FString::Format(TEXT("Duplicate error. Not exists src material instance : {0}"), {srcMaterialPath}));
		return nullptr;
	}

	// Materialフォルダの作成
	FString asserDir, _dummyName, _dummyExt;
	FPaths::Split(destPath, asserDir, _dummyName, _dummyExt);
	if(!FPaths::DirectoryExists(asserDir)) {
		IFileManager::Get().MakeDirectory(*asserDir, true);
	}

	if(!UEditorAssetLibrary::DoesAssetExist(destPath)) {
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("%s"), *FString::Format(TEXT("Duplicate material {0} -> {1}"), {srcMaterialPath, destPath}));
		UEditorAssetLibrary::DuplicateAsset(srcMaterialPath, destPath);
		//UEditorAssetLibrary::SaveAsset(destPath);
	}

	FSoftObjectPath       editMatPath(destPath);
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

bool UCharacterModelImporterCommandlet::IsReimport(const FString& contentPath) const
{
	return UEditorAssetLibrary::DoesAssetExist(contentPath);
}
