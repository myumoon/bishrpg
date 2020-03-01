// Copyright © 2018 nekoatsume_atsuko. All rights reserved.
#include "BattleDataTableAccessor.h"

#include "Engine/DataTable.h"
#include "Engine/CurveTable.h"
#include "Curves/CurveBase.h"
#include "ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Battle/BattleSystem.h"
#include "bishrpg.h"


namespace {
	class TablePathFactory {
	public:
		TablePathFactory(const FName& id) : Id(id) {}

		FString GetPath(const FString& fileTypeName) const
		{
			const FString fileBaseName = FString::Format(TEXT("{0}_{1}"), { *Id.ToString(), *fileTypeName });
			return FString::Format(TEXT("DataTable'/Game/GameData/Character/{0}/{1}.{2}'"), { *Id.ToString(), *fileBaseName, *fileBaseName });
		}

	private:
		FName Id;
	};
}

// Sets default values
UBattleDataTableAccessor::UBattleDataTableAccessor()
{
	auto actor = static_cast<UBattleDataTableAccessorFactory*>(GetOuter());
	if(!actor) {
		return;
	}
	UBattleSystem* system = static_cast<UBattleSystem*>(actor->GetOwner()->GetComponentByClass(UBattleSystem::StaticClass()));
	if(!system) {
		return;
	}
	
	for(int32 i = 0; i < static_cast<int32>(EPlayerGroup::Max); ++i) {
		const auto* party = system->GetParty(static_cast<EPlayerGroup>(i));
		for(const auto& character : party->Characters) {
			if(CharacterDataHolders.Contains(character.Id)) {
				continue;
			}

			TablePathFactory pathFactory(character.Id);
			FCharacterDataHolder addHolder;

			static ConstructorHelpers::FObjectFinder<UDataTable> timingTbl(*pathFactory.GetPath("AttackTimingTbl"));
			addHolder.AttackTimingTbl.Reset();
			addHolder.AttackTimingTbl.Reserve(32);
			FString errorMsg;
			timingTbl.Object->GetAllRows<FAttackTimingDataAsset>(errorMsg, addHolder.AttackTimingTbl);
			// 時間順にソート
			addHolder.AttackTimingTbl.Sort([](const FAttackTimingDataAsset& lhs, const FAttackTimingDataAsset& rhs) {
				return (lhs.TimingSec < rhs.TimingSec);
			});

			static ConstructorHelpers::FObjectFinder<UCurveBase> curveHp(*pathFactory.GetPath("Hp"));
			addHolder.HpCurve = static_cast<UCurveFloat*>(curveHp.Object);

			static ConstructorHelpers::FObjectFinder<UCurveBase> curveAttack(*pathFactory.GetPath("Attack"));
			addHolder.AttackCurve = static_cast<UCurveFloat*>(curveAttack.Object);

			static ConstructorHelpers::FObjectFinder<UCurveBase> curveDeffence(*pathFactory.GetPath("Deffence"));
			addHolder.DeffenceCurve = static_cast<UCurveFloat*>(curveDeffence.Object);

			CharacterDataHolders.Emplace(character.Id, addHolder);
		}
	}
	
}

void UBattleDataTableAccessor::ErrorMessage(const FName& id) const
{
	GAME_ERROR("UBattleDataTableAccessor 読み込まれていないId : ", *id.ToString());
}

float UBattleDataTableAccessor::GetLevelValue(const UCurveFloat* curve, int32 level) const
{
	if(!curve) {
		return 0.0f;
	}
	return curve->GetFloatValue(static_cast<float>(level));
}

float UBattleDataTableAccessor::GetHpByLevel(const FName& id, int32 level) const
{
	if(auto holder = GetHolderWithKey(id)) {
		return GetLevelValue(holder->HpCurve, level);
	}
	return 0.0f;
}

float UBattleDataTableAccessor::GetAttackByLevel(const FName& id, int32 level) const
{
	if(auto holder = GetHolderWithKey(id)) {
		return GetLevelValue(holder->AttackCurve, level);
	}
	return 0.0f;
}

float UBattleDataTableAccessor::GetDeffenceByLevel(const FName& id, int32 level) const
{
	if(auto holder = GetHolderWithKey(id)) {
		return GetLevelValue(holder->DeffenceCurve, level);
	}
	return 0.0f;
}

const TArray<FAttackTimingDataAsset*>& UBattleDataTableAccessor::GetAttackTimingTbl(const FName& id) const
{
	if(auto holder = GetHolderWithKey(id)) {
		holder->AttackTimingTbl;
	}

	static const TArray<FAttackTimingDataAsset*> _dummy;
	return _dummy;
}



UBattleDataTableAccessorFactory::UBattleDataTableAccessorFactory()
{

}

UBattleDataTableAccessor* UBattleDataTableAccessorFactory::CreateAccessor(const FParty& playerParty, const FParty& opponentParty)
{
	Accessor = NewObject<UBattleDataTableAccessor>(this);
	return Accessor;
}
