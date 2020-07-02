// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture.h"
#include "Engine/DataTable.h"
#include "Classes/Curves/CurveFloat.h"
#include "Battle/BattleDataType.h"
#include "BattleAnimType.h"
#include "LevelSequence.h"
#include "SkillData.generated.h"


/*! キャラのタイプ
Rock > Sing > Humor > Rock...
*/
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Attack,         //!< 攻撃
	Heal,           //!< 回復
	Spawn,          //!< 召喚
	Effect,         //!< 特殊効果
};

/**	スキルデータ
 */
USTRUCT(BlueprintType)
struct BISHRPG_API FSkillData : public FTableRowBase {
	GENERATED_USTRUCT_BODY()

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	//FName NameTextLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	UTexture* Image =  nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	ESkillType Type = ESkillType::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	EBattleSelectMethod SelectType = EBattleSelectMethod::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EBattleSelectRange SelectRange = EBattleSelectRange::Single;

	// Top1    : 未使用
	// Col     : 未使用   
	// Row     : 何列前か？  
	// Ahead1  : 未使用
	// Ahead4  : 未使用
	// All     : 未使用
	// Random  : 何マス選択するか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	int32 SelectParam = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	UCurveFloat* ValueAtLevel = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (MustImplement = "IBattleAction"))
	TSoftClassPtr<class AActor> BattleAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	EBattleAnimType BattleAnim = EBattleAnimType::Anim000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSoftObjectPtr<class ULevelSequence> ActionSequence;
};
