// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleAIComponent.h"
#include "BattleSystem.h"
#include "BattleCommandQueue.h"
#include "bishrpg.h"

UBattleAIComponent::UBattleAIComponent()
{
}

UBattleAIComponent::~UBattleAIComponent()
{
}

void UBattleAIComponent::Setup(UBattleCommandQueue* commandQueue, int32 lv, int32 randSeed, bool playerSide)
{
	BattleSystem = Cast<UBattleSystem>(GetOwner()->GetComponentByClass(UBattleSystem::StaticClass()));
	if(BattleSystem == nullptr) {
		GAME_ERROR("UBattleSystem is not attached.");
		return;
	}

	CommandQueue = commandQueue;
	if(CommandQueue == nullptr) {
		GAME_ERROR("CommandQueue is null.");
		return;
	}

	Lv = lv;
	RandomGenerator.Initialize(randSeed);
}

void UBattleAIComponent::Execute()
{
	if(BattleSystem == nullptr) {
		GAME_ERROR("UBattleAIComponent::Execute() : UBattleSystem is null.");
		return;
	}
	if(CommandQueue == nullptr) {
		GAME_ERROR("UBattleAIComponent::Execute() : CommandQueue is null.");
		return;
	}

	for(int32 i = 0; i < GetMyParty()->Formation.Num(); ++i) {
		const int32 handle = GetMyParty()->Formation[i];
		if(0 <= handle) {
			CommandQueue->PushAttackCommand(i);
		}
	}
	
}

const FBattleParty* UBattleAIComponent::GetMyParty() const
{
	return BattleSystem->GetParty(PlayerSide);
}

const FBattleParty* UBattleAIComponent::GetOpponentParty() const
{
	return BattleSystem->GetParty(!PlayerSide);
}

const FBattleCharacterStatus* UBattleAIComponent::GetMyCharacterStatusByPos(int32 pos) const
{
	return GetMyParty()->GetCharacterByPos(pos);
}

const FBattleCharacterStatus* UBattleAIComponent::GetOpponentCharacterStatusByPos(int32 pos) const
{
	return GetOpponentParty()->GetCharacterByPos(pos);
}




