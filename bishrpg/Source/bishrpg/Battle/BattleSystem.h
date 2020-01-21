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

		FBattleCommandContext()
		{
			GroupContext.SetNum(MaxGroupNum);
		}
		~FBattleCommandContext() = default;

		TArray<FGroupContext> GroupContext;
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

	/*!	バトル開始準備
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Prepare();

	/*!	バトルコマンド処理
		
	AttackDelegate,SkillDelegate,MoveDelegateが呼ばれます。
	@param[in]	groupOneCommands	グループ1コマンド
	@param[in]	groupTwoCommands	グループ2コマンド
	@return		消費するコマンドがあった場合はtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool ConsumeCommand(const UBattleCommandQueue* groupOneCommands, const UBattleCommandQueue* groupTwoCommands);

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

	static const FRandomStream& GetRandStream();
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

	/*!	スキルコマンド実行
	*/
	void ExecSkill(const FBattleCommand& command, EPlayerGroup group);

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
	FBattleTarget GetAttackTargetByPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos, EPlayerGroup playerSide) const;

	/*!	攻撃対象選択
	*/
	void GetSkillTargetsByPos(TArray<FBattleTarget>& targets, const FBattleCharacterStatus& attacker, int32 attackerPos, EPlayerGroup selectPlayerSide, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam) const;

	/*!	死亡更新
	*/
	void UpdateDie();

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
	FBattleCommandContext            CommandContext;        //!< バトル進行管理
	//FGroupCommandList                GroupCommandList;      //!< 実行コマンドリスト
	//TArray<Command>                  MergedCommandList;     //!< 全バトルコマンドリスト
	//TArray<Command>                  MergedMoveCommandList; //!< 移動コマンドをまとめたやつ
	//static const FRandomStream*      RandStream;            //!< ランダム
};
