// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Battle/BattleDataType.h"
#include "BattleAIComponent.generated.h"

class UBattleSystem;
class UBattleCommandQueue;
struct FBattleParty;
struct FBattleCharacterStatus;

/**
 * バトルAI
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BISHRPG_API UBattleAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBattleAIComponent();
	~UBattleAIComponent();

	/*!	セットアップ
		@param	lv		 強さ
		@param	randSeed シード値
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
		void Setup(UBattleCommandQueue* commandQueue, int32 lv, int32 randSeed = 0, bool playerSide = false);

	UFUNCTION(BlueprintCallable, Category = "Battle")
		void Execute();

protected:
	const FBattleParty* GetMyParty() const;
	const FBattleParty* GetOpponentParty() const;
	const FBattleCharacterStatus* GetMyCharacterStatusByPos(int32 pos) const;
	const FBattleCharacterStatus* GetOpponentCharacterStatusByPos(int32 pos) const;

private:
	UBattleSystem*       BattleSystem = nullptr;
	UBattleCommandQueue* CommandQueue = nullptr;
	FRandomStream        RandomGenerator;
	bool                 PlayerSide = false;
	EPlayerGroup         Group = EPlayerGroup::One;
	int32                Lv = 0;
};
