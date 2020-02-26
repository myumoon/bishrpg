// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleSystem.h"

#include "bishrpg.h"
#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"
#include "GameData/CharacterAsset.h"
#include "GameData/SkillData.h"
#include "BattleCellSelector.h"
#include "ObjectHandleLibrary.h"
#include "GameData/BishRPGDataTblAccessor.h"

namespace {
	FRandomStream RandStream;
}



// Sets default values for this component's properties
UBattleSystem::UBattleSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	//MergedCommandList.Reserve(32); // 12マスなので余裕を持ってこのくらい
	
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
void UBattleSystem::GetCharacterStatusByHandle(FBattleCharacterStatus& stat, int32 index, bool playerSide) const
{
    const FBattleCharacterStatus* status = const_cast<FBattleParty*>(GetParty(playerSide))->GetCharacterByIndex(index);
    check(status != nullptr);
    if(status != nullptr) {
        stat = *status;
    }
}

// バトル用キャラ情報取得
void UBattleSystem::GetCharacterStatusByHandle2(FBattleCharacterStatus& stat, const FBattleObjectHandle& handle) const
{
	const FBattleCharacterStatus* status = GetCharacterByHandle2(handle);
    //const FBattleCharacterStatus* status = const_cast<FBattleParty*>(GetParty(playerSide))->GetCharacterByHandle2(handle);
    check(status != nullptr);
    if(status != nullptr) {
        stat = *status;
    }
}

// HPレートを取得
float UBattleSystem::GetHpRate(const FBattleObjectHandle& handle) const
{
	const FBattleCharacterStatus* status = GetCharacterByHandle2(handle);
	if(status == nullptr) {
		return 0.0f;
	}
	return (static_cast<float>(status->Hp) / status->HpMax);
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
void UBattleSystem::Initialize(const FParty& playerParty, const FParty& opponentParty, const FRandomStream& randStream, const FBattleSettings& battleSettings)
{
	const FParty* partyList[] = { &playerParty, &opponentParty };

	for(auto* party : partyList) {
		FBattleParty battleParty = MakeFromParty(*party);
		PartyList.Add(battleParty);
	}

	RandStream     = randStream;
	BattleSettings = battleSettings;
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
void UBattleSystem::EnqueueCommands(const TArray<FBattleCommand>& commandList, EPlayerGroup playerSide)
{
#if 0
	FBattleParty* insertParty   = GetParty(playerSide);
	FBattleParty* opponentParty = GetParty(InvertGroup(playerSide));
	if((insertParty == nullptr) || (opponentParty == nullptr)) {
		GAME_ERROR("EnqueueCommands : Party is null");
		return;
	}

	Command addCommand;
	int32 checkIndex  = 0;
	int32 addedCommands = 0;
	bool existCommand = false;
	EPlayerGroup prevAddSide = playerSide;

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

	auto addCommandList = [](TArray<Command>& list, const FBattleCommand& addCommand, EPlayerGroup playerSide) {
		GAME_LOG("AddCommand : pos(%d), player(%s)", addCommand.ActionPosIndex, IsPlayerOne(playerSide) ? TEXT("true") : TEXT("false"));
		Command add;
		add.BattleCommand = addCommand;
		add.PlayerSide = playerSide;
		list.Add(add);
	};

	// 速度が早い方から順に入れていく
	int32   insertIndex = 0;

	for( ; insertIndex < tempCommands.Num(); ++insertIndex) {
		// tempCommandsの余った分は外で入れる
		if(checkIndex == MergedCommandList.Num()) {
			break;
		}

		const int32 insertCharacterIndex = tempCommands[insertIndex].CharacterIndex;
		if(insertCharacterIndex < 0 || insertParty->Characters.Num() <= insertCharacterIndex) {
			continue;
		}
		
		const auto& insertChar   = insertParty->Characters[insertCharacterIndex];

		for( ; checkIndex < MergedCommandList.Num(); ) {
			const auto& actorHandle = MergedCommandList[checkIndex].BattleCommand.ActorHandle;
			if(!actorHandle.IsValid()) {
				++checkIndex;
				continue;
			}

			/*
			const int32 characterIndex = MergedCommandList[checkIndex].BattleCommand.CharacterIndex;
			if(characterIndex < 0 || opponentParty->Characters.Num() <= characterIndex) {
				++checkIndex;
				continue;
			}
			*/

			FBattleCharacterStatus status;
			GetCharacterStatusByHandle2(status, actorHandle);

			//const auto& checkChar = opponentParty->Characters[checkIndex];
			if((IsPlayerOne(prevAddSide) && status.Speed <= insertChar.Speed) || (!IsPlayerOne(prevAddSide) && status.Speed < insertChar.Speed)) {
				addCommandList(mergedCommands, tempCommands[characterIndex], playerSide);
				prevAddSide = playerSide;
				break;
			}
			else {
				addCommandList(mergedCommands, MergedCommandList[checkIndex].BattleCommand, InvertGroup(playerSide));
				prevAddSide = InvertGroup(playerSide);
				++checkIndex;
			}
		}
	}

	for(; insertIndex < tempCommands.Num(); ++insertIndex) {
		if(tempCommands[insertIndex].ActorHandle.IsValid()) {
			addCommandList(mergedCommands, tempCommands[insertIndex], playerSide);
		}
	}
	for(; checkIndex < MergedCommandList.Num(); ++checkIndex) {
		const int32 characterIndex = MergedCommandList[checkIndex].BattleCommand.CharacterIndex;
		if(0 <= characterIndex) {
			addCommandList(mergedCommands, MergedCommandList[checkIndex].BattleCommand, InvertGroup(playerSide));
		}
	}

	MergedCommandList = mergedCommands;
#endif
}

// 行動結果予測
void UBattleSystem::PredictTargetCells(int32& mainCellPos, TArray<int32>& resultCells, const FBattleCommand& command)
{
	switch(command.ActionType) {
		case ECommandType::Attack: {
		} break;

		case ECommandType::Skill: {
			BattleCell mainCell;
			TArray<BattleCell> cells;
			GetSkillTargetPositions(mainCell, cells, command);
			mainCellPos = mainCell.GetIndex();
			BattleCell::ToInt32Array(resultCells, cells);
		} break;

		case ECommandType::Move:
		case ECommandType::Swap: {
		} break;

		default:
			break;
	}
	
}


// バトル準備
void UBattleSystem::Prepare()
{
	for(auto& context : CommandContext.GroupContext) {
		context.Reset();
	}
}

void UBattleSystem::ConsumeCommand(bool& isConsumed, int32& consumedCommandCount, const UBattleCommandQueue* groupOneCommands, const UBattleCommandQueue* groupTwoCommands)
{
	GAME_LOG("Start ConsumeCommand");
	consumedCommandCount = 0;

	for(int32 groupIndex = 0; groupIndex < MaxGroupNum; ++groupIndex) {
		const EPlayerGroup group = static_cast<EPlayerGroup>(groupIndex);
		auto& groupContext = CommandContext.GroupContext[groupIndex];
		auto* commandQueue = SelectWithGroup(groupOneCommands, groupTwoCommands, group);

		for( ; groupContext.ConsumedIndex < commandQueue->GetCount(); ) {
			const auto& execCommand = commandQueue->GetCommand(groupContext.ConsumedIndex);
			++consumedCommandCount;

			switch(execCommand.ActionType) {
				case ECommandType::Attack: {
					ExecAttack(execCommand, group);
				} break;

				case ECommandType::Skill: {
					ExecSkill(execCommand, group);
				} break;

				case ECommandType::Move: 
				case ECommandType::Swap: {
					ExecMove(execCommand, group);
				} break;

				default:
					break;
			}

			++groupContext.ConsumedIndex;
			break;
		}
	}

	const bool isAllCommandDone = IsDoneTurnCommand(groupOneCommands, groupTwoCommands);
	isConsumed = !isAllCommandDone;
}

bool UBattleSystem::IsDoneTurnCommand(const UBattleCommandQueue* groupOneCommands, const UBattleCommandQueue* groupTwoCommands) const
{
	for(int32 groupIndex = 0; groupIndex < MaxGroupNum; ++groupIndex) {
		const EPlayerGroup group = static_cast<EPlayerGroup>(groupIndex);
		const auto& groupContext = CommandContext.GroupContext[groupIndex];

		const auto* command = SelectWithGroup(groupOneCommands, groupTwoCommands, group);
		const bool  isLast  = command->IsLastCommandIndex(groupContext.ConsumedIndex);
		const bool  isOver  = (BattleSettings.MaxTurnCommandNum <= groupContext.ConsumedIndex);

		if(!isLast && !isOver) {
			return false;
		}
	}

	return true;
}


// コマンド実行
#if 0
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
	//result.Actor.TargetPosIndex = GetParty(execCommand.PlayerSide)->GetCharacterPosByHandle(execCommand.BattleCommand.CharacterIndex);
	result.Actor.TargetHandle   = execCommand.BattleCommand.CharacterIndex;
	GAME_LOG("Consume handle(%d), playerSide(%s), skill(%s)", execCommand.BattleCommand.CharacterIndex, IsPlayerOne(execCommand.PlayerSide) ? TEXT("true") : TEXT("false"), *execCommand.BattleCommand.SkillName.ToString());
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
#endif

// 移動コマンド全消費
bool UBattleSystem::ConsumeMoveCommands(TArray<FBattleActionResult>& result)
{
#if 0
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
		const EPlayerGroup playerSide = MergedMoveCommandList[i].PlayerSide;
		const int32        actorPos   = MergedMoveCommandList[i].BattleCommand.ActionPosIndex;
		const int32        moveto     = MergedMoveCommandList[i].BattleCommand.TargetPosIndex;
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
					moveResult.Actor.PlayerSide = (partyIndex == 0) ? EPlayerGroup::One : EPlayerGroup::Two;
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
#else
	return false;
#endif
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
int32 UBattleSystem::CalcDamage(int32 attack, int32 deffence, int32 randMin, int32 randMax, EBattleStyle attackerStyle, EBattleStyle targetStyle, float diffAcc/*=1.3f*/, int32 minDamage, int32 maxDamage)
{
	// 乱数取得
	const int32 rand = FMath::RandRange(FMath::Min(randMin, randMax), FMath::Max(randMin, randMax));
	
	// 攻撃-防御
	const float actualDef  = static_cast<float>(deffence) * 0.5;
	const float attackDiff = FMath::Max(static_cast<float>(attack - actualDef), 1.0f);
	
	// タイプ補正
	const int32 typeDamage = static_cast<int32>(attackDiff * GetTypeDamageRate(attackerStyle, targetStyle));

	// 攻撃-防御の差が開くほどダメージが大きくなるようにする
	const float damageAdjust = FMath::Pow(typeDamage * 0.1f, diffAcc);

	// 範囲内に収める
	const float damage = FMath::Clamp(damageAdjust, static_cast<float>(minDamage), static_cast<float>(maxDamage));
	GAME_LOG_FMT("Rand:{0} AtkDiff:{1} TypeDmg:{2} Adjust:{3} Damage:{4}", rand, attackDiff, typeDamage, damageAdjust, damage);
	return damage;
}

// ダメージ補正値
float UBattleSystem::GetTypeDamageRate(EBattleStyle attackerStyle, EBattleStyle targetStyle)
{
	static const float RateWeak   = 1.5f;
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
void UBattleSystem::ExecAttack(const FBattleCommand& command, EPlayerGroup group)
{
	auto* attackChar       = GetCharacterByHandle2(command.ActorHandle);
	if(attackChar == nullptr) {
		return;
	}
	const int32 attackerPos = GetObjectPos(command.ActorHandle);
	const auto target       = GetAttackTargetByPos(GetParty(InvertGroup(group)), *attackChar, attackerPos, group);
	auto* targetChar        = GetCharacterByHandle2(target.Handle);
	if(targetChar == nullptr) {
		return;
	}
	const float attack     = attackChar->Attack;
	const float deffence   = targetChar->Deffence;
	const int32 damage     = CalcDamage(attack, deffence, -50, 50, attackChar->Style, targetChar->Style, 1.2f, BattleSettings.MinDamage, BattleSettings.MaxDamage);
	targetChar->ReceiveDamage(damage);

	FBattleAttackResult result;
	FBattleTargetValue targetResult;
	targetResult.Target = target.Handle;
	targetResult.Value  = damage;
	targetResult.Status |= static_cast<int32>(targetChar->IsDie() ? EStatusFlag::Status_Die : EStatusFlag::None);
	result.TargetResults.Add(targetResult);
	result.Actor = command.ActorHandle;
	result.TargetGroup = InvertGroup(group);
	result.AffectedPositions.Add(GetObjectPos(target.Handle));

#if defined(UE_BUILD_DEBUG)
	GAME_ASSERT(result.Actor.IsValid());
	for(const auto& checkTarget : result.TargetResults) {
		GAME_ASSERT(checkTarget.Target.IsValid());
	}
#endif
	GAME_LOG("BroadcastAttackEvent");
	AttackDelegate.Broadcast(result);
}

// スキル
void UBattleSystem::ExecSkill(const FBattleCommand& command, EPlayerGroup group)
{
	auto* attackChar = GetCharacterByHandle2(command.ActorHandle);
	if(attackChar == nullptr) {
		return;
	}
	const int32 attackerPos = GetObjectPos(command.ActorHandle);
	const auto* skillTbl    = ABishRPGDataTblAccessor::GetTbl(ETblType::SkillTbl);
	const auto* skillData   = skillTbl->FindRow<FSkillData>(command.SkillName, FString(""));
	if(skillData == nullptr) {
		GAME_ERROR("ExecSkill : Not found '%s' in the SkillTbl", *command.SkillName.ToString());
		return;
	}

	FBattleSkillResult result;
	FBattleTargetValue targetResult;
	result.SkillName = command.SkillName;
	result.AttackResult.Actor = command.ActorHandle;

	if(skillData->Type == ESkillType::Attack || skillData->Type == ESkillType::Heal) {
		TArray<FBattleTarget> targets;
		TArray<BattleCell> positions;
		BattleCell mainCell;
		positions.Reserve(UBattleBoardUtil::MAX_BOARD_CELLS);
		GetSkillTargetPositions(mainCell, positions, command.ActorHandle, skillData->Type, skillData->SelectType, skillData->SelectParam, skillData->SelectRange);
		for(const auto& cell : positions) {
			result.AttackResult.AffectedPositions.Add(cell.GetIndex());
		}
		const EPlayerGroup targetGroup = InvertGroup(command.ActorHandle.GetGroup());
		GetTargetsByCell(targets, positions, targetGroup);
		result.AttackResult.TargetGroup = targetGroup;

		for(auto& target : targets) {
			auto* targetChar = GetCharacterByHandle2(target.Handle);
			if(targetChar == nullptr) {
				GAME_ERROR("target char is null");
				return;
			}
			
			if(skillData->Type == ESkillType::Attack) {
				const float attack   = attackChar->Attack;
				const float deffence = targetChar->Deffence;
				const int32 damage   = CalcDamage(attack, deffence, 0, 100, attackChar->Style, targetChar->Style, 1.2f, BattleSettings.MinDamage, BattleSettings.MaxDamage);
				targetChar->ReceiveDamage(damage);

				FBattleTargetValue value;
				value.Target = target.Handle;
				value.Value  = damage;
				value.Status |= static_cast<int32>(targetChar->IsDie() ? EStatusFlag::Status_Die : EStatusFlag::None);

				//GAME_LOG_FMT("AddResult {0} {1}", value.Target.GetIndex(), value.Value);
				result.AttackResult.TargetResults.Add(value);
			}
			else {
				// todo : 回復
				//skillData->ValueAtLevel->GetFloatValue();
				//targetChar->Heal();
			}
		}

#ifdef UE_BUILD_DEBUG
		GAME_ASSERT(result.AttackResult.Actor.IsValid());
		for(const auto& target : result.AttackResult.TargetResults) {
			GAME_ASSERT(target.Target.IsValid());
		}
		
		GAME_LOG("===SkillResult===");
		GAME_LOG("- ActorSide   :%d", result.AttackResult.Actor.GetGroupIndex());
		GAME_LOG("- ActorIndex  :%d", result.AttackResult.Actor.GetObjectIndex());
		GAME_LOG("- Skill       :%s", *result.SkillName.ToString());
		GAME_LOG("- TargetGroup :%d", ToIndex01(result.AttackResult.TargetGroup));
		GAME_LOG("- Targers:");
		int32 index = 0;
		for(auto target : result.AttackResult.TargetResults) {
			GAME_LOG(" - [%d]TargetGroup :%d", index, target.Target.GetGroupIndex());
			GAME_LOG(" - [%d]TargetIndex :%d", index, target.Target.GetObjectIndex());
			++index;
		}
#endif
		// BPにイベント送信
		GAME_LOG("BroadcastSkillEvent");
		SkillDelegate.Broadcast(result);
	}

}

// 移動
void UBattleSystem::ExecMove(const FBattleCommand& command, EPlayerGroup group)
{
	const int32 moveFrom = GetObjectPos(command.ActorHandle);
	const int32 moveTo   = command.ActionPosIndex;
	auto*       party    = GetParty(group);

	party->Move(moveFrom, moveTo);

	FBattleMoveResult result;

	result.MoveFrom    = moveFrom;
	result.MoveTo      = moveTo;
	result.Actor       = command.ActorHandle;

	GAME_ASSERT(result.Actor.IsValid());
	MoveDelegate.Broadcast(result);

	// 移動先に誰かいたらその人の分の移動結果も生成
	if(party->GetCharacterByPos(moveTo)) {
		const auto targetHandle = MakeObjectHandle(moveTo, group);

		result.MoveFrom    = moveTo;
		result.MoveTo      = moveFrom;
		result.Actor       = targetHandle;

		GAME_ASSERT(result.Actor.IsValid());
		MoveDelegate.Broadcast(result);
	}
	
}

// 攻撃対象選択
FBattleTarget UBattleSystem::GetAttackTargetByPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos, EPlayerGroup playerSide) const
{
	BattleCellSelector cellSelector(GetParty(InvertGroup(playerSide)));
	cellSelector.SelectTarget(attackerPos, EBattleSelectMethod::E_Top1);
	const auto& selectedCells = cellSelector.GetResult();

	if(selectedCells.Num() == 0) {
		GAME_ERROR("GetAttackTargetByPos : not selected. attackerPos(%d), playerPos(%s)", attackerPos, IsPlayerOne(playerSide) ? TEXT("true") : TEXT("false"));
		return FBattleTarget();
	}

	FBattleTarget target;
	for(const auto& cell : cellSelector.GetResult()) {
		target.PlayerSide   = InvertGroup(playerSide);
		target.Handle       = MakeObjectHandle(cell, target.PlayerSide);
		break;
	}

	return target;
}

// スキル対象
void UBattleSystem::GetSkillTargetsByPos(TArray<FBattleTarget>& targets, const FBattleCharacterStatus& actor, int32 actorPos, EPlayerGroup actorSide, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam) const
{
	const EPlayerGroup targetPlayerSide = (skillType == ESkillType::Heal) ? actorSide : InvertGroup(actorSide);
	const auto*        targetParty      = GetParty(targetPlayerSide);

	BattleCellSelector cellSelector(GetParty(targetPlayerSide));
	cellSelector.SelectTarget(actorPos, selectType);
	const auto& selectedCells = cellSelector.GetResult();

	if(selectedCells.Num() == 0) {
		//GAME_ERROR("GetSkillTargetsByPos : not selected. actorPos(%d), actorSide(%s), selectType(%d), selectParam(%d)", actorPos, actorSide ? TEXT("true") : TEXT("false"), static_cast<int32>(selectType), selectParam);
		return;
	}

	FBattleTarget target;
	
	for(const auto& cell : selectedCells) {
		target.PlayerSide   = targetPlayerSide;
		target.Handle       = MakeObjectHandle(cell, targetPlayerSide);
		//target.TargetHandle = handle;
		targets.Add(target);
	}
	
}

// スキル対象
bool UBattleSystem::GetSkillTargetPositions(BattleCell& mainCell, TArray<BattleCell>& positions, const FBattleCommand& command) const
{
	auto* attackChar = GetCharacterByHandle2(command.ActorHandle);
	if(attackChar == nullptr) {
		return false;
	}
	const int32 attackerPos = GetObjectPos(command.ActorHandle);
	const auto* skillTbl = ABishRPGDataTblAccessor::GetTbl(ETblType::SkillTbl);
	const auto* skillData = skillTbl->FindRow<FSkillData>(command.SkillName, FString(""));
	if(skillData == nullptr) {
		GAME_ERROR("ExecSkill : Not found '%s' in the SkillTbl", *command.SkillName.ToString());
		return false;
	}

	return GetSkillTargetPositions(mainCell, positions, command.ActorHandle, skillData->Type, skillData->SelectType, skillData->SelectParam, skillData->SelectRange);
}

// スキル対象
bool UBattleSystem::GetSkillTargetPositions(BattleCell& mainCell, TArray<BattleCell>& positions, const FBattleObjectHandle& actor, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam, EBattleSelectRange expandMethod) const
{
	const EPlayerGroup  targetPlayerSide = (skillType == ESkillType::Heal) ? actor.GetGroup() : InvertGroup(actor.GetGroup());
	const FBattleParty* targetParty      = GetParty(targetPlayerSide);
	const BattleCell    actorCell        = GetObjectCell(actor);
	BattleCellSelector cellSelector(targetParty);
	cellSelector.SelectTarget(actorCell, selectType);
	if(cellSelector.GetResult().Num() <= 0) {
		return false;
	}
	mainCell = cellSelector.GetResult()[0];
	cellSelector.ExpandCell(expandMethod);
	const auto& selectedCells = cellSelector.GetResult();

	if(selectedCells.Num() == 0) {
		//GAME_ERROR("GetSkillTargetsByPos : not selected. actorPos(%d), actorSide(%s), selectType(%d), selectParam(%d)", actorPos, actorSide ? TEXT("true") : TEXT("false"), static_cast<int32>(selectType), selectParam);
		return false;
	}

	positions.Reset();
	positions.Reserve(selectedCells.Num());
	for(const auto& cell : selectedCells) {
		positions.Add(cell);
	}

	return true;
}

// ターゲット取得
void UBattleSystem::GetTargetsByCell(TArray<FBattleTarget>& targets, const TArray<BattleCell>& cells, EPlayerGroup group) const
{
	targets.Reset();
	targets.Reserve(UBattleBoardUtil::CELL_NUM);

	const FBattleParty* party = GetParty(group);
	FBattleTarget addTarget;
	for(const auto& cell : cells) {
		if(party->ExistsPos(cell.GetIndex())) {
			addTarget.Handle = MakeObjectHandle(cell, group, EObjectType::Character);
			targets.Add(addTarget);
		}
	}

}



FBattleCharacterStatus* UBattleSystem::GetCharacterByHandle(FBattleParty* party, int32 CharacterIndex) const
{
	if(party == nullptr) {
		GAME_ERROR("GetCharacterByPos : party is nullptr");
		return nullptr;
	}
	if((CharacterIndex < 0) || (party->Characters.Num() <= CharacterIndex)) {
		GAME_ERROR("GetCharacterByPos : CharacterIndex is out of range (0 <= CharacterIndex:%d < %d)", CharacterIndex, party->Characters.Num());
		return nullptr;
	}
	//check(posIndex < party->Formation.Num());
	//check(party->Formation[posIndex] < party->Characters.Num());
	return &party->Characters[CharacterIndex];
}

const FBattleCharacterStatus* UBattleSystem::GetCharacterByHandle2(const FBattleObjectHandle& handle) const
{
	auto* party = GetParty(handle.IsManualPlayer());
	if(party == nullptr) {
		GAME_ERROR("GetCharacterByPos : party is nullptr");
		return nullptr;
	}
	int32 index = handle.GetObjectIndex();
	if((index < 0) || (party->Characters.Num() <= index)) {
		GAME_ERROR("GetCharacterByPos : index is out of range (0 <= index:%d < %d)", index, party->Characters.Num());
		return nullptr;
	}
	//check(posIndex < party->Formation.Num());
	//check(party->Formation[posIndex] < party->Characters.Num());
	return &party->Characters[index];
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

FBattleObjectHandle UBattleSystem::MakeObjectHandle(int32 posIndex, EPlayerGroup side, EObjectType type) const
{
	const int32 charIndex = GetCharacterIndex(posIndex, side);
	FObjectHandleDesc desc = {charIndex, type, side };
	return UObjectHandleLibrary::MakeObjectHandle(desc);
}

FBattleObjectHandle UBattleSystem::MakeObjectHandle(const BattleCell& cell, EPlayerGroup side, EObjectType type) const
{
	return MakeObjectHandle(cell.GetIndex(), side, type);
}

int32 UBattleSystem::GetObjectPos(const FBattleObjectHandle& handle) const
{
	if(!handle.IsValid()) {
		return -1;
	}
	const auto* party  = GetParty(handle.GetGroup());
	const int32 pos    = party->GetCharacterPosByIndex(handle.GetObjectIndex());
	
	return pos;
}

BattleCell UBattleSystem::GetObjectCell(const FBattleObjectHandle& handle) const
{
	if(!handle.IsValid()) {
		return BattleCell();
	}
	const auto*      party = GetParty(handle.GetGroup());
	const BattleCell cell  = party->GetCharacterCellByIndex(handle.GetObjectIndex());

	return cell;
}



const FRandomStream& UBattleSystem::GetRandStream()
{
	return RandStream;
}


// デバッグ用
// ハンドルリスト取得
void UBattleSystem::DebugGetHandleList(TArray<FBattleObjectHandle>& handles, EPlayerGroup group) const
{
	const auto* party = GetParty(group);
	for(int32 pos : party->Formation) {
		FBattleObjectHandle handle = MakeObjectHandle(BattleCell(pos), group);
		handles.Add(handle);
	}
	
}

// デバッグ用
// ハンドルリスト取得
void UBattleSystem::DebugCallBattleEvent()
{
	GAME_LOG("DebugCallBattleEvent");

	auto makeDebugCommand = [this](EPlayerGroup group) {
		FBattleCommand command;
		const auto* party = GetParty(group);
		BattleCell  cell(party->GetCharacterPosByIndex(0));
		command.ActorHandle = MakeObjectHandle(cell, group);
		command.ActionType  = ECommandType::Skill;
		command.SkillName   = "Shout";
		return command;
	};

	for(int32 groupIndex = 0; groupIndex < MaxGroupNum; ++groupIndex) {
		EPlayerGroup group = static_cast<EPlayerGroup>(groupIndex);
		auto command = makeDebugCommand(group);
		
		switch(command.ActionType) {
			case ECommandType::Attack: {
				ExecAttack(command, group);
			} break;

			case ECommandType::Skill: {
				ExecSkill(command, group);
			} break;

			case ECommandType::Move:
			case ECommandType::Swap: {
				ExecMove(command, group);
			} break;

			default:
				break;
		}
	}

}
