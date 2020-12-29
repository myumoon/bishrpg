// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelCrackHelper.generated.h"

class USplineComponent;

/**	壁フラグ
*/
enum EWallFlagMask : uint8 {
	ExistBlock = 1 << 0,
	PositiveX  = 1 << 1,
	NegativeX  = 1 << 2,
	PositiveY  = 1 << 3,
	NegativeY  = 1 << 4,
};

/**	地割れ情報
*/
USTRUCT(BlueprintType)
struct FVoxelBlockInfo {
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector CenterPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	int32   WallFlag = 0;
};

USTRUCT(BlueprintType)
struct BISHRPG_API FVoxelBlockCornerContext {
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector Center;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector LU;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector LD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector RU;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector RD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector Min;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	FVector Max;
};

// todo : 
USTRUCT(BlueprintType)
struct BISHRPG_API FVoxelBlockCalcContext {
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	TArray<FVoxelBlockCornerContext> CurveRectBuffer;

	FVector2D SplineMinPos = {};
	FVector2D SplineMaxPos = {};
	float     BlockSize    = 0.0f;
};

/**	ボクセルの地割れ計算
*/
UCLASS()
class BISHRPG_API UVoxelCrackHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVoxelCrackHelper() = default;
	~UVoxelCrackHelper() = default;

	//! 地割れ計算
	//!
	//! スプライン上の点から解析するので早いが急カーブで隙間ができる
	//! 
	//! @param[out] Results    結果
	//! @param[in]  spline     スプラインコンポーネント
	//! @param[in]  startWidth 開始地点の太さ
	//! @param[in]  endWidth   終了地点の太さ
	//! @param[in]  interval   計算間隔(小さい方が精度は高いが重くなる)
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void CalcBlockPlacementsWithLine(TArray<FVoxelBlockInfo>& Results, const USplineComponent* spline, float startWidth, float endWidth, float interval = 20.0f, float blockSize = 100.0f);

	//! 地割れ計算
	//! 
	//! 箱状に地点を解析するので精度は高いがヒット判定をするので少し重い
	//! 
	//! @param[out] Results             結果
	//! @param[in]  spline              スプラインコンポーネント
	//! @param[in]  context             計算情報
	//! @param[in]  startWidth          開始地点の太さ
	//! @param[in]  endWidth            終了地点の太さ
	//! @param[in]  interval            計算間隔(小さい方が精度は高いが重くなる)
	//! @param[in]  setOffset           配置エリア設置時のオフセット
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void CalcBlockPlacementsWithArea(TArray<FVoxelBlockInfo>& Results, UPARAM(ref) FVoxelBlockCalcContext& context, const USplineComponent* spline, float startWidth, float endWidth, float interval = 200.0f, float setOffset = -50.0f, float blockSize = 100.0f);

	//! 地割れの穴の位置をフラグ設定
	//! 
	//! CalcBlockPlacementsWithAreaで計算した結果に対して穴の位置を計算する
	//! この関数を呼び出さずにCalcHolePositionsで計算した座標をResultsに設定することでも同様の結果になる
	//! 
	//! @param[in] Results              結果
	//! @param[in]  context             計算情報
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void UpdateHoleFlag(UPARAM(ref) TArray<FVoxelBlockInfo>& Results, UPARAM(ref) FVoxelBlockCalcContext& context);

	//! 地割れの壁の位置をフラグ設定
	//! 
	//! UpdateHoleFlag相当の穴位置設定を行ったあとに呼ぶこと
	//! 
	//! @param[in] Results              結果
	//! @param[in]  context             計算情報
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void UpdateWallFlag(UPARAM(ref) TArray<FVoxelBlockInfo>& Results, UPARAM(ref) FVoxelBlockCalcContext& context);
	
	//! +X方向に壁を作るか
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsSetWallPositiveX(const FVoxelBlockInfo& info);

	//! -X方向に壁を作るか
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsSetWallNegativeX(const FVoxelBlockInfo& info);

	//! +Y方向に壁を作るか
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsSetWallPositiveY(const FVoxelBlockInfo& info);

	//! -Y方向に壁を作るか
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsSetWallNegativeY(const FVoxelBlockInfo& info);

	//! 穴を作るか
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsSetHole(const FVoxelBlockInfo& info);

	//! インデックスに変換
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static int32 GetBlockLinearIndex(const FVector2D& checkPos, const FVector2D& minPos, const FVector2D& maxPos, float blockSize);

	//! XYのボードサイズを取得
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static void GetBoardSectionsWithArea(int32& x, int32& y, const FVector2D& minPos, const FVector2D& maxPos, float blockSize);

	//! XYのボードサイズを取得
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static void GetBoardSections(int32& x, int32& y, const FVector2D& size, float blockSize);

	//! 軸に対してのインデックスを取得
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static int32 GetBoardIndex(float axisPos, float blockSize);

	//! 軸に対してのインデックスを取得
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static FVector2D FixBlockCenterPos(const FVector2D& pos, float blockSize);

	//! AABB生成
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static FBox2D MakeAABBBySpline(const USplineComponent* spline, float startWidth, float endWidth, float interval);

	//! Splineの幅取得
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static float GetSplineWidth(float length, float distance, float startWidth, float endWidth);

	//! 軸に対してのインデックスを取得
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void DebugDrawVoxelCrack(UObject* WorldContextObject, const FVoxelBlockCalcContext& context, FLinearColor color, float heightOffset = 1.0f, float thickness = 1.0f);
};
