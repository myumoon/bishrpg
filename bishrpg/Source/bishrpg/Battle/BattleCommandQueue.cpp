// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleCommandQueue.h"

#include "bishrpg.h"
#include "BattleSystem.h"
#include "ObjectHandleLibrary.h"

// 初期化
void UBattleCommandQueue::Initialize(UBattleSystem* system, bool playerSide)
{
	if(system == nullptr) {
		UE_LOG(BishRPG, Error, TEXT("UBattleCommandQueue::Initialize : system is nullptr"));
		//GAME_ERROR("UBattleCommandQueue::Initialize : system is nullptr");
	}
	BattleSystem = system;

	// todo : 直接設定するようにする
	PlayerSide = playerSide ? EPlayerGroup::One : EPlayerGroup::Two;
}

// 攻撃コマンド追加
bool UBattleCommandQueue::PushAttackCommand(int32 posIndex)
{
	const int32 prevPosIndex = GetPrevPosIndex(posIndex);
	FBattleCommand addCommand;
	BattleCell cell(prevPosIndex);
	addCommand.ActionType = ECommandType::Attack;
	addCommand.ActorHandle = BattleSystem->MakeObjectHandle(cell, PlayerSide);
	addCommand.ActionPosIndex = posIndex;
	//addCommand.CharacterIndex = BattleSystem->GetCharacterIndex(prevPosIndex, PlayerSide);
	GAME_LOG("PushAttack PlayerSide(%s) : pos(%d) -> prev(%d) -> handle(%d)", PlayerSide == EPlayerGroup::One ? TEXT("true") : TEXT("false"), posIndex, prevPosIndex, addCommand.CharacterIndex);
	addCommand.TargetPosIndex = 0;
	CommandList.Add(addCommand);

	return CanStartCommand();
}

// 攻撃コマンド追加
bool UBattleCommandQueue::PushAttackCommand2(const FBattleObjectHandle& actor)
{
	//const int32 prevPosIndex = GetPrevPosIndex(posIndex);
	FBattleCommand addCommand;
	addCommand.ActionType = ECommandType::Attack;
	addCommand.ActionPosIndex = GetInitialCharacterPos2(actor);
	addCommand.ActorHandle = actor;
	//addCommand.CharacterIndex = BattleSystem->GetCharacterIndex(prevPosIndex, PlayerSide);
	GAME_LOG("PushAttack PlayerSide(%s) : index(%d)", IsPlayerOne(PlayerSide) ? TEXT("true") : TEXT("false"), addCommand.CharacterIndex);
	addCommand.TargetPosIndex = 0;
	CommandList.Add(addCommand);

	return CanStartCommand();
}

// スキルコマンド追加
bool UBattleCommandQueue::PushSkillCommand(int32 posIndex, const FName& skillName)
{
	const int32 prevPosIndex = GetPrevPosIndex(posIndex);
	FBattleCommand addCommand;
	addCommand.ActionType = ECommandType::Skill;
	addCommand.ActionPosIndex = posIndex;
	addCommand.CharacterIndex = BattleSystem->GetCharacterIndex(prevPosIndex, PlayerSide);
	addCommand.TargetPosIndex = 0;
	addCommand.SkillName = skillName;
	CommandList.Add(addCommand);

	return CanStartCommand();
}

// スキルコマンド追加
bool UBattleCommandQueue::PushSkillCommand2(const FBattleObjectHandle& actor, const FName& skillName)
{
	//const int32 prevPosIndex = GetPrevPosIndex(posIndex);
	FBattleCommand addCommand;
	addCommand.ActionType = ECommandType::Skill;
	addCommand.ActionPosIndex = GetInitialCharacterPos2(actor);
	addCommand.ActorHandle = actor;
	//addCommand.CharacterIndex = BattleSystem->GetCharacterIndex(prevPosIndex, PlayerSide);
	addCommand.TargetPosIndex = 0;
	addCommand.SkillName = skillName;
	CommandList.Add(addCommand);

	return CanStartCommand();
}

// 移動コマンド追加
bool UBattleCommandQueue::PushMoveCommand(int32 posIndex, int32 moveTo)
{
	const int32 prevPosIndex = GetPrevPosIndex(posIndex);
	const int32 prevMoveToPosIndex = GetPrevPosIndex(moveTo);
	const int32 moveToCharHandle = BattleSystem->GetCharacterIndex(prevMoveToPosIndex, PlayerSide);

	FBattleCommand addCommand;
	addCommand.ActionType = (0 <= moveToCharHandle) ? ECommandType::Swap : ECommandType::Move;
	addCommand.ActionPosIndex = posIndex;
	addCommand.CharacterIndex = BattleSystem->GetCharacterIndex(prevPosIndex, PlayerSide);
	addCommand.TargetPosIndex = moveTo;
	CommandList.Add(addCommand);

	return CanStartCommand();
}

// 移動コマンド追加
bool UBattleCommandQueue::PushMoveCommand2(const FBattleObjectHandle& actor, int32 moveTo)
{
	//const int32 prevPosIndex = GetPrevPosIndex(posIndex);
	const int32 prevMoveToPosIndex = GetPrevPosIndex(moveTo);
	const int32 moveToCharHandle = BattleSystem->GetCharacterIndex(prevMoveToPosIndex, PlayerSide);

	FBattleCommand addCommand;
	addCommand.ActionType = (0 <= moveToCharHandle) ? ECommandType::Swap : ECommandType::Move;
	addCommand.ActionPosIndex = GetInitialCharacterPos2(actor);
	addCommand.ActorHandle = actor;
	//addCommand.CharacterIndex = BattleSystem->GetCharacterIndex(prevPosIndex, PlayerSide);
	addCommand.TargetPosIndex = moveTo;
	CommandList.Add(addCommand);

	return CanStartCommand();
}

// 前回の位置を取得
int32 UBattleCommandQueue::GetPrevPosIndex(int32 posIndex) const
{
	int32 prevPosIndex = posIndex;

	for(int i = CommandList.Num() - 1; 0 <= i; --i) {
		const auto& command = CommandList[i];
		if(command.ActionType == ECommandType::Move) {
			if(command.TargetPosIndex == prevPosIndex) {
				prevPosIndex = command.ActionPosIndex;
			}
		}
		else if(command.ActionType == ECommandType::Swap) {
			if(command.TargetPosIndex == prevPosIndex) {
				prevPosIndex = command.ActionPosIndex;
			}
			else if(command.ActionPosIndex == prevPosIndex) {
				prevPosIndex = command.TargetPosIndex;
			}
		}
	}

	return prevPosIndex;
}

// 巻き戻す
bool UBattleCommandQueue::RevertCommand(FBattleCommand& revertedCommand)
{
	if(CommandList.Num() == 0) {
		return false;
	}

	revertedCommand = CommandList.Last();
	CommandList.RemoveAt(CommandList.Num() - 1);

	return true;
}

// リセット
void UBattleCommandQueue::ResetCommand()
{
	CommandList.Reset();
}

// コマンド実行可能か
bool UBattleCommandQueue::CanStartCommand() const
{
	if(BattleSystem == nullptr) {
		GAME_ERROR("CanStartCommand : Not initialized");
		return false;
	}
	return (BattleSystem->CalcAlivePlayers() == GetCount(true));
}

// コマンド送信
void UBattleCommandQueue::Commit()
{
	GAME_LOG("UBattleCommandQueue::Commit");
	if(BattleSystem) {
		BattleSystem->EnqueueCommands(CommandList, PlayerSide);
		ResetCommand();
	}
}

// コマンドの数をカウント
int32 UBattleCommandQueue::GetCount(bool includingMoveCommand) const
{
	int32 count = 0;

	if(includingMoveCommand) {
		count = CommandList.Num();
	}
	else {
		for(const auto& command : CommandList) {
			//if((command.ActionType == ECommandType::Attack) || (command.ActionType == ECommandType::Skill)) 
			{
				++count;
			}
		}
	}

	return count;
}

// コマンド実行前のキャラの位置を取得
int32 UBattleCommandQueue::GetInitialCharacterPos(int32 posIndex) const
{
	if(BattleSystem == nullptr) {
		return -1;
	}

	int32 searchPosIndex = posIndex;
	for(int32 i = CommandList.Num() - 1; 0 <= i; --i) {
		const auto& command = CommandList[i];
		if((command.ActionType == ECommandType::Move) || (command.ActionType == ECommandType::Swap)) {
			if(command.TargetPosIndex == searchPosIndex) {
				searchPosIndex = command.ActionPosIndex;
			}
			else if(command.ActionPosIndex == searchPosIndex) {
				searchPosIndex = command.TargetPosIndex;
			}
		}
	}

	return searchPosIndex;
}

// コマンド実行前のキャラの位置を取得
int32 UBattleCommandQueue::GetInitialCharacterPos2(const FBattleObjectHandle& handle) const
{
	if(BattleSystem == nullptr) {
		return -1;
	}

	for(int32 i = 0; i < CommandList.Num(); ++i) {
		const auto& command = CommandList[i];
		if(handle == command.ActorHandle) {
			if((command.ActionType == ECommandType::Move) || (command.ActionType == ECommandType::Swap)) {
				return command.ActionPosIndex;
			}
		}
	}

	return BattleSystem->GetObjectPos(handle);
}

// 移動コマンド実行前のキャラを取得
int32 UBattleCommandQueue::GetMovedCharacterIndex(int32 posIndex, bool playerSide) const
{
	const EPlayerGroup side = playerSide ? EPlayerGroup::One : EPlayerGroup::Two;
	return GetOriginCharacterIndex(posIndex, side);
}

// 移動コマンド実行前のキャラを取得
int32 UBattleCommandQueue::GetOriginCharacterIndex(int32 posIndex, EPlayerGroup side) const
{
	int32 initialPos = GetInitialCharacterPos(posIndex);
	if(initialPos < 0) {
		return -1;
	}
	const int32 charIndex = BattleSystem->GetCharacterIndex(initialPos, side);

	return charIndex;
}

// コマンド取得
const FBattleCommand& UBattleCommandQueue::GetCommand(int32 index) const
{
	if((index < 0) || (CommandList.Num() <= index)) {
		GAME_FATAL("ivalid index : CommandListSize(%d), index(%d)", CommandList.Num(), index);
		return CommandList[0];
	}
	return CommandList[index];
}