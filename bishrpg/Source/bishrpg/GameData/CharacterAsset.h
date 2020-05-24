// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Engine/DataTable.h"
#include "Curves/CurveFloat.h"
#include "System/CharacterStatus.h"
#include "Battle/BattleDataType.h"
#include "CharacterAsset.generated.h"

constexpr const TCHAR* EndTag = TEXT("[End]");

USTRUCT(BlueprintType)
struct FAttackTimingDataAsset : public FTableRowBase {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	float TimingSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	FName SkillCommandName = EndTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	float AttackRate = 1.0f;

	/*!	終了タイミングデータかどうかを判定
	*/
	bool IsEndData() const
	{
		return (SkillCommandName.IsEqual(EndTag));
	}
};


USTRUCT(BlueprintType)
struct FCharacterAsset : public FTableRowBase {
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	TSoftObjectPtr<UTexture> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	EBattleStyle Style;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	UCurveFloat* LvExpTbl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	UCurveFloat* HpTbl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	UCurveFloat* AttackTbl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	UCurveFloat* DeffenceTbl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	UCurveFloat* SpeedTbl;
};

/*!	レベルごとのステータス値
*/
USTRUCT(BlueprintType)
struct BISHRPG_API FCharacterLevelStatus : public FTableRowBase {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 Hp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 Deffence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 Speed;
};


/*!	アセット関係のutil
*/
UCLASS(ClassGroup = (Custom))
class BISHRPG_API UCharacterAssetUtil : public UFunction {
public:
	GENERATED_BODY()


	UFUNCTION(BlueprintCallable, Category = "GameData")
	static UTexture* LoadIcon(TSoftObjectPtr<UTexture> texture);

};

