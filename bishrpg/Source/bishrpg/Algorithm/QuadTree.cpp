// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "QuadTree.h"

#include "AutomationTest.h"
#include "bishrpg.h"

// 統計情報
DECLARE_STATS_GROUP(TEXT("QuadTreeStat"), STATGROUP_QuadTree, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("QuadTree::DepthTraverser::DepthTraverser"), STAT_DepthTraverser, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::DepthTraverser::Traverse"), STAT_Traverse, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::DepthTraverser::TraverseToTarget"), STAT_TraverseToTarget, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::DepthTraverser::TraverseChildren"), STAT_TraverseChildren, STATGROUP_QuadTree);

DECLARE_CYCLE_STAT(TEXT("QuadTree::QuadTree"), STAT_QuadTree, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::CalcMortonIndex"), STAT_CalcMortonIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::CalcLinearSpaceMortonIndex"), STAT_CalcLinearSpaceMortonIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::CalcLinearSpaceIndex"), STAT_CalcLinearSpaceIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::IsInRange"), STAT_IsInRange, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::IsInRangePartially"), STAT_IsInRangePartially, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::GetDepthTraverser"), STAT_GetDepthTraverser, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::GetSpaceLevel"), STAT_GetSpaceLevel, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::CalcSideSeparationCount"), STAT_CalcSideSeparationCount, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::IsValidLinearSpaceMortonIndex"), STAT_IsValidLinearSpaceMortonIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::Clamp"), STAT_Clamp, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::WidenBit"), STAT_WidenBit, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::GetMortonIndex"), STAT_GetMortonIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::GetCommonLevel"), STAT_GetCommonLevel, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::GetAxisIndex"), STAT_GetAxisIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::CalcCommonMortonIndex"), STAT_CalcCommonMortonIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::GetSpaceLevelByLinearSpaceIndex"), STAT_GetSpaceLevelByLinearSpaceIndex, STATGROUP_QuadTree);
DECLARE_CYCLE_STAT(TEXT("QuadTree::ConvertToLinearSpaceMortonIndex"), STAT_ConvertToLinearSpaceMortonIndex, STATGROUP_QuadTree);


QuadTree::DepthTraverser::DepthTraverser(QuadTree* tree, int32 spaceMortonIndex) :
	Tree(tree),
	SpaceMortonIndex(spaceMortonIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_DepthTraverser);
	int32 parentIndex = Tree->GetLinearSpaceParentIndex(SpaceMortonIndex);
	while(0 <= parentIndex) {
		//GAME_ASSERT_FMT(Parents.Num() <= Parents.GetAllocatedSize(), "Size={0}, parentIndex={1}", Parents.GetAllocatedSize(), parentIndex);
		Parents.Insert(parentIndex, 0);
		parentIndex = Tree->GetLinearSpaceParentIndex(parentIndex);
	}
	
}

QuadTree::DepthTraverser::DepthTraverser(DepthTraverser&& traverser) :
	Tree(traverser.Tree),
	SpaceMortonIndex(traverser.SpaceMortonIndex),
	Parents(MoveTemp(traverser.Parents))
{
}


void QuadTree::DepthTraverser::Traverse(QuadTree::IVisitor* visitor)
{
	SCOPE_CYCLE_COUNTER(STAT_Traverse);
	// ルートからターゲットまで
	if(!TraverseToTarget(visitor, 0)) {
		return;
	}

	// 自身の空間
	if(!visitor->Visit(SpaceMortonIndex)) {
		return;
	}

	// 子空間
	TraverseChildren(visitor, SpaceMortonIndex);
}

void QuadTree::DepthTraverser::Traverse(TFunction<bool(int32)> visitor)
{
	//GAME_LOG("Traverse");

	SCOPE_CYCLE_COUNTER(STAT_Traverse);

	{
	//DEBUG_SCOPE_TIME_SPAN("ToTarget")
	// ルートからターゲットまで
	if(!TraverseToTarget(visitor, 0)) {
		return;
	}
	}

	{
	//DEBUG_SCOPE_TIME_SPAN("visit")
	// 自身の空間
	if(!visitor(SpaceMortonIndex)) {
		return;
	}
	}

	{
	//DEBUG_SCOPE_TIME_SPAN("TraverseChildren")
	// 子空間
	TraverseChildren(visitor, SpaceMortonIndex);
	}
}

bool QuadTree::DepthTraverser::TraverseToTarget(IVisitor* visitor, uint32 linearSpaceMortonIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TraverseToTarget);

	for(int32 mortonIndex : Parents) {
		if(!visitor->Visit(mortonIndex)) {
			return false;
		}
	}
	return true;
}

bool QuadTree::DepthTraverser::TraverseToTarget(TFunction<bool(int32)> visitor, uint32 linearSpaceMortonIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TraverseToTarget);

	for(int32 mortonIndex : Parents) {
		//GAME_LOG_FMT("TraverseToTarget morton({0})", mortonIndex);
		if(!visitor(mortonIndex)) {
			return false;
		}
	}
	return true;
}

void QuadTree::DepthTraverser::TraverseChildren(IVisitor* visitor, uint32 linearSpaceMortonIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TraverseChildren);

	// 子空間
	const int32 childTopIndex = QuadTree::GetLinearSpaceChildIndex(linearSpaceMortonIndex);

	// 子階層をトラバース
	for(int32 i = 0; i < 4; ++i) {
		const int32 childMortonIndex = childTopIndex + i;
		if(!visitor->Visit(childMortonIndex)) {
			continue;
		}
		TraverseChildren(visitor, childMortonIndex);
	}
}

void QuadTree::DepthTraverser::TraverseChildren(TFunction<bool(int32)> visitor, uint32 linearSpaceMortonIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TraverseChildren);

	//GAME_LOG_FMT("TraverseChildren linearSpaceMortonIndex({0})", linearSpaceMortonIndex);

	// 子空間
	const int32 childTopIndex = QuadTree::GetLinearSpaceChildIndex(linearSpaceMortonIndex);
	if(!Tree->IsValidLinearSpaceMortonIndex(childTopIndex)) {
		//GAME_LOG_FMT("TraverseChildren end linearSpaceMortonIndex({0}), childTopIndex({1})", linearSpaceMortonIndex, childTopIndex);
		return;
	}

	// 子階層をトラバース
	for(int32 i = 0; i < 4; ++i) {
		const int32 childMortonIndex = childTopIndex + i;
		if(!visitor(childMortonIndex)) {
			continue;
		}
		TraverseChildren(visitor, childMortonIndex);
	}
}

QuadTree::QuadTree(const FVector& begin, const FVector& end, int32 separateLevel)
{
	SCOPE_CYCLE_COUNTER(STAT_QuadTree);

	Initialize(begin, end, separateLevel);

	TArray<uint32, TFixedAllocator<GetLinearSpaceSize(4)>> a;
	//GAME_LOG("size = %d", a.GetAllocatedSize());
}

void QuadTree::Initialize(const FVector& begin, const FVector& end, int32 separateLevel)
{
	SeparateLevel = separateLevel;
	BeginXY       = begin;
	EndXY         = end;

	// 配列のインデックスに変換する場合に使うオフセットを事前計算
	for(int32 i = 0; i <= MaxSeparationLevel; ++i) {
		LevelOffsets.Add(static_cast<int32>((FMath::Pow(4, i) - 1) / 3));
	}
	SeparationNum                  = CalcSideSeparationCount();
}

uint32 QuadTree::CalcSideSeparationCount() const
{
	return CalcSideSeparationCount(SeparateLevel);
}

uint32 QuadTree::CalcSideSeparationCount(int32 separationLevel)
{
	SCOPE_CYCLE_COUNTER(STAT_CalcSideSeparationCount);

	return static_cast<int32>(FMath::Pow(2, separationLevel));
}

bool QuadTree::IsValid() const
{
	const FVector toEnd = EndXY - BeginXY;
	const bool initialized = (0 < LevelOffsets.Num());
	const bool validRange  = (0.0f < toEnd.X) && (0.0f < toEnd.Y);
	const bool validLevel  = ((0 < SeparateLevel) && (SeparateLevel <= MaxSeparationLevel));
	
	return initialized && validRange && validLevel;
}

int32 QuadTree::CalcMortonIndex(const FVector& pos) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetMortonIndex);

	if(!IsInRange(pos)) {
		return -1;
	}
	const uint32 xAxisIndex  = GetAxisIndex(pos.X, BeginXY.X, EndXY.X);
	const uint32 yAxisIndex  = GetAxisIndex(pos.Y, BeginXY.Y, EndXY.Y);
	//GAME_LOG("xAxisIndex %u yAxisIndex %u", xAxisIndex, yAxisIndex);
	const uint64 mortonIndex = GetMortonIndex(xAxisIndex, yAxisIndex);

	return mortonIndex;
}

int32 QuadTree::CalcLinearSpaceMortonIndex(const FVector& pos) const
{
	SCOPE_CYCLE_COUNTER(STAT_CalcLinearSpaceMortonIndex);

	const int32 mortonIndex = CalcMortonIndex(pos);
	if(mortonIndex < 0) {
		return -1;
	}
	const int32 linearSpaceMortonIndex = ConvertToLinearSpaceMortonIndex(SeparateLevel, mortonIndex);

	return linearSpaceMortonIndex;
}

int32 QuadTree::CalcLinearSpaceIndex(const FVector& begin, const FVector& end) const
{
	SCOPE_CYCLE_COUNTER(STAT_CalcLinearSpaceIndex);

	//GAME_LOG("CalcLinearSpaceIndex begin(%f, %f), end(%f, %f)", begin.X, begin.Y, end.X, end.Y);
	FVector clampedBegin, clampedEnd;
	if(!Clamp(clampedBegin, clampedEnd, begin, end)) {
		return -1;
	}
	
	//GAME_LOG("CalcLinearSpaceIndex success clamp");
	const int32  beginMortonIndex  = CalcMortonIndex(clampedBegin);
	//GAME_LOG("CalcLinearSpaceIndex beginMortonIndex %d", beginMortonIndex);
	const int32  endMortonIndex    = CalcMortonIndex(clampedEnd);
	//GAME_LOG("CalcLinearSpaceIndex endMortonIndex %d", endMortonIndex);

	const uint32 commonLevel       = GetCommonLevel(beginMortonIndex, endMortonIndex);
	//GAME_LOG("CalcLinearSpaceIndex commonLevel %u", commonLevel);
	const uint32 commonMortonIndex = CalcCommonMortonIndex(beginMortonIndex, commonLevel);
	//GAME_LOG("CalcLinearSpaceIndex commonMortonIndex %u", commonMortonIndex);
	const int32  spaceIndex        = ConvertToLinearSpaceMortonIndex(commonLevel, commonMortonIndex);
	//GAME_LOG("CalcLinearSpaceIndex spaceIndex %d", spaceIndex);
	
	return spaceIndex;
}

bool QuadTree::IsInRange(const FVector& pos) const
{
	SCOPE_CYCLE_COUNTER(STAT_IsInRange);

	return IsInRange(pos.X, BeginXY.X, EndXY.X) && IsInRange(pos.Y, BeginXY.Y, EndXY.Y);
}

bool QuadTree::IsInRangePartially(const FVector& begin, const FVector& end) const
{
	SCOPE_CYCLE_COUNTER(STAT_IsInRangePartially);

	const float X1 = begin.X;
	const float Y1 = begin.Y;
	const float X2 = BeginXY.X;
	const float Y2 = BeginXY.Y;
	return ((X2 <= end.X) && (X1 <= EndXY.X) && (Y2 <= end.Y) && (Y1 <= EndXY.Y));

	// beginとendが反転していたら失敗
	if((end.X < begin.X) || (end.Y < begin.Y)) {
		GAME_LOG("end.X %f < begin.X %f, end.Y %f < begin.Y %f", end.X, begin.X, end.Y, begin.Y);
		return false;
	}

	// 部分的にかかっていなかったら失敗
	const FVector& leftUp    = begin;
	const FVector  leftDown  = { begin.X, end.Y, 0.0f };
	const FVector  rightUp   = { end.X, begin.Y, 0.0f };
	const FVector& rightDown = end;
	
	if(!IsInRange(leftUp) && !IsInRange(leftDown) && !IsInRange(rightUp) && !IsInRange(rightDown)) {
		//GAME_LOG("!IsInRange begin %s, !IsInRange end %s", !IsInRange(begin) ? TEXT("out") : TEXT("in"), !IsInRange(end) ? TEXT("out") : TEXT("in"));
		return false;
	}

	return true;
}

bool QuadTree::IsInRange(float point, float begin, float end) const
{
	SCOPE_CYCLE_COUNTER(STAT_IsInRange);
	return (begin <= point) && (point <= end);
}

bool QuadTree::Clamp(FVector& clampedBegin, FVector& clampedEnd, const FVector& begin, const FVector& end) const
{
	SCOPE_CYCLE_COUNTER(STAT_Clamp);
	if(!IsInRangePartially(begin, end)) {
		return false;
	}
	
	clampedBegin.X = FMath::Max(begin.X, BeginXY.X);
	clampedBegin.Y = FMath::Max(begin.Y, BeginXY.Y);
	clampedEnd.X   = FMath::Min(end.X, EndXY.X - 0.001f); // End丁度だとインデックスが繰り上がるので調整
	clampedEnd.Y   = FMath::Min(end.Y, EndXY.Y - 0.001f);
	//GAME_LOG("Clamped (%f, %f) (%f, %f)", clampedBegin.X, clampedBegin.Y, clampedEnd.X, clampedEnd.Y);

	return true;
}

uint64 QuadTree::WidenBit(uint32 num) const
{
	SCOPE_CYCLE_COUNTER(STAT_WidenBit);
	uint32 result = 0;
	for(int32 i = 0; i < SeparateLevel; ++i) {
		result |= (num & (1 << i)) << i;
	}
	return result;
}

uint64 QuadTree::GetMortonIndex(uint32 xIndex, uint32 yIndex) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetMortonIndex);
	return WidenBit(xIndex) | WidenBit(yIndex) << 1;
}

uint32 QuadTree::GetCommonLevel(uint64 mortonIndex1, uint64 mortonIndex2) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetCommonLevel);
	uint64 xorIndex    = mortonIndex1 ^ mortonIndex2;
	return GetSpaceLevel(xorIndex);
}

uint32 QuadTree::GetSpaceLevel(uint32 mortonIndex) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetSpaceLevel);
	int32  commonLevel = SeparateLevel;
	while(0 < mortonIndex) {
		mortonIndex = mortonIndex >> 2;
		--commonLevel;
	}
	return commonLevel;
}

uint32 QuadTree::GetAxisIndex(float point, float begin, float end) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetAxisIndex);
	const float  width = end - begin;
	const uint32 index = static_cast<uint32>((point - begin) / (width / SeparationNum));

	return index;
}

uint32 QuadTree::CalcCommonMortonIndex(uint32 beginMortonIndex, uint32 commonLevel) const
{
	SCOPE_CYCLE_COUNTER(STAT_CalcCommonMortonIndex);
	return beginMortonIndex >> ((SeparateLevel - commonLevel) * 2);
}

QuadTree::DepthTraverser QuadTree::GetDepthTraverser(const FVector& point)
{
	SCOPE_CYCLE_COUNTER(STAT_GetDepthTraverser);
	const int32 mortonIndex = CalcMortonIndex(point);
	return DepthTraverser(this, mortonIndex);
}

QuadTree::DepthTraverser QuadTree::GetDepthTraverser(const FVector& begin, const FVector& end)
{
	SCOPE_CYCLE_COUNTER(STAT_GetDepthTraverser);
	const int32 mortonIndex = CalcLinearSpaceIndex(begin, end);
	
	//GAME_LOG("QuadTree::GetDepthTraverser morton(%d)", mortonIndex);
	
	auto traverser = DepthTraverser(this, mortonIndex);

	//GAME_LOG("QuadTree::GetDepthTraverser ret");

	return traverser;
}

uint32 QuadTree::GetSpaceLevelByLinearSpaceIndex(uint32 linearSpaceMortonIndex) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetSpaceLevelByLinearSpaceIndex);
	for(int32 i = 0; i < LevelOffsets.Num() - 1; ++i) {
		if((LevelOffsets[0] <= linearSpaceMortonIndex) && (linearSpaceMortonIndex < LevelOffsets[i + 1])) {
			return i;
		}
	}
	return LevelOffsets.Num() - 1;
}

uint32 QuadTree::ConvertToLinearSpaceMortonIndex(uint32 level, uint32 mortonIndex) const
{
	SCOPE_CYCLE_COUNTER(STAT_ConvertToLinearSpaceMortonIndex);
	GAME_ASSERT_FMT(level < static_cast<uint32>(LevelOffsets.Num()), "level:{0} Num:{1}", level, LevelOffsets.Num());
	return LevelOffsets[level] + mortonIndex;
}

uint32 QuadTree::GetLinearSpaceSize() const
{
	//GAME_LOG("SeparationNum:%d, SeparateLevel:%d", SeparationNum, SeparateLevel)
	return GetLinearSpaceSize(SeparateLevel);
}

bool QuadTree::IsValidLinearSpaceMortonIndex(int32 linearSpaceMortonIndex) const
{
	const int32 cellNum        = SeparationNum * SeparationNum;
	const int32 maxMortonIndex = ConvertToLinearSpaceMortonIndex(SeparateLevel, cellNum);

	return ((0 <= linearSpaceMortonIndex) && (linearSpaceMortonIndex < maxMortonIndex));
}

FVector QuadTree::GetMinSpaceSize() const
{
	const FVector range = EndXY - BeginXY;
	return (range / SeparationNum);
}

