// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleObjectHandle.h"
#include "ObjectHandleLibrary.generated.h"

struct FBattleObjectHandle;

USTRUCT(BlueprintType)
struct FObjectHandleDesc {
	GENERATED_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category="Battle/Object")
	int32 Index = 0;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category="Battle/Object")
	EObjectType ObjectType = EObjectType::None;
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category="Battle/Object")
	EPlayerGroup PlayerGroup = EPlayerGroup::One;
};

/**
 * 
 */
UCLASS()
class BISHRPG_API UObjectHandleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/*!	オブジェクトハンドルを生成
	*/
	UFUNCTION(BlueprintPure, Category="Battle/Object")
    static FBattleObjectHandle MakeObjectHandle(const FObjectHandleDesc& desc);

    /*! プレイヤーかどうかを判定
	*/
    UFUNCTION(BlueprintPure, Category="Battle/Object")
    static bool IsPlayer(const FBattleObjectHandle& handle);

	/*!	相手プレイヤーかどうかを取得
	*/
	UFUNCTION(BlueprintPure, Category="Battle/Object")
	static bool IsOpponent(const FBattleObjectHandle& handle);

	/*!	プレイヤーグループを取得
	*/
	UFUNCTION(BlueprintPure, Category="Battle/Object")
	static EPlayerGroup GetGroup(const FBattleObjectHandle& handle);

	/*!	プレイヤーグループインデックスを取得
	*/
	UFUNCTION(BlueprintPure, Category="Battle/Object")
	static int32 GetGroupIndex(const FBattleObjectHandle& handle);

	/*!	オブジェクトインデックスを取得
	*/
	UFUNCTION(BlueprintPure, Category="Battle/Object")
	static int32 GetObjectIndex(const FBattleObjectHandle& handle);
};
