// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleSystem.h"

#include "Engine/DataTable.h"
#include "ConstructorHelpers.h"

// Sets default values for this component's properties
UBattleSystem::UBattleSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UBattleSystem::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
/*
void UBattleSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
*/


// バトル用キャラ情報生成
//FBattleCharacterStatus::FBattleCharacterStatus(const FCharacterStatus& stat)
FBattleCharacterStatus UBattleSystem::MakeFromCharacterStatus(const FCharacterStatus& stat)
{
	//static ConstructorHelpers::FObjectFinder<UDataTable> charAssetTable(TEXT("DataTable'/GameData/Character/CharacterAssetTbl.GameObjectLookup'"));
	//UDataTable* charAssetTbl = charAssetTable.Object;

	//ConstructorHelpers::FObjectFinder<UDataTable> charStatusable(*FString::Printf("DataTable'/GameData/Character/Status_%s.GameObjectLookup'", stat.Id.ToString()));
	//UDataTable* charStatTbl = charStatusable.Object;
	//*charStatTbl->FindRow<FCharacterStatusData>(FName(*FString::Printf("%d", stat.AttackLv)), "")

	//EBattleStyle battleStyle = *charAssetTbl->FindRow<EBattleStyle>(stat.Id, FString(""));


	FBattleCharacterStatus battleStatus;
	battleStatus.Id = stat.Id;
	battleStatus.Style = EBattleStyle::Rock; // todo:
	battleStatus.HpMax = stat.Hp; // todo:
	battleStatus.Hp = stat.Hp; // todo:
	battleStatus.Attack = stat.AttackLv; // todo:
	battleStatus.Deffence = stat.DeffenceLv; // todo:
	battleStatus.Speed = 0; // todo:
	battleStatus.Hate = 0; // todo:

	return battleStatus;
}

// バトル用パーティ情報生成
FBattleParty UBattleSystem::MakeFromParty(const FParty& party)
{
	FBattleParty battleParty;
	battleParty.Characters.AddUninitialized(FBattleParty::MAX_PARTY_NUM);

	for(int i = 0; i < party.Characters.Num(); ++i) {
		battleParty.Characters[i] = MakeFromCharacterStatus(party.Characters[i]);
	}
	battleParty.Formation = party.Formation;

	return battleParty;
}


// 初期化
void UBattleSystem::Initialize(const FParty& playerParty, const FParty& opponentParty, int32 aaa)
{
}
