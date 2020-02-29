// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BishRPGDataTblAccessor.generated.h"

class UDataTable;

//UENUM(BlueprintType)
enum class ETblType : uint8 {
	CharacterAssetTbl,
	SkillTbl,

	Num                /*UMETA(Hidden)*/,
};

/*!	ゲーム共通データテーブル
	インゲームアウトゲームの両方で参照される
*/
UCLASS()
class BISHRPG_API ABishRPGDataTblAccessor : public AActor
{
	GENERATED_BODY()
	
public:	

	ABishRPGDataTblAccessor();
	
public:	
	
	static UDataTable* GetTbl(ETblType type);
};
