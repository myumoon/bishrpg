// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "ContentBrowserFunctionLibraryEd.h"

#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "bishrpgEd.h"

namespace {
	IContentBrowserSingleton& GetContentBrowser()
	{
		auto& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		return contentBrowserModule.Get();
	}
}

void UContentBrowserFunctionLibraryEd::GetSelectedAssetsEd(TArray<FString>& assetsData)
{
	assetsData.Reset();

	TArray<FAssetData> assets;
	GetContentBrowser().GetSelectedAssets(assets);
	for(const auto& asset : assets) {
		assetsData.Add(asset.ObjectPath.ToString());
	}
}

void UContentBrowserFunctionLibraryEd::GetSelectedDirsEd(TArray<FString>& dirPaths)
{
	GetContentBrowser().GetSelectedFolders(dirPaths);
}

void UContentBrowserFunctionLibraryEd::SetSelectedPathEd(const FString& folderPath, bool needsRefresh)
{
	TArray<FString> paths;
	paths.Add(folderPath);
	GetContentBrowser().SetSelectedPaths(paths, needsRefresh);
}

void UContentBrowserFunctionLibraryEd::BishrpgEdTest()
{
	EDITOR_LOG("BishrpgEdTest");
}