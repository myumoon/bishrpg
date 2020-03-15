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
		GAME_LOG_FMT("Load DataTable : {0} : timing({1}), hp({2}), attack({3}), def({4})", id.ToString(), attackTimngTblPath, pathFactory.GetPath("Hp"), pathFactory.GetPath("Attack"), pathFactory.GetPath("Deffence"));
		FCharacterDataHolder addHolder;
		
		ConstructorHelpers::FObjectFinder<UDataTable> timingTbl(*attackTimngTblPath);
		if(auto* tblObj = timingTbl.Object) {
			addHolder.TimingTbl = static_cast<UDataTable*>(tblObj);
		}
		
		ConstructorHelpers::FObjectFinder<UCurveBase> curveHp(*pathFactory.GetPath("Hp"));
		if(auto* curveObj = curveHp.Object) {
			addHolder.HpCurve = static_cast<UCurveFloat*>(curveObj);
		}

		ConstructorHelpers::FObjectFinder<UCurveBase> curveAttack(*pathFactory.GetPath("Attack"));
		if(auto* curveObj = curveAttack.Object) {
			addHolder.AttackCurve = static_cast<UCurveFloat*>(curveObj);
		}

		ConstructorHelpers::FObjectFinder<UCurveBase> curveDeffence(*pathFactory.GetPath("Deffence"));
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

bool UBattleDataTableHolder::GetTimingTbl(TArray<const FAttackTimingDataAsset*>& out, const FName& id, bool includingEndData) const
{
	if(auto holder = GetHolderWithKey(id)) {
		//GAME_LOG("GetTimingTbl tbl holder({%p}) timing({%p})", (void*)holder, (void*)holder->TimingTbl);
		const auto* dataTbl = holder->TimingTbl;
		FString errorContext;       // エラー時用
		TArray<FAttackTimingDataAsset*> fetched;
		dataTbl->GetAllRows<FAttackTimingDataAsset>(errorContext, fetched);
		out.Reset();

		if(fetched.Num() == 0) {
			return false;
		}

		out.Reserve(fetched.Num());
		for(const auto* asset : fetched) {
			if(!includingEndData && asset->IsEndData()) {
				break;
			}
			out.Emplace(asset);
		}
		return true;
	}

	return false;
}

bool UBattleDataTableHolder::GetTimingTblRange(TArray<TPair<const FAttackTimingDataAsset*, float>>& out, const FName& id, float start, float end) const
{
	GAME_ASSERT(start <= end);

	TArray<const FAttackTimingDataAsset*> timingDataList;
	if(GetTimingTbl(timingDataList, id, false)) {
		//GAME_LOG_FMT("+ id:{0}, num:{1} start:{2}, end:{3}", id.ToString(), timingDataList.Num(), start, end);
		//for(auto& data : timingDataList) {
		//	GAME_LOG_FMT("* name:{0}, time:{1}", data->SkillCommandName.ToString(), data->TimingSec);
		//}
		// データ的に時間順に並んでいることが保証されるようになったらこの処理を消したい
		// 
		// 時間順にソート
		timingDataList.Sort([](const FAttackTimingDataAsset& lhs, const FAttackTimingDataAsset& rhs) {
			return (lhs.TimingSec < rhs.TimingSec);
		});

		const float wholeTime = GetTableEndTime(id);
		auto getNormalizedTime = [wholeTime](float time) {
			if(FMath::IsNearlyZero(wholeTime)) {
				return 0.0f;
			}
			return FMath::Fmod(time, wholeTime);
		};
		//GAME_LOG_FMT("wholetime {0}", wholeTime);
		const float normStartTime     = getNormalizedTime(start);
		const float normEndTime       = getNormalizedTime(end);
		const float divStartWhole     = start / wholeTime;
		const int32 ceiledStartIndex  = FMath::CeilToInt(divStartWhole);
		const int32 flooredStartIndex = FMath::FloorToInt(divStartWhole);
		const float flooredStartTime  = FMath::FloorToFloat(divStartWhole);
		const float divEndWhole       = end / wholeTime;
		const int32 flooredEndIndex   = FMath::FloorToInt(divEndWhole);
		const float flooredEndTime    = FMath::FloorToFloat(divEndWhole);
		const int32 ceiledEndIndex    = FMath::CeilToInt(divEndWhole);
		const int32 loopCount         = flooredEndIndex - ceiledStartIndex;
		/*
		GAME_LOG_FMT(
		"\n"\
		"normStartTime    {0}\n"\
		"normEndTime      {1}\n"\
		"divStartWhole    {2}\n"\
		"ceiledStartIndex {3}\n"\
		"flooredStartTime {4}\n"\
		"divEndWhole      {5}\n"\
		"flooredEndIndex  {6}\n"\
		"flooredEndTime   {7}\n"\
		"ceiledEndIndex   {8}\n"\
		"loopCount        {9}\n",
		normStartTime, 
		normEndTime, 
		divStartWhole, 
		ceiledStartIndex, 
		flooredStartTime, 
		divEndWhole,
		flooredEndIndex,
		flooredEndTime,
		ceiledEndIndex,
		loopCount);
		*/

		// 同じ範囲内のデータ
		//   |   +--------+  |
		//     start     end
		if(flooredStartIndex == flooredEndIndex) {
			// 範囲外のデータを削除
			timingDataList.RemoveAll([normStartTime, normEndTime](const FAttackTimingDataAsset* data) {
				return (data->TimingSec < normStartTime) || (normEndTime <= data->TimingSec);
			});
			for(const auto* timingData : timingDataList) {
				const float playSec = flooredStartTime + timingData->TimingSec;
				out.Emplace(MakeTuple(timingData, playSec));
			}
		}
		// 1周はしてないデータ
		//   |   +-------|---------+  |
		//     start              end
		// or
		// 1周以上しているデータ
		//   |   +-------|-----------|--------+    |
		//     start                         end
		else {
			// start
			for(const auto* timingData : timingDataList) {
				if(normStartTime <= timingData->TimingSec) {
					const float playSec = flooredStartTime + timingData->TimingSec;
					out.Emplace(MakeTuple(timingData, playSec));
				}
			}
			for(int32 loop = 0; loop < loopCount; ++loop) {
				for(const auto* timingData : timingDataList) {
					const float playSec = flooredStartTime + (loop + 1) * wholeTime + timingData->TimingSec;
					out.Emplace(MakeTuple(timingData, playSec));
				}
			}
			// end
			for(const auto* timingData : timingDataList) {
				if(timingData->TimingSec < normEndTime) {
					const float playSec = flooredEndTime + timingData->TimingSec;
					out.Emplace(MakeTuple(timingData, playSec));
				}
			}
		}

		return true;
	}

	return false;
}

float UBattleDataTableHolder::GetTableEndTime(const FName& id) const
{
	TArray<const FAttackTimingDataAsset*> timingTbl;
	if(GetTimingTbl(timingTbl, id)) {
		const auto* endData = timingTbl.Last();
		if(endData) {
			GAME_ASSERT(endData->IsEndData());
			GAME_ASSERT(!FMath::IsNearlyZero(endData->TimingSec));
			return endData->TimingSec;
		}
	}
	return 0.0f;
}

float UBattleDataTableHolder::GetNormalizedTimeWithTableEnd(const FName& id, float time) const
{
	const float endTime = GetTableEndTime(id);
	if(FMath::IsNearlyZero(endTime)) {
		return 0.0f;
	}
	return FMath::Fmod(time, endTime);
}


UBattleDataTableHolder* UBattleDataTableHolder::MakeBattleDataTable()
{
	return NewObject<UBattleDataTableHolder>();
}

void UBattleDataTableHolder::MakeManagedIDList(TArray<FName>& ids) const
{
	ids.Reset();
	ids.Reserve(CharacterDataHolders.Num());
	for(auto& holder : CharacterDataHolders) {
		ids.Emplace(holder.Key);
	}
}

