// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BattleBoardUtil.h"

/** バトル用のマス目計算用
 * 
 */
class BattleCell {
public:
	BattleCell();
	constexpr BattleCell(int32 cellIndex) : CellIndex(cellIndex) {}
	BattleCell(int32 row, int32 col);
	BattleCell(const BattleCell& rhs);
	~BattleCell() = default;

	/*!	無効な値を取得
	*/
	static constexpr BattleCell Invalid()
	{
		return BattleCell(UBattleBoardUtil::INVALID_CELL_NO);
	}
	
	//! @brief 有効なセルかどうか
	bool IsValid() const;

	/*!	行取得
	*/
	int32 GetRow() const;

	/*!	列取得
	*/
	int32 GetCol() const;

	/*!	行取得
	*/
	int32 GetIndex() const;
	
	/*
	const BattleCell& operator=(const BattleCell& rhs)
	{
		CellIndex = rhs.CellIndex;
		return *this;
	}
	*/
	
	bool operator==(const BattleCell& rhs) const
	{
		return CellIndex == rhs.CellIndex;
	}

	bool operator!=(const BattleCell& rhs) const
	{
		return !this->operator==(rhs);
	}

	/*!	int32の配列に変換
	*/
	static void ToInt32Array(TArray<int32>& out, const TArray<BattleCell>& cells);

private:
	int32 CellIndex;
};

