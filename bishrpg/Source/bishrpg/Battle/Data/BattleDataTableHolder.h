// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "SharedPointer.h"
#include "UniquePtr.h"
#include "GameData/CharacterAsset.h"
#include "Battle/BattleData.h"
#include "BattleDataTableHolder.generated.h"

/*!	キャラごとのデータ保持
*/
USTRUCT()
struct BISHRPG_API FCharacterDataHolder {
	GENERATED_BODY()
	//using AttackTimingTbl = TArray<FAttackTimingDataAsset*>;
	//AttackTimingTbl AttackTimingTbl; // usingするとビルドエラーになる

	TArray<FAttackTimingDataAsset*> AttackTimingTbl;
	UCurveFloat*    HpCurve = nullptr;
	UCurveFloat*    AttackCurve = nullptr;
	UCurveFloat*    DeffenceCurve = nullptr;
};

/*!	バトルで使用するデータのアクセサ
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BISHRPG_API UBattleDataTableHolder : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UBattleDataTableHolder();

	/*!	レベルに対応したHPを取得
	*/
	float GetHpByLevel(const FName& id, int32 level) const;

	/*!	レベルに対応した攻撃力を取得
	*/
	float GetAttackByLevel(const FName& id, int32 level) const;

	/*!	レベルに対応した防御力を取得
	*/
	float GetDeffenceByLevel(const FName& id, int32 level) const;

	/*!	攻撃タイミングテーブルを取得
	*/
	const TArray<FAttackTimingDataAsset*>& GetAttackTimingTbl(const FName& id) const;

	/*!	バトル用のデータテーブルを生成
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static UBattleDataTableHolder* MakeBattleDataTable();

protected:
	void ErrorMessage(const FName& id) const;

	const FCharacterDataHolder* GetHolderWithKey(const FName& id) const
	{
		if(!CharacterDataHolders.Contains(id)) {
			ErrorMessage(id);
			return nullptr;
		}
		return CharacterDataHolders.Find(id);
	}

	float GetLevelValue(const UCurveFloat*, int32 level) const;

private:
	UPROPERTY()
	TMap<FName, FCharacterDataHolder> CharacterDataHolders;
};
