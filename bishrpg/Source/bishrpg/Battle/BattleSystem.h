// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedPointer.h"
#include "System/CharacterStatus.h"
#include "GameData/SkillData.h"
#include "BattleDataType.h"
#include "BattleData.h"
#include "BattleSystem.generated.h"







UCLASS()
class BISHRPG_API UBattleBoardUtil : public UObject {
	GENERATED_BODY()

public:
	static const int32 BOARD_ROW = 4;
	static const int32 BOARD_COL = 3;
	static const int32 COL_LEFT = 0;
	static const int32 COL_CENTER = 1;
	static const int32 COL_RIGHT = 2;

	static const int32 CELL_NUM = BOARD_ROW * BOARD_COL;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetBoardRow() { return BOARD_ROW; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetBoardCol() { return BOARD_COL; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColLeft() { return COL_LEFT; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColRight() { return COL_RIGHT; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColCenter() { return COL_CENTER; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetCellNum() { return CELL_NUM; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void MakePositionListRow(TArray<int32>& madePosList, int32 row);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void MakePositionListCol(TArray<int32>& madePosList, int32 col, bool up = true);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void MakePositionListRandom(TArray<int32>& madePosList, int32 positions, const FRandomStream& randStream);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetFacedCol(int32 pos)
	{
		if(0 <= pos) {
			const int32 col = pos % GetBoardCol();
			return GetBoardCol() - col - 1;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColReverse(int32 col)
	{
		if(0 <= col && col < GetBoardCol()) {
			return GetBoardCol() - col - 1;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetRowReverse(int32 row)
	{
		if(0 <= row && row < GetBoardRow()) {
			return GetBoardRow() - row - 1;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetCol(int32 pos)
	{
		if(0 <= pos) {
			return (pos % GetBoardCol());
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetRow(int32 pos)
	{
		if(0 <= pos) {
			return (pos / GetBoardCol());
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetRowInverse(int32 pos)
	{
		const int32 row = GetRow(pos);
		if(0 <= row) {
			return (GetBoardRow() - row - 1);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPos(int32 row, int32 col)
	{
		if((0 <= row) && (row < GetBoardRow()) && (0 <= col) && (col < GetBoardCol())) {
			const int32 pos = row * GetBoardRow() + col;
			return pos;
		}
		return -1;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BISHRPG_API UBattleSystem : public UActorComponent
{
	GENERATED_BODY()

private:
	enum PartyIndex {
		Player,
		Opponent,

		PartyNum,
	};

	struct Command {
		bool           PlayerSide;
		FBattleCommand BattleCommand;
	};

public:	
	// Sets default values for this component's properties
	UBattleSystem();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*! 初期化
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize(const FParty& playerParty, const FParty& opponentParty, const FRandomStream& randStream);

	/*!	行動を行う
		@param[out] result 計算結果
		@return     消費するコマンドが合った場合はtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool ConsumeCommand(FBattleActionResult& result);

	/*!	移動だけ全消費する
	@param[out] result 計算結果
	@return     消費するコマンドが合った場合はtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool ConsumeMoveCommands(TArray<FBattleActionResult>& result);

	/*!	指定位置にいるキャラのステータスを取得
	@param[out] stat ステータス
	@param[it]  posIndex 位置
	@param[it]  playerSide プレイヤーか敵か
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void GetCharacterStatusByPos(FBattleCharacterStatus& stat, int32 posIndex, bool playerSide = true) const;

    /*! 指定キャラのステータスを取得
     @param[out] stat ステータス
     @param[it]  handle キャラハンドル
     @param[it]  playerSide プレイヤーか敵か
     */
    UFUNCTION(BlueprintCallable, Category = "Battle")
    void GetCharacterStatusByHandle(FBattleCharacterStatus& stat, int32 handle, bool playerSide = true) const;

	/*!	バトル用のキャラステータス生成
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static FBattleCharacterStatus MakeBattleCharacterStatus(const FCharacterStatus& stat);

	/*!	バトル用のキャラステータス生成（数値オフセット付き）
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static FBattleCharacterStatus MakeBattleCharacterStatusWithOffset(const FCharacterStatus& stat, int32 offsetHp, int32 offsetAttack, int32 offsetDeffence, int32 offsetSpeed);
	
	/*!	バトル用パーティ生成
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static FBattleParty MakeFromParty(const FParty& party);


	/*!	生存しているプレイヤー数
	*/
	int32 CalcAlivePlayers() const;

	/*!	コマンドリストをマージ
	*/
	void EnqueueCommands(const TArray<FBattleCommand>& commandList, bool playerSide);

	/*!	位置からキャラを取得
	*/
	int32 GetCharacterHandle(int32 posIndex, bool playerSide) const
	{
		const FBattleParty* party = GetParty(playerSide);
		if(party) {
			check(UBattleBoardUtil::GetCellNum() == party->Formation.Num());
			if(0 <= posIndex && posIndex < party->Formation.Num()) {
				return party->Formation[posIndex];
			}
		}
		return -1;
	}

	//! @{
	//! キャラ選択


	/*!	パーティ取得
	*/
	const FBattleParty* GetParty(bool playerSide) const
	{
		if(PartyList.Num() < PartyIndex::PartyNum) {
			return nullptr;
		}
		if(playerSide) {
			return &PartyList[PartyIndex::Player];
		}
		return &PartyList[PartyIndex::Opponent];
	}
	FBattleParty* GetParty(bool playerSide)
	{
		return const_cast<FBattleParty*>(static_cast<const UBattleSystem*>(this)->GetParty(playerSide));
	}

	/*!	指定ハンドルのキャラを取得
	*/
	FBattleCharacterStatus* GetCharacterByHandle(FBattleParty* party, int32 characterHandle) const;

	/*!	指定場所のキャラを取得
	*/
	FBattleCharacterStatus* GetCharacterByPos(FBattleParty* party, int32 posIndex) const;

protected:

	static EBattleActionType ConvertAction(ECommandType type)
	{
		switch(type) {
			case ECommandType::Attack:
				return EBattleActionType::Attack;
			case ECommandType::Skill:
				return EBattleActionType::Skill;
			default:
				return EBattleActionType::None;
		}
	}

	// Called when the game starts
	virtual void BeginPlay() override;

	/*!	バトル行動を追加
	*/
	//UFUNCTION(BlueprintCallable, Category = "Battle")
	//void PushAction(const FBattleCommand& command);
	/*!	攻撃コマンド実行
	*/
	void ExecAttack(FBattleActionResult& result, const Command& command);

	/*!	移動コマンド実行
	*/
	void ExecMove(FBattleActionResult& result, const Command& command);

	/*!	スキルコマンド実行
	*/
	void ExecSkill(FBattleActionResult& result, const Command& command);

	/*!	ダメージ計算基礎式
		@param attack        攻撃力(100～2000くらいを想定)
		@param deffence      防御力(攻撃より低い値でないとダメージが出ない)
		@param randMin       ダメージ幅（最小）
		@param randMax       ダメージ幅（最大）
		@param attackerStyle 攻撃側のスタイル
		@param targetStyle   相手側のスタイル
		@param diffAcc       攻撃と防御の差があるときのダメージ増加量（累乗される）
		@param minDamage     最低保証ダメージ
		@return ダメージ量
	*/
	static int32 CalcDamage(int32 attack, int32 deffence, int32 randMin, int32 randMax, EBattleStyle attackerStyle, EBattleStyle targetStyle, float diffAcc = 1.3f, int32 minDamage = 10);

	/*!	タイプごとのダメージ補正
	*/
	static float GetTypeDamageRate(EBattleStyle attackerStyle, EBattleStyle targetStyle);

	/*!	攻撃対象選択
	*/
	FBattleTarget GetAttackTargetByPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos, bool playerSide) const;

	/*!	攻撃対象選択
	*/
	void GetSkillTargetsByPos(TArray<FBattleTarget>& targets, const FBattleCharacterStatus& attacker, int32 attackerPos, bool selectPlayerSide, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam) const;

	/*!	死亡更新
	*/
	void UpdateDie();

private:
	TArray<FBattleParty>             PartyList;             //!< パーティ
	TArray<Command>                  MergedCommandList;     //!< 全バトルコマンドリスト
	TArray<Command>                  MergedMoveCommandList; //!< 移動コマンドをまとめたやつ
	const FRandomStream*             RandStream;            //!< ランダム
};
