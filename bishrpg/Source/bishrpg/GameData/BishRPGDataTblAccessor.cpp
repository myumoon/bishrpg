// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BishRPGDataTblAccessor.h"

#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"
#include "bishrpg.h"

namespace {
	UDataTable* s_dataTbl[static_cast<int32>(ETblType::Num)] = { 0 };

	const TCHAR* s_dataPathList[] = {
		TEXT("DataTable'/Game/GameData/Character/CharacterAssetTbl.CharacterAssetTbl'"),
		TEXT("DataTable'/Game/GameData/Skill/SkillAssetTbl.SkillAssetTbl'"),
	};
	static_assert(ARRAY_COUNT(s_dataPathList) == static_cast<int32>(ETblType::Num), "s_dataPathList != ETblType::Num");
}


// Sets default values
ABishRPGDataTblAccessor::ABishRPGDataTblAccessor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GAME_LOG("ABishRPGDataTblAccessor Created");
	for(int i = 0; i < static_cast<int>(ETblType::Num); ++i) {
		//if(s_dataTbl[i] == nullptr) 
		{
			GAME_LOG_FMT("ABishRPGDataTblAccessor Load : {0}", s_dataPathList[i]);
			ConstructorHelpers::FObjectFinder<UDataTable> charAssetTable(s_dataPathList[i]);
			s_dataTbl[i] = charAssetTable.Object;
		}
	}

}

UDataTable* ABishRPGDataTblAccessor::GetTbl(ETblType type)
{
	check(s_dataTbl[static_cast<int32>(type)] != nullptr);
	return s_dataTbl[static_cast<int32>(type)];
}

