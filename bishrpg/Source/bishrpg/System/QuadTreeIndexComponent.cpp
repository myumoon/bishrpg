// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "QuadTreeIndexComponent.h"
#include "DrawDebugHelpers.h"
#include "bishrpg.h"

namespace {
	class TimeSpan {
	public:
		TimeSpan(const FString& label)
		{
			Label = label;
			StartTime = FDateTime::Now();
		}
		~TimeSpan()
		{
			FTimespan RemainingTimespan = FDateTime::Now() - StartTime;
			double RemainingSeconds = RemainingTimespan.GetTotalMilliseconds();
			GAME_LOG("Timespan(%s) %fms", *Label, RemainingSeconds);
		}

		FString   Label;
		FDateTime StartTime;
	};

}


// Sets default values for this component's properties
UQuadTreeIndexComponent::UQuadTreeIndexComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	TimeSpan span("reset");
	QuadTreeCalculator.Reset(new QuadTree());
}


// Called when the game starts
void UQuadTreeIndexComponent::BeginPlay()
{
	Super::BeginPlay();

	const float   halfWidth = Width / 2.0f;
	const float   halfDepth = Depth / 2.0f;
	const FVector begin = { CenterPos.X - halfWidth, CenterPos.Y - halfDepth, 0.0f };
	const FVector end   = { CenterPos.X + halfWidth, CenterPos.Y + halfDepth, 0.0f };
	QuadTreeCalculator->Initialize(begin, end, SeparationLevel);

	const int32 size = QuadTreeCalculator->GetLinearSpaceSize();

	MortonAlignedDataList.AddDefaulted(size);
}


// Called every frame
void UQuadTreeIndexComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UQuadTreeIndexComponent::Add(FMortonIndex& mortonindex, const FVector& pos, int32 value)
{
	const int32 mortonIndex = QuadTreeCalculator->CalcLinearSpaceMortonIndex(pos);
	if(mortonIndex < 0) {
		return false;
	}

	GAME_LOG("[Add] pos(%f, %f, %f) -> morton[%d].list[%d] = value(%d)", pos.X, pos.Y, pos.Z, mortonIndex, MortonAlignedDataList[mortonIndex].Num() - 1, value);
	MortonAlignedDataList[mortonIndex].Add(value);
	
	return true;
}

bool UQuadTreeIndexComponent::Remove(int32 value, const FMortonIndex& targetMortonIndex)
{
	if(!UMortonIndexFunctionLibrary::IsValid(targetMortonIndex)) {
		return false;
	}
	
	auto& list = MortonAlignedDataList[targetMortonIndex.Index];
	const int32 removedCount = list.Remove(value);

	GAME_LOG("[Remove] morton[%d].Remove(%d) -> removedCount(%d), listCount(%d)", targetMortonIndex.Index, value, removedCount, list.Num());

	return (0 < removedCount);
}


bool UQuadTreeIndexComponent::Replace(int32 from, int32 to, bool first)
{
	TimeSpan span("Replace");
	for(int32 mortonIdx = 0; mortonIdx < MortonAlignedDataList.Num(); ++mortonIdx) {
		auto& list = MortonAlignedDataList[mortonIdx];
		for(int32 i = 0; i < list.Num(); ++i) {
			if(list[i] == from) {
				//GAME_LOG("[Replace] morton[%d].list[%d].value(%d==from(%d)) = %d, listNum(%d)", mortonIdx, i, list[i], from, to, list.Num());
				list[i] = to;
				if(first) {
					return true;
				}
			}
		}
	}

	return false;
}


bool UQuadTreeIndexComponent::Find(TArray<int32>& registered, FMortonIndex& mortonIndex, const FVector& pos) const
{
	const int32 index = QuadTreeCalculator->CalcLinearSpaceMortonIndex(pos);
	mortonIndex.Index = index;
	GAME_LOG("[Find] pos(%f, %f, %f), morton(%d)", pos.X, pos.Y, pos.Z, index);
	if(index < 0) {
		return false;
	}
	
	registered = MortonAlignedDataList[index];

	return true;
}

bool UQuadTreeIndexComponent::FindRange(TArray<FValueMortonPair>& registered, const FVector& begin, const FVector& end) const
{
	auto traverser(QuadTreeCalculator->GetDepthTraverser(begin, end));

	FValueMortonPair addMortonInfo;
	auto visitor   = [this, &registered, &addMortonInfo](int32 linearSpaceMortonIndex) -> bool {
		GAME_ASSERT(linearSpaceMortonIndex < MortonAlignedDataList.Num());
		for(int32 value : MortonAlignedDataList[linearSpaceMortonIndex]) {
			addMortonInfo.Value             = value;
			addMortonInfo.MortonIndex.Index = linearSpaceMortonIndex;
			registered.Add(addMortonInfo);
		}
		return true;
	};
	traverser.Traverse(visitor);

	return true;
}

bool UQuadTreeIndexComponent::Get(TArray<int32>& registered, const FMortonIndex& mortonIndex) const
{
	if(!UMortonIndexFunctionLibrary::IsValid(mortonIndex)) {
		return false;
	}

	registered = MortonAlignedDataList[mortonIndex.Index];

	return true;
}

bool UQuadTreeIndexComponent::Clear(const FVector& pos)
{
	const int32 index = QuadTreeCalculator->CalcLinearSpaceMortonIndex(pos);
	if(index < 0) {
		return false;
	}

	MortonAlignedDataList[index].Reset();

	return true;
}

void UQuadTreeIndexComponent::ClearAll()
{
	for(auto& list : MortonAlignedDataList) {
		list.Reset();
	}
}

int32 UQuadTreeIndexComponent::GetCount() const
{
	int32 count = 0;

	for(const auto& list : MortonAlignedDataList) {
		count += list.Num();
	}

	return count;
}

void UQuadTreeIndexComponent::DebugDraw(FLinearColor color, float thickness, float time)
{
	DebugDraw(color, thickness, time, false);
}

void UQuadTreeIndexComponent::DebugDrawAtConstructionScript(FLinearColor color, float thickness)
{
	// 線が残るので消す
	FlushPersistentDebugLines(GetWorld());

	DebugDraw(color, thickness, -1.0f, true);
}

void UQuadTreeIndexComponent::DebugDraw(const FLinearColor& color, float thickness, float time, bool persistant)
{
	const int32 sepCount  = QuadTree::CalcSideSeparationCount(SeparationLevel);
	const float cellWidth = Width / sepCount;
	const float cellDepth = Depth / sepCount;

	{
		const FVector begin = {CenterPos.X - Width / 2, CenterPos.Y - Depth / 2, CenterPos.Z};
		const FVector end  = {CenterPos.X - Width / 2, CenterPos.Y + Depth / 2, CenterPos.Z};

		for(int32 w = 0; w < sepCount + 1; ++w) {
			const FVector offset = {cellWidth * w, 0.0f, 0.0f};
			GAME_LOG("(%f, %f, %f) (%f, %f, %f)", begin.X, begin.Y, begin.Z, end.X, end.Y, end.Z)
				DrawDebugLine(GetWorld(), begin + offset, end + offset, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		}
	}

	{
		const FVector begin = {CenterPos.X - Width / 2, CenterPos.Y - Depth / 2, CenterPos.Z};
		const FVector end = {CenterPos.X + Width / 2, CenterPos.Y - Depth / 2, CenterPos.Z};
		for(int32 d = 0; d < sepCount + 1; ++d) {
			const FVector offset = {0.0f, cellDepth * d, 0.0f};
			DrawDebugLine(GetWorld(), begin + offset, end + offset, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		}
	}
}