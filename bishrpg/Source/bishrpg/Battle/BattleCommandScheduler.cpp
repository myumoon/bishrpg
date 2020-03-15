// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleCommandScheduler.h"
#include "bishrpg.h"


UBattleCommandScheduler::FCharacterScheduler::FCharacterScheduler(const FBattleObjectHandle& handle) : Handle(handle)
{
}

void UBattleCommandScheduler::FCharacterScheduler::Update(TArray<FCommandSchedule>& scheduleList, float prevTime, float currentTime, const UBattleDataTableHolder* dataHolder, const UBattleSystem* system)
{
	if(!Handle.IsValid()) {
		return;
	}
	FBattleCharacterStatus status;
	system->GetCharacterStatusByHandle2(status, Handle);

	//GAME_LOG_FMT("FCharacterScheduler : {0}", status.Id.ToString());
	TArray<TPair<const FAttackTimingDataAsset*, float>> timingDataList;
	dataHolder->GetTimingTblRange(timingDataList, status.Id, prevTime, currentTime);
	MakeBattleCommandSchedule(scheduleList, timingDataList, prevTime, currentTime);
	
	/*
	for(const auto& timing : timingDataList) {
		GAME_LOG_FMT("- timing:{0}, {1}, {2}", timing.Get<0>()->TimingSec, timing.Get<0>()->SkillCommandName.ToString(), timing.Get<1>());
	}
	*/
}

void UBattleCommandScheduler::FCharacterScheduler::MakeBattleCommandSchedule(TArray<FCommandSchedule>& scheduleList, TArray<TPair<const FAttackTimingDataAsset*, float>>& timingDataList, float prevTime, float currentTime) const
{
	for(const auto& timing : timingDataList) {
		FCommandSchedule addSchedule;
		addSchedule.Command.ActionType  = ECommandType::Skill;
		addSchedule.Command.ActorHandle = Handle;
		addSchedule.Command.SkillName   = timing.Get<0>()->SkillCommandName;
		addSchedule.Time                = timing.Get<1>();

		scheduleList.Emplace(addSchedule);
	}
}

// Sets default values for this component's properties
UBattleCommandScheduler::UBattleCommandScheduler()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBattleCommandScheduler::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UBattleCommandScheduler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!BattleSystem || !DataHolder) {
		return;
	}

	PrevTime    = CurrentTime;
	CurrentTime += DeltaTime;

	//GAME_LOG_FMT("UBattleCommandScheduler tick current:{0} prev:{1}", CurrentTime, PrevTime);
	UpdateSchedule(DeltaTime);
}

void UBattleCommandScheduler::Initialize(UBattleSystem* system, const UBattleDataTableHolder* dataHolder)
{
	BattleSystem = system;
	DataHolder   = dataHolder;
	CurrentTime  = 0.0f;
	PrevTime     = 0.0f;

	GAME_ASSERT(BattleSystem);
	GAME_ASSERT(DataHolder);

	TArray<FBattleObjectHandle> hanleList;
	BattleSystem->MakeObjectHandleList(hanleList);
	for(const auto& handle : hanleList) {
		CharacterSchedulers.Emplace(FCharacterScheduler(handle));
	}
}

void UBattleCommandScheduler::EnqueueCommand(const FBattleCommand& command)
{

}

void UBattleCommandScheduler::UpdateSchedule(float deltaTime)
{
	TArray<FCommandSchedule> scheduleList;

	UpdateCharacterSchedule(scheduleList);
	SendToBattleSystem(scheduleList);
}

void UBattleCommandScheduler::UpdateCharacterSchedule(TArray<FCommandSchedule>& scheduleList)
{
	scheduleList.Reset();
	scheduleList.Reserve(128); // 十分なサイズ

	for(auto& characterScheduler : CharacterSchedulers) {
		characterScheduler.Update(scheduleList, PrevTime, CurrentTime, DataHolder, BattleSystem);
	}
	scheduleList.StableSort([](const FCommandSchedule& lhs, const FCommandSchedule& rhs) {
		return (lhs.Time < rhs.Time);
	});
}

void UBattleCommandScheduler::SendToBattleSystem(TArray<FCommandSchedule>& scheduleList)
{
	for(const auto& schedule : scheduleList) {
		BattleSystem->ExecSkill(schedule.Command);
	}
	
}
