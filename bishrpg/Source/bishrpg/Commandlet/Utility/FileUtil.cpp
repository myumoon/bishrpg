// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "FileUtil.h"

#include "HAL/PlatformFilemanager.h"
//#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/LocalTimestampDirectoryVisitor.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

#include "bishrpg.h"

DEFINE_LOG_CATEGORY_STATIC(FileUtilLog, Log, All);

TArray<FString> FileUtil::GetFilesInDirectory(const FString& dir)
{
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

	const TArray<FString> emptyDirectory;
	FLocalTimestampDirectoryVisitor visitor(platformFile, emptyDirectory, emptyDirectory, false);

	platformFile.IterateDirectory(*dir, visitor);
	TArray<FString> files;
	files.Reserve(1024);
	for(auto timestampIt = TMap<FString, FDateTime>::TIterator(visitor.FileTimes); timestampIt; ++timestampIt) {
		const FString filePath = timestampIt.Key();
		//const FString fileName = FPaths::GetCleanFilename(filePath);
		files.Add(filePath);
	}
	
	return MoveTemp(files);
}

bool FileUtil::CopyFile(const FString& src, const FString& destPath)
{
	FString normalizedSrcPath = src;
	FPaths::NormalizeFilename(normalizedSrcPath);
	FString normalizedDestPath = destPath;
	FPaths::NormalizeFilename(normalizedDestPath);

	UE_LOG(FileUtilLog, Display, TEXT("CopyFile %s -> %s"), *normalizedSrcPath, *normalizedDestPath);
	if(!FPaths::FileExists(normalizedSrcPath)) {
		UE_LOG(FileUtilLog, Display, TEXT("CopyFile %s diesn't exist"), *normalizedSrcPath);
		return false;
	}
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	//if(!FPaths::DirectoryExists(normalizedDestDir)) {
	//	platformFile.CreateDirectoryTree(*normalizedDestDir);
	//}
	platformFile.CopyFile(*normalizedDestPath, *normalizedSrcPath, EPlatformFileRead::AllowWrite, EPlatformFileWrite::AllowRead);
	return true;
}

bool FileUtil::CopyTree(const FString& srcDir, const FString& destDir, bool overrideExists)
{
	UE_LOG(FileUtilLog, Display, TEXT("CopyTree %s -> %s"), *srcDir, *destDir);
	if(!FPaths::DirectoryExists(srcDir)) {
		UE_LOG(FileUtilLog, Display, TEXT("CopyTree %s directory diesn't exist"), *srcDir);
		return false;
	}
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	if(!FPaths::DirectoryExists(destDir)) {
		platformFile.CreateDirectoryTree(*destDir);
	}
	platformFile.CopyDirectoryTree(*destDir, *srcDir, overrideExists);
	return true;
}
