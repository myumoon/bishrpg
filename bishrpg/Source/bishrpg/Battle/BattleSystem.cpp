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


const FBattleCharacterStatus* FBattleParty::GetCharacterByPos(int32 posIndex) const
{
	check(posIndex < Formation.Num());
	int32 charIndex = Formation[posIndex];
	if(charIndex < 0) {
		return nullptr;
	}
	return &Characters[charIndex];
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
void UBattleSystem::Initialize(const FParty& playerParty, const FParty& opponentParty)
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
		else {
			GAME_ERROR("CalcAlivePlayers : Formation -1");
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
		//GAME_ERROR("ConsumeCommand : Empty CommandList");
		return false;
	}

	FBattleCommand execCommand = CommandList.Pop();
	result.ActionType = execCommand.ActionType;
	result.Actor      = execCommand.PosIndex;
	
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

// ダメージ基礎式
int32 UBattleSystem::CalcDamage(int32 attack, int32 deffence, int32 randMin, int32 randMax, float diffAcc/*=1.3f*/, int32 minDamage/*=10*/)
{
	// 攻撃-防御
	const float attackDiff = FMath::Max(static_cast<float>(attack - deffence), 0.0f);

	// 攻撃力に加算する
	const int32 rand = FMath::RandRange(FMath::Min(randMin, randMax), FMath::Max(randMin, randMax));

	// 攻撃-防御の差が開くほどダメージが大きくなるようにする
	const float damageBase = FMath::Pow(attackDiff * 0.01f, diffAcc) * (attack + rand);

	// 最低ダメージ保証
	const int32 damage = FMath::Max(FMath::FloorToInt(damageBase), minDamage);

	return damage;
}

// 攻撃
void UBattleSystem::ExecAttack(FBattleActionResult& result, const FBattleCommand& command)
{
	auto* attackChar       = GetCharacterByPos(GetTurnParty(), command.PosIndex);
	if(attackChar == nullptr) {
		return;
	}
	const auto target      = GetAttackTargetPos(GetNotTurnParty(), *attackChar, command.PosIndex);
	auto* targetChar       = GetCharacterByPos(GetNotTurnParty(), target.Target);
	if(targetChar == nullptr) {
		return;
	}
	const float attack     = attackChar->Attack;
	const float deffence   = targetChar->Deffence;
	const int32 damage     = CalcDamage(attack, deffence, -50, 50, 1.3f, 10);

	FBattleTargetValue targetResult;
	targetResult.Target = target;
	targetResult.Value  = damage;
	result.TargetResults.Add(targetResult);
}

// スキル
void UBattleSystem::ExecSkill(FBattleActionResult& result, const FBattleCommand& command)
{
	auto* attackChar = GetCharacterByPos(GetTurnParty(), command.PosIndex);
	if(attackChar == nullptr) {
		return;
	}
	const auto target = GetAttackTargetPos(GetNotTurnParty(), *attackChar, command.PosIndex);
	auto* targetChar = GetCharacterByPos(GetNotTurnParty(), target.Target);
	if(targetChar == nullptr) {
		return;
	}
	const float attack = attackChar->Attack;
	const float deffence = targetChar->Deffence;
	const int32 damage = CalcDamage(attack, deffence, -50, 50, 1.3f, 10);

	FBattleTargetValue targetResult;
	targetResult.Target = target;
	targetResult.Value = damage;
	result.TargetResults.Add(targetResult);
	result.SkillName = command.SkillName;
}

// 移動
void UBattleSystem::ExecMove(FBattleActionResult& result, const FBattleCommand& command)
{

}

// 攻撃対象選択
FBattleTarget UBattleSystem::GetAttackTargetPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos) const
{
	float hatemap[FBattleParty::MAX_PARTY_NUM] = { 0 }; // ヘイト

	const float HATE_ADJUST_NEAREST[3][FBattleParty::MAX_PARTY_NUM] = {
		// 左側
		{
			10.0f, 9.0f, 8.0f,
			 8.0f, 7.0f, 5.0f,
			 6.0f, 5.0f, 3.0f,
			 4.0f, 3.0f, 1.0f,
		},

		// 真ん中
		{
			9.0f, 10.0f, 9.0f,
			7.0f,  8.0f, 7.0f,
			5.0f,  6.0f, 5.0f,
			3.0f,  4.0f, 3.0f,
		},

		// 右側
		{
			8.0f, 9.0f, 10.0f,
			5.0f, 7.0f,  8.0f,
			3.0f, 5.0f,  6.0f,
			1.0f, 3.0f,  4.0f,
		},
	};

	const float HATE_ADJUST_STRAIGHT[3][FBattleParty::MAX_PARTY_NUM] = {
		// 左側
		{
			12.0f, 7.0f, 2.0f,
			11.5f, 6.5f, 1.5f,
			11.0f, 6.0f, 1.0f,
			10.5f, 5.5f, 0.5f,
		},

		// 真ん中
		{
			7.0f, 12.0f, 7.0f,
			6.5f, 11.5f, 6.5f,
			6.0f, 11.0f, 6.0f,
			5.5f, 10.5f, 5.5f,
		},

		// 右側
		{
			2.0f, 7.0f, 12.0f,
			1.5f, 6.5f, 11.5f,
			1.0f, 6.0f, 11.0f,
			0.5f, 5.5f, 10.5f,
		},
	};

	if((0 <= attackerPos) && (attackerPos <= FBattleParty::MAX_PARTY_NUM)) {

		for(int32 i = 0; i < FBattleParty::MAX_PARTY_NUM; ++i) {
			const auto* character = opponentParty->GetCharacterByPos(i);
			if(character) {
				hatemap[i] = static_cast<float>(character->Hate); // キャラが持っているヘイトの絶対値
				const int32 attackerCol = i % 3; // 縦列
				const float hateOffset  = HATE_ADJUST_NEAREST[attackerCol][attackerPos];
				hatemap[i] += hateOffset;
			}
		}
	}

	float maxHate      = hatemap[0];
	int32 maxHateIndex = 0;
	for(int i = 1; i < FBattleParty::MAX_PARTY_NUM; ++i) {
		if(maxHate < hatemap[i]) {
			maxHate      = hatemap[i];
			maxHateIndex = i;
		}
	}

	FBattleTarget target;
	target.PlayerSide = false;			// todo:trueかfaiseか設定
	target.Target     = maxHateIndex;

	return target;
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