// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleBoardUtil.h"
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

void UBattleBoardUtil::MakeTargetPositions(TArray<int32>& outPositions, EBattleSelectRange selectRange, int32 basePos)
{
}

//void UBattleBoardUtil::SelectTargetPositions(TArray<int32>& outPositions, const FBattleParty* playerParty, int32 actorPos, EBattleSelectMethod selectMethod, EBattleSelectRange selectRange, const FBattleParty* opponentParty)
//{
//}
