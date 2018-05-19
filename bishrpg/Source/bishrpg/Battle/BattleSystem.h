// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "System/CharacterStatus.h"
#include "BattleSystem.generated.h"

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

/*! 行動パターン
*/
UENUM(BlueprintType)
enum class EBattleActionType : uint8
{
	Attack,    //!< 攻撃
	Skill,     //!< スキル
	Heal,      //!< 回復
	Spawn,     //!< 空きマスに召喚
	Move,
};

/*! ターゲット
*/
USTRUCT(BlueprintType)
struct FBattleTarget {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	bool PlayerSide;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Target;

	FBattleTarget()
	{
		PlayerSide = true;
		Target = 0;
	}
};


/*! ターゲットに対してのダメージ/ヒール量
*/
USTRUCT(BlueprintType)
struct FBattleTargetValue {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	FBattleTarget Target; //!< 対象

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Value;  //!< ダメージorヒール量

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 MoveTo; //!< 移動座標
};


/*! 行動結果
*/
USTRUCT(BlueprintType)
struct FBattleActionResult {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	EBattleActionType ActionType;   //!< 行動

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	FName SkillName;                //!< スキル名(ActionTypeがSkillのときのみ)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	TArray<FBattleTargetValue> TargetResults;  //!< 対象ごとのダメージ/ヒール量

};

/*! バトルステータス計算用
*/
USTRUCT(BlueprintType)
struct FBattleCharacterStatus {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	EBattleStyle Style; //!< スタイル

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 HpMax;  //!< 最大HP

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Hp;     //!< 現在HP

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Attack; //!< 攻撃力

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Deffence; //!< 防御力

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Speed;  //!< 速度(回避)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Hate;   //!< ヘイト
};

/*! パーティ情報
*/
USTRUCT(BlueprintType)
struct FBattleParty {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TArray<FBattleCharacterStatus> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TArray<int32>                  Formation;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BISHRPG_API UBattleSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBattleSystem();



private:
	enum PartyIndex {
		Player,
		Opponent,

		PartyNum,
	};

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*! 初期化
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize(const FParty& playerParty, const FParty& opponentParty, int32 aaa);

private:
	TArray<FBattleParty> PartyList;
};
