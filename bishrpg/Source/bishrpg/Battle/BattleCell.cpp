// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleCell.h"
#include "BattleBoardUtil.h"


BattleCell::BattleCell() : CellIndex(UBattleBoardUtil::INVALID_CELL_NO)
{

}

BattleCell::BattleCell(int32 row, int32 col) : CellIndex(UBattleBoardUtil::GetBoardCol() * row + col)
{
	check(row < UBattleBoardUtil::GetBoardRow());
	check(col < UBattleBoardUtil::GetBoardCol());
}

BattleCell::BattleCell(const BattleCell& rhs) : CellIndex(rhs.CellIndex)
{
}


bool BattleCell::IsValid() const
{
	return (0 <= CellIndex) && (CellIndex < UBattleBoardUtil::CELL_NUM);
}

int32 BattleCell::GetRow() const
{
	return UBattleBoardUtil::GetRow(CellIndex);
}

/*!	列取得
*/
int32 BattleCell::GetCol() const
{
	return UBattleBoardUtil::GetCol(CellIndex);
}

/*!	行取得
*/
int32 BattleCell::GetIndex() const
{
	return CellIndex;
}
	
