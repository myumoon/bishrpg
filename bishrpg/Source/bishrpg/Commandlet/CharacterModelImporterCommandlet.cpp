// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "CharacterModelImporterCommandlet.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(CharacterModelImporterCommandlet, Log, All);

UCharacterModelImporterCommandlet::UCharacterModelImporterCommandlet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	LogToConsole = false;
}

int32 UCharacterModelImporterCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("Start UTestCmdFunctionCommandlet"));

	auto dir = FPaths::GameDir();
	auto destPath = FPaths::Combine(dir, TEXT("test.txt"));
	//UE_LOG(LogTestCmdFunctionCommandlet, Display, TEXT("file : %s"), *destPath);
	FFileHelper::SaveStringToFile(TEXT("aaaaa"), *destPath);
	
	FString Value;
	if(FParse::Value(*CmdLineParams, TEXT("param0="), Value))
	{
		UE_LOG(CharacterModelImporterCommandlet, Display, TEXT("param0=%s"), *Value);
		return 0;
	}
	return 1;
}
