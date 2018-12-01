// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BattleDataType.h"
#include "BattleData.h"
#include "BattleBoardUtil.generated.h"


UCLASS()
class BISHRPG_API UBattleBoardUtil : public UObject {
	GENERATED_BODY()

public:
	static const int32 INVALID_CELL_NO = -1;
	static const int32 BOARD_ROW = 4;
	static const int32 BOARD_COL = 3;
	static const int32 COL_LEFT = 0;
	static const int32 COL_CENTER = 1;
	static const int32 COL_RIGHT = 2;
	static const int32 MAX_BOARD_CELLS = BOARD_COL * BOARD_ROW;

	static const int32 CELL_NUM = BOARD_ROW * BOARD_COL;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static bool IsValidCellNo(int32 index) { return (0 <= index && index < CELL_NUM); }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetBoardRow() { return BOARD_ROW; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetBoardCol() { return BOARD_COL; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColLeft() { return COL_LEFT; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColRight() { return COL_RIGHT; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColCenter() { return COL_CENTER; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetCellNum() { return CELL_NUM; }

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void MakePositionListRow(TArray<int32>& madePosList, int32 row);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void MakePositionListCol(TArray<int32>& madePosList, int32 col, bool up = true);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void MakePositionListRandom(TArray<int32>& madePosList, int32 positions, const FRandomStream& randStream);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetFacedCol(int32 pos)
	{
		if(0 <= pos) {
			const int32 col = pos % GetBoardCol();
			return GetBoardCol() - col - 1;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetColReverse(int32 col)
	{
		if(0 <= col && col < GetBoardCol()) {
			return GetBoardCol() - col - 1;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetRowReverse(int32 row)
	{
		if(0 <= row && row < GetBoardRow()) {
			return GetBoardRow() - row - 1;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetCol(int32 pos)
	{
		if(0 <= pos) {
			return (pos % GetBoardCol());
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetRow(int32 pos)
	{
		if(0 <= pos) {
			return (pos / GetBoardCol());
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetRowInverse(int32 pos)
	{
		const int32 row = GetRow(pos);
		if(0 <= row) {
			return (GetBoardRow() - row - 1);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPos(int32 row, int32 col)
	{
		if((0 <= row) && (row < GetBoardRow()) && (0 <= col) && (col < GetBoardCol())) {
			const int32 pos = row * GetBoardRow() + col;
			return pos;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosForward(int32 pos)
	{
		if((0 <= pos) && (pos < GetCellNum())) {
			const int32 nextPos = pos + GetBoardCol();
			return FixCol(nextPos, pos);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosBack(int32 pos)
	{
		if((0 <= pos) && (pos < GetCellNum())) {
			const int32 nextPos = pos - GetBoardCol();
			return FixCol(nextPos, pos);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosLeft(int32 pos)
	{
		if((0 <= pos) && (pos < GetCellNum())) {
			const int32 nextPos = pos - 1;
			return FixRow(nextPos, pos);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosRight(int32 pos)
	{
		if((0 <= pos) && (pos < GetCellNum())) {
			const int32 nextPos = pos + 1;
			return FixRow(nextPos, pos);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosForwardLeft(int32 pos)
	{
		const int32 forward = GetPosForward(pos);
		if(pos != forward) {
			return GetPosLeft(forward);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosForwardRight(int32 pos)
	{
		const int32 forward = GetPosForward(pos);
		if(pos != forward) {
			return GetPosRight(forward);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosBackLeft(int32 pos)
	{
		const int32 back = GetPosBack(pos);
		if(pos != back) {
			return GetPosLeft(back);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 GetPosBackRight(int32 pos)
	{
		const int32 back = GetPosBack(pos);
		if(pos != back) {
			return GetPosRight(back);
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 Clamp(int32 pos, int32 originPos)
	{
		if((pos < 0) || (GetCellNum() <= pos)) {
			return pos;
		}
		return -1;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 FixCol(int32 pos, int32 originPos)
	{
		const int32 baseCol  = GetCol(originPos);
		const int32 nextCol  = GetCol(pos);
		const int32 diff     = baseCol - nextCol;
		const int32 fixedPos = pos + diff;

		return Clamp(fixedPos, originPos);
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static int32 FixRow(int32 pos, int32 originPos)
	{
		const int32 baseRow  = GetRow(originPos);
		const int32 nextRow  = GetRow(pos);
		const int32 diff     = baseRow - nextRow;
		if(diff < 0) {
			return (baseRow * GetBoardCol());
		}
		else if (0 < diff) {
			return (baseRow * GetBoardCol() + GetBoardRow() - 1);
		}

		return pos;
	}

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void MakeTargetPositions(TArray<int32>& outPositions, EBattleSelectRange selectRange, int32 basePos);

	//UFUNCTION(BlueprintCallable, Category = "Battle")
	//static void SelectTargetPositions(TArray<int32>& outPositions, const FBattleParty* playerParty, int32 actorPos, EBattleSelectMethod selectMethod, EBattleSelectRange selectRange, const FBattleParty* opponentParty);
};
