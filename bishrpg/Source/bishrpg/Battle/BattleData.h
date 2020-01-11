// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include <functional>
#include "Runtime/Core/Public/GenericPlatform/GenericPlatform.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedPointer.h"
#include "BattleCell.h"
#include "System/CharacterStatus.h"
#include "GameData/SkillData.h"
#include "BattleDataType.h"
#include "BattleBoardDef.h"
#include "BattleObjectHandle.h"
#include "BattleData.generated.h"



class UBattleSystem;

/*! ターゲット
*/
USTRUCT(BlueprintType)
struct FBattleTarget {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	EPlayerGroup PlayerSide = EPlayerGroup::One;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 TargetHandle = 0;
};

/**	削除予定
*
*/
UCLASS()
class BISHRPG_API UBattleTargetConverter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/*!	一時的な変換(削除予定)
	*/
	UFUNCTION(BlueprintPure, Category = "Battle/Object")
	static bool ConvertPlayerGroupToBool(EPlayerGroup group)
	{
		return (group == EPlayerGroup::One) ? true : false;
	}

};

UENUM(BlueprintType, meta=(Bitflags))
enum class EStatusFlag : uint8 {
	None = 0,
	Status_Die = 1 << 0,
};
ENUM_CLASS_FLAGS( EStatusFlag )


struct FBattleCharacterStatus;
/*! ターゲットに対してのダメージ/ヒール量
*/
USTRUCT(BlueprintType)
struct FBattleTargetValue {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	FBattleTarget Target; //!< 対象

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 Value = 0;  //!< ダメージorヒール量

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta=(Bitmask, BitmaskEnum=EStatusFlag))
	int32 Status; //!< EStatusFlagのand
};


/*! 行動結果
*/
USTRUCT(BlueprintType)
struct FBattleActionResult {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	FBattleTarget Actor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	EBattleActionType ActionType;   //!< 行動

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	FName SkillName;                //!< スキル名(ActionTypeがSkillのときのみ)

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FBattleTargetValue> TargetResults;  //!< 対象ごとのダメージ/ヒール量

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 MoveFrom;                 //!< 移動座標

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 MoveTo;                   //!< 移動座標
};

/*! バトルステータス計算用
*/
USTRUCT(BlueprintType)
struct FBattleCharacterStatus {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FName Id; //!< id

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

	 //	死亡判定
	bool IsDie() const { return (Hp <= 0); }

	// ダメージを受ける
	void ReceiveDamage(int32 damage)
	{
		Hp = FMath::Clamp(Hp - damage, 0, HpMax);
	}

	void Heal(int32 heal)
	{
		Hp = FMath::Clamp(Hp + heal, 0, HpMax);
	}
};

/*! パーティ情報
*/
USTRUCT(BlueprintType)
struct FBattleParty {
	GENERATED_USTRUCT_BODY()

	using SelectFunc = void (FBattleParty::*)(TArray<int32>&, int32) const;
	using RangeFunc  = void (FBattleParty::*)(TArray<int32>&, const TArray<int32>&) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TArray<FBattleCharacterStatus> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TArray<int32>                  Formation;

	// 乱数
	FRandomStream RandStream;

	//キャラを取得
	const FBattleCharacterStatus* GetCharacterByPos(int32 posIndex) const;

	// キャラを取得 非const
	FBattleCharacterStatus* GetCharacterByPos(int32 posIndex)
	{
		return const_cast<FBattleCharacterStatus*>(static_cast<const FBattleParty*>(this)->GetCharacterByPos(posIndex));
	}

	// キャラを取得
	const FBattleCharacterStatus* GetCharacterByIndex(int32 index) const;
	FBattleCharacterStatus* GetCharacterByIndex(int32 index)
	{
		return const_cast<FBattleCharacterStatus*>(static_cast<const FBattleParty*>(this)->GetCharacterByIndex(index));
	}

	// 位置取得
	int32 GetCharacterPosByIndex(int32 index) const;

	// 位置からハンドルを取得
	int32 GetCharacterIndexByPos(int32 pos, bool silent = false) const;

	// 移動(fromとtoを交換)
	void Move(int32 from, int32 to);

	// 指定セルにキャラがいるかどうか
	bool ExistsPos(int32 posIndex) const
	{
		return Formation[posIndex] != Battle::Def::INVALID_CELL_NO;
	}
#if 0
	/*!	指定の方法でキャラを取得
	@param[out] selectedHandles 取得結果
	@param[in]  selectedParty   選択対象パーティ
	@param[in]  pattern         選択パターン
	@param[in]  param           選択オプション(行、列、ランダム数、...)
	@param[in]  clearResult     結果をクリアするかどうか
	*/
	void Select(TArray<int32>& selectedHandles, int32 actorHandle, EBattleSelectMethod pattern, int32 param, const FRandomStream& randStream, bool clearResult = true) const;
	void SelectTop(TArray<int32>& selectedHandles, int32 actorHandle, int32 index, bool clearResult = true) const;

	void SelectCol(TArray<int32>& selectedHandles, int32 col, bool clearResult = true) const;
	void SelectRow(TArray<int32>& selectedHandles, int32 row, bool clearResult = true) const;
	void SelectAhead1(TArray<int32>& selectedHandles, int32 actorPos, bool clearResult = true) const;
	void SelectAhead4(TArray<int32>& selectedHandles, int32 actorPos, bool clearResult = true) const;
	void SelectAll(TArray<int32>& selectedHandles, bool clearResult = true) const;
	void SelectRandom(TArray<int32>& selectedHandles, int selectPosNum, const FRandomStream& randStream, bool clearResult = true) const;
	
	void Filter(TArray<int32>& selectedPositions, std::function<bool(int32)> filter, std::function<bool(int32, int32)> comp = nullptr) const;

	void MakeCharacterListByPositionList(TArray<int32>& CharacterIndexs, const TArray<int32>& selectedPositions) const;
	//! }
#endif
protected:
	// 選択準備
	void PrepareSelecting(TArray<int32>& selectedHandles, bool clearResult) const;


	bool FilterAll(int32 actorPos) const;
	bool FilterExistsAll(int32 actorPos) const;
	/*
	bool FilterFront(int32 actorPos) const;
	//bool FilterFrontTop1(int32 actorPos) const;
	//bool FilterFrontTop2(int32 actorPos) const;
	//bool FilterFrontTop3(int32 actorPos) const;
	//bool FilterFrontTop4(int32 actorPos) const;
	bool FilterRowTop1(int32 actorPos) const;
	bool FilterRowTop2(int32 actorPos) const;
	bool FilterRowTop3(int32 actorPos) const;
	bool FilterRowTop4(int32 actorPos) const;
	*/
protected:
#if 0
	BattleCell FetchTop(int32 index) const;

	//@{
	void SelectTarget(TArray<int32>& selectedPos, int32 actorPos, EBattleSelectMethod selectMethod) const;
	//! 敵選択
	void SelectTop1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectTop2(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectTop3(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectTop4(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectTop5(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectTop6(TArray<int32>& selectedPos, int32 actorPos) const;

	void SelectFacedTop1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectAhead1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectAhead4(TArray<int32>& selectedPos, int32 actorPos) const;

	void SelectAttackTop1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectDeffenceTop1(TArray<int32>& selectedPos, int32 actorPos) const;

	void SelectRockTop1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRockBack1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectSingTop1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectSingBack1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectHurmorTop1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectHurmorBack1(TArray<int32>& selectedPos, int32 actorPos) const;

	void SelectCell_0_Faced(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_0_Right(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_0_Center(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_0_Left(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_1_Right(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_1_Center(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_1_Left(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_2_Right(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_2_Center(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_2_Left(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_3_Right(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_3_Center(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectCell_3_Left(TArray<int32>& selectedPos, int32 actorPos) const;

	void SelectAllCells(TArray<int32>& selectedPos, int32 actorPos) const;

	void SelectRandom1(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom2(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom3(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom4(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom5(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom6(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom7(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom8(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom9(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom10(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom11(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectRandom12(TArray<int32>& selectedPos, int32 actorPos) const;
	//@}

	//@{
	//! 味方選択
	void SelectMyself_P(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectFront1_P(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectTop1_P(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectBack1_P(TArray<int32>& selectedPos, int32 actorPos) const;
	void SelectAll_P(TArray<int32>& selectedPos, int32 actorPos) const;
	//@!}
	//@{
	//!! 入力セルを指定ルールに従って拡張する
//! @param[out] expandedPos 拡張結果
	//! @param[in]  basePos     拡張元
	//!! @param[in]  range       拡張タイプ
void ExpandCell(TArray<int32>& expandedPos, const TArray<int32>& basePos, EBattleSelectRange range) const;

	//! 指定セルにキャラがいるかどうか
void AddPos(TArray<int32>& expandedPos, int32 posIndex) const;
	void SelectRangeSingle(TArray<int32>& expandedPos, int32 basePos) const;
	void SelectRangeCol(TArray<int32>& expandedPos, int32posIndex basePos) const; != Battle::Def::INVALID_CELL_NO;
	void SelectRangeRow(TArray<int32>& expandedPos, int32 basePos) const;
	void SelectRangeSide(TArray<int32>& expandedPos, int32 basePos) const;
	void SelectRangeFrontBack(TArray<int32>& expandedPos, int32 basePos) const;
	void SelectRangeAroundPlus4(TArray<int32>& expandedPos, int32 basePos) const;
	void SelectRangeAroundCross4(TArray<int32>& expandedPos, int32 basePos) const;
	void SelectRangeAround9(TArray<int32>& expandedPos, int32 basePos) const;
	void SelectRangeBack(TArray<int32>& expandedPos, int32 basePos, int32 count) const;
	//void SelectRangeBack1(TArray<int32>& expandedPos, int32 basePos) const;
	//void SelectRangeBack2(TArray<int32>& expandedPos, int32 basePos) const;
	//void SelectRangeBack3(TArray<int32>& expandedPos, int32 basePos) const;
	//void SelectRangeBack4(TArray<int32>& expandedPos, int32 basePos) const;

	//@{
	void ExpandRangeSingle(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeCol(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeRow(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeSide(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeFrontBack(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeAroundPlus4(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeAroundCross4(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeAround9(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeBack1(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeBack2(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeBack3(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	void ExpandRangeBack4(TArray<int32>& expandedPos, const TArray<int32>& basePos) const;
	//@}
	//@}
#endif
};


UENUM(BlueprintType)
enum class ECommandType : uint8 {
	Attack,
	Skill,
	Move,
	Swap,
};

/*! バトルコマンド
*/
USTRUCT(BlueprintType)
struct FBattleCommand {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	ECommandType ActionType;  //!< 行動

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 ActionPosIndex;     //!< 発動時の位置(最終的な位置ではない)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FBattleObjectHandle ActorHandle;  //!< 発動者

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 CharacterIndex;        //!< 行動キャラハンドル

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 TargetPosIndex;  //!< 対象

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName SkillName;       //!< 発動スキル
};
