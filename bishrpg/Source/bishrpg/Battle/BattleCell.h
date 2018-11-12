// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BattleBoardUtil.h"

/** バトル用のマス目計算用
 * 
 */
struct BattleCell {
private:
	static const int32 INVALID_CELL = -1;
public:
	BattleCell();
	BattleCell(int32 cellIndex);
	BattleCell(int32 row, int32 col);
	~BattleCell();

	//! @brief 有効なセルかどうか
	bool IsValid() const
	{
		return (0 <= CellIndex) && (CellIndex < UBattleBoardUtil::CELL_NUM);
	}

private:
	int32 CellIndex = INVALID_CELL;
};
