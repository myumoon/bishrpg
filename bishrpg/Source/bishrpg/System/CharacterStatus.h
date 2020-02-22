// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterStatus.generated.h"

class UDataTable;

/*! スキル配置場所
*/
UENUM(BlueprintType)
enum class ESkillPos : uint8
{
	Up,
	Right,
	Down,
	Left,

	Num,
};

/*! キャラのタイプ
Rock > Sing > Humor > Rock...
*/
UENUM(BlueprintType)
enum class EBattleStyle : uint8
{
	Rock,    //!< ロック（ソソソソ、D子...）
	Humor,   //!< 面白さ（社長、ポーポー、おハグ...）
	Sing,    //!< 歌唱力（びりけん、おはじゃ...）
};
constexpr const char* ToStr(EBattleStyle style)
{
	constexpr const char* names[] = {"Rock", "Humor", "Sing"};
	return names[static_cast<int32>(style)];
}


USTRUCT(BlueprintType)
struct BISHRPG_API FCharacterStatus {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	bool Valid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 Lv;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 HpLv;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 AttackLv;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 DeffenceLv;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 SpeedLv;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 PartsFace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 PartsHair;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 PartsUpper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 PartsLower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 PartsAccessory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 Onemesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TArray<FName> Skills;

	FCharacterStatus();

	// 指定のスキルを取得
	const FName& GetSkill(ESkillPos skill) const
	{
		check(Skills.Num() == static_cast<int32>(ESkillPos::Num));
		return Skills[static_cast<int32>(ESkillPos::Num)];
	}
	
};




USTRUCT(BlueprintType)
struct BISHRPG_API FParty {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TArray<FCharacterStatus> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TArray<int32>            Formation;
};
