// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "VoxelGroundUpdater.h"
#include "DrawDebugHelpers.h"

// 統計情報
DECLARE_STATS_GROUP(TEXT("UVoxelGroundUpdaterStat"), STATGROUP_VoxelGroundUpdater, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Find"), STAT_VoxelGroundUpdater_Find, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRange"), STAT_VoxelGroundUpdater_FindRange, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRangeCopy"), STAT_VoxelGroundUpdater_FindRangeCopy, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRangeSphere"), STAT_VoxelGroundUpdater_FindRangeSphere, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::FindRangeSphereCopy"), STAT_VoxelGroundUpdater_FindRangeSphereCopy, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Replace"), STAT_VoxelGroundUpdater_Replace, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Remove"), STAT_VoxelGroundUpdater_Remove, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Clear"), STAT_VoxelGroundUpdater_Clear, STATGROUP_VoxelGroundUpdater);
DECLARE_CYCLE_STAT(TEXT("UVoxelGroundUpdater::Add"), STAT_VoxelGroundUpdater_Add, STATGROUP_VoxelGroundUpdater);


void UBlockDataFunctionLibrary::Sort(TArray<FBlockData>& blockDataArray)
{
	blockDataArray.Sort();
}

void UBlockDataFunctionLibrary::ReverseSort(TArray<FBlockData>& blockDataArray)
{
	blockDataArray.Sort([](const FBlockData& lhs, const FBlockData& rhs) { return rhs.InstanceIndex < lhs.InstanceIndex; });
}

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
	SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_Add);
	mortonindex = VoxelDataTree->Add(pos, value);
	return UMortonIndexFunctionLibrary::IsValid(mortonindex);
}

bool UVoxelGroundUpdater::Remove(const FMortonIndex& targetMortonIndex, const FBlockData& value)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_Remove);
	return VoxelDataTree->Remove(targetMortonIndex, value);
}

bool UVoxelGroundUpdater::Replace(const FBlockData& from, const FBlockData& to, bool first)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_Replace);
	return VoxelDataTree->Replace(from, to, first);
}

bool UVoxelGroundUpdater::Find(TArray<FBlockData>& registered, FMortonIndex& mortonIndex, const FVector& pos) const
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_Find);
	return VoxelDataTree->Find(registered, mortonIndex, pos);
}

bool UVoxelGroundUpdater::FindRange(TArray<FBlockData>& registered, const FVector& begin, const FVector& end) const
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_FindRange);
	TArray<FMortonIndex> mortonIndexList;
	const bool success = VoxelDataTree->FindRange(registered, mortonIndexList, begin, end);
	if(success) {
		SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_FindRangeCopy);
		for(int32 i = 0; i < mortonIndexList.Num(); ++i) {
			registered[i].MortonIndex = mortonIndexList[i];
		}
	}
	return success;
}

bool UVoxelGroundUpdater::FindRangeSphere(TArray<FBlockData>& registered, const FVector& center, float r) const
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_FindRangeSphere);
	TArray<FMortonIndex> mortonIndexList;
	const bool success = VoxelDataTree->FindRange(registered, mortonIndexList, center, r);
	if(success) {
		SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_FindRangeSphereCopy);
		for(int32 i = 0; i < mortonIndexList.Num(); ++i) {
			registered[i].MortonIndex = mortonIndexList[i];
		}
	}
	return success;
}

bool UVoxelGroundUpdater::Clear(const FVector& pos)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelGroundUpdater_Clear);
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

void UVoxelGroundUpdater::DrawDebugArea(FLinearColor color, float heightOffset, float thickness)
{
#if WITH_EDITOR
	// 線が残るので消す
	FlushPersistentDebugLines(GetWorld());

	const int32 sepCount = QuadTree::CalcSideSeparationCount(SeparationLevel);
	const float cellWidth = Width / sepCount;
	const float cellDepth = Depth / sepCount;
	const bool  persistant = true;
	const float time = 0.1f;

	{
		const FVector begin = {CenterPos.X - Width / 2, CenterPos.Y - Depth / 2, CenterPos.Z + heightOffset};
		const FVector end = {CenterPos.X - Width / 2, CenterPos.Y + Depth / 2, CenterPos.Z + heightOffset};

		for(int32 w = 0; w < sepCount + 1; ++w) {
			const FVector offset = {cellWidth * w, 0.0f, 0.0f};
			//GAME_LOG("(%f, %f, %f) (%f, %f, %f)", begin.X, begin.Y, begin.Z, end.X, end.Y, end.Z);
			DrawDebugLine(GetWorld(), begin + offset, end + offset, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		}
	}

	{
		const FVector begin = {CenterPos.X - Width / 2, CenterPos.Y - Depth / 2, CenterPos.Z + heightOffset};
		const FVector end = {CenterPos.X + Width / 2, CenterPos.Y - Depth / 2, CenterPos.Z + heightOffset};
		for(int32 d = 0; d < sepCount + 1; ++d) {
			const FVector offset = {0.0f, cellDepth * d, 0.0f};
			DrawDebugLine(GetWorld(), begin + offset, end + offset, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		}
	}
#endif
}