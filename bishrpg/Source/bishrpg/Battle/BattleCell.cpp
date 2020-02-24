// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleCell.h"
#include "BattleBoardUtil.h"


BattleCell::BattleCell() : CellIndex(UBattleBoardUtil::INVALID_CELL_NO)
{

}

BattleCell::BattleCell(int32 cellIndex) : CellIndex(cellIndex)
{
	if(!IsValid()) {
		CellIndex = UBattleBoardUtil::INVALID_CELL_NO;
	}
}

BattleCell::BattleCell(int32 row, int32 col) : CellIndex(UBattleBoardUtil::GetBoardCol() * row + col)
{
	const bool isRowValid = (0 <= row) && (row < UBattleBoardUtil::GetBoardRow());
	const bool isColValid = (0 <= col) && (col < UBattleBoardUtil::GetBoardCol());
	if(!isRowValid || !isColValid) {
		CellIndex = UBattleBoardUtil::INVALID_CELL_NO;
	}
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
	
/*!	int32配列に変換
*/
void BattleCell::ToInt32Array(TArray<int32>& out, const TArray<BattleCell>& cells)
{
	out.SetNum(0);
	out.Reserve(cells.Num());
	for(const auto& cell : cells) {
		out.Add(cell.GetIndex());
	}
}
