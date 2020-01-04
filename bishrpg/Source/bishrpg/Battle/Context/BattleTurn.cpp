// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleTurn.h"

#include "bishrpg.h"

void FBattleTurn::Setup(EPlayerGroup myTag, EPlayerGroup firstPlayerTag)
{
	MyPlayerTag      = myTag;
	CurrentPlayerTag = firstPlayerTag;
}

bool FBattleTurn::IsManualPlayerTurn() const
{
	return (CurrentPlayerTag == MyPlayerTag);
}

void FBattleTurn::SwapTurn()
{
	CurrentPlayerTag = static_cast<EPlayerGroup>(1 - static_cast<int32>(CurrentPlayerTag));
}
