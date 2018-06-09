// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "System/CharacterStatus.h"
#include "BattleDataType.h"
#include "BattleSystem.generated.h"

class UBattleSystem;

/*! ターゲット
*/
USTRUCT(BlueprintType)
struct FBattleTarget {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	bool PlayerSide;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 TargetHandle;

	FBattleTarget()
	{
		PlayerSide = true;
		TargetHandle = 0;
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

	//FBattleCharacterStatus() {}

	//UFUNCTION(BlueprintCallable, Category = "Battle")
	//FBattleCharacterStatus(const FCharacterStatus& stat);

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

	//キャラを取得
	const FBattleCharacterStatus* GetCharacterByPos(int32 posIndex) const;
	
	// キャラを取得 非const
	FBattleCharacterStatus* GetCharacterByPos(int32 posIndex)
	{
		return const_cast<FBattleCharacterStatus*>(static_cast<const FBattleParty*>(this)->GetCharacterByPos(posIndex));
	}

	// キャラを取得
	const FBattleCharacterStatus* GetCharacterByHandle(int32 handle) const;
	FBattleCharacterStatus* GetCharacterByHandle(int32 handle)
	{
		return const_cast<FBattleCharacterStatus*>(static_cast<const FBattleParty*>(this)->GetCharacterByHandle(handle));
	}

	// 位置取得
	int32 GetCharacterPosByHandle(int32 handle) const;

	// 位置からハンドルを取得
	int32 GetCharacterHandleByPos(int32 pos, bool silent = false) const;

	// 移動(fromとtoを交換)
	void Move(int32 from, int32 to);

	/*!	指定の方法でキャラを取得
	@param[out] selectedHandles 取得結果
	@param[in]  selectedParty   選択対象パーティ
	@param[in]  pattern         選択パターン
	@param[in]  param           選択オプション(行、列、ランダム数、...)
	@param[in]  clearResult     結果をクリアするかどうか
	*/
	void Select(TArray<int32>& selectedHandles, int32 actorHandle, EBattleSelectPattern pattern, int32 param, bool clearResult = true) const;
	void SelectTop(TArray<int32>& selectedHandles, int32 actorHandle, bool clearResult = true) const;

	void SelectCol(TArray<int32>& selectedHandles, int32 col, bool clearResult = true) const;
	void SelectRow(TArray<int32>& selectedHandles, int32 row, bool clearResult = true) const;
	void SelectAhead1(TArray<int32>& selectedHandles, int32 actorPos, bool clearResult = true) const;
	void SelectAhead4(TArray<int32>& selectedHandles, int32 actorPos, bool clearResult = true) const;
	void SelectAll(TArray<int32>& selectedHandles, bool clearResult = true) const;
	void SelectRandom(TArray<int32>& selectedHandles, int selectPosNum, bool clearResult = true) const;

	void MakeCharacterListByPositionList(TArray<int32>& characterHandles, const TArray<int32>& selectedPositions) const;
	//! }

protected:
	// 選択準備
	void PrepareSelecting(TArray<int32>& selectedHandles, bool clearResult) const;

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
	int32 CharacterHandle;        //!< 行動キャラハンドル

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 TargetPosIndex;  //!< 対象

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName SkillName;       //!< 発動スキル
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BISHRPG_API UBattleCommandQueue : public UActorComponent
{
	GENERATED_BODY()

public:
	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize(UBattleSystem* battleSystem, bool playerSide);

	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool PushSkillCommand(int32 posIndex, const FName& skillName);

	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool PushMoveCommand(int32 posIndex, int32 moveTo);

	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool PushAttackCommand(int32 posIndex);

	/*!	バトル行動を戻す
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool RevertCommand(FBattleCommand& revertedCommand);

	/*!	バトル行動を除去
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ResetCommand();

	/*!	バトル開始可能かどうか
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool CanStartCommand() const;

	/*!	バトルシステムに送信
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Commit();

	/*!	コマンド数をカウント
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetCount(bool includingMoveCommand = false) const;

protected:
	int32 GetPrevPosIndex(int32 posIndex) const;

private:
	TArray<FBattleCommand> CommandList;         //!< バトルコマンドリスト
	UBattleSystem*         BattleSystem = nullptr;
	bool                   PlayerSide = true;
};


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
	static int32 GetFacedCol(int32 col)
	{
		if(0 <= col && col < GetBoardCol()) {
			return GetBoardCol() - col - 1;
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
	void Initialize(const FParty& playerParty, const FParty& opponentParty);


	/*!	ターン設定
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void SetTurn(bool playerTurn) { PlayerTurn = playerTurn; }

	/*!	ターン設定
		@return プレイヤーターンかどうか
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool SwapTurn() { PlayerTurn = !PlayerTurn; return PlayerTurn; }

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
	int32 GetCharacterHandle(int32 posIndex, bool playerSide)
	{
		FBattleParty* party = GetParty(playerSide);
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

	/*!	ターンのパーティーを取得
	*/
	FBattleParty* GetTurnParty() { return PartyNum <= PartyList.Num() ? &PartyList[PlayerTurn ? 0 : 1] : nullptr; }

	/*!	ターンでないパーティーを取得
	*/
	FBattleParty* GetNotTurnParty() { return PartyNum <= PartyList.Num() ? &PartyList[PlayerTurn ? 1 : 0] : nullptr; }

	

	/*!	ダメージ計算基礎式
		@param attack        攻撃力(100～2000くらいを想定)
		@param deffence      防御力(攻撃より低い値でないとダメージが出ない)
		@param randMin       ダメージ幅（最小）
		@param randMax       ダメージ幅（最大）
		@param attackerStyle 攻撃側のスタイル
		@param targetStyle   攻撃側のスタイル
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

private:
	TArray<FBattleParty>   PartyList;           //!< パーティ
	TArray<Command>        MergedCommandList;   //!< 全バトルコマンドリスト
	TArray<Command>        MergedMoveCommandList; //!< 移動コマンドをまとめたやつ
	bool                   PlayerTurn = true;   //!< プレイヤーターンかどうか
};
