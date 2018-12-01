// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BattleBoardUtil.h"

/** バトル用のマス目計算用
 * 
 */
class BattleCell {
private:
public:
	BattleCell();
	BattleCell(int32 cellIndex);
	BattleCell(int32 row, int32 col);
	BattleCell(const BattleCell& rhs);
	~BattleCell();

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
	
	const BattleCell& operator=(const BattleCell& rhs)
	{
		CellIndex = rhs.CellIndex;
		return *this;
	}
	
	bool operator==(const BattleCell& rhs) const
	{
		return CellIndex == rhs.CellIndex;
	}

	bool operator!=(const BattleCell& rhs) const
	{
		return !this->operator==(rhs);
	}
private:
	int32 CellIndex;
};
