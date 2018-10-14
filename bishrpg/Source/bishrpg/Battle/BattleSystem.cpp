// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleSystem.h"

#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"
#include "GameData/CharacterAsset.h"
#include "GameData/SkillData.h"
#include "GameData/BishRPGDataTblAccessor.h"

#include "bishrpg.h"


void UBattleBoardUtil::MakePositionListCol(TArray<int32>& madePosList, int32 col, bool up)
{
	if(UBattleBoardUtil::GetBoardCol() <= col) {
		GAME_ERROR("invalid col %d", col);
		return;
	}

	// 後衛から
	if(up) {
		for(int32 i = col; i < UBattleBoardUtil::GetCellNum(); i += UBattleBoardUtil::GetBoardCol()) {
			madePosList.Add(i);
		}
	}
	// 前線から
	else {
		for(int32 i = UBattleBoardUtil::GetCellNum() - UBattleBoardUtil::GetBoardCol() + col; 0 <= i; i -= UBattleBoardUtil::GetBoardCol()) {
			madePosList.Add(i);
		}
	}
}

void UBattleBoardUtil::MakePositionListRow(TArray<int32>& madePosList, int32 row)
{
	const int32 begin = row * UBattleBoardUtil::GetBoardRow();
	const int32 end   = begin + UBattleBoardUtil::GetBoardRow();
	for(int32 i = begin; i < end; ++i) {
		madePosList.Add(i);
	}
}
void UBattleBoardUtil::MakePositionListRandom(TArray<int32>& madePosList, int32 positions, const FRandomStream& randStream)
{
	TArray<int32> poslist;
	poslist.Reserve(CELL_NUM);
	for(int32 i = 0; i < CELL_NUM; ++i) {
		poslist.Add(i);
	}

	for(int32 i = 0; i < positions; ++i) {
		const int32 index = randStream.RandRange(0, poslist.Num());
		madePosList.Add(poslist[index]);
		poslist.RemoveAt(index, 1, false);
	}
}




// 初期化
void UBattleCommandQueue::Initialize(UBattleSystem* system, bool playerSide)
{
	if(system == nullptr) {
		UE_LOG(BishRPG, Error, TEXT("UBattleCommandQueue::Initialize : system is nullptr"));
		//GAME_ERROR("UBattleCommandQueue::Initialize : system is nullptr");
	}
	BattleSystem = system;
	PlayerSide = playerSide;
}

// 攻撃コマンド追加
bool UBattleCommandQueue::PushAttackCommand(int32 posIndex)
{
	const int32 prevPosIndex = GetPrevPosIndex(posIndex);
	FBattleCommand addCommand;
	addCommand.ActionType = ECommandType::Attack;
	addCommand.ActionPosIndex = posIndex;
	addCommand.CharacterHandle = BattleSystem->GetCharacterHandle(prevPosIndex, PlayerSide);
	GAME_LOG("PushAttack PlayerSide(%s) : pos(%d) -> prev(%d) -> handle(%d)", PlayerSide ? TEXT("true") : TEXT("false"), posIndex, prevPosIndex, addCommand.CharacterHandle);
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
	addCommand.CharacterHandle = BattleSystem->GetCharacterHandle(prevPosIndex, PlayerSide);
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
	const int32 moveToCharHandle = BattleSystem->GetCharacterHandle(prevMoveToPosIndex, PlayerSide);

	FBattleCommand addCommand;
	addCommand.ActionType = (0 <= moveToCharHandle) ? ECommandType::Swap : ECommandType::Move;
	addCommand.ActionPosIndex = posIndex;
	addCommand.CharacterHandle = BattleSystem->GetCharacterHandle(prevPosIndex, PlayerSide);
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
	return (BattleSystem->CalcAlivePlayers() == GetCount());
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

// 移動コマンド実行前のキャラを取得
int32 UBattleCommandQueue::GetMovedCharacterHandle(int32 posIndex, bool playerSide) const
{
	int32 initialPos = GetInitialCharacterPos(posIndex);
	if(initialPos < 0) {
		return -1;
	}
	int32 charHandle = BattleSystem->GetCharacterHandle(initialPos, playerSide);

	return charHandle;
}



// Sets default values for this component's properties
UBattleSystem::UBattleSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	MergedCommandList.Reserve(32); // 12マスなので余裕を持ってこのくらい
	PartyList.Reserve(2);
}


// Called when the game starts
void UBattleSystem::BeginPlay()
{
	Super::BeginPlay();

	// ...
	UE_LOG(BishRPG, Log, TEXT("UBattleSystem::BeginPlay()"));
}


// Called every frame
/*
void UBattleSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
*/

// バトル用キャラ情報取得
void UBattleSystem::GetCharacterStatusByPos(FBattleCharacterStatus& stat, int32 posIndex, bool playerSide) const
{
    auto status = GetCharacterByPos(const_cast<FBattleParty*>(GetParty(playerSide)), posIndex);
    if(status != nullptr) {
        stat = *status;
    }
}

// バトル用キャラ情報取得
void UBattleSystem::GetCharacterStatusByHandle(FBattleCharacterStatus& stat, int32 handle, bool playerSide) const
{
    const FBattleCharacterStatus* status = const_cast<FBattleParty*>(GetParty(playerSide))->GetCharacterByHandle(handle);
    check(status != nullptr);
    if(status != nullptr) {
        stat = *status;
    }
}

// バトル用キャラ情報生成
FBattleCharacterStatus UBattleSystem::MakeBattleCharacterStatus(const FCharacterStatus& stat)
{
	return MakeBattleCharacterStatusWithOffset(stat, 0, 0, 0, 0);
}

// バトル用モンスター情報生成
FBattleCharacterStatus UBattleSystem::MakeBattleCharacterStatusWithOffset(const FCharacterStatus& stat, int32 offsetHp, int32 offsetAttack, int32 offsetDeffence, int32 offsetSpeed)
{
	FBattleCharacterStatus battleStatus;
	battleStatus.Id = stat.Id;

	const auto* characterAssetTbl = ABishRPGDataTblAccessor::GetTbl(ETblType::CharacterAssetTbl);
	const FCharacterAsset* charData = characterAssetTbl->FindRow<FCharacterAsset>(battleStatus.Id, "");

	const auto* hpTbl       = charData ? charData->HpTbl : nullptr;
	const auto* attackTbl   = charData ? charData->AttackTbl : nullptr;
	const auto* deffenceTbl = charData ? charData->DeffenceTbl : nullptr;
	const auto* speedTbl    = charData ? charData->SpeedTbl : nullptr;

	auto getCurveFrom = [](const UCurveFloat* curve, int32 lv, int32 defaultValue) {
		return curve ? static_cast<int32>(curve->GetFloatValue(static_cast<float>(lv))) : defaultValue;
	};

	battleStatus.Hate     = 0;	battleStatus.Style    = charData ? charData->Style : EBattleStyle::Humor;
	battleStatus.HpMax    = getCurveFrom(hpTbl, stat.HpLv, 1) + offsetHp;
	battleStatus.Hp       = battleStatus.HpMax;
	battleStatus.Attack   = getCurveFrom(attackTbl, stat.AttackLv, 1) + offsetAttack;
	battleStatus.Deffence = getCurveFrom(deffenceTbl, stat.DeffenceLv, 1) + offsetDeffence;
	battleStatus.Speed    = getCurveFrom(speedTbl, stat.SpeedLv, 1) + offsetSpeed;


	return battleStatus;
}



// バトル用パーティ情報生成
FBattleParty UBattleSystem::MakeFromParty(const FParty& party)
{
	FBattleParty battleParty;
	battleParty.Characters.AddUninitialized(UBattleBoardUtil::GetCellNum());

	for(int i = 0; i < party.Characters.Num(); ++i) {
		battleParty.Characters[i] = MakeBattleCharacterStatus(party.Characters[i]);
	}
	battleParty.Formation = party.Formation;

	return battleParty;
}


// 初期化
void UBattleSystem::Initialize(const FParty& playerParty, const FParty& opponentParty, const FRandomStream& randStream)
{
	const FParty* partyList[] = { &playerParty, &opponentParty };

	for(auto* party : partyList) {
		FBattleParty battleParty = MakeFromParty(*party);
		PartyList.Add(battleParty);
	}

	RandStream = &randStream;
}


// コマンド実行可能か
int32 UBattleSystem::CalcAlivePlayers() const
{
	//check(PartyList.Num() == PartyIndex::PartyNum);
	if(PartyList.Num() != static_cast<int32>(PartyIndex::PartyNum)) {
		GAME_ERROR("UBattleSystem : Not initialized.");
		return 0;
	}

	int32 aliveCount = 0;
	const auto& party = PartyList[PartyIndex::Player];
	for(auto pos : party.Formation) {
		if(0 <= pos) {
			if(0 < party.Characters[pos].Hp) {
				++aliveCount;
			}
		}
	}
	return aliveCount;
}


// コマンド実行
void UBattleSystem::EnqueueCommands(const TArray<FBattleCommand>& commandList, bool playerSide)
{
	FBattleParty* insertParty   = GetParty(playerSide);
	FBattleParty* opponentParty = GetParty(!playerSide);
	if((insertParty == nullptr) || (opponentParty == nullptr)) {
		GAME_ERROR("EnqueueCommands : Party is null");
		return;
	}

	Command addCommand;
	int32 checkIndex = 0;
	int32 addedCommands = 0;
	bool existCommand = false;
	bool prevAddSide = playerSide;

	TArray<Command> mergedCommands;
	mergedCommands.Reserve(UBattleBoardUtil::GetCellNum() * 2);

	TArray<FBattleCommand> tempCommands = commandList;

	// 移動コマンドは先に処理
	{
		Command add;
		for(int i = 0; i < tempCommands.Num(); ++i) {
			if((tempCommands[i].ActionType == ECommandType::Move) || (tempCommands[i].ActionType == ECommandType::Swap)) {
			
				add.BattleCommand = tempCommands[i];
				add.PlayerSide = playerSide;
				MergedMoveCommandList.Add(add);
				tempCommands.RemoveAt(i, 1, false);
				--i; // 削除してNum()が減るので減らす
			}
		}
		tempCommands.Shrink();
	}

	auto addCommandList = [](TArray<Command>& list, const FBattleCommand& addCommand, bool PlayerSide) {
		GAME_LOG("AddCommand : pos(%d), player(%s)", addCommand.ActionPosIndex, PlayerSide ? TEXT("true") : TEXT("false"));
		Command add;
		add.BattleCommand = addCommand;
		add.PlayerSide = PlayerSide;
		list.Add(add);
	};

	// 速度が早い方から順に入れていく
	int insertIndex = 0;
	for( ; insertIndex < tempCommands.Num(); ++insertIndex) {
		// tempCommandsの余った分は外で入れる
		if(checkIndex == MergedCommandList.Num()) {
			break;
		}

		const int32 insertHandle = tempCommands[insertIndex].CharacterHandle;
		if(insertHandle < 0 || insertParty->Characters.Num() <= insertHandle) {
			continue;
		}
		
		const auto& insertChar   = insertParty->Characters[insertHandle];

		for( ; checkIndex < MergedCommandList.Num(); ) {
			const int32 checkHandle = MergedCommandList[checkIndex].BattleCommand.CharacterHandle;
			if(checkHandle < 0 || opponentParty->Characters.Num() <= checkHandle) {
				++checkIndex;
				continue;
			}

			const auto& checkChar = opponentParty->Characters[checkHandle];
			if((prevAddSide && checkChar.Speed <= insertChar.Speed) || (!prevAddSide && checkChar.Speed < insertChar.Speed)) {
				addCommandList(mergedCommands, tempCommands[insertIndex], playerSide);
				prevAddSide = playerSide;
				break;
			}
			else {
				addCommandList(mergedCommands, MergedCommandList[checkIndex].BattleCommand, !playerSide);
				prevAddSide = !playerSide;
				++checkIndex;
			}
		}
	}

	for(; insertIndex < tempCommands.Num(); ++insertIndex) {
		const int32 checkHandle = tempCommands[insertIndex].CharacterHandle;
		if(0 <= checkHandle) {
			addCommandList(mergedCommands, tempCommands[insertIndex], playerSide);
		}
	}
	for(; checkIndex < MergedCommandList.Num(); ++checkIndex) {
		const int32 checkHandle = MergedCommandList[checkIndex].BattleCommand.CharacterHandle;
		if(0 <= checkHandle) {
			addCommandList(mergedCommands, MergedCommandList[checkIndex].BattleCommand, !playerSide);
		}
	}

	MergedCommandList = mergedCommands;
}


// コマンド実行
bool UBattleSystem::ConsumeCommand(FBattleActionResult& result)
{
	if(MergedCommandList.Num() == 0) {
		//GAME_ERROR("ConsumeCommand : Empty PlayerCommandList");
		return false;
	}
	
	Command execCommand = MergedCommandList[0];
	MergedCommandList.RemoveAt(0);

	result.TargetResults.Reset();
	result.ActionType           = ConvertAction(execCommand.BattleCommand.ActionType);
	//result.Actor.TargetPosIndex = GetParty(execCommand.PlayerSide)->GetCharacterPosByHandle(execCommand.BattleCommand.CharacterHandle);
	result.Actor.TargetHandle   = execCommand.BattleCommand.CharacterHandle;
	GAME_LOG("Consume handle(%d), playerSide(%s), skill(%s)", execCommand.BattleCommand.CharacterHandle, execCommand.PlayerSide ? TEXT("true") : TEXT("false"), *execCommand.BattleCommand.SkillName.ToString());
	result.Actor.PlayerSide     = execCommand.PlayerSide;
	
	switch(execCommand.BattleCommand.ActionType) {
		case ECommandType::Attack : {
			ExecAttack(result, execCommand);
		} break;
		
		case ECommandType::Skill: {
			ExecSkill(result, execCommand);
		} break;

		default:
			break;
	}

	UpdateDie();

	return true;
}

// 移動コマンド全消費
bool UBattleSystem::ConsumeMoveCommands(TArray<FBattleActionResult>& result)
{
	FBattleActionResult moveResult;
	//moveResult.ActionType = EBattleActionType::Move;

	bool moved = (0 < MergedMoveCommandList.Num());

	// 古い情報をバックアップ
	int32 oldPlayerFormation[UBattleBoardUtil::CELL_NUM] = { 0 };
	int32 oldOpponentFormation[UBattleBoardUtil::CELL_NUM] = { 0 };
	const auto* playerParty = GetParty(true);
	const auto* opponentParty = GetParty(false);
	for(int i = 0; i < playerParty->Formation.Num(); ++i) {
		oldPlayerFormation[i] = playerParty->Formation[i];
	}
	for(int i = 0; i < playerParty->Formation.Num(); ++i) {
		oldOpponentFormation[i] = opponentParty->Formation[i];
	}

	for(int i = 0; i < MergedMoveCommandList.Num(); ++i) {
		const bool playerSide = MergedMoveCommandList[i].PlayerSide;
		const int32 actorPos  = MergedMoveCommandList[i].BattleCommand.ActionPosIndex;
		const int32 moveto    = MergedMoveCommandList[i].BattleCommand.TargetPosIndex;
		auto* party = GetParty(playerSide);

		/*
		moveResult.ActionType = EBattleActionType::Move;
		moveResult.Actor.PlayerSide = playerSide;
		moveResult.Actor.TargetPosIndex = actorPos;
		moveResult.MoveTo = moveto;
		result.Add(moveResult);
		*/

		party->Move(actorPos, moveto);

		/*
		// 交換するときは相手側もコマンドに入れておく
		if(MergedMoveCommandList[i].BattleCommand.ActionType == ECommandType::Swap) {
			moveResult.Actor.TargetPosIndex = MergedMoveCommandList[i].BattleCommand.TargetPosIndex; 
			moveResult.MoveTo = MergedMoveCommandList[i].BattleCommand.ActionPosIndex;
			result.Add(moveResult);
		}
		*/
	}

	// 移動後の配置と比較してコマンドを生成
	{
		const int32* oldFormationList[] = { oldPlayerFormation, oldOpponentFormation };
		const TArray<int32>* currentFormationList[] = { &playerParty->Formation, &opponentParty->Formation };
		const FBattleParty* partyList[] = { playerParty, opponentParty };

		for(int partyIndex = 0; partyIndex < ARRAY_COUNT(oldFormationList); ++partyIndex) {
			for(int i = 0; i < UBattleBoardUtil::GetCellNum(); ++i) {
				const int32* oldFormation = oldFormationList[partyIndex];
				const auto&  currentFormation = *currentFormationList[partyIndex];

				if((0 <= oldFormation[i]) && (oldFormation[i] != currentFormation[i])) {
					moveResult.ActionType = EBattleActionType::Move;
					moveResult.Actor.PlayerSide = (partyIndex == 0);
					moveResult.Actor.TargetHandle = oldFormation[i];
					moveResult.MoveFrom = i;
					currentFormation.Find(oldFormation[i], moveResult.MoveTo);
					result.Add(moveResult);
				}
			} 
		}
	}

	MergedMoveCommandList.Reset();

	return moved;
}

// 死亡チェック
void UBattleSystem::UpdateDie()
{
	for(auto& party : PartyList) {
		for(int32 pos = 0; pos < party.Formation.Num(); ++pos) {
			if(0 < party.Formation[pos]) {
				const auto* character = party.GetCharacterByPos(pos);
				if(character->Hp <= 0) {
					party.Formation[pos] = -1;
				}
			}
		}
	}

}

// ダメージ基礎式
int32 UBattleSystem::CalcDamage(int32 attack, int32 deffence, int32 randMin, int32 randMax, EBattleStyle attackerStyle, EBattleStyle targetStyle, float diffAcc/*=1.3f*/, int32 minDamage/*=10*/)
{
	// 攻撃-防御
	const float attackDiff = FMath::Max(static_cast<float>(attack - deffence), 0.0f);

	// 攻撃力に加算する
	const int32 rand = FMath::RandRange(FMath::Min(randMin, randMax), FMath::Max(randMin, randMax));

	// 攻撃-防御の差が開くほどダメージが大きくなるようにする
	const float damageBase = FMath::Pow(attackDiff * 0.01f, diffAcc) * (attack + rand);

	// 最低ダメージ保証
	const int32 damage = FMath::Max(FMath::FloorToInt(damageBase), minDamage);

	// タイプ補正
	const int32 typeDamage = static_cast<int32>(damage * GetTypeDamageRate(attackerStyle, targetStyle));

	return typeDamage;
}

// ダメージ補正値
float UBattleSystem::GetTypeDamageRate(EBattleStyle attackerStyle, EBattleStyle targetStyle)
{
	static const float RateWeak   = 2.0f;
	static const float RateNormal = 1.0f;
	static const float RateStrog  = 0.5f;

	static const float damageRateTbl[] = {
		//                    相手
	    //          Rock,        Hormor,     Sing,
		/*Rock*/    RateNormal,  RateWeak,   RateStrog,
		/*Hurmor*/  RateStrog,   RateNormal, RateWeak,
		/*Sing*/    RateWeak,    RateStrog,  RateNormal,
		// 自身
	};
	
	const int32 index = static_cast<int32>(attackerStyle) * 3 + static_cast<int32>(targetStyle);
	check(0 <= index && index < ARRAY_COUNT(damageRateTbl));
	return damageRateTbl[index];
}

// 攻撃
// 通常攻撃は手前のキャラを対象に殴る
void UBattleSystem::ExecAttack(FBattleActionResult& result, const Command& command)
{
	auto* attackChar       = GetCharacterByHandle(GetParty(command.PlayerSide), command.BattleCommand.CharacterHandle);
	if(attackChar == nullptr) {
		return;
	}
	const int32 attackerPos = GetParty(command.PlayerSide)->GetCharacterPosByHandle(command.BattleCommand.CharacterHandle);
	const auto target       = GetAttackTargetByPos(GetParty(!command.PlayerSide), *attackChar, attackerPos, command.PlayerSide);
	auto* targetChar        = GetCharacterByHandle(GetParty(!command.PlayerSide), target.TargetHandle);
	if(targetChar == nullptr) {
		return;
	}
	const float attack     = attackChar->Attack;
	const float deffence   = targetChar->Deffence;
	const int32 damage     = CalcDamage(attack, deffence, -50, 50, attackChar->Style, targetChar->Style, 1.3f, 10);
	targetChar->ReceiveDamage(damage);

	FBattleTargetValue targetResult;
	targetResult.Target = target;
	targetResult.Value  = damage;
	targetResult.Status |= static_cast<int32>(targetChar->IsDie() ? EStatusFlag::Status_Die : EStatusFlag::None);
	result.TargetResults.Add(targetResult);
}

// スキル
void UBattleSystem::ExecSkill(FBattleActionResult& result, const Command& command)
{
	auto* attackChar = GetCharacterByHandle(GetParty(command.PlayerSide), command.BattleCommand.CharacterHandle);
	if(attackChar == nullptr) {
		return;
	}
	const int32 attackerPos = GetParty(command.PlayerSide)->GetCharacterPosByHandle(command.BattleCommand.CharacterHandle);
	const auto* skillTbl    = ABishRPGDataTblAccessor::GetTbl(ETblType::SkillTbl);
	const auto* skillData   = skillTbl->FindRow<FSkillData>(command.BattleCommand.SkillName, FString(""));
	if(skillData == nullptr) {
		GAME_ERROR("ExecSkill : Not found '%s' in the SkillTbl", *command.BattleCommand.SkillName.ToString());
		return;
	}

	FBattleTargetValue targetResult;
	result.SkillName = command.BattleCommand.SkillName;

	if(skillData->Type == ESkillType::Attack || skillData->Type == ESkillType::Heal) {
		TArray<FBattleTarget> targets;
		GetSkillTargetsByPos(targets, *attackChar, attackerPos, command.PlayerSide, skillData->Type, skillData->SelectType, skillData->SelectParam);
		for(auto& target : targets) {
			auto* targetChar = GetCharacterByHandle(GetParty(target.PlayerSide), target.TargetHandle);
			if(targetChar == nullptr) {
				GAME_ERROR("target char is null");
				return;
			}
			
			if(skillData->Type == ESkillType::Attack) {
				const float attack   = attackChar->Attack;
				const float deffence = targetChar->Deffence;
				const int32 damage   = CalcDamage(attack, deffence, 0, 100, attackChar->Style, targetChar->Style, 1.4f, 20);
				targetChar->ReceiveDamage(damage);

				targetResult.Target = target;
				targetResult.Value = damage;
				targetResult.Status |= static_cast<int32>(targetChar->IsDie() ? EStatusFlag::Status_Die : EStatusFlag::None);
				result.TargetResults.Add(targetResult);
			}
			else {
				// todo : 回復
				//skillData->ValueAtLevel->GetFloatValue();
				//targetChar->Heal();
			}
		}
	}


}

// 移動
void UBattleSystem::ExecMove(FBattleActionResult& result, const Command& command)
{

}

// 攻撃対象選択
FBattleTarget UBattleSystem::GetAttackTargetByPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos, bool playerSide) const
{
	TArray<int32> selectedTarget;
	opponentParty->SelectTop(selectedTarget, 0, attackerPos);
	if(selectedTarget.Num() == 0) {
		GAME_ERROR("GetAttackTargetByPos : not selected. attackerPos(%d), playerPos(%s)", attackerPos, playerSide ? TEXT("true") : TEXT("false"));
		return FBattleTarget();
	}

	FBattleTarget target;
	target.PlayerSide   = !playerSide;
	target.TargetHandle = selectedTarget[0];

	return target;
}

// スキル対象
void UBattleSystem::GetSkillTargetsByPos(TArray<FBattleTarget>& targets, const FBattleCharacterStatus& actor, int32 actorPos, bool actorSide, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam) const
{
	const bool targetPlayerSide = (skillType == ESkillType::Heal) ? actorSide : !actorSide;
	const auto* targetParty = GetParty(targetPlayerSide);

	TArray<int32> selectedTarget;
	targetParty->Select(selectedTarget, actorPos, selectType, selectParam, *RandStream);
	if(selectedTarget.Num() == 0) {
		//GAME_ERROR("GetSkillTargetsByPos : not selected. actorPos(%d), actorSide(%s), selectType(%d), selectParam(%d)", actorPos, actorSide ? TEXT("true") : TEXT("false"), static_cast<int32>(selectType), selectParam);
		return;
	}

	FBattleTarget target;
	for(int32 handle : selectedTarget) {
		target.PlayerSide   = targetPlayerSide;
		target.TargetHandle = handle;
		targets.Add(target);
	}
	
}



FBattleCharacterStatus* UBattleSystem::GetCharacterByHandle(FBattleParty* party, int32 characterHandle) const
{
	if(party == nullptr) {
		GAME_ERROR("GetCharacterByPos : party is nullptr");
		return nullptr;
	}
	if((characterHandle < 0) || (party->Characters.Num() <= characterHandle)) {
		GAME_ERROR("GetCharacterByPos : characterHandle is out of range (0 <= characterHandle:%d < %d)", characterHandle, party->Characters.Num());
		return nullptr;
	}
	//check(posIndex < party->Formation.Num());
	//check(party->Formation[posIndex] < party->Characters.Num());
	return &party->Characters[characterHandle];
}

FBattleCharacterStatus* UBattleSystem::GetCharacterByPos(FBattleParty* party, int32 posIndex) const
{
	if(party == nullptr) {
		GAME_ERROR("GetCharacterByPos : party is nullptr");
		return nullptr;
	}
	if((posIndex < 0) || (party->Formation.Num() <= posIndex)) {
		GAME_ERROR("GetCharacterByPos : posIndex is out of range (0 <= posIndex:%d < %d)", posIndex, party->Formation.Num());
		return nullptr;
	}
	if((party->Formation[posIndex] < 0) || (party->Characters.Num() <= party->Formation[posIndex])) {
		GAME_ERROR("GetCharacterByPos : Invalid Formation index %d", party->Formation[posIndex]);
		return nullptr;
	}
	//check(posIndex < party->Formation.Num());
	//check(party->Formation[posIndex] < party->Characters.Num());
	return &party->Characters[party->Formation[posIndex]];
}

