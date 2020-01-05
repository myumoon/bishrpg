// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedPointer.h"
#include "System/CharacterStatus.h"
#include "GameData/SkillData.h"
#include "BattleDataType.h"
#include "BattleData.h"
#include "BattleObjectHandle.h"
#include "BattleCommandQueue.generated.h"

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
	bool PushSkillCommand2(const FBattleObjectHandle& actor, const FName& skillName);

	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool PushMoveCommand(int32 posIndex, int32 moveTo);

	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool PushMoveCommand2(const FBattleObjectHandle& actor, int32 moveTo);

	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool PushAttackCommand(int32 posIndex);

	/*!	バトル行動を追加
	@return コマンド数がいっぱいになったらtrue
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool PushAttackCommand2(const FBattleObjectHandle& actor);

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

	/*!	コマンド実行前のキャラの位置を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetInitialCharacterPos(int32 posIndex) const;

	/*!	コマンド実行前のキャラの位置を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetInitialCharacterPos2(const FBattleObjectHandle& handle) const;

	/*!	移動コマンド実行前のキャラを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetMovedCharacterIndex(int32 posIndex, bool playerSide) const;

	/*!	移動コマンド実行前のキャラを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetOriginCharacterIndex(int32 posIndex, EPlayerGroup side) const;

protected:
	int32 GetPrevPosIndex(int32 posIndex) const;

private:
	TArray<FBattleCommand> CommandList;         //!< バトルコマンドリスト
	UBattleSystem*         BattleSystem = nullptr;
	//bool                   PlayerSide = true;
	EPlayerGroup           PlayerSide = EPlayerGroup::One;
};
