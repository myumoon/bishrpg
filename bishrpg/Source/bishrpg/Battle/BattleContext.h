// Copyright © 2019 nekoatsume_atsuko. All rights reserved.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedPointer.h"
#include "BattleDataType.h"
#include "BattleData.h"
#include "Context/BattleTurn.h" // deleterのために必要
#include "BattleContext.generated.h"

class FBattleTurn;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BISHRPG_API UBattleContext : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBattleContext();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*! 初期化
	*/
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize();


protected:
	// Called when the game starts
	void BeginPlay() override;

private:
	TUniquePtr<FBattleTurn> BattleTurn;
};
