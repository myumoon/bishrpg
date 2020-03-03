// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedPointer.h"
#include "UniquePtr.h"
#include "GameData/CharacterAsset.h"
#include "Battle/BattleData.h"
#include "Battle/Data/BattleDataTableHolder.h"

#include "BattleDataTableAccessor.generated.h"



/*!	バトルで使用するデータのアクセサ
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BISHRPG_API UBattleDataTableAccessorFactory : public UActorComponent
{
	GENERATED_BODY()
	friend class UBattleDataTableHolder;

public:
	UBattleDataTableAccessorFactory();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	UBattleDataTableHolder* CreateAccessor(const FParty& playerParty, const FParty& opponentParty);

private:
	UPROPERTY()
	UBattleDataTableHolder* Accessor = nullptr;

	TArray<FName> LoadIdList;
};
