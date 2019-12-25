// Copyright © 2019 nekoatsume_atsuko. All rights reserved.

#include "BattleContext.h"

#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"
#include "Context/BattleTurn.h"

#include "bishrpg.h"


// Sets default values for this component's properties
UBattleContext::UBattleContext() : BattleTurn(MakeUnique<FBattleTurn>())
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UBattleContext::BeginPlay()
{
	Super::BeginPlay();

	// ...
	UE_LOG(BishRPG, Log, TEXT("UBattleContext::BeginPlay()"));
}


// Called every frame
/*
void UBattleContext::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
*/

// 初期化
void UBattleContext::Initialize()
{

}

