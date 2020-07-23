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
#include "BattleBoardUtil.h"
#include "BattleObjectHandle.h"
#include "BattleCommandQueue.h"
#include "Battle/Data/BattleDataTableHolder.h"
#include "BattleSystem.generated.h"

class UBattleCommandQueue;

//! 攻撃結果デリゲート宣言
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBattleAttackDelegate, 
	const FBattleAttackResult&, result
);
//! スキル結果デリゲート宣言
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBattleSkillDelegate,
	const FBattleSkillResult&, result
);
//! バトル結果デリゲート宣言
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBattleMoveDelegate,
	const FBattleMoveResult&, result
);

//! バトル設定
USTRUCT(BlueprintType)
struct BISHRPG_API FBattleSettings {
	GENERATED_BODY()

	//!< 1ターンの最大コマンド数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle/System")
	int32 MaxTurnCommandNum = 1;

	//!< 最小ダメージ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle/System")
	int32 MinDamage = 10;

	//!< 最大ダメージ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle/System")
	int32 MaxDamage = 9999;
};

//! バトルシステム
//! 
//! バトルのコマンド結果を計算するシステム
//! 基本的に内部にコンテキストは持たない。
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BISHRPG_API UBattleSystem : public UActorComponent
{
	GENERATED_BODY()

	friend struct FBattleObjectHandle;

private:
	enum PartyIndex {
		Player,
		Opponent,

		PartyNum,
	};

	struct Command {
		EPlayerGroup   PlayerSide;
		FBattleCommand BattleCommand;
	};

	using FBattleCommandList = TArray<FBattleCommand>;
	using FGroupCommandList  = TArray<FBattleCommandList>;

	struct FGroupContext {
		int32 ConsumedIndex = 0;

		void Reset()
		{
			ConsumedIndex = 0;
		}
	};

	// 戦闘進行管理
	struct FBattleCommandContext {

		struct FBattleCommandInfo {
			FBattleCommand command;
			EPlayerGroup   group;
		};

		FBattleCommandContext()
		{
			GroupContext.SetNum(MaxGroupNum);
		}
		~FBattleCommandContext() = default;

		void Reset()
		{
			ExecCommandIndex = 0;
			CommandQueue.Reset();
			for(auto& context : GroupContext) {
				context.Reset();
			}
		}

		TArray<FGroupContext>      GroupContext;
		TArray<FBattleCommandInfo> CommandQueue;
		int32                      ExecCommandIndex = 0;
	};

public:	
	// Sets default values for this component's properties
	UBattleSystem();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*! 初期化
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize(const FParty& playerParty, const FParty& opponentParty, const UBattleDataTableHolder* dataAccessor, const FRandomStream& randStream, const FBattleSettings& battleSettings);

	/*!	効果範囲を予測する
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void PredictTargetCells(int32& mainCellPos, TArray<int32>& cells, const FBattleCommand& command);

	/*!	バトル開始準備
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Prepare(const UBattleCommandQueue* groupOneCommands, const UBattleCommandQueue* groupTwoCommands);

	/*!	バトルコマンド処理
		
	AttackDelegate,SkillDelegate,MoveDelegateが呼ばれます。
	@param[out] isConsumed           コマンドが消費されたか
	@param[out] consumedCommandCount 消費コマンド数
	@param[in]	groupOneCommands	 グループ1コマンド
	@param[in]	groupTwoCommands	 グループ2コマンド
	@return		消費するコマンドがあった場合はtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ConsumeCommand(bool& isConsumed, int32& consumedCommandCount, const UBattleCommandQueue* groupOneCommands, const UBattleCommandQueue* groupTwoCommands);

	/*!	行動を行う
	@param[out] result 計算結果
	@return     消費するコマンドが合った場合はtrue
	*/
	//UFUNCTION(BlueprintCallable, Category = "Battle")
	//bool ConsumeCommand(FBattleActionResult& result);

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

	/*! 指定キャラのステータスを取得
    @param[out] stat ステータス
    @param[it]  handle オブジェクトハンドル
    */
    UFUNCTION(BlueprintCallable, Category = "Battle")
    void GetCharacterStatusByHandle2(FBattleCharacterStatus& stat, const FBattleObjectHandle& handle) const;

	/*!	HPレートを取得
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	float GetHpRate(const FBattleObjectHandle& handle) const;

	/*!	バトル開始準備
		@note Prepare以降に有効
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void GetCommandOrder(TArray<FBattleCommand>& commandOrder) const;

	/*!	バトル用のキャラステータス生成
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static FBattleCharacterStatus MakeBattleCharacterStatus(const FCharacterStatus& stat, const UBattleDataTableHolder* dataAccessor);

	/*!	バトル用のキャラステータス生成（数値オフセット付き）
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static FBattleCharacterStatus MakeBattleCharacterStatusWithOffset(const FCharacterStatus& stat, const UBattleDataTableHolder* dataAccessor, int32 offsetHp, int32 offsetAttack, int32 offsetDeffence, int32 offsetSpeed);
	
	/*!	バトル用パーティ生成
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static FBattleParty MakeFromParty(const FParty& party, const UBattleDataTableHolder* dataAccessor);

	/*!	生存しているプレイヤー数
	*/
	int32 CalcAlivePlayers() const;

	/*!	コマンドリストをマージ
	*/
	void EnqueueCommands(const TArray<FBattleCommand>& commandList, EPlayerGroup playerSide);

	/*!	位置からキャラを取得
	*/
	int32 GetCharacterIndex(int32 posIndex, EPlayerGroup side) const
	{
		const FBattleParty* party = GetParty(side);
		if(party) {
			check(UBattleBoardUtil::GetCellNum() == party->Formation.Num());
			if(0 <= posIndex && posIndex < party->Formation.Num()) {
				return party->Formation[posIndex];
			}
		}
		return -1;
	}

	/*!	位置からキャラを取得
	*/
	FBattleObjectHandle MakeObjectHandle(int32 posIndex, EPlayerGroup side, EObjectType type = EObjectType::Character) const;

	/*!	位置からキャラを取得
	*/
	FBattleObjectHandle MakeObjectHandle(const BattleCell& cell, EPlayerGroup side, EObjectType type = EObjectType::Character) const;

	/*!	キャラのハンドルリストを取得
	*/
	void MakeObjectHandleList(TArray<FBattleObjectHandle>& out) const;
	

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

	/*!	パーティ取得
	*/
	const FBattleParty* GetParty(EPlayerGroup side) const
	{
		if(static_cast<int32>(side) < PartyList.Num()) {
			return &PartyList[static_cast<int32>(side)];
		}
		return nullptr;
	}
	FBattleParty* GetParty(EPlayerGroup side)
	{
		return const_cast<FBattleParty*>(static_cast<const UBattleSystem*>(this)->GetParty(side));
	}

	/*!	指定ハンドルのキャラを取得
	*/
	FBattleCharacterStatus* GetCharacterByHandle(FBattleParty* party, int32 characterHandle) const;

	/*!	指定ハンドルのキャラを取得
	*/
	const FBattleCharacterStatus* GetCharacterByHandle2(const FBattleObjectHandle& handle) const;
	FBattleCharacterStatus*       GetCharacterByHandle2(const FBattleObjectHandle& handle)
	{
		return const_cast<FBattleCharacterStatus*>(static_cast<const UBattleSystem*>(this)->GetCharacterByHandle2(handle));
	}

	/*!	指定場所のキャラを取得
	*/
	FBattleCharacterStatus* GetCharacterByPos(FBattleParty* party, int32 posIndex) const;

	/*!	オブジェクトの現在位置を取得
	*/
	int32 GetObjectPos(const FBattleObjectHandle& handle) const;

	/*!	オブジェクトの現在位置を取得
	*/
	BattleCell GetObjectCell(const FBattleObjectHandle& handle) const;

	/*!	設定取得
	*/
	const FBattleSettings& GetSettings() const { return BattleSettings; }

	static const FRandomStream& GetRandStream();


	/*!	デバッグ用ハンドルリスト
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Debug")
	void DebugGetHandleList(TArray<FBattleObjectHandle>& handles, EPlayerGroup group = EPlayerGroup::One) const;

	/*!	デバッグ用戦闘コマンド
	*/
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DebugCallBattleEvent();

	/*!	デバッグ用戦闘コマンド
	*/
	UFUNCTION(BlueprintCallable, Category = "Debug")
	bool DebugMakeBattleResult(FBattleSkillResult& result, EPlayerGroup attackerGroup, int32 attackerPos, FString skillName);

	/*!	スキルコマンド実行
	*/
	void ExecSkill(const FBattleCommand& command);

	/*	最大コマンド数を取得
	*/
	int32 GetMaxCommandCount(EPlayerGroup group) const;

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
	void ExecAttack(const FBattleCommand& command, EPlayerGroup group);

	/*!	移動コマンド実行
	*/
	void ExecMove(const FBattleCommand& command, EPlayerGroup group);

	/*!	ダメージ計算基礎式
		@param attack        攻撃力(100～2000くらいを想定)
		@param deffence      防御力(攻撃より低い値でないとダメージが出ない)
		@param randMin       ダメージ幅（最小）
		@param randMax       ダメージ幅（最大）
		@param attackerStyle 攻撃側のスタイル
		@param targetStyle   相手側のスタイル
		@param diffAcc       攻撃と防御の差があるときのダメージ増加量（累乗される）
		@param minDamage     最小ダメージ
		@param maxDamage     最大ダメージ
		@return ダメージ量
	*/
	static int32 CalcDamage(int32 attack, int32 deffence, int32 randMin, int32 randMax, EBattleStyle attackerStyle, EBattleStyle targetStyle, float diffAcc, int32 minDamage, int32 maxDamage);

	/*!	タイプごとのダメージ補正
	*/
	static float GetTypeDamageRate(EBattleStyle attackerStyle, EBattleStyle targetStyle);

	/*!	攻撃対象選択
	*/
	FBattleTarget GetAttackTargetByPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos, EPlayerGroup playerSide) const;

	/*!	攻撃対象選択
	*/
	void GetSkillTargetsByPos(TArray<FBattleTarget>& targets, const FBattleCharacterStatus& attacker, int32 attackerPos, EPlayerGroup selectPlayerSide, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam) const;

	/*!	攻撃セル取得
	*/
	bool GetSkillTargetPositions(BattleCell& mainCell, TArray<BattleCell>& positions, const FBattleCommand& command) const;

	/*!	攻撃セル取得
	*/
	bool GetSkillTargetPositions(BattleCell& mainCell, TArray<BattleCell>& positions, const FBattleObjectHandle& actor, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam, EBattleSelectRange expandMethod = EBattleSelectRange::Single) const;

	/*!	攻撃セル取得
	*/
	void GetTargetsByCell(TArray<FBattleTarget>& targets, const TArray<BattleCell>& positions, EPlayerGroup group) const;

	/*!	死亡更新
	*/
	void UpdateDie();

	/*! ターン処理のコマンドがなくなったらtrueを返す
	*/
	bool IsDoneTurnCommand(const UBattleCommandQueue* groupOneCommands, const UBattleCommandQueue* groupTwoCommands) const;
	
	/*! グループで選択
	*/
	template<typename T>
	static constexpr T& SelectWithGroup(T& groupOne, T& groupTwo, EPlayerGroup group)
	{
		return IsPlayerOne(group) ? groupOne : groupTwo;
	}

public:

	//! 攻撃デリゲート
	UPROPERTY(BlueprintAssignable, Category = "Battle/System")
	FBattleAttackDelegate AttackDelegate;

	//! スキル使用デリゲート
	UPROPERTY(BlueprintAssignable, Category = "Battle/System")
	FBattleSkillDelegate SkillDelegate;

	//! 移動デリゲート
	UPROPERTY(BlueprintAssignable, Category = "Battle/System")
	FBattleMoveDelegate MoveDelegate;

private:
	TArray<FBattleParty>             PartyList;             //!< パーティ
	FBattleSettings                  BattleSettings;        //!< バトル設定
	FBattleCommandContext            CommandContext;        //!< バトル進行管理
};
