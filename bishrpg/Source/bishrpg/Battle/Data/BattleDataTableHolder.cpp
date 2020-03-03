// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleDataTableHolder.h"

#include "Engine/DataTable.h"
#include "Engine/CurveTable.h"
#include "Curves/CurveBase.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Battle/BattleSystem.h"
#include "Battle/Data/BattleDataTableAccessor.h"
#include "bishrpg.h"

namespace {
	class TablePathFactory {
	public:
		TablePathFactory(const FString& id) : Id(id) {}

		FString GetPath(const FString& fileTypeName) const
		{
			//GAME_LOG_FMT("UBattleDataTableHolder 1 GetPath({0})", fileTypeName);
			const FString fileBaseName = FString::Format(TEXT("{0}_{1}"), {*Id, *fileTypeName});
			//GAME_LOG_FMT("UBattleDataTableHolder 2 fileBaseName({0})", fileBaseName);
			return FString::Format(TEXT("DataTable'/Game/GameData/Character/{0}/{1}.{2}'"), {*Id, *fileBaseName, *fileBaseName});
		}

	private:
		FString Id;
	};
}

// Sets default values
UBattleDataTableHolder::UBattleDataTableHolder()
{
	auto* factory = Cast<UBattleDataTableAccessorFactory>(GetOuter());
	if(!factory) {
		return;
	}
	CharacterDataHolders.Reset();
	for(const auto& id : factory->LoadIdList) {
		if(CharacterDataHolders.Contains(id)) {
			continue;
		}
		TablePathFactory pathFactory(id.ToString());
		const auto attackTimngTblPath = pathFactory.GetPath("AttackTimingTbl");

		FCharacterDataHolder addHolder;
		
		#if 0
		static ConstructorHelpers::FObjectFinder<UDataTable> timingTbl(*attackTimngTblPath);
		if(auto* tblObj = timingTbl.Object) {
				
			addHolder.AttackTimingTbl.Reset();
			addHolder.AttackTimingTbl.Reserve(32);
			FString errorMsg;
			tblObj->GetAllRows<FAttackTimingDataAsset>(errorMsg, addHolder.AttackTimingTbl);
			// 時間順にソート
			addHolder.AttackTimingTbl.Sort([](const FAttackTimingDataAsset& lhs, const FAttackTimingDataAsset& rhs) {
				return (lhs.TimingSec < rhs.TimingSec);
			});
		}
		#endif
		static ConstructorHelpers::FObjectFinder<UCurveBase> curveHp(*pathFactory.GetPath("Hp"));
		if(auto* curveObj = curveHp.Object) {
			addHolder.HpCurve = static_cast<UCurveFloat*>(curveObj);
		}

		static ConstructorHelpers::FObjectFinder<UCurveBase> curveAttack(*pathFactory.GetPath("Attack"));
		if(auto* curveObj = curveAttack.Object) {
			addHolder.AttackCurve = static_cast<UCurveFloat*>(curveObj);
		}

		static ConstructorHelpers::FObjectFinder<UCurveBase> curveDeffence(*pathFactory.GetPath("Deffence"));
		if(auto* curveObj = curveDeffence.Object) {
			addHolder.DeffenceCurve = static_cast<UCurveFloat*>(curveObj);
		}

		CharacterDataHolders.Emplace(id, addHolder);
	}
}

void UBattleDataTableHolder::ErrorMessage(const FName& id) const
{
	GAME_ERROR("UBattleDataTableHolder 読み込まれていないId : ", *id.ToString());
}

float UBattleDataTableHolder::GetLevelValue(const UCurveFloat* curve, int32 level) const
{
	if(!curve) {
		return 0.0f;
	}
	return curve->GetFloatValue(static_cast<float>(level));
}

float UBattleDataTableHolder::GetHpByLevel(const FName& id, int32 level) const
{
	if(auto holder = GetHolderWithKey(id)) {
		return GetLevelValue(holder->HpCurve, level);
	}
	return 0.0f;
}

float UBattleDataTableHolder::GetAttackByLevel(const FName& id, int32 level) const
{
	if(auto holder = GetHolderWithKey(id)) {
		return GetLevelValue(holder->AttackCurve, level);
	}
	return 0.0f;
}

float UBattleDataTableHolder::GetDeffenceByLevel(const FName& id, int32 level) const
{
	if(auto holder = GetHolderWithKey(id)) {
		return GetLevelValue(holder->DeffenceCurve, level);
	}
	return 0.0f;
}

const TArray<FAttackTimingDataAsset*>& UBattleDataTableHolder::GetAttackTimingTbl(const FName& id) const
{
	if(auto holder = GetHolderWithKey(id)) {
		holder->AttackTimingTbl;
	}

	static const TArray<FAttackTimingDataAsset*> _dummy;
	return _dummy;
}

UBattleDataTableHolder* UBattleDataTableHolder::MakeBattleDataTable()
{
	return NewObject<UBattleDataTableHolder>();
}
