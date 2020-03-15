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
	
	UDataTable*   TimingTbl     = nullptr;
	UCurveFloat*  HpCurve       = nullptr;
	UCurveFloat*  AttackCurve   = nullptr;
	UCurveFloat*  DeffenceCurve = nullptr;
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

	/*!	タイミングデータを範囲で取得
	@param[out]	out		範囲内のタイミングデータ
	@param[in]	id		取得ID
	@retval		取得できたらtrueを返す
	*/
	bool GetTimingTbl(TArray<const FAttackTimingDataAsset*>& out, const FName& id, bool includingEndData = true) const;

	/*!	タイミングデータを範囲で取得
	@param[out]	out		範囲内のタイミングデータ, 発動時間
	@param[in]	id		取得ID
	@param[in]	begin	開始範囲(含む)
	@param[in]	end		終了範囲(含まない)
	@retval		取得できたらtrueを返す
	@note       end<begin の場合は、
				[end～テーブル終了]と[0.0f～begin]のデータ範囲を取得
	*/
	bool GetTimingTblRange(TArray<TPair<const FAttackTimingDataAsset*, float>>& out, const FName& id, float begin, float end) const;

	/*!	テーブル終了時間を取得
	*/
	float GetTableEndTime(const FName& id) const;

	/*!	テーブル終了時間で正規化した時間を取得
	*/
	float GetNormalizedTimeWithTableEnd(const FName& id, float time) const;

	/*!	バトル用のデータテーブルを生成
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battle")
	static UBattleDataTableHolder* MakeBattleDataTable();

	/*!	管理しているキャラクターID一覧を取得
	*/
	void MakeManagedIDList(TArray<FName>& ids) const;

protected:
	void ErrorMessage(const FName& id) const;

	const FCharacterDataHolder* GetHolderWithKey(const FName& id) const
	{
		const auto* holder = CharacterDataHolders.Find(id);
		if(!holder) {
			ErrorMessage(id);
			return nullptr;
		}
		return holder;
	}

	float GetLevelValue(const UCurveFloat*, int32 level) const;

private:
	UPROPERTY()
	TMap<FName, FCharacterDataHolder> CharacterDataHolders;
};
