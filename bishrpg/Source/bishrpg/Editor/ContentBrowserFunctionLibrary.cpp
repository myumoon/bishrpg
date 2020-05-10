// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "ContentBrowserFunctionLibrary.h"

#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

namespace {
	IContentBrowserSingleton& GetContentBrowser()
	{
		auto& contentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		return contentBrowserModule.Get();
	}
}

void UContentBrowserFunctionLibrary::GetSelectedAssets(TArray<FString>& assetsData)
{
	assetsData.Reset();

	TArray<FAssetData> assets;
	GetContentBrowser().GetSelectedAssets(assets);
	for(const auto& asset : assets) {
		assetsData.Add(asset.ObjectPath.ToString());
	}
}

void UContentBrowserFunctionLibrary::GetSelectedDirs(TArray<FString>& dirPaths)
{
	GetContentBrowser().GetSelectedFolders(dirPaths);
}

void UContentBrowserFunctionLibrary::SetSelectedPath(const FString& folderPath, bool needsRefresh)
{
	TArray<FString> paths;
	paths.Add(folderPath);
	GetContentBrowser().SetSelectedPaths(paths, needsRefresh);
}
