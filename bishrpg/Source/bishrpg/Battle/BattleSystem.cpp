// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleSystem.h"

#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"
#include "GameData/CharacterAsset.h"
#include "GameData/BishRPGDataTblAccessor.h"

#include "bishrpg.h"

const FBattleCharacterStatus* FBattleParty::GetCharacterByPos(int32 posIndex) const
{
	check(posIndex < Formation.Num());
	int32 charIndex = Formation[posIndex];
	if(charIndex < 0) {
		return nullptr;
	}
	return &Characters[charIndex];
}

const FBattleCharacterStatus* FBattleParty::GetCharacterByHandle(int32 handle) const
{
	check(0 <= handle && handle < Characters.Num());
	if(handle < 0 || Characters.Num() <= handle) {
		return nullptr;
	}
	return &Characters[handle];
}

int32 FBattleParty::GetCharacterPosByHandle(int32 handle) const
{
	for(int32 i = 0; i < Formation.Num(); ++i) {
		if(0 <= Formation[i] && Formation[i] < Characters.Num()) {
			if(Formation[i] == handle) {
				return i;
			}
		}
	}
	return -1;
}

void FBattleParty::Move(int32 from, int32 to)
{
	if(from < 0 || Formation.Num() <= from || to < 0 || Formation.Num() <= to) {
		GAME_ERROR("FBattleParty::Move : invalid position (from:%d) (to:%d)", from, to);
	}
	//GAME_LOG("mov : Formation[%d](%d) -> Formation[%d](%d)", from, Formation[from], to, Formation[to]);
	const int32 oldTo = Formation[to];
	Formation[to]   = Formation[from];
	Formation[from] = oldTo;
}



// 初期化
void UBattleCommandQueue::Initialize(UBattleSystem* system, bool playerSide)
{
	if(system == nullptr) {
		GAME_ERROR("UBattleCommandQueue::Initialize : system is nullptr");
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

	return posIndex;
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
	}
	int32 actionCount = 0;
	for(const auto& command : CommandList) {
		if((command.ActionType == ECommandType::Attack) || (command.ActionType == ECommandType::Skill)) {
			++actionCount;
		}
	}

	return (BattleSystem->CalcAlivePlayers() == actionCount);
}

// コマンド送信
void UBattleCommandQueue::Commit()
{
	GAME_LOG("UBattleCommandQueue::Commit");
	if(BattleSystem) {
		BattleSystem->EnqueueCommands(CommandList, PlayerSide);
	}
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
void UBattleSystem::Initialize(const FParty& playerParty, const FParty& opponentParty)
{
	const FParty* partyList[] = { &playerParty, &opponentParty };

	for(auto* party : partyList) {
		FBattleParty battleParty = MakeFromParty(*party);
		PartyList.Add(battleParty);
	}
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
	mergedCommands.Reserve(FBattleParty::MAX_PARTY_NUM * 2);

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

	result.ActionType           = ConvertAction(execCommand.BattleCommand.ActionType);
	result.Actor.TargetPosIndex = GetParty(execCommand.PlayerSide)->GetCharacterPosByHandle(execCommand.BattleCommand.CharacterHandle);
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

	return true;
}

// 移動コマンド全消費
bool UBattleSystem::ConsumeMoveCommands(TArray<FBattleActionResult>& result)
{
	FBattleActionResult moveResult;
	//moveResult.ActionType = EBattleActionType::Move;

	bool moved = (0 < MergedMoveCommandList.Num());

	// 古い情報をバックアップ
	int32 oldPlayerFormation[FBattleParty::MAX_PARTY_NUM] = { 0 };
	int32 oldOpponentFormation[FBattleParty::MAX_PARTY_NUM] = { 0 };
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

		for(int partyIndex = 0; partyIndex < ARRAY_COUNT(oldFormationList); ++partyIndex) {
			for(int i = 0; i < FBattleParty::MAX_PARTY_NUM; ++i) {
				const int32* oldFormation = oldFormationList[partyIndex];
				const auto&  currentFormation = *currentFormationList[partyIndex];

				if((0 <= oldFormation[i]) && (oldFormation[i] != currentFormation[i])) {
					moveResult.ActionType = EBattleActionType::Move;
					moveResult.Actor.PlayerSide = (partyIndex == 0);
					moveResult.Actor.TargetPosIndex = i;
					currentFormation.Find(oldFormation[i], moveResult.MoveTo);
					result.Add(moveResult);
				}
			} 
		}
	}

	MergedMoveCommandList.Reset();

	return moved;
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
void UBattleSystem::ExecAttack(FBattleActionResult& result, const Command& command)
{
	auto* attackChar       = GetCharacterByHandle(GetParty(command.PlayerSide), command.BattleCommand.CharacterHandle);
	if(attackChar == nullptr) {
		return;
	}
	const int32 attackerPos = GetParty(command.PlayerSide)->GetCharacterPosByHandle(command.BattleCommand.CharacterHandle);
	const auto target       = GetAttackTargetByPos(GetParty(!command.PlayerSide), *attackChar, attackerPos, command.PlayerSide);
	auto* targetChar        = GetCharacterByPos(GetParty(!command.PlayerSide), target.TargetPosIndex);
	if(targetChar == nullptr) {
		return;
	}
	const float attack     = attackChar->Attack;
	const float deffence   = targetChar->Deffence;
	const int32 damage     = CalcDamage(attack, deffence, -50, 50, attackChar->Style, targetChar->Style, 1.3f, 10);

	FBattleTargetValue targetResult;
	targetResult.Target = target;
	targetResult.Value  = damage;
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
	const auto target = GetAttackTargetByPos(GetParty(!command.PlayerSide), *attackChar, attackerPos, command.PlayerSide);
	auto* targetChar = GetCharacterByPos(GetNotTurnParty(), target.TargetPosIndex);
	if(targetChar == nullptr) {
		return;
	}
	const float attack = attackChar->Attack;
	const float deffence = targetChar->Deffence;
	const int32 damage = CalcDamage(attack, deffence, 0, 100, attackChar->Style, targetChar->Style, 1.4f, 20);

	FBattleTargetValue targetResult;
	targetResult.Target = target;
	targetResult.Value = damage;
	result.TargetResults.Add(targetResult);
	result.SkillName = command.BattleCommand.SkillName;
}

// 移動
void UBattleSystem::ExecMove(FBattleActionResult& result, const Command& command)
{

}

// 攻撃対象選択
FBattleTarget UBattleSystem::GetAttackTargetByPos(const FBattleParty* opponentParty, const FBattleCharacterStatus& attacker, int32 attackerPos, bool playerSide) const{
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
		const int32 attackerCol = attackerPos % 3; // 縦列

		for(int32 i = 0; i < FBattleParty::MAX_PARTY_NUM; ++i) {
			const auto* character = opponentParty->GetCharacterByPos(i);
			if(character) {
				hatemap[i] = static_cast<float>(character->Hate); // キャラが持っているヘイトの絶対値
				
				//const int32 attackerPos = GetParty(playerSide)->GetCharacterPosByHandle(attackerHandle);
				//if(0 <= attackerPos) 
				{
					const float hateOffset  = HATE_ADJUST_NEAREST[attackerCol][i];
					hatemap[i] += hateOffset;
				}
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
	target.PlayerSide     = !playerSide;
	target.TargetPosIndex = maxHateIndex;

	return target;
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