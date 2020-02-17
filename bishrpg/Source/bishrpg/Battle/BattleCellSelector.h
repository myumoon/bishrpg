// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "BattleDataType.h"
#include "System/CharacterStatus.h"
#include "BattleCell.h"
//#include "BattleCellSelector.generated.h"

struct FBattleParty;

class BattleCellSelector {
public:
	using CellList   = TArray<BattleCell>;

private:
	using SelectFunc = void (BattleCellSelector::*)(int32);
	using RangeFunc  = void (BattleCellSelector::*)();
	
public:
	//BattleCellSelector();
    BattleCellSelector(const FBattleParty* party);
    //BattleCellSelector(const TArray<BattleCell>& cells);
	
	void FilterResult(std::function<bool(int32)> posFilter);
	void SortResult(std::function<bool(BattleCell, BattleCell)> posComp);

	void AddPos(int32 pos);
	//@{

    //! 選択
	void SelectTarget(BattleCell actorPos, EBattleSelectMethod selectMethod);

	//! 敵選択
	void SelectTop(int32 actorPos, int32 index);
	void SelectTop1(int32 actorPos);
	void SelectTop2(int32 actorPos);
	void SelectTop3(int32 actorPos);
	void SelectTop4(int32 actorPos);
	void SelectTop5(int32 actorPos);
	void SelectTop6(int32 actorPos);

	void SelectFacedTop1(int32 actorPos);
	void SelectAhead1(int32 actorPos);
	void SelectAhead4(int32 actorPos);

	void SelectAttackTop1(int32 actorPos);
	void SelectDeffenceTop1(int32 actorPos);

	void SelectType(EBattleStyle style, int32 actorPos, bool top);
	void SelectRockTop1(int32 actorPos);
	void SelectRockBack1(int32 actorPos);
	void SelectSingTop1(int32 actorPos);
	void SelectSingBack1(int32 actorPos);
	void SelectHurmorTop1(int32 actorPos);
	void SelectHurmorBack1(int32 actorPos);

	void SelectCellFaced(int32 actorPos, int32 row);
	void SelectCell(int32 row, int32 col);

	void SelectCell_0_Faced(int32 actorPos);
	void SelectCell_0_Right(int32 actorPos);
	void SelectCell_0_Center(int32 actorPos);
	void SelectCell_0_Left(int32 actorPos);
	void SelectCell_1_Right(int32 actorPos);
	void SelectCell_1_Center(int32 actorPos);
	void SelectCell_1_Left(int32 actorPos);
	void SelectCell_2_Right(int32 actorPos);
	void SelectCell_2_Center(int32 actorPos);
	void SelectCell_2_Left(int32 actorPos);
	void SelectCell_3_Right(int32 actorPos);
	void SelectCell_3_Center(int32 actorPos);
	void SelectCell_3_Left(int32 actorPos);

	void SelectAllCells(int32 actorPos);

	void SelectRandom(int32 selectNum);
	void SelectRandom1(int32 actorPos);
	void SelectRandom2(int32 actorPos);
	void SelectRandom3(int32 actorPos);
	void SelectRandom4(int32 actorPos);
	void SelectRandom5(int32 actorPos);
	void SelectRandom6(int32 actorPos);
	void SelectRandom7(int32 actorPos);
	void SelectRandom8(int32 actorPos);
	void SelectRandom9(int32 actorPos);
	void SelectRandom10(int32 actorPos);
	void SelectRandom11(int32 actorPos);
	void SelectRandom12(int32 actorPos);
	//@}

	//@{
	//! 味方選択
	void SelectMyself_P(int32 actorPos);
	void SelectFront1_P(int32 actorPos);
	void SelectTop1_P(int32 actorPos);
	void SelectBack1_P(int32 actorPos);
	void SelectAll_P(int32 actorPos);
	//@}

	//@{
	void SelectDummy([[maybe_unused]]int32 actorPos) {}
	//@}

	//@{
	//! 入力セルを指定ルールに従って拡張する
	//! @param[in]  range       拡張タイプ
	void ExpandCell(EBattleSelectRange range);
	
	//@{
	void ExpandRangeSingle_Based(int32 basePos);
	void ExpandRangeCol_Based(int32 basePos);
	void ExpandRangeRow_Based(int32 basePos);
	void ExpandRangeSide_Based(int32 basePos);
	void ExpandRangeFrontBack_Based(int32 basePos);
	void ExpandRangeAroundPlus4_Based(int32 basePos);
	void ExpandRangeAroundCross4_Based(int32 basePos);
	void ExpandRangeAround9_Based(int32 basePos);
	void ExpandRangeBack_Based(int32 basePos, int32 count);
	//@}

	//@{
	void ExpandRangeSingle();
	void ExpandRangeCol();
	void ExpandRangeRow();
	void ExpandRangeSide();
	void ExpandRangeFrontBack();
	void ExpandRangeAroundPlus4();
	void ExpandRangeAroundCross4();
	void ExpandRangeAround9();
	void ExpandRangeBack1();
	void ExpandRangeBack2();
	void ExpandRangeBack3();
	void ExpandRangeBack4();
	//@}
	//@}

	//! 結果バッファのサイズを1にする
	void ShurinkResultTo(int32 size);

	//@{

	//! 選択セルリストを取得
	const CellList& GetResult() const { return ResultCells; }

	//@}

protected:
	void         Initialize(const FBattleParty* party);
	TArray<bool> MakeExistMap() const;

private:
	const FBattleParty* SelectedParty = nullptr;
    //TArray<BattleCell>  SourceCells;
    TArray<BattleCell>  ResultCells;
    TArray<bool>        SelectedCells;
	//int32               WorkBuffer[UBattleBoardUtil::MAX_BOARD_CELLS];
};
