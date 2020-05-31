// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "VoxelGroundUpdater.h"

// 統計情報
DECLARE_STATS_GROUP(TEXT("UVoxelGroundUpdaterStat"), STATGROUP_VoxelGroundUpdater, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Find"), STAT_Find, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRange"), STAT_FindRange, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRangeCopy"), STAT_FindRangeCopy, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRangeSphere"), STAT_FindRangeSphere, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRangeSphereCopy"), STAT_FindRangeSphereCopy, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Replace"), STAT_Replace, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Remove"), STAT_Remove, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Clear"), STAT_Clear, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Add"), STAT_Add, STATGROUP_VoxelGroundUpdater);


// Sets default values for this component's properties
UVoxelGroundUpdater::UVoxelGroundUpdater()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}


// Called when the game starts
void UVoxelGroundUpdater::BeginPlay()
{
	Super::BeginPlay();
	
	AreaTree = MakeUnique<TreeAlgorithm>();
	const float   halfWidth = Width / 2.0f;
	const float   halfDepth = Depth / 2.0f;
	const FVector begin = {CenterPos.X - halfWidth, CenterPos.Y - halfDepth, 0.0f};
	const FVector end = {CenterPos.X + halfWidth, CenterPos.Y + halfDepth, 0.0f};
	AreaTree->Initialize(begin, end, SeparationLevel);

	VoxelDataTree = MakeUnique<BlockDataHolder>(*AreaTree.Get());
	VoxelDataTree->Initialize();
}


// Called every frame
void UVoxelGroundUpdater::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UVoxelGroundUpdater::Add(FMortonIndex& mortonindex, const FVector& pos, const FBlockData& value)
{
	SCOPE_CYCLE_COUNTER(STAT_Add);
	mortonindex = VoxelDataTree->Add(pos, value);
	return UMortonIndexFunctionLibrary::IsValid(mortonindex);
}

bool UVoxelGroundUpdater::Remove(const FMortonIndex& targetMortonIndex, const FBlockData& value)
{
	SCOPE_CYCLE_COUNTER(STAT_Remove);
	return VoxelDataTree->Remove(targetMortonIndex, value);
}

bool UVoxelGroundUpdater::Replace(const FBlockData& from, const FBlockData& to, bool first)
{
	SCOPE_CYCLE_COUNTER(STAT_Replace);
	return VoxelDataTree->Replace(from, to, first);
}

bool UVoxelGroundUpdater::Find(TArray<FBlockData>& registered, FMortonIndex& mortonIndex, const FVector& pos) const
{
	SCOPE_CYCLE_COUNTER(STAT_Find);
	return VoxelDataTree->Find(registered, mortonIndex, pos);
}

bool UVoxelGroundUpdater::FindRange(TArray<FBlockData>& registered, const FVector& begin, const FVector& end) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindRange);
	TArray<FMortonIndex> mortonIndexList;
	const bool success = VoxelDataTree->FindRange(registered, mortonIndexList, begin, end);
	if(success) {
		SCOPE_CYCLE_COUNTER(STAT_FindRangeCopy);
		for(int32 i = 0; i < mortonIndexList.Num(); ++i) {
			registered[i].MortonIndex = mortonIndexList[i];
		}
	}
	return success;
}

bool UVoxelGroundUpdater::FindRangeSphere(TArray<FBlockData>& registered, const FVector& center, float r) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindRangeSphere);
	TArray<FMortonIndex> mortonIndexList;
	const bool success = VoxelDataTree->FindRange(registered, mortonIndexList, center, r);
	if(success) {
		SCOPE_CYCLE_COUNTER(STAT_FindRangeSphereCopy);
		for(int32 i = 0; i < mortonIndexList.Num(); ++i) {
			registered[i].MortonIndex = mortonIndexList[i];
		}
	}
	return success;
}

bool UVoxelGroundUpdater::Clear(const FVector& pos)
{
	SCOPE_CYCLE_COUNTER(STAT_Clear);
	return VoxelDataTree->Clear(pos);
}

void UVoxelGroundUpdater::ClearAll()
{
	VoxelDataTree->ClearAll();
}

int32 UVoxelGroundUpdater::GetCount() const
{
	return VoxelDataTree->GetCount();
}
