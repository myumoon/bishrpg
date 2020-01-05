// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleSystem.h"

#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"
#include "GameData/CharacterAsset.h"
#include "GameData/SkillData.h"
#include "BattleCellSelector.h"
#include "ObjectHandleLibrary.h"
#include "GameData/BishRPGDataTblAccessor.h"

#include "bishrpg.h"

namespace {
	FRandomStream RandStream;
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

	RandStream = randStream;
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
			const int32 characterIndex = MergedCommandList[checkIndex].BattleCommand.CharacterIndex;
			if(characterIndex < 0 || opponentParty->Characters.Num() <= characterIndex) {
				++checkIndex;
				continue;
			}

			const auto& checkChar = opponentParty->Characters[checkIndex];
			if((IsPlayerOne(prevAddSide) && checkChar.Speed <= insertChar.Speed) || (!IsPlayerOne(prevAddSide) && checkChar.Speed < insertChar.Speed)) {
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
		const int32 characterIndex = tempCommands[insertIndex].CharacterIndex;
		if(0 <= characterIndex) {
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
	auto* attackChar       = GetCharacterByHandle(GetParty(command.PlayerSide), command.BattleCommand.CharacterIndex);
	if(attackChar == nullptr) {
		return;
	}
	const int32 attackerPos = GetParty(command.PlayerSide)->GetCharacterPosByIndex(command.BattleCommand.CharacterIndex);
	const auto target       = GetAttackTargetByPos(GetParty(InvertGroup(command.PlayerSide)), *attackChar, attackerPos, command.PlayerSide);
	auto* targetChar        = GetCharacterByHandle(GetParty(InvertGroup(command.PlayerSide)), target.TargetHandle);
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
	auto* attackChar = GetCharacterByHandle(GetParty(command.PlayerSide), command.BattleCommand.CharacterIndex);
	if(attackChar == nullptr) {
		return;
	}
	const int32 attackerPos = GetParty(command.PlayerSide)->GetCharacterPosByIndex(command.BattleCommand.CharacterIndex);
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
FBattleTarget UBattleSystem::GetAttackTargetByPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos, EPlayerGroup playerSide) const
{
	TArray<int32> selectedTarget;
	// todo:BattleCellSelectorに差し替え
	//opponentParty->SelectTop(selectedTarget, 0, attackerPos);
	BattleCellSelector cellSelector(GetParty(InvertGroup(playerSide)));
	cellSelector.SelectTarget(attackerPos, EBattleSelectMethod::E_Top1);
	if(selectedTarget.Num() == 0) {
		GAME_ERROR("GetAttackTargetByPos : not selected. attackerPos(%d), playerPos(%s)", attackerPos, IsPlayerOne(playerSide) ? TEXT("true") : TEXT("false"));
		return FBattleTarget();
	}

	FBattleTarget target;
	target.PlayerSide   = InvertGroup(playerSide);
	target.TargetHandle = selectedTarget[0];

	return target;
}

// スキル対象
void UBattleSystem::GetSkillTargetsByPos(TArray<FBattleTarget>& targets, const FBattleCharacterStatus& actor, int32 actorPos, EPlayerGroup actorSide, ESkillType skillType, EBattleSelectMethod selectType, int32 selectParam) const
{
	const EPlayerGroup targetPlayerSide = (skillType == ESkillType::Heal) ? actorSide : InvertGroup(actorSide);
	const auto*        targetParty      = GetParty(targetPlayerSide);

	TArray<int32> selectedTarget;
	//targetParty->Select(selectedTarget, actorPos, selectType, selectParam, *RandStream);
	// todo:選択処理を入れる
	BattleCellSelector cellSelector(GetParty(targetPlayerSide));
	//cellSelector.SelectTarget();

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

FBattleObjectHandle UBattleSystem::GetObjectHandle(int32 posIndex, EPlayerGroup side) const
{
	const int32 index = GetCharacterIndex(posIndex, side);
	FObjectHandleDesc desc = {};
	if(0 <= index) {
		desc.Index       = posIndex;
		// todo : オブジェクトの種類を入れる
		//desc.ObjectType  = GetParty(side)->Characters[index].Style;
		desc.PlayerGroup = side;
	}
	return UObjectHandleLibrary::MakeObjectHandle(desc);
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


const FRandomStream& UBattleSystem::GetRandStream()
{
	return RandStream;
}

