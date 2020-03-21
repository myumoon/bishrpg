// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleCellSelector.h"

#include "BattleData.h"
#include "BattleBoardUtil.h"
#include "BattleBoardDef.h"
#include "BattleSystem.h"
#include "bishrpg.h"

namespace {
	class FilterBase {
	public:
		FilterBase(const FBattleParty* party) : TargetParty(party)
		{
		}
		virtual ~FilterBase() = default;

		bool operator()(int32 pos) const
		{
			if(!TargetParty) {
				return false;
			}

			if(!UBattleBoardUtil::IsValidCellNo(pos)) {
				return false;
			}
			if(!TargetParty->ExistsPos(pos)) {
				return false;
			}

			return FilterCell(BattleCell(pos), *TargetParty);
		}

	protected:
		virtual bool FilterCell(const BattleCell& cell, const FBattleParty& party) const = 0;

	private:
		const FBattleParty* TargetParty = nullptr;
	};

	// 全セルをフィルタリング
	class FilterAll : public FilterBase {
		using FilterBase::FilterBase;
	protected:
		bool FilterCell(const BattleCell& cell, const FBattleParty& party) const override
		{
			return true;
		}
	};

	// 生きているオブジェクトをフィルタリング
	class FilterAlive : public FilterBase {
		using FilterBase::FilterBase;
	protected:
		bool FilterCell(const BattleCell& cell, const FBattleParty& party) const override
		{
			return party.GetCharacterByCell(cell)->IsAlive();
		}
	};

	struct SortCompNearDistance {
		BattleCell BasePos = 0;

		SortCompNearDistance(int32 actorPos) : BasePos(actorPos)
		{
		}

		bool operator()(const BattleCell& lhs, const BattleCell& rhs) const
		{
			if(rhs.GetRow() < lhs.GetRow()) {
				return true;
			}
			
			const int32 facedColL = UBattleBoardUtil::GetFacedCol(lhs.GetIndex());
			const int32 facedColR = UBattleBoardUtil::GetFacedCol(rhs.GetIndex());
			const int32 distL     = FMath::Abs(UBattleBoardUtil::GetCol(BasePos.GetIndex()) - facedColL);
			const int32 distR     = FMath::Abs(UBattleBoardUtil::GetCol(BasePos.GetIndex()) - facedColR);
			return (distL < distR);
		}
	};

	struct SortCompFarDistance {
		SortCompNearDistance NearDistComp;

		SortCompFarDistance(int32 actorPos) : NearDistComp(actorPos)
		{
		}

		bool operator()(const BattleCell lhs, const BattleCell rhs) const
		{
			return NearDistComp(lhs, rhs);
		}
	};

	struct SortCompContainer {
		std::function<bool(BattleCell, BattleCell)> Comp;
		SortCompContainer(std::function<bool(BattleCell, BattleCell)> comp) {
			Comp = comp;
		}

		bool operator()(const BattleCell lhs, const BattleCell rhs) const
		{
			return Comp(lhs, rhs);
		}
	};
}

/*
BattleCellSelector::BattleCellSelector(const FBattleSystem* battleSystem)
{
	Initialize(battleSystem);
}
*/

BattleCellSelector::BattleCellSelector(const FBattleParty* party)
{
    Initialize(party);
}

void BattleCellSelector::Initialize(const FBattleParty* party)
{
	//SelectedCells.Init(false, UBattleBoardUtil::CELL_NUM);
	ResultCells.Reserve(Battle::Def::MAX_BOARD_CELLS);
	SelectedParty = party;
}

// 位置追加
void BattleCellSelector::AddPos(const BattleCell& posIndex)
{
	if(posIndex.IsValid()) {
		if(!ResultCells.Contains(posIndex)) {
			ResultCells.Add(posIndex);
		}
	}
}

// ----- セル選択 -----

void BattleCellSelector::SelectTarget(BattleCell actorPos, EBattleSelectMethod selectMethod)
{
	SelectFunc selectMethodFunc[] = {
		&BattleCellSelector::SelectDummy,
		&BattleCellSelector::SelectTop1,
		&BattleCellSelector::SelectTop2,
		&BattleCellSelector::SelectTop3,
		&BattleCellSelector::SelectTop4,
		&BattleCellSelector::SelectTop5,
		&BattleCellSelector::SelectTop6,
		&BattleCellSelector::SelectFacedTop1,
		&BattleCellSelector::SelectAhead1,
		&BattleCellSelector::SelectAhead4,
		&BattleCellSelector::SelectAttackTop1,
		&BattleCellSelector::SelectDeffenceTop1,
		&BattleCellSelector::SelectRockTop1,
		&BattleCellSelector::SelectRockBack1,
		&BattleCellSelector::SelectSingTop1,
		&BattleCellSelector::SelectSingBack1,
		&BattleCellSelector::SelectHurmorTop1,
		&BattleCellSelector::SelectHurmorBack1,
		&BattleCellSelector::SelectCell_0_Faced,
		&BattleCellSelector::SelectCell_0_Right,
		&BattleCellSelector::SelectCell_0_Center,
		&BattleCellSelector::SelectCell_0_Left,
		&BattleCellSelector::SelectCell_1_Right,
		&BattleCellSelector::SelectCell_1_Center,
		&BattleCellSelector::SelectCell_1_Left,
		&BattleCellSelector::SelectCell_2_Right,
		&BattleCellSelector::SelectCell_2_Center,
		&BattleCellSelector::SelectCell_2_Left,
		&BattleCellSelector::SelectCell_3_Right,
		&BattleCellSelector::SelectCell_3_Center,
		&BattleCellSelector::SelectCell_3_Left,
		&BattleCellSelector::SelectAllCells,
		&BattleCellSelector::SelectRandom1,
		&BattleCellSelector::SelectRandom2,
		&BattleCellSelector::SelectRandom3,
		&BattleCellSelector::SelectRandom4,
		&BattleCellSelector::SelectRandom5,
		&BattleCellSelector::SelectRandom6,
		&BattleCellSelector::SelectRandom7,
		&BattleCellSelector::SelectRandom8,
		&BattleCellSelector::SelectRandom9,
		&BattleCellSelector::SelectRandom10,
		&BattleCellSelector::SelectRandom11,
		&BattleCellSelector::SelectRandom12,
		&BattleCellSelector::SelectMyself_P,
		&BattleCellSelector::SelectFront1_P,
		&BattleCellSelector::SelectTop1_P,
		&BattleCellSelector::SelectBack1_P,
		&BattleCellSelector::SelectAll_P,
	};
	static_assert(ARRAY_COUNT(selectMethodFunc) == static_cast<int32>(EBattleSelectMethod::Max), "Invalid array size : selectMethodFunc");
	const int32 selectMethodIndex = static_cast<int32>(selectMethod);
	(this->*selectMethodFunc[selectMethodIndex])(actorPos.GetIndex());

	// 選択済みセルとして記憶して拡張時に使用する
	SelectedCells = ResultCells;
}

void BattleCellSelector::SortResult(std::function<bool(BattleCell, BattleCell)> posComp)
{
	if(posComp != nullptr) {
		ResultCells.Sort(SortCompContainer(posComp));

		/*
		SelectedCells.Init(false, Battle:Def::MAX_BOARD_CELLS);
		for(const auto& cell : ResultCells) {
			if(cell.IsValid()) {
				SelectedCells[cell.GetIndex()] = true;
			}
		}
		*/
	}
}

void BattleCellSelector::FilterResult(std::function<bool(int32)> posFilter)
{
	for(int i = 0; i < Battle::Def::MAX_BOARD_CELLS; ++i) {
		if(posFilter(i)) {
			AddPos(BattleCell(i));
		}
	}
}

void BattleCellSelector::ShurinkResultTo(int32 size)
{
	if((0 <= size) && (size < ResultCells.Num())) {
		ResultCells.SetNum(size, false);
	}
}

void BattleCellSelector::SelectTop(int32 actorPos, int32 index)
{
	FilterResult(FilterAlive(SelectedParty));
	SortResult(SortCompNearDistance(actorPos));

	const int32 selectedIndex = FMath::Clamp(index, 0, ResultCells.Num() - 1);
	check(0 <= selectedIndex && selectedIndex < ResultCells.Num());
	const BattleCell selectedCell = ResultCells[selectedIndex];
	ResultCells.SetNum(1, false);
	ResultCells[0] = selectedCell;
}

void BattleCellSelector::SelectTop1(int32 actorPos)
{
	SelectTop(actorPos, 0);
}
void BattleCellSelector::SelectTop2(int32 actorPos)
{
	SelectTop(actorPos, 1);
}
void BattleCellSelector::SelectTop3(int32 actorPos)
{
	SelectTop(actorPos, 2);
}
void BattleCellSelector::SelectTop4(int32 actorPos)
{
	SelectTop(actorPos, 3);
}
void BattleCellSelector::SelectTop5(int32 actorPos)
{
	SelectTop(actorPos, 4);
}
void BattleCellSelector::SelectTop6(int32 actorPos)
{
	SelectTop(actorPos, 5);
}
void BattleCellSelector::SelectFacedTop1(int32 actorPos)
{
	FilterResult([=](int32 pos) { return UBattleBoardUtil::GetCol(pos) == UBattleBoardUtil::GetFacedCol(actorPos);});
	SortResult(SortCompNearDistance(actorPos));
	ShurinkResultTo(1);
}
void BattleCellSelector::SelectAhead1(int32 actorPos)
{
	FilterResult([=](int32 pos) { return UBattleBoardUtil::GetCol(pos) == UBattleBoardUtil::GetFacedCol(actorPos);});
	SortResult(SortCompNearDistance(actorPos));
	ShurinkResultTo(1);
}
void BattleCellSelector::SelectAhead4(int32 actorPos)
{
	FilterResult([=](int32 pos) { return UBattleBoardUtil::GetCol(pos) == UBattleBoardUtil::GetFacedCol(actorPos);});
	SortResult(SortCompFarDistance(actorPos));
	ShurinkResultTo(1);
}

void BattleCellSelector::SelectAttackTop1(int32 actorPos)
{
	FilterResult(FilterAlive(SelectedParty));
	SortResult([&](BattleCell lhs, BattleCell rhs) {
		const auto* charL = SelectedParty->GetCharacterByPos(lhs.GetIndex());
		const auto* charR = SelectedParty->GetCharacterByPos(rhs.GetIndex());
		return (charL->Attack < charR->Attack);
	});
	ShurinkResultTo(1);
}
void BattleCellSelector::SelectDeffenceTop1(int32 actorPos)
{
	FilterResult(FilterAlive(SelectedParty));
	SortResult([&](BattleCell lhs, BattleCell rhs) {
		const auto* charL = SelectedParty->GetCharacterByPos(lhs.GetIndex());
		const auto* charR = SelectedParty->GetCharacterByPos(rhs.GetIndex());
		return (charL->Deffence < charR->Deffence);
	});
	ShurinkResultTo(1);
}
void BattleCellSelector::SelectType(EBattleStyle style, int32 actorPos, bool top)
{
	FilterResult([=](int32 pos) {
		return (SelectedParty->GetCharacterByPos(pos)->Style == style);
	});
	if(top) {
		SortResult(SortCompNearDistance(actorPos));
	}
	else {
		SortResult(SortCompFarDistance(actorPos));
	}
	ShurinkResultTo(1);
}

void BattleCellSelector::SelectRockTop1(int32 actorPos)
{
	SelectType(EBattleStyle::Rock, actorPos, true);
}

void BattleCellSelector::SelectRockBack1(int32 actorPos)
{
	SelectType(EBattleStyle::Rock, actorPos, false);
}

void BattleCellSelector::SelectSingTop1(int32 actorPos)
{
	SelectType(EBattleStyle::Sing, actorPos, true);
}

void BattleCellSelector::SelectSingBack1(int32 actorPos)
{
	SelectType(EBattleStyle::Sing, actorPos, false);
}

void BattleCellSelector::SelectHurmorTop1(int32 actorPos)
{
	SelectType(EBattleStyle::Humor, actorPos, true);
}

void BattleCellSelector::SelectHurmorBack1(int32 actorPos)
{
	SelectType(EBattleStyle::Humor, actorPos, false);
}

void BattleCellSelector::SelectCellFaced(int32 actorPos, int32 row)
{
	FilterResult([=](int32 pos) { 
		bool valid = (UBattleBoardUtil::GetCol(pos) == UBattleBoardUtil::GetFacedCol(actorPos));
		valid &= (UBattleBoardUtil::GetRow(pos) == row);
		return valid;
	});
}
void BattleCellSelector::SelectCell(int32 row, int32 col)
{
	ResultCells.Reset();
	ResultCells.Add(BattleCell(row, col));
}

void BattleCellSelector::SelectCell_0_Faced(int32 actorPos)
{
	SelectCellFaced(actorPos, 0);
}
void BattleCellSelector::SelectCell_0_Right(int32 actorPos)
{
	SelectCell(0, 2);
}
void BattleCellSelector::SelectCell_0_Center(int32 actorPos)
{
	SelectCell(0, 1);
}
void BattleCellSelector::SelectCell_0_Left(int32 actorPos)
{
	SelectCell(0, 0);
}
void BattleCellSelector::SelectCell_1_Right(int32 actorPos)
{
	SelectCell(1, 2);
}
void BattleCellSelector::SelectCell_1_Center(int32 actorPos)
{
	SelectCell(1, 1);
}
void BattleCellSelector::SelectCell_1_Left(int32 actorPos)
{
	SelectCell(1, 0);
}
void BattleCellSelector::SelectCell_2_Right(int32 actorPos)
{
	SelectCell(2, 2);
}
void BattleCellSelector::SelectCell_2_Center(int32 actorPos)
{
	SelectCell(2, 1);
}
void BattleCellSelector::SelectCell_2_Left(int32 actorPos)
{
	SelectCell(2, 0);
}
void BattleCellSelector::SelectCell_3_Right(int32 actorPos)
{
	SelectCell(3, 2);
}
void BattleCellSelector::SelectCell_3_Center(int32 actorPos)
{
	SelectCell(3, 1);
}
void BattleCellSelector::SelectCell_3_Left(int32 actorPos)
{
	SelectCell(3, 0);
}

void BattleCellSelector::SelectAllCells(int32 actorPos)
{
	for(int32 i = 0; i < Battle::Def::MAX_BOARD_CELLS; ++i) {
		ResultCells.Add(BattleCell(i));
	}
}

void BattleCellSelector::SelectRandom(int32 selectNum)
{
	TArray<int32> cellList = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	for(int32 i = 0; i < selectNum && 0 < cellList.Num(); ++i) {
		const int32 selectedIdx  = UBattleSystem::GetRandStream().RandHelper(cellList.Num() - 1);
		const int32 selectedCell = cellList[selectedIdx];
		cellList.RemoveAt(selectedIdx);
		AddPos(BattleCell(selectedCell));
	}
}
void BattleCellSelector::SelectRandom1(int32 actorPos)
{
	SelectRandom(1);
}
void BattleCellSelector::SelectRandom2(int32 actorPos)
{
	SelectRandom(2);
}
void BattleCellSelector::SelectRandom3(int32 actorPos)
{
	SelectRandom(3);
}
void BattleCellSelector::SelectRandom4(int32 actorPos)
{
	SelectRandom(4);
}
void BattleCellSelector::SelectRandom5(int32 actorPos)
{
	SelectRandom(5);
}
void BattleCellSelector::SelectRandom6(int32 actorPos)
{
	SelectRandom(6);
}
void BattleCellSelector::SelectRandom7(int32 actorPos)
{
	SelectRandom(7);
}
void BattleCellSelector::SelectRandom8(int32 actorPos)
{
	SelectRandom(8);
}
void BattleCellSelector::SelectRandom9(int32 actorPos)
{
	SelectRandom(9);
}
void BattleCellSelector::SelectRandom10(int32 actorPos)
{
	SelectRandom(10);
}
void BattleCellSelector::SelectRandom11(int32 actorPos)
{
	SelectRandom(11);
}
void BattleCellSelector::SelectRandom12(int32 actorPos)
{
	SelectRandom(12);
}

void BattleCellSelector::SelectMyself_P(int32 actorPos)
{
	GAME_ERROR("未実装 : SelectMyself_P");
}

void BattleCellSelector::SelectFront1_P(int32 actorPos)
{
	GAME_ERROR("未実装 : SelectFront1_P");
}

void BattleCellSelector::SelectTop1_P(int32 actorPos)
{
	GAME_ERROR("未実装 : SelectTop1_P");
}

void BattleCellSelector::SelectBack1_P(int32 actorPos)
{
	GAME_ERROR("未実装 : SelectBack1_P");
}

void BattleCellSelector::SelectAll_P(int32 actorPos)
{
	GAME_ERROR("未実装 : SelectAll_P");
}


// ----- セル拡張 -----

void BattleCellSelector::ExpandCell(EBattleSelectRange range)
{
	RangeFunc rangeFuncTbl[]  = {
		&BattleCellSelector::ExpandRangeSingle,
		&BattleCellSelector::ExpandRangeCol,
		&BattleCellSelector::ExpandRangeRow,
		&BattleCellSelector::ExpandRangeSide,
		&BattleCellSelector::ExpandRangeFrontBack,
		&BattleCellSelector::ExpandRangeAroundPlus4,
		&BattleCellSelector::ExpandRangeAroundCross4,
		&BattleCellSelector::ExpandRangeAround9,
		&BattleCellSelector::ExpandRangeBack1,
		&BattleCellSelector::ExpandRangeBack2,
		&BattleCellSelector::ExpandRangeBack3,
		&BattleCellSelector::ExpandRangeBack4,
	};
	static_assert(ARRAY_COUNT(rangeFuncTbl) == static_cast<int32>(EBattleSelectRange::Max), "Invalid array size : rangeFuncTbl");

	const int32 rangeFuncIndex = (int)range;
	check(0 <= rangeFuncIndex && range < EBattleSelectRange::Max);
	(this->*rangeFuncTbl[rangeFuncIndex])();

}

// 範囲選択
void BattleCellSelector::ExpandRangeSingle_Based(const BattleCell& basePos)
{
	// そのまま返す
	AddPos(basePos); 
}

// 縦方向選択
void BattleCellSelector::ExpandRangeCol_Based(const BattleCell& basePos)
{
	const int32 col = UBattleBoardUtil::GetCol(basePos.GetIndex());
	//int32 base = col;
	//AddPos(BattleCell(base));

	for(int32 i = 0; i < UBattleBoardUtil::GetBoardRow(); ++i) {
		AddPos(BattleCell(i, col));
		//const int32 nextPos = UBattleBoardUtil::GetPosForward(base);
		//if(base != nextPos) {
		//	AddPos(BattleCell(nextPos));
		//	base = nextPos;
		//}
	}
}

// 横方向選択
void BattleCellSelector::ExpandRangeRow_Based(const BattleCell& basePos)
{
	const int32 row = UBattleBoardUtil::GetRow(basePos.GetIndex());
	//int32 base = row;
	//AddPos(BattleCell(base));

	for(int32 i = 0; i < UBattleBoardUtil::GetBoardCol(); ++i) {
		AddPos(BattleCell(row, i));
		//const int32 nextPos = UBattleBoardUtil::GetPosRight(base);
		//if(base != nextPos) {
		//	AddPos(nextPos);
		//	base = nextPos;
		//}
	}
}

// 左右選択
void BattleCellSelector::ExpandRangeSide_Based(const BattleCell& basePos)
{
	AddPos(BattleCell(basePos.GetRow(), basePos.GetCol() + 1));
	AddPos(BattleCell(basePos.GetRow(), basePos.GetCol() - 1));
#if 0
	const int32 left  = UBattleBoardUtil::GetPosLeft(basePos.GetIndex()); 
	const int32 right = UBattleBoardUtil::GetPosRight(basePos.GetIndex());
	
	//AddPos(basePos);

	if(left != basePos.GetIndex()) {
		AddPos(left);
	}
	if(right != basePos.GetIndex()) {
		AddPos(right);
	}
#endif

}

// 上下選択
void BattleCellSelector::ExpandRangeFrontBack_Based(const BattleCell& basePos)
{
	AddPos(BattleCell(basePos.GetRow() + 1, basePos.GetCol()));
	AddPos(BattleCell(basePos.GetRow() - 1, basePos.GetCol()));
#if 0
	const int32 forward = UBattleBoardUtil::GetPosForward(basePos.GetIndex()); 
	const int32 back    = UBattleBoardUtil::GetPosBack(basePos.GetIndex());
	
	//AddPos(basePos);

	if(forward != basePos.GetIndex()) {
		AddPos(forward);
	}
	if(back != basePos.GetIndex()) {
		AddPos(back);
	}
#endif
}

// 上下左右選択
void BattleCellSelector::ExpandRangeAroundPlus4_Based(const BattleCell& basePos)
{
	ExpandRangeSide_Based(basePos);
	ExpandRangeFrontBack_Based(basePos);
}


// 斜め4方向選択
void BattleCellSelector::ExpandRangeAroundCross4_Based(const BattleCell& basePos)
{
	AddPos(BattleCell(basePos.GetRow() + 1, basePos.GetCol() + 1));
	AddPos(BattleCell(basePos.GetRow() + 1, basePos.GetCol() - 1));
	AddPos(BattleCell(basePos.GetRow() - 1, basePos.GetCol() + 1));
	AddPos(BattleCell(basePos.GetRow() - 1, basePos.GetCol() - 1));
#if 0
	auto addPosOf = [&](bool left, bool forward) {
		const int32 sidePos = left ? UBattleBoardUtil::GetPosLeft(basePos.GetIndex()) : UBattleBoardUtil::GetPosRight(basePos.GetIndex());
		if(sidePos == basePos.GetIndex()) {
			return;
		}
		const int32 finalPos = left ? UBattleBoardUtil::GetPosLeft(sidePos) : UBattleBoardUtil::GetPosRight(sidePos);
		if(finalPos == sidePos) {
			return;
		}

		AddPos(finalPos);
	};

	addPosOf(true, true);
	addPosOf(true, false);
	addPosOf(false, true);
	addPosOf(false, false);
#endif
}


// 周囲選択
void BattleCellSelector::ExpandRangeAround9_Based(const BattleCell& basePos)
{
	ExpandRangeAroundPlus4_Based(basePos);
	ExpandRangeAroundCross4_Based(basePos);
}


// 後ろ選択
void BattleCellSelector::ExpandRangeBack_Based(const BattleCell& basePos, int32 count)
{
	int32 base = basePos.GetIndex();
	int32 back = 0;
	
	for(int32 i = 0; i < count; ++i) {
		back = UBattleBoardUtil::GetPosBack(base);
		if(back == base) {
			return;
		}

		AddPos(back);
		base = back;
	}

}




//----- 複数選択 -----

void BattleCellSelector::ForeachSelectedCells(TFunction<void(const BattleCell&)> expandFunc)
{
	TArray<BattleCell> selectedCells = SelectedCells;
	for(const auto& cell : selectedCells) {
		expandFunc(cell);
	}
}

void BattleCellSelector::ExpandRangeSingle()
{
	ForeachSelectedCells([this](const BattleCell& cell) { 
		ExpandRangeSingle_Based(cell); 
	});
}

void BattleCellSelector::ExpandRangeCol()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeCol_Based(cell);
	});	
}

void BattleCellSelector::ExpandRangeRow()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeRow_Based(cell);
	});	
}

void BattleCellSelector::ExpandRangeSide()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeSide_Based(cell);
	});	
}

void BattleCellSelector::ExpandRangeFrontBack()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeFrontBack_Based(cell);
	});
}

void BattleCellSelector::ExpandRangeAroundPlus4()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeAroundPlus4_Based(cell);
	});
}

void BattleCellSelector::ExpandRangeAroundCross4()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeAroundCross4_Based(cell);
	});
}

void BattleCellSelector::ExpandRangeAround9()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeAround9_Based(cell);
	});
}

void BattleCellSelector::ExpandRangeBack1()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeBack_Based(cell, 1);
	});
}

void BattleCellSelector::ExpandRangeBack2()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeBack_Based(cell, 2);
	});
}

void BattleCellSelector::ExpandRangeBack3()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeBack_Based(cell, 3);
	});
}

void BattleCellSelector::ExpandRangeBack4()
{
	ForeachSelectedCells([this](const BattleCell& cell) {
		ExpandRangeBack_Based(cell, 4);
	});
}
