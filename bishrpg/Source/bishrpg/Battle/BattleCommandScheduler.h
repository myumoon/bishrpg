// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Battle/BattleSystem.h"
#include "Battle/Data/BattleDataTableHolder.h"
#include "BattleCommandScheduler.generated.h"

/*!	バトルコマンドを時間に応じて呼び出す仕組み
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BISHRPG_API UBattleCommandScheduler : public UActorComponent
{
	GENERATED_BODY()

	struct FCommandSchedule {
		float          Time = 0.0f;
		FBattleCommand Command;
	};
	
	// キャラごとのスケジューラー
	class FCharacterScheduler {
	public:
		FCharacterScheduler(const FBattleObjectHandle& handle);

		void Update(TArray<FCommandSchedule>& scheduleList, float prevTime, float currentTime, const UBattleDataTableHolder* dataHolder, const UBattleSystem* system);

	private:
		void  MakeBattleCommandSchedule(TArray<FCommandSchedule>& scheduleList, TArray<TPair<const FAttackTimingDataAsset*, float>>& timingDataList, float prevTime, float currentTime) const;
	private:
		FBattleObjectHandle Handle;
	};

public:	
	// Sets default values for this component's properties
	UBattleCommandScheduler();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize(UBattleSystem* system, const UBattleDataTableHolder* dataHolder, bool play = true);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EnqueueCommand(const FBattleCommand& command);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Resume();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Pause();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateSchedule(float deltaTime);
	void UpdateCharacterSchedule(TArray<FCommandSchedule>& scheduleList);
	void SendToBattleSystem(TArray<FCommandSchedule>& scheduleList);

	//float GetNormalizedTimeByCommandTable() const;

private:
	UBattleSystem*                BattleSystem = nullptr;
	const UBattleDataTableHolder* DataHolder   = nullptr;

	// キャラ情報
	TArray<FCharacterScheduler>   CharacterSchedulers;
	
	// ゲーム進行状況
	float                         CurrentTime  = 0.0f;
	float                         PrevTime     = 0.0f;
	bool                          Playing      = true;
};
