// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "System/CharacterStatus.h"
#include "BattleSystem.generated.h"


/*! 行動パターン
*/
UENUM(BlueprintType)
enum class EBattleActionType : uint8
{
	Attack,    //!< 攻撃
	Skill,     //!< スキル
	Move,      //!< 移動
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

/*! 行動エフェクト
*/
UENUM(BlueprintType)
enum class EBattleActionEffect : uint8
{
	None,
	KnockBack,     //!< ノックバック
	Stan,          //!< スタン
	BufAttack,     //!< 攻撃力上昇
	BufDeffence,   //!< 防御力上昇
	DebufAttack,   //!< 攻撃力減少
	DebufDeffence, //!< 防御力減少
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Actor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	EBattleActionType ActionType;   //!< 行動

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	FName SkillName;                //!< スキル名(ActionTypeがSkillのときのみ)

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FBattleTargetValue> TargetResults;  //!< 対象ごとのダメージ/ヒール量

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

	static const int32 MAX_PARTY_NUM = 12;

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
};

/*! バトルコマンド
*/
USTRUCT(BlueprintType)
struct FBattleCommand {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EBattleActionType ActionType;  //!< 行動

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 PosIndex;        //!< マス目

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 TargetPosIndex;  //!< 対象

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName SkillName;       //!< 発動スキル
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

public:	
	// Sets default values for this component's properties
	UBattleSystem();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*! 初期化
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize(const FParty& playerParty, const FParty& opponentParty);

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


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/*!	バトル行動を追加
	*/
	//UFUNCTION(BlueprintCallable, Category = "Battle")
	//void PushAction(const FBattleCommand& command);

	/*!	生存しているプレイヤー数
	*/
	int32 CalcAlivePlayers() const;

	/*!	攻撃コマンド実行
	*/
	void ExecAttack(FBattleActionResult& result, const FBattleCommand& command);

	/*!	移動コマンド実行
	*/
	void ExecMove(FBattleActionResult& result, const FBattleCommand& command);

	/*!	スキルコマンド実行
	*/
	void ExecSkill(FBattleActionResult& result, const FBattleCommand& command);

	/*!	ターンのパーティーを取得
	*/
	FBattleParty* GetTurnParty() { return PartyNum <= PartyList.Num() ? &PartyList[PlayerTurn ? 0 : 1] : nullptr; }

	/*!	ターンでないパーティーを取得
	*/
	FBattleParty* GetNotTurnParty() { return PartyNum <= PartyList.Num() ? &PartyList[PlayerTurn ? 1 : 0] : nullptr; }

	/*!	指定場所のキャラを取得
	*/
	FBattleCharacterStatus* GetCharacterByPos(FBattleParty* party, int32 posIndex) const;
	

	/*!	ダメージ計算基礎式
		@param attack    攻撃力(100～2000くらいを想定)
		@param deffence  防御力(攻撃より低い値でないとダメージが出ない)
		@param randMin   ダメージ幅（最小）
		@param randMax   ダメージ幅（最大）
		@param diffAcc   攻撃と防御の差があるときのダメージ増加量（累乗される）
		@param minDamage 最低保証ダメージ
		@return ダメージ量
	*/
	static int32 CalcDamage(int32 attack, int32 deffence, int32 randMin, int32 randMax, float diffAcc = 1.3f, int32 minDamage = 10);

	/*!	攻撃対象選択
	*/
	FBattleTarget GetAttackTargetPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos) const;

private:
	TArray<FBattleParty>   PartyList;   //!< パーティ
	TArray<FBattleCommand> CommandList; //!< バトルコマンドリスト
	bool                   PlayerTurn = true;  //!< プレイヤーターンかどうか
};
