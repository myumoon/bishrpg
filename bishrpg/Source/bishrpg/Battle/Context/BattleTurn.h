// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Battle/BattleDataType.h"

/**
 * バトルのターン
 */
class BISHRPG_API FBattleTurn
{
public:
	FBattleTurn() = default;
	~FBattleTurn() = default;

	/*!	初期化
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Setup(EPlayerGroup myTag, EPlayerGroup firstPlayerTag);

	/*!	操作プレイヤーのターンか
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	bool IsManualPlayerTurn() const;

	/*!	敵・AIのターンか
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	bool IsOpponentTurn() const;

	/*!	ターン切り替え
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	void SwapTurn();

private:
	EPlayerGroup MyPlayerTag      = EPlayerGroup::One;
	EPlayerGroup CurrentPlayerTag = EPlayerGroup::One;
};
