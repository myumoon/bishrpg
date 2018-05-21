// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleSystem.h"

#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"
#include "GameData/CharacterAsset.h"
#include "GameData/BishRPGDataTblAccessor.h"

#include "bishrpg.h"



// Sets default values for this component's properties
UBattleSystem::UBattleSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	CommandList.Reserve(32); // 12マスなので余裕を持ってこのくらい
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
	battleParty.Characters.AddUninitialized(FBattleParty::MAX_PARTY_NUM);

	for(int i = 0; i < party.Characters.Num(); ++i) {
		battleParty.Characters[i] = MakeBattleCharacterStatus(party.Characters[i]);
	}
	battleParty.Formation = party.Formation;

	return battleParty;
}


// 初期化
void UBattleSystem::Initialize(const FParty& playerParty, const FParty& opponentParty, int32 aaa)
{
	const FParty* partyList[] = { &playerParty, &opponentParty };

	for(auto* party : partyList) {
		FBattleParty battleParty = MakeFromParty(*party);
		PartyList.Add(battleParty);
	}
}

// 攻撃コマンド追加
bool UBattleSystem::PushAttackCommand(int32 posIndex)
{
	FBattleCommand addCommand;
	addCommand.ActionType     = EBattleActionType::Attack;
	addCommand.PosIndex       = posIndex;
	addCommand.TargetPosIndex = 0;
	CommandList.Push(addCommand);

	return CanStartCommand();
}

// スキルコマンド追加
bool UBattleSystem::PushSkillCommand(int32 posIndex, const FName& skillName)
{
	FBattleCommand addCommand;
	addCommand.ActionType      = EBattleActionType::Skill;
	addCommand.PosIndex        = posIndex;
	addCommand.TargetPosIndex  = 0;
	addCommand.SkillName       = skillName;
	CommandList.Push(addCommand);

	return CanStartCommand();
}

// 移動コマンド追加
bool UBattleSystem::PushMoveCommand(int32 posIndex, int32 moveTo)
{
	FBattleCommand addCommand;
	addCommand.ActionType     = EBattleActionType::Move;
	addCommand.PosIndex       = posIndex;
	addCommand.TargetPosIndex = moveTo;
	CommandList.Push(addCommand);

	return CanStartCommand();
}

// 巻き戻す
bool UBattleSystem::RevertCommand(FBattleCommand& revertedCommand)
{
	if(CommandList.Num() == 0) {
		return false;
	}

	revertedCommand = CommandList.Last();
	CommandList.RemoveAt(CommandList.Num() - 1);

	return true;
}

// リセット
void UBattleSystem::ResetCommand()
{
	CommandList.Reset();
}

// コマンド実行可能か
int32 UBattleSystem::CalcAlivePlayers() const
{
	int32 aliveCount = 0;

	const auto& party = PartyList[PartyIndex::Player];
	for(auto pos : party.Formation) {
		if(0 < party.Characters[pos].Hp) {
			++aliveCount;
		}
	}
	return aliveCount;
}

// コマンド実行可能か
bool UBattleSystem::CanStartCommand() const
{
	return (CalcAlivePlayers() == CommandList.Num());
}

// コマンド実行
bool UBattleSystem::ConsumeCommand(FBattleActionResult& result)
{
	if(CommandList.Num() == 0) {
		return false;
	}

	FBattleCommand execCommand = CommandList.Pop();
	result.ActionType = execCommand.ActionType;

	switch(execCommand.ActionType) {
		case EBattleActionType::Attack : {
			ExecAttack(result, execCommand);
		} break;

		case EBattleActionType::Move : {
			ExecMove(result, execCommand);
		} break;

		case EBattleActionType::Skill : {
			ExecSkill(result, execCommand);
		} break;

		default:
			break;
	}

	return true;
}

// 攻撃
void UBattleSystem::ExecAttack(FBattleActionResult& result, const FBattleCommand& command)
{
	auto& attackChar = GetCharacterByPos(GetTurnParty(), command.PosIndex);
	const int32 targetPos  = 0;
	auto& targetChar = GetCharacterByPos(GetNotTurnParty(), targetPos);
	const float attack     = attackChar.Attack;
	const float deffence   = targetChar.Deffence;

	// 攻撃-防御
	const float attackDiff = FMath::Max(attack - deffence, 0.0f);

	// 攻撃力に加算する
	const int32 rand       = FMath::RandRange(-50, 50);

	// 攻撃-防御の差が開くほどダメージが大きくなるようにする
	const float damageBase = FMath::Pow(attackDiff * 0.01f, 1.3f) * (attack + rand);

	// 最低ダメージ保証
	const float damage     = FMath::Max(damageBase, 10.0f);
}

// スキル
void UBattleSystem::ExecSkill(FBattleActionResult& result, const FBattleCommand& command)
{
	
}

// 移動
void UBattleSystem::ExecMove(FBattleActionResult& result, const FBattleCommand& command)
{

}

