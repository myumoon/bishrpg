// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Algorithm/LinearTreeDadaHolder.h"
#include "VoxelGroundUpdater.generated.h"


USTRUCT(BlueprintType)
struct BISHRPG_API FBlockData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuadTree")
	int32 InstanceIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree")
	FMortonIndex MortonIndex;

	bool operator==(const FBlockData& rhs) const
	{
		return (InstanceIndex == rhs.InstanceIndex);
	}
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BISHRPG_API UVoxelGroundUpdater : public USceneComponent
{
	GENERATED_BODY()

	//! ブロックデータ保持
	using TreeAlgorithm   = QuadTree;
	using BlockDataHolder = LinearTreeDadaHolder<FBlockData, TreeAlgorithm>;

public:	
	// Sets default values for this component's properties
	UVoxelGroundUpdater();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*!	指定座標に特定座標を設定
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	bool Add(FMortonIndex& mortonindex, const FVector& pos, const FBlockData& value);

	/*!	特定の値を削除
		@param	value				削除する値
		@param	targetMortonIndex	このモートンインデックスの範囲を対象にする
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	bool Remove(const FMortonIndex& targetMortonIndex, const FBlockData& value);

	/*!	特定の値を差し替え
		@param	from				差し替え元の値
		@param	to					差し替え先の値
		@param	first				最初に見つかったものだけ
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	bool Replace(const FBlockData& from, const FBlockData& to, bool first = true);

	/*!	指定範囲内に登録されている情報を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	bool Find(TArray<FBlockData>& registered, FMortonIndex& mortonIndex, const FVector& pos) const;

	/*!	指定範囲内に登録されている情報を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	bool FindRange(TArray<FBlockData>& registered, const FVector& begin, const FVector& end) const;

	/*!	指定範囲内に登録されている情報を取得(球)
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	bool FindRangeSphere(TArray<FBlockData>& registered, const FVector& center, float r) const;

	/*!	指定座標に登録されている情報をすべて削除
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	bool Clear(const FVector& pos);

	/*!	全クリア
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	void ClearAll();

	/*!	要素数を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelGround")
	int32 GetCount() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelGround")
	FVector CenterPos; //!< 中心位置

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelGround")
	float Width = 5000.0f;  //!< 平面サイズ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelGround")
	float Depth = 5000.0f;  //!< 平面サイズ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelGround", meta = (UIMin = '0', UIMax = '4'))
	int32 SeparationLevel = 4;  //!< 分割レベル(0:分割無し, 1:4分割, 2:16分割, 3:64分割, 4:256分割)

private:
	TUniquePtr<TreeAlgorithm>   AreaTree;
	TUniquePtr<BlockDataHolder> VoxelDataTree;
};
