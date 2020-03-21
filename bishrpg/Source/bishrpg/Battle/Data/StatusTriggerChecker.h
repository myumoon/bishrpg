// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Battle/BattleDataType.h"
#include "Battle/BattleObjectHandle.h"
#include "StatusTriggerChecker.generated.h"

//! ステータス変化時デリゲート
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBattleChangedStatusDelegate,
	FBattleObjectHandle, handle,
	EStatusFlag,         status
);

UCLASS(Blueprintable)
class BISHRPG_API UStatusTriggerChecker : public UObject
{
	GENERATED_BODY()
public:
	UStatusTriggerChecker() = default;
	~UStatusTriggerChecker() = default;

	/**	初期化
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle/Status")
	void Initialize();

	/**	ステータスの差分をチェック
	@param	handle	対象オブジェクト
	@param	status	現在のステータス
	@return	差分発見時にtrue
	@note	登録されたイベントを飛ばす
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle/Status")
	bool Check(const FBattleObjectHandle& handle, int32 status);

public:

	UPROPERTY(BlueprintAssignable, Category = "Battle/System")
	FBattleChangedStatusDelegate		AddedStatusEvent;	//!< ステータス付与時イベント

	UPROPERTY(BlueprintAssignable, Category = "Battle/System")
	FBattleChangedStatusDelegate		RemovedStatusEvent;	//!< ステータス解除時イベント

private:
	TMap<uint32, int32>	CurrentStatusMap; //!< ハンドルのハッシュとステータスの対応表
};
