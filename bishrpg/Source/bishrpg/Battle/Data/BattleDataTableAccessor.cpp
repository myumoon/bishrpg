// Copyright © 2018 nekoatsume_atsuko. All rights reserved.
#include "BattleDataTableAccessor.h"
#include "Engine/CurveTable.h"
#include "Curves/CurveBase.h"
#include "ConstructorHelpers.h"

#include "bishrpg.h"



UBattleDataTableAccessorFactory::UBattleDataTableAccessorFactory()
{

}

UBattleDataTableHolder* UBattleDataTableAccessorFactory::CreateAccessor(const FParty& playerParty, const FParty& opponentParty)
{
	auto extractId = [this](const FParty& party) {
		for(const auto& character : party.Characters) {
			if(!LoadIdList.Contains(character.Id)) {
				LoadIdList.Add(character.Id);
			}
		}
	};

	LoadIdList.Reset();
	extractId(playerParty);
	extractId(opponentParty);
	Accessor = NewObject<UBattleDataTableHolder>(this);
	return Accessor;
}
