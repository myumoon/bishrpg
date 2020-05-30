// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Algorithm/QuadTree.h"
#include "Algorithm/MortonIndex.h"
#include "QuadTreeIndexComponent.generated.h"


USTRUCT(BlueprintType)
struct BISHRPG_API FValueMortonPair {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree")
	int32 Value;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuadTree")
	FMortonIndex MortonIndex;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BISHRPG_API UQuadTreeIndexComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuadTreeIndexComponent();

	/*!	指定座標に特定座標を設定
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool Add(FMortonIndex& mortonindex, const FVector& pos, int32 value);

	/*!	特定の値を削除
		@param	value				削除する値
		@param	targetMortonIndex	このモートンインデックスの範囲を対象にする
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool Remove(int32 value, const FMortonIndex& targetMortonIndex);

	/*!	特定の値を差し替え
		@param	from				差し替え元の値
		@param	to					差し替え先の値
		@param	first				最初に見つかったものだけ
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool Replace(int32 from, int32 to, bool first = true);

	//UFUNCTION(BlueprintCallable, Category = "QuadTree")
	//bool Add(FMortonIndex& mortonindex, const FVector& begin, const FVector& end);

	/*!	指定範囲内に登録されている情報を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool Find(TArray<int32>& registered, FMortonIndex& mortonIndex, const FVector& pos) const;

	/*!	指定範囲内に登録されている情報を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool FindRange(TArray<FValueMortonPair>& registered, const FVector& begin, const FVector& end) const;

	/*!	指定範囲内に登録されている情報を取得(球)
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool FindRangeSphere(TArray<FValueMortonPair>& registered, const FVector& center, float r) const;

	/*!	指定モートンインデックスの情報を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool Get(TArray<int32>& registered, const FMortonIndex& mortonIndex) const;

	/*!	指定座標に登録されている情報をすべて削除
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	bool Clear(const FVector& pos);

	/*!	全クリア
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	void ClearAll();

	/*!	指定範囲に登録されている情報をすべて削除
	*/
	//UFUNCTION(BlueprintCallable, Category = "QuadTree")
	//bool ClearRange(const FVector& begin, const FVector& end);

	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	int32 GetCount() const;

	/*!	デバッグ描画
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	void DebugDraw(FLinearColor color, float thickness = 5.0f, float time = -1.0f);

	/*!	デバッグ描画
	*/
	UFUNCTION(BlueprintCallable, Category = "QuadTree")
	void DebugDrawAtConstructionScript(FLinearColor color, float thickness = 5.0f);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void DebugDraw(const FLinearColor& color, float thickness, float time, bool persistant);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuadTree")
	FVector CenterPos; //!< 中心位置

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuadTree")
	float Width;  //!< 平面サイズ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuadTree")
	float Depth;  //!< 平面サイズ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuadTree", meta=(UIMin = '0', UIMax = '4'))
	int32 SeparationLevel;  //!< 分割レベル(0:分割無し, 1:4分割, 2:16分割, 3:64分割, 4:256分割)

private:
	TUniquePtr<QuadTree> QuadTreeCalculator;

	using DataArray = TArray<int32>;
	TArray<DataArray> MortonAlignedDataList;
};
