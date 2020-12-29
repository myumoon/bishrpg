// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "VoxelCrackHelper.h"

#include "bishrpg.h"
#include "Components/SplineComponent.h"
#include "Math/NumericLimits.h"
#include "DrawDebugHelpers.h"

#define ENABLE_LOG_VOXEL_CRACK_HELPER false
#define ENABLE_MEASURE_TIME_SPAN      false


// 地割れ計算
void UVoxelCrackHelper::CalcBlockPlacementsWithLine(TArray<FVoxelBlockInfo>& Results, const USplineComponent* spline, float startWidth, float endWidth, float interval, float blockSize)
{
#if ENABLE_MEASURE_TIME_SPAN
	FDateTime   startTime = FDateTime::Now();
#endif
	const float spliteLength     = spline->GetSplineLength();
	const int32 splineSliceCount = (int32)(spliteLength / interval);

	// AABB計算
	const auto splineAabb = MakeAABBBySpline(spline, startWidth, endWidth, interval);
	int32 boardX, boardY;
	GetBoardSections(boardX, boardY, splineAabb.GetSize(), blockSize);
	Results.SetNumZeroed(boardX * boardY);

	// マス目計算
	for(int32 i = 0; i < splineSliceCount; ++i) {
		const float   dist      = i * interval;
		const FVector position  = spline->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		FVector       right     = spline->GetRightVectorAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		right.Normalize();

		const float   distRate       = dist / spliteLength;
		const float   witdh          = FMath::Lerp(startWidth, endWidth, distRate);
		const int32   sideCount      = static_cast<int32>(witdh / blockSize);
		const float   sideBlockWidth = witdh / sideCount;

		for(int32 sideIdx = 0; sideIdx < sideCount; ++sideIdx) {
			// 両サイド分
			for(int32 signIdx = 0; signIdx < 2; ++signIdx) {
				const int32     sign     = (signIdx % 2 == 0) ? 1 : -1;
				const FVector   offset   = right * sign * sideBlockWidth * sideIdx - right * sign * sideBlockWidth * 0.5f;
				const FVector2D checkPos = FVector2D(position + offset);
				const FVector2D fixedPos = FixBlockCenterPos(checkPos, blockSize);
				const int32     xIndex   = GetBoardIndex(fixedPos.X - splineAabb.Min.X, blockSize);
				const int32     yIndex   = GetBoardIndex(fixedPos.Y - splineAabb.Min.Y, blockSize);
				const int32     index    = yIndex * boardX + xIndex;
				GAME_ASSERT_FMT(0 <= index && index < Results.Num(), "0 <= index({0}) < size({1}) : checkPos({2}, {3}), min({4}, {5}), max({6}, {7}), blockSize({8})",
					index,
					Results.Num(),
					checkPos.X, checkPos.Y,
					splineAabb.Min.X, splineAabb.Min.Y,
					splineAabb.Max.X, splineAabb.Max.Y,
					blockSize);
				if(index < 0 || Results.Num() <= index) {
					continue;
				}
				Results[index].CenterPos = FVector(fixedPos, 0.0f);
				Results[index].WallFlag  = EWallFlagMask::ExistBlock;
			}
		}
	}
	

#if ENABLE_MEASURE_TIME_SPAN
	FTimespan elapsedTime = FDateTime::Now() - startTime;
	GAME_LOG_FMT("CalcBlockPlacementsWithArray time = {0}ms", elapsedTime.GetTotalMilliseconds());
#endif
}

// 地割れ計算
void UVoxelCrackHelper::CalcBlockPlacementsWithArea(TArray<FVoxelBlockInfo>& Results, UPARAM(ref) FVoxelBlockCalcContext& context, const USplineComponent* spline, float startWidth, float endWidth, float interval, float setOffset, float blockSize)
{
#if ENABLE_MEASURE_TIME_SPAN
	FDateTime   startTime        = FDateTime::Now();
#endif

	const float halfInterval     = interval * 0.5f;
	const float halfBlockSize    = blockSize * 0.5f;
	const float splineLength     = spline->GetSplineLength();
	const float splineSliceCount = splineLength / (interval + setOffset) + 1;

#if ENABLE_LOG_VOXEL_CRACK_HELPER
	GAME_LOG_FMT("@@@CalcBlockPlacementsWithArray interval({0}) spline->GetSplineLength({1}) splineSliceCount({2}) startWidth({3}), endWidth({4}) blockSize({5})", interval, spline->GetSplineLength(), splineSliceCount, startWidth, endWidth, blockSize);
#endif

	// 計算用バッファ確保
	context.CurveRectBuffer.Reset();
	context.CurveRectBuffer.Reserve(splineSliceCount);

	FVector2D splineMinPos = {
		TNumericLimits<float>::Max(),
		TNumericLimits<float>::Max(),
	};
	FVector2D splineMaxPos = {
		TNumericLimits<float>::Min(),
		TNumericLimits<float>::Min(),
	};

	FVoxelBlockCornerContext addCornerInfo;
	for(int32 i = 0; i < splineSliceCount; ++i) {
		// 地割れの幅
		const float   widthRate = static_cast<float>(i) / (splineSliceCount - 1);
		const float   width     = FMath::Lerp(startWidth, endWidth, widthRate);
#if ENABLE_LOG_VOXEL_CRACK_HELPER
		GAME_LOG_FMT("@@@i({0}) width({1})", i, width);
#endif

		// 分割情報
		const float   dist      = i * (interval + setOffset);
		const FVector position  = spline->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		const FVector direction = spline->GetDirectionAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		FVector       right     = spline->GetRightVectorAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		right.Normalize();
		

		// カーブローカル座標
		const FVector posLU     = position + direction * halfInterval - right * width;
		const FVector posLD     = position - direction * halfInterval - right * width;
		const FVector posRU     = position + direction * halfInterval + right * width;
		const FVector posRD     = position - direction * halfInterval + right * width;
#if ENABLE_LOG_VOXEL_CRACK_HELPER
		GAME_LOG_FMT("@@@i({0}), pos({1}, {2}, {3}), dir({4}, {5}, {6}), half({7}), right({8}, {9}, {10}), wid({11})", i, position.X, position.Y, position.Z, direction.X, direction.Y, direction.Z, halfInterval, right.X, right.Y, right.Z, width);
		GAME_LOG_FMT("@@@ posLU({0}, {1}, {2})", posLU.X, posLU.Y, posLU.Z);
		GAME_LOG_FMT("@@@ posLD({0}, {1}, {2})", posLD.X, posLD.Y, posLD.Z);
		GAME_LOG_FMT("@@@ posRU({0}, {1}, {2})", posRU.X, posRU.Y, posRU.Z);
		GAME_LOG_FMT("@@@ posRD({0}, {1}, {2})", posRD.X, posRD.Y, posRD.Z);
#endif

		// 2次元でのAABBを作成
		const float   minX      = FMath::Min(FMath::Min(posLU.X, posLD.X), FMath::Min(posRU.X, posRD.X));
		const float   minY      = FMath::Min(FMath::Min(posLU.Y, posLD.Y), FMath::Min(posRU.Y, posRD.Y));
		const float   maxX      = FMath::Max(FMath::Max(posLU.X, posLD.X), FMath::Max(posRU.X, posRD.X));
		const float   maxY      = FMath::Max(FMath::Max(posLU.Y, posLD.Y), FMath::Max(posRU.Y, posRD.Y));

		addCornerInfo.Center = position;
		addCornerInfo.LU     = posLU;
		addCornerInfo.LD     = posLD;
		addCornerInfo.RU     = posRU;
		addCornerInfo.RD     = posRD;
		addCornerInfo.Min    = FVector(minX, minY, 0.0f);
		addCornerInfo.Max    = FVector(maxX, maxY, 0.0f);
		context.CurveRectBuffer.Add(addCornerInfo);
	
		splineMinPos.X = FMath::Min(splineMinPos.X, minX);
		splineMinPos.Y = FMath::Min(splineMinPos.Y, minY);
		splineMaxPos.X = FMath::Max(splineMaxPos.X, maxX);
		splineMaxPos.Y = FMath::Max(splineMaxPos.Y, maxY);
	}

	context.SplineMinPos = splineMinPos;
	context.SplineMaxPos = splineMaxPos;
	context.BlockSize    = blockSize;

#if ENABLE_MEASURE_TIME_SPAN
	FTimespan elapsedTime = FDateTime::Now() - startTime;
	GAME_LOG_FMT("CalcBlockPlacementsWithArray time = {0}ms", elapsedTime.GetTotalMilliseconds());
#endif
}

// 穴位置計算
void UVoxelCrackHelper::UpdateHoleFlag(UPARAM(ref) TArray<FVoxelBlockInfo>& Results, UPARAM(ref) FVoxelBlockCalcContext& context)
{
	const FVector2D fixedSplineMinPos = {
		GetBoardIndex(context.SplineMinPos.X, context.BlockSize) * context.BlockSize,
		GetBoardIndex(context.SplineMinPos.Y, context.BlockSize) * context.BlockSize,
	};
	const FVector2D fixedSplineMaxPos = {
		GetBoardIndex(context.SplineMaxPos.X, context.BlockSize) * context.BlockSize,
		GetBoardIndex(context.SplineMaxPos.Y, context.BlockSize) * context.BlockSize,
	};

	// ブロック情報格納バッファ初期化
	const int32 separateX      = GetBoardIndex((fixedSplineMaxPos.X - fixedSplineMinPos.X), context.BlockSize);
	const int32 separateY      = GetBoardIndex((fixedSplineMaxPos.Y - fixedSplineMinPos.Y), context.BlockSize);
	const int32 blockAreaCount = separateX * separateY;

#if ENABLE_LOG_VOXEL_CRACK_HELPER
	GAME_LOG_FMT("@@@ separateX({0}) separateY({1}) fixedSplineMinMax({2}, {3} - {4}, {5}) blockSize({6})", separateX, separateY, fixedSplineMinPos.X, fixedSplineMinPos.Y, fixedSplineMaxPos.X, fixedSplineMaxPos.Y, blockSize);
#endif

	Results.Reset();
	Results.SetNumZeroed(blockAreaCount);

	auto isInEdge = [](const FVector2D& start, const FVector2D& end, const FVector2D& point) {
		const FVector2D toEnd   = end - start;
		const FVector2D toPoint = point - start;
		const float     cross   = FVector2D::CrossProduct(toEnd, toPoint);
		return (0 <= cross);
	};
	auto intersectOBBAndPoint = [&isInEdge](const FVector2D& pos1, const FVector2D& pos2, const FVector2D& pos3, const FVector2D& pos4, const FVector2D& point) {
		if(!isInEdge(pos1, pos2, point)) {
			return false;
		}
		if(!isInEdge(pos2, pos3, point)) {
			return false;
		}
		if(!isInEdge(pos3, pos4, point)) {
			return false;
		}
		if(!isInEdge(pos4, pos1, point)) {
			return false;
		}
		return true;
	};

	const float baseOffsetX = -context.SplineMinPos.X;
	const float baseOffsetY = -context.SplineMinPos.Y;
#if ENABLE_LOG_VOXEL_CRACK_HELPER
	GAME_LOG_FMT("@@@ result size({0}) separateX({1}) separateY({2})", Results.Num(), separateX, separateY);
	GAME_LOG_FMT("@@@ baseOffset({0}, {1})", baseOffsetX, baseOffsetY);
	GAME_LOG_FMT("@@@ spline min({0}, {1}) max({2}, {3})", splineMinPos.X, splineMinPos.X, splineMaxPos.X, splineMaxPos.Y);
#endif

	// スプラインの分割情報からブロック情報生成
	for(int32 i = 0; i < context.CurveRectBuffer.Num(); ++i) {
		const FVector& min    = context.CurveRectBuffer[i].Min;
		const FVector& max    = context.CurveRectBuffer[i].Max;

		const int32 minPosXIndex = GetBoardIndex(min.X, context.BlockSize);
		const int32 minPosYIndex = GetBoardIndex(min.Y, context.BlockSize);
		const int32 maxPosXIndex = GetBoardIndex(max.X, context.BlockSize);
		const int32 maxPosYIndex = GetBoardIndex(max.Y, context.BlockSize);
		const int32 blockCountX  = maxPosXIndex - minPosXIndex;
		const int32 blockCountY  = maxPosYIndex - minPosYIndex;

		const FVector2D corner1(context.CurveRectBuffer[i].LD);
		const FVector2D corner2(context.CurveRectBuffer[i].LU);
		const FVector2D corner3(context.CurveRectBuffer[i].RU);
		const FVector2D corner4(context.CurveRectBuffer[i].RD);

		const FVector2D center(context.CurveRectBuffer[i].Center);

#if ENABLE_LOG_VOXEL_CRACK_HELPER
		GAME_LOG_FMT("@@@ ----- i = {0}", i);
		GAME_LOG_FMT("@@@ min({0}, {1}), max({2}, {3})", min.X, min.Y, max.X, max.Y);
		GAME_LOG_FMT("@@@ idx({0}, {1}), idx({2}, {3})", minPosXIndex, minPosYIndex, maxPosXIndex, maxPosYIndex);
		GAME_LOG_FMT("@@@ blockCountX({0}), blockCountY({1})", blockCountX, blockCountY);
		GAME_LOG_FMT("@@@ cen({0}, {1})", center.X, center.Y);
		GAME_LOG_FMT("@@@ co1({0}, {1})", corner1.X, corner1.Y);
		GAME_LOG_FMT("@@@ co2({0}, {1})", corner2.X, corner2.Y);
		GAME_LOG_FMT("@@@ co3({0}, {1})", corner3.X, corner3.Y);
		GAME_LOG_FMT("@@@ co4({0}, {1})", corner4.X, corner4.Y);
#endif

		// スプラインごとの範囲でループ
		// 当たり判定
		// https://yttm-work.jp/collision/collision_0007.html
		for(int32 yIdx = 0; yIdx < blockCountY; ++yIdx) {
			for(int32 xIdx = 0; xIdx < blockCountX; ++xIdx) {
				const FVector2D checkPos = FixBlockCenterPos({min.X + xIdx * context.BlockSize, min.Y + yIdx * context.BlockSize}, context.BlockSize);

#if ENABLE_LOG_VOXEL_CRACK_HELPER
				GAME_LOG_FMT("@@@ min({0}) xIdx({1}) yIdx({2}) blockSize({3})", min.ToString(), xIdx, yIdx, context.BlockSize);
				GAME_LOG_FMT("@@@ co1({0}) co2({1}) co3({2}) co4({3}) check({4})", corner1.ToString(), corner2.ToString(), corner3.ToString(), corner4.ToString(), checkPos.ToString());
#endif
				const int32 index = GetBlockLinearIndex(checkPos, context.SplineMinPos, context.SplineMaxPos, context.BlockSize);
				GAME_ASSERT_FMT(0 <= index && index < Results.Num(), "0 <= index({0}) < size({1}) : checkPos({2}, {3}), min({4}, {5}), max({6}, {7}), blockSize({8})",
					index,
					Results.Num(),
					checkPos.X, checkPos.Y,
					context.SplineMinPos.X, context.SplineMinPos.Y,
					context.SplineMaxPos.X, context.SplineMaxPos.Y,
					context.BlockSize);
				// エラーチェック
				if((index < 0) || (Results.Num() <= index)) {
					continue;
				}
				auto& result = Results[index];

				// すでにチェック済みの場所はスルー
				if(IsSetHole(result)) {
					continue;
				}

				// マスの中央とヒット判定
				if(intersectOBBAndPoint(corner1, corner2, corner3, corner4, checkPos)) {
#if ENABLE_LOG_VOXEL_CRACK_HELPER
					GAME_LOG_FMT("@@@ hit xy({0}, {1}) index({2}) checkpos({3}, {4})", xIdx, yIdx, index, checkPos.X, checkPos.Y);
#endif
					result.WallFlag = EWallFlagMask::ExistBlock;
					result.CenterPos = FVector(checkPos, 0.0f);
				}
			}
		}
	}
}

// 壁位置計算
void UVoxelCrackHelper::UpdateWallFlag(UPARAM(ref) TArray<FVoxelBlockInfo>& Results, UPARAM(ref) FVoxelBlockCalcContext& context)
{
	const FVector2D fixedSplineMinPos = {
			GetBoardIndex(context.SplineMinPos.X, context.BlockSize) * context.BlockSize,
			GetBoardIndex(context.SplineMinPos.Y, context.BlockSize) * context.BlockSize,
	};
	const FVector2D fixedSplineMaxPos = {
		GetBoardIndex(context.SplineMaxPos.X, context.BlockSize) * context.BlockSize,
		GetBoardIndex(context.SplineMaxPos.Y, context.BlockSize) * context.BlockSize,
	};

	const int32 separateX   = GetBoardIndex((fixedSplineMaxPos.X - fixedSplineMinPos.X), context.BlockSize);
	const int32 separateY   = GetBoardIndex((fixedSplineMaxPos.Y - fixedSplineMinPos.Y), context.BlockSize);
	const int32 row         = separateY;
	const int32 col         = separateX;

	const auto isLeftEdge   = [col](int32 index) { return (index % col == 0); };
	const auto isRightEdge  = [col](int32 index) { return ((index + 1) % col == 0); };
	const auto isTopEdge    = [col](int32 index) { return (index / col == 0); };
	const auto isBottomEdge = [col, row](int32 index) { return (index / col == row - 1); };

	for(int32 y = 0; y < row; ++y) {
		for(int32 x = 0; x < col; ++x) {
			const int32 index = x + y * col;
			if((index < 0) || (Results.Num() <= index)) {
				GAME_ERROR("out of index : x(%d), y(%d), index(%d)", x, y, index);
				continue;
			}
			int32& flag = Results[index].WallFlag;

			// 指定方向に壁があれば壁フラグを設定する関数
			const auto setWallFlag = [&flag, &Results, index](EWallFlagMask wall, auto edgeTestFunc, int32 checkIndex) {
				if(!(flag & wall)) {
					if(edgeTestFunc(index)) {
						flag |= wall;
					}
					else {
						flag |= Results[checkIndex].WallFlag & EWallFlagMask::ExistBlock ? 0 : wall;
					}
				}
			};

			if(flag & EWallFlagMask::ExistBlock) {
				setWallFlag(EWallFlagMask::NegativeX, isLeftEdge, index - 1);
				setWallFlag(EWallFlagMask::PositiveX, isRightEdge, index + 1);
				setWallFlag(EWallFlagMask::NegativeY, isTopEdge, index - col);
				setWallFlag(EWallFlagMask::PositiveY, isBottomEdge, index + col);
			}
		}
	}
}

// +X方向に壁を作るか
bool UVoxelCrackHelper::IsSetWallPositiveX(const FVoxelBlockInfo& info)
{
	return (info.WallFlag & EWallFlagMask::PositiveX);
}

// -X方向に壁を作るか
bool UVoxelCrackHelper::IsSetWallNegativeX(const FVoxelBlockInfo& info)
{
	return (info.WallFlag & EWallFlagMask::NegativeX);
}

// +Y方向に壁を作るか
bool UVoxelCrackHelper::IsSetWallPositiveY(const FVoxelBlockInfo& info)
{
	return (info.WallFlag & EWallFlagMask::PositiveY);
}

// -Y方向に壁を作るか
bool UVoxelCrackHelper::IsSetWallNegativeY(const FVoxelBlockInfo& info)
{
	return (info.WallFlag & EWallFlagMask::NegativeY);
}

// 穴を作るか
bool UVoxelCrackHelper::IsSetHole(const FVoxelBlockInfo& info)
{
	return (info.WallFlag & EWallFlagMask::ExistBlock);
}

// インデックスに変換
int32 UVoxelCrackHelper::GetBlockLinearIndex(const FVector2D& checkPos, const FVector2D& minPos, const FVector2D& maxPos, float blockSize)
{
	int32 xBlockCount = 0;
	int32 yBlockCount = 0;

	const FVector2D fixedMinPos = {
		GetBoardIndex(minPos.X, blockSize) * blockSize,
		GetBoardIndex(minPos.Y, blockSize) * blockSize,
	};
	const FVector2D fixedMaxPos = {
		GetBoardIndex(maxPos.X, blockSize) * blockSize,
		GetBoardIndex(maxPos.Y, blockSize) * blockSize,
	};
	GetBoardSectionsWithArea(xBlockCount, yBlockCount, fixedMinPos, fixedMaxPos, blockSize);

	const auto PosituveCheckPos  = checkPos - fixedMinPos;
	
	const int32 x = GetBoardIndex(PosituveCheckPos.X, blockSize);
	const int32 y = GetBoardIndex(PosituveCheckPos.Y, blockSize);

#if ENABLE_LOG_VOXEL_CRACK_HELPER
	GAME_LOG_FMT("GetBlockLinearIndex index({0}) xy({1}, {2}) blockCount({3}, {4})", y * xBlockCount + x, x, y, xBlockCount, yBlockCount);
	GAME_LOG_FMT("                    checkpos({0}, {1}) min-max({2}, {3} - {4}, {5}) fixedMinPos({6}, {7}) fizedMaxPos({8}, {9})", 
		checkPos.X, checkPos.Y,
		minPos.X, minPos.Y,
		maxPos.X, maxPos.Y,
		fixedMinPos.X, fixedMinPos.Y, 
		fixedMaxPos.X, fixedMaxPos.Y);
#endif

	return y * xBlockCount + x;
}

// XYのボードサイズを取得
void UVoxelCrackHelper::GetBoardSectionsWithArea(int32& x, int32& y, const FVector2D& minPos, const FVector2D& maxPos, float blockSize)
{
	const auto localMax = maxPos - minPos;
	GetBoardSections(x, y, localMax, blockSize);
}

// XYのボードサイズを取得
void UVoxelCrackHelper::GetBoardSections(int32& x, int32& y, const FVector2D& size, float blockSize)
{
	x = FMath::CeilToInt(size.X / blockSize);
	y = FMath::CeilToInt(size.Y / blockSize);
}

// XYのボードサイズを取得
int32 UVoxelCrackHelper::GetBoardIndex(float axisPos, float blockSize)
{
	const int32 index      = static_cast<int32>(axisPos / blockSize);
	const int32 fixedIndex = (0 <= axisPos) ? index : index - 1;
	return fixedIndex;
}

// ボクセル用に位置補正
FVector2D UVoxelCrackHelper::FixBlockCenterPos(const FVector2D& pos, float blockSize)
{
	const float halfBlockSize = blockSize * 0.5f;
	const int32 xIdx = GetBoardIndex(pos.X, blockSize);
	const int32 yIdx = GetBoardIndex(pos.Y, blockSize);
	return { xIdx * blockSize + halfBlockSize, yIdx * blockSize + halfBlockSize };
}

// SplineからAABBを作成
FBox2D UVoxelCrackHelper::MakeAABBBySpline(const USplineComponent* spline, float startWidth, float endWidth, float interval)
{
	FVector2D min = {
		TNumericLimits<float>::Max(),
		TNumericLimits<float>::Max()
	};
	FVector2D max = {
		TNumericLimits<float>::Min(),
		TNumericLimits<float>::Min()
	};

	const float length        = spline->GetSplineLength();
	const int32 separateCount = static_cast<int32>(length / interval) + 1;

	for(int32 i = 0; i < separateCount; ++i) {
		const float   dist      = FMath::Min(i * interval, length);
		const FVector position  = spline->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		FVector       right     = spline->GetRightVectorAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
		right.Normalize();

		const float width = GetSplineWidth(length, dist, startWidth, endWidth);
		const auto r = position + right * width;
		const auto l = position - right * width;
		min.X = FMath::Min3(min.X, l.X, r.X);
		min.Y = FMath::Min3(min.Y, l.Y, r.Y);
		max.X = FMath::Max3(max.X, l.X, r.X);
		max.Y = FMath::Max3(max.Y, l.Y, r.Y);
	}

	return FBox2D(min, max);
}


// Splineの幅取得
float UVoxelCrackHelper::GetSplineWidth(float length, float distance, float startWidth, float endWidth)
{
	const float rate  = FMath::Clamp(distance / length, 0.0f, 1.0f);
	const float width = FMath::Lerp(startWidth, endWidth, rate);
	return width;
}


// ボクセル用に位置補正
void UVoxelCrackHelper::DebugDrawVoxelCrack(UObject* WorldContextObject, const FVoxelBlockCalcContext& context, FLinearColor color, float heightOffset, float thickness)
{
	constexpr bool  persistant = true;
	constexpr float time       = 0.1f;

	UWorld* world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	FlushPersistentDebugLines(world);

	const FVector offset = { 0.0f, 0.0f, heightOffset };

	for(const auto& rect : context.CurveRectBuffer) {
		const FVector center = rect.Center + offset;
		const FVector ld     = rect.LD + offset;
		const FVector lu     = rect.LU + offset;
		const FVector ru     = rect.RU + offset;
		const FVector rd     = rect.RD + offset;
		DrawDebugLine(world, center, ld, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		DrawDebugLine(world, ld,     lu, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		DrawDebugLine(world, lu,     ru, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		DrawDebugLine(world, ru,     rd, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
		DrawDebugLine(world, rd,     ld, color.ToFColor(true), persistant, time, (uint8)'\000', thickness);
	}
}
