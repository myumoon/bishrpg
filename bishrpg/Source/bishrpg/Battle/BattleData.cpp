// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleData.h"

#include "bishrpg.h"
#include "BattleSystem.h"

namespace {
	struct SortCompDistance {
		int32 basePos = 0;

		SortCompDistance(int32 actorPos) : basePos(actorPos)
		{
		}

		bool operator()(int32 lhs, int32 rhs) const
		{
			if(UBattleBoardUtil::GetRow(lhs) > UBattleBoardUtil::GetRow(rhs)) {
				return true;
			}

			int32 distL = FMath::Abs(UBattleBoardUtil::GetCol(basePos) - UBattleBoardUtil::GetCol(lhs));
			int32 distR = FMath::Abs(UBattleBoardUtil::GetCol(basePos) - UBattleBoardUtil::GetCol(rhs));
			return (distL < distR);
		}
	};

}


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

int32 FBattleParty::GetCharacterHandleByPos(int32 pos, bool silent) const
{
	if(pos < 0 || Formation.Num() <= pos) {
		if(!silent) {
			GAME_ERROR("GetCharacterHandleByPos : out of range 0 <= %d < %d", pos, Formation.Num());
		}
		return -1;
	}

	return Formation[pos];
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

#if 0
void FBattleParty::Select(TArray<int32>& selectedHandles, int32 actorPos, EBattleSelectMethod pattern, int32 param, const FRandomStream& randStream, bool clearResult) const
{
	SortCompDistance sortCompDist(actorPos);
	auto allFilter = [](int32) { return true; };

	TArray<int32> positions;
	positions.Reserve(UBattleBoardUtil::MAX_BOARD_CELLS);

	auto selectTop = [&](int32 index) {
		Filter(selectedHandles, [&](int32 pos) { return FilterExistsAll(pos); }, sortCompDist);
		int32 handle = 0;
		if(index < selectedHandles.Num()) {
			handle = selectedHandles[index];
		}
		else {
			handle = selectedHandles[selectedHandles.Num() - 1];
		}
		selectedHandles.Reset();
		selectedHandles.Add(handle);
	};

	

	switch(pattern) {
	case EBattleSelectMethod::E_Top1 : {
		selectTop(0);
		//SelectTop(selectedHandles, actorPos, 0, clearResult);
	} break;

	case EBattleSelectMethod::E_Top2 : {
		selectTop(1);
		//SelectTop(selectedHandles, actorPos, 1, clearResult);
	} break;

	case EBattleSelectMethod::E_Top3 : {
		selectTop(2);
		//SelectTop(selectedHandles, actorPos, 2, clearResult);
	} break;

	case EBattleSelectMethod::E_Top4 : {
		selectTop(3);
		//SelectTop(selectedHandles, actorPos, 3, clearResult);
	} break;

	case EBattleSelectMethod::E_Top5 : {
		selectTop(4);
		//SelectTop(selectedHandles, actorPos, 4, clearResult);
	} break; 

	case EBattleSelectMethod::E_Top6 : {
		selectTop(5);
		//SelectTop(selectedHandles, actorPos, 5, clearResult);
	} break;

	case EBattleSelectMethod::E_Ahead1 : {
		//SelectAhead1(selectedHandles, actorPos, clearResult);
		//Filter(selectedHandles, [](int32 pos) { return pos == actorPos },
	} break;

	case EBattleSelectMethod::E_Ahead4 : {
		//SelectAhead4(selectedHandles, actorPos, clearResult);
	} break;

	case EBattleSelectMethod::E_AllCells : {
		SelectAll(selectedHandles, clearResult);
	} break;

	case EBattleSelectMethod::E_Random1 : {
		SelectRandom(selectedHandles, param, randStream, clearResult);
	} break;

	default: {
		GAME_ERROR("Select not implemented %s", *GETENUMSTRING("EBattleSelectPattern", pattern));
	} break;
	}
}

void FBattleParty::PrepareSelecting(TArray<int32>& selectedPositions, bool clearResult) const
{
	if(clearResult) {
		selectedPositions.Reset();
	}

}

void FBattleParty::SelectTop(TArray<int32>& selectedHandles, int32 actorPos, int32 index, bool clearResult) const
{
	TArray<int32> posList;
	PrepareSelecting(posList, clearResult);
	if(actorPos < 0) {
		GAME_ERROR("SelectTop");
		return;
	}
	const int32 actorCol = actorPos % UBattleBoardUtil::GetBoardCol();

	TArray<int32> row;
	for(int32 pos = UBattleBoardUtil::GetCellNum() - 1; 0 < pos; pos -= 3) {
		const int32 faced     = pos - actorCol; // 向かい合っているマス
		const auto* facedChar = GetCharacterByPos(faced);
		if(facedChar) {
			posList.Add(faced);
			break;
		}

		const int32 right = pos - 0; // ボードの右側（向かって左）
		const int32 left  = pos - 2; // ボードの左側（向かって右）
		const auto* rightChar = GetCharacterByPos(right);
		const auto* leftChar  = GetCharacterByPos(left);

		// 中央のときは両サイドを見る
		if(actorCol == UBattleBoardUtil::GetColCenter()) {
			// 両隣にキャラがいるならHPが低い方を選択
			if(rightChar && leftChar) {
				if(rightChar->Hp < leftChar->Hp) {
					posList.Add(right);
					break;
				}
				else if(leftChar->Hp < rightChar->Hp) {
					posList.Add(left);
					break;
				}
				// HPが同じ時は左を選ぶ
				else {
					posList.Add(left);
					break;
				}
			}
			else if(rightChar) {
				posList.Add(right);
				break;
			}
			else if(leftChar) {
				posList.Add(left);
				break;
			}
		}
		// 左右どっちかに寄っているので真ん中を調べる
		else {
			const int32 center = pos - 1;
			const auto* centerChar = GetCharacterByPos(center);
			if(centerChar) {
				posList.Add(center);
				break;
			}
			else if (rightChar) {
				posList.Add(right);
				break;
			}
			else if(leftChar) {
				posList.Add(left);
				break;
			}
		}
	}
	MakeCharacterListByPositionList(selectedHandles, posList);
}
void FBattleParty::SelectCol(TArray<int32>& selectedHandles, int32 col, bool clearResult) const
{
	PrepareSelecting(selectedHandles, clearResult);

	TArray<int32> positions;
	UBattleBoardUtil::MakePositionListCol(positions, col, false);

	MakeCharacterListByPositionList(selectedHandles, positions);
}

void FBattleParty::SelectRow(TArray<int32>& selectedHandles, int32 row, bool clearResult) const
{
	if(UBattleBoardUtil::GetBoardRow() <= row) {
		GAME_ERROR("invalid row %d", row);
		return;
	}

	PrepareSelecting(selectedHandles, clearResult);

	TArray<int32> positions;
	UBattleBoardUtil::MakePositionListRow(positions, row);
	MakeCharacterListByPositionList(selectedHandles, positions);
}
void FBattleParty::SelectAhead1(TArray<int32>& selectedHandles, int32 actorPos, bool clearResult) const
{
	PrepareSelecting(selectedHandles, clearResult);

	TArray<int32> positions;
	const int32 facedCol = UBattleBoardUtil::GetFacedCol(actorPos);
	const int32 row      = UBattleBoardUtil::GetRow(actorPos);
	if((0 <= facedCol) && (0 <= row)) {
		// 最前列
		if(row == UBattleBoardUtil::GetBoardRow() - 1) {
			const int32 selectedPos = row * UBattleBoardUtil::GetBoardRow() + facedCol;
			positions.Add(selectedPos);

			MakeCharacterListByPositionList(selectedHandles, positions);
		}
	}

}
void FBattleParty::SelectAhead4(TArray<int32>& selectedHandles, int32 actorPos, bool clearResult) const
{
	PrepareSelecting(selectedHandles, clearResult);

	TArray<int32> positions;
	const int32 facedCol = UBattleBoardUtil::GetFacedCol(actorPos);
	const int32 row      = UBattleBoardUtil::GetRowInverse(actorPos);
	if((0 <= facedCol) && (0 <= row)) {
		const int32 selectedPos = UBattleBoardUtil::GetPos(row, facedCol);
		positions.Add(selectedPos);

		MakeCharacterListByPositionList(selectedHandles, positions);
	}

}
void FBattleParty::SelectAll(TArray<int32>& selectedHandles, bool clearResult) const
{
	PrepareSelecting(selectedHandles, clearResult);

	TArray<int32> positions;
	for(int32 i = 0; i < UBattleBoardUtil::GetCellNum(); ++i) {
		positions.Add(i);
	}
	MakeCharacterListByPositionList(selectedHandles, positions);

}
void FBattleParty::SelectRandom(TArray<int32>& selectedHandles, int selectPosNum, const FRandomStream& randStream, bool clearResult) const
{
	PrepareSelecting(selectedHandles, clearResult);

	TArray<int32> selectedPositions;
	selectedPositions.Reserve(selectPosNum);

	TArray<int32> tempPosList;
	SelectAll(tempPosList, false);

	for(int i = 0; i < selectPosNum; ++i) {
		const int32 selectedIdx = randStream.RandRange(0, tempPosList.Num() - 1);
		selectedPositions.Add(tempPosList[selectedIdx]);
		tempPosList.RemoveAt(selectedIdx, 1, false);
	}
	MakeCharacterListByPositionList(selectedHandles, selectedPositions);

}


void FBattleParty::Filter(TArray<int32>& selectedPositions, std::function<bool(int32)> posFilter, std::function<bool(int32, int32)> posComp) const
{
	//PrepareSelecting(selectedHandles, clearResult);

	selectedPositions.Reserve(Formation.Num());

	for(int i = 0; i < Formation.Num(); ++i) {
		if(posFilter(i)) {
			selectedPositions.Add(i);
		}
	}

	if(posComp != nullptr) {
		selectedPositions.Sort(posComp);
	}

}

void FBattleParty::MakeCharacterListByPositionList(TArray<int32>& characterHandles, const TArray<int32>& selectedPositions) const
{
	for(int32 pos : selectedPositions) {
		const int32 charHandle = GetCharacterHandleByPos(pos, true);
		if(0 <= charHandle) {
			characterHandles.Add(charHandle);
		}
	}
}

bool FBattleParty::FilterExistsAll(int32 pos) const
{
	check(0 <= pos && pos < UBattleBoardUtil::MAX_BOARD_CELLS);
	return (0 <= Formation[pos]);
}

// 位置追加
void FBattleParty::AddPos(TArray<int32>& expandedPos, int32 posIndex) const
{
	if(!expandedPos.Contains(posIndex)) {
		expandedPos.Add(posIndex);
	}
}


// ----- セル選択 -----
void FBattleParty::SelectTarget(TArray<int32>& selectedPos, int32 actorPos, EBattleSelectMethod selectMethod) const
{
	SelectFunc selectFuncTbl[]  = {
		&FBattleParty::SelectTop1,
		&FBattleParty::SelectTop2,
		&FBattleParty::SelectTop3,
		&FBattleParty::SelectTop4,
		&FBattleParty::SelectTop5,
		&FBattleParty::SelectTop6,

		&FBattleParty::SelectFacedTop1,
		&FBattleParty::SelectAhead1,
		&FBattleParty::SelectAhead4,

		&FBattleParty::SelectAttackTop1,
		&FBattleParty::SelectDeffenceTop1,

		&FBattleParty::SelectRockTop1,
		&FBattleParty::SelectRockBack1,
		&FBattleParty::SelectSingTop1,
		&FBattleParty::SelectSingBack1,
		&FBattleParty::SelectHurmorTop1,
		&FBattleParty::SelectHurmorBack1,

		&FBattleParty::SelectCell_0_Faced,
		&FBattleParty::SelectCell_0_Right,
		&FBattleParty::SelectCell_0_Center,
		&FBattleParty::SelectCell_0_Left,
		&FBattleParty::SelectCell_1_Right,
		&FBattleParty::SelectCell_1_Center,
		&FBattleParty::SelectCell_1_Left,
		&FBattleParty::SelectCell_2_Right,
		&FBattleParty::SelectCell_2_Center,
		&FBattleParty::SelectCell_2_Left,
		&FBattleParty::SelectCell_3_Right,
		&FBattleParty::SelectCell_3_Center,
		&FBattleParty::SelectCell_3_Left,

		&FBattleParty::SelectMyself_P,
		&FBattleParty::SelectFront1_P,
		&FBattleParty::SelectTop1_P,
		&FBattleParty::SelectBack1_P,
		&FBattleParty::SelectAll_P,
	};

	const int32 funcIndex = (int)selectMethod;
	check(0 <= funcIndex && selectMethod < EBattleSelectMethod::Max);
	(this->*selectFuncTbl[funcIndex])(expandedPos, actorPos);
}

BattleCell FBattleParty::FetchTop(int32 index) const
{
	SortCompDistance sortCompDist(index);
	auto allFilter = [](int32) { return true; };
	TArray<int32> selectedHandles;
	
	auto selectTop = [&](int32 index) -> int32 {
		Filter(selectedHandles, [&](int32 pos) { return FilterExistsAll(pos); }, sortCompDist);
		int32 handle = 0;
		if(index < selectedHandles.Num()) {
			handle = selectedHandles[index];
		}
		else {
			handle = selectedHandles[selectedHandles.Num() - 1];
		}
		for(int32 i = 0; i < Formation.Num(); ++i) {
			if(handle == Formation[i]) {
				return i;
			}
		}
		check(false);
		return -1;
	};

	const int32 pos = selectTop(index);
	return BattleCell(pos);
}


void FBattleParty::SelectTop1(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectTop2(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectTop3(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectTop4(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectTop5(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectTop6(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectFacedTop1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectAhead1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectAhead4(TArray<int32>& selectedPos, int32 actorPos) const
{

}

void FBattleParty::SelectAttackTop1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectDeffenceTop1(TArray<int32>& selectedPos, int32 actorPos) const
{

}

void FBattleParty::SelectRockTop1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectRockBack1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectSingTop1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectSingBack1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectHurmorTop1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectHurmorBack1(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectCell_0_Faced(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectCell_0_Right(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_0_Center(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_0_Left(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_1_Right(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_1_Center(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_1_Left(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_2_Right(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_2_Center(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_2_Left(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_3_Right(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_3_Center(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectCell_3_Left(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}

void FBattleParty::SelectAllCells(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}

void FBattleParty::SelectRandom1(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom2(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom3(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom4(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom5(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom6(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom7(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom8(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom9(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom10(TArray<int32>& selectedPos, int32 actorPos) const
{
	
}
void FBattleParty::SelectRandom11(TArray<int32>& selectedPos, int32 actorPos) const
{

}
void FBattleParty::SelectRandom12(TArray<int32>& selectedPos, int32 actorPos) const
{

}
#endif

#if 0
// ----- セル拡張 -----

void FBattleParty::ExpandCell(TArray<int32>& expandedPos, const TArray<int32>& basePos, EBattleSelectRange range) const
{
	RangeFunc rangeFuncTbl[]  = {
		&FBattleParty::ExpandRangeSingle,
		&FBattleParty::ExpandRangeCol,
		&FBattleParty::ExpandRangeRow,
		&FBattleParty::ExpandRangeSide,
		&FBattleParty::ExpandRangeFrontBack,
		&FBattleParty::ExpandRangeAroundPlus4,
		&FBattleParty::ExpandRangeAroundCross4,
		&FBattleParty::ExpandRangeAround9,
		&FBattleParty::ExpandRangeBack1,
		&FBattleParty::ExpandRangeBack2,
		&FBattleParty::ExpandRangeBack3,
		&FBattleParty::ExpandRangeBack4,
	};

	const int32 rangeFuncIndex = (int)range;
	check(0 <= rangeFuncIndex && range < EBattleSelectRange::Max);
	(this->*rangeFuncTbl[rangeFuncIndex])(expandedPos, basePos);

}

// 範囲選択
void FBattleParty::SelectRangeSingle(TArray<int32>& expandedPos, int32 basePos) const
{
	// そのまま返す
	AddPos(expandedPos, basePos); 
}

// 縦方向選択
void FBattleParty::SelectRangeCol(TArray<int32>& expandedPos, int32 basePos) const
{
	const int32 col = UBattleBoardUtil::GetCol(basePos);
	int32 base = col;
	AddPos(expandedPos, base);

	for(int32 i = 0; i < UBattleBoardUtil::GetBoardRow() - 1; ++i) {
		const int32 nextPos = UBattleBoardUtil::GetPosForward(base);
		if(base != nextPos) {
			AddPos(expandedPos, nextPos);
			base = nextPos;
		}
	}
}

// 横方向選択
void FBattleParty::SelectRangeRow(TArray<int32>& expandedPos, int32 basePos) const
{
	const int32 row = UBattleBoardUtil::GetRow(basePos);
	int32 base = row;
	AddPos(expandedPos, base);

	for(int32 i = 0; i < UBattleBoardUtil::GetBoardCol() - 1; ++i) {
		const int32 nextPos = UBattleBoardUtil::GetPosRight(base);
		if(base != nextPos) {
			AddPos(expandedPos, nextPos);
			base = nextPos;
		}
	}
}

// 左右選択
void FBattleParty::SelectRangeSide(TArray<int32>& expandedPos, int32 basePos) const
{
	const int32 left  = UBattleBoardUtil::GetPosLeft(basePos); 
	const int32 right = UBattleBoardUtil::GetPosRight(basePos);
	
	AddPos(expandedPos, basePos);

	if(left != basePos) {
		AddPos(expandedPos, left);
	}
	if(right != basePos) {
		AddPos(expandedPos, right);
	}

}

// 上下選択
void FBattleParty::SelectRangeFrontBack(TArray<int32>& expandedPos, int32 basePos) const
{
	const int32 forward = UBattleBoardUtil::GetPosForward(basePos); 
	const int32 back    = UBattleBoardUtil::GetPosBack(basePos);
	
	AddPos(expandedPos, basePos);

	if(forward != basePos) {
		AddPos(expandedPos, forward);
	}
	if(back != basePos) {
		AddPos(expandedPos, back);
	}

}

// 上下左右選択
void FBattleParty::SelectRangeAroundPlus4(TArray<int32>& expandedPos, int32 basePos) const
{
	SelectRangeSide(expandedPos, basePos);
	SelectRangeFrontBack(expandedPos, basePos);
}


// 斜め4方向選択
void FBattleParty::SelectRangeAroundCross4(TArray<int32>& expandedPos, int32 basePos) const
{
	auto addPosOf = [&](bool left, bool forward) {
		const int32 sidePos = left ? UBattleBoardUtil::GetPosLeft(basePos) : UBattleBoardUtil::GetPosRight(basePos);
		if(sidePos == basePos) {
			return;
		}
		const int32 finalPos = left ? UBattleBoardUtil::GetPosLeft(sidePos) : UBattleBoardUtil::GetPosRight(sidePos);
		if(finalPos == sidePos) {
			return;
		}

		AddPos(expandedPos, finalPos);
	};

	addPosOf(true, true);
	addPosOf(true, false);
	addPosOf(false, true);
	addPosOf(false, false);
}


// 周囲選択
void FBattleParty::SelectRangeAround9(TArray<int32>& expandedPos, int32 basePos) const
{
	SelectRangeAroundPlus4(expandedPos, basePos);
	SelectRangeAroundCross4(expandedPos, basePos);
}

#if 0
// 後ろ選択
void FBattleParty::SelectRangeBack1(TArray<int32>& expandedPos, int32 basePos) const
{
	const int32 back = UBattleBoardUtil::GetPosBack(basePos[0]);
	if(back != basePos[0]) {
		AddPos(expandedPos, back);
	}

}


// 後ろ選択
void FBattleParty::SelectRangeBack2(TArray<int32>& expandedPos, int32 basePos) const
{
	int32 base = basePos;
	int32 back = UBattleBoardUtil::GetPosBack(base);
	if(back == base) {
		return;
	}

	AddPos(expandedPos, back);
	base = back;

	back = UBattleBoardUtil::GetPosBack(base);
	if(back == base) {
		return;
	}

	AddPos(expandedPos, back);

}
#endif

// 後ろ選択
void FBattleParty::SelectRangeBack(TArray<int32>& expandedPos, int32 basePos, int32 count) const
{
	int32 base = basePos;
	int32 back = 0;
	
	for(int32 i = 0; i < count; ++i) {
		back = UBattleBoardUtil::GetPosBack(base);
		if(back == base) {
			return;
		}

		AddPos(expandedPos, back);
		base = back;
	}

}




//----- 複数選択 -----


void FBattleParty::ExpandRangeSingle(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeSingle(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeCol(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeCol(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeRow(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeRow(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeSide(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeSide(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeFrontBack(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeFrontBack(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeAroundPlus4(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeAroundPlus4(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeAroundCross4(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeAroundCross4(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeAround9(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeAround9(expandedPos, base);
	}
	
}

void FBattleParty::ExpandRangeBack1(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeBack(expandedPos, base, 1);
	}
	
}

void FBattleParty::ExpandRangeBack2(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeBack(expandedPos, base, 2);
	}
	
}

void FBattleParty::ExpandRangeBack3(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeBack(expandedPos, base, 3);
	}
	
}

void FBattleParty::ExpandRangeBack4(TArray<int32>& expandedPos, const TArray<int32>& basePos) const
{
	for(const auto& base : basePos) {
		SelectRangeBack(expandedPos, base,4);
	}
	
}
#endif
