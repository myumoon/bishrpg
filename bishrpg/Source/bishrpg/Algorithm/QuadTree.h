// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**	
 *  四分木
 */
class BISHRPG_API QuadTree
{
private:
	// 最大空間分割レベル
	static constexpr uint32 MaxSeparationLevel = 8;

public:
	
	/*!	ツリー探索時の訪問者
	*/
	class IVisitor {
	public:
		virtual ~IVisitor() = default;

		// トラバーサーの内容を取得
		// @retval true 
		virtual bool Visit(uint32 linearSpaceMortonIndex) = 0;
	};

	// トラバーサー
	class ITraverser {
	public:
		virtual void Traverse(IVisitor* visitor) = 0;
	};

	// 深さ優先トラバーサー
	class DepthTraverser : public ITraverser
	{
	public:
		DepthTraverser() = default;
		DepthTraverser(QuadTree* tree, int32 targetSpaceMortonIndex);
	
		void Traverse(IVisitor* visitor) override;

	private:
		bool TraverseToTarget(IVisitor* visitor, uint32 linearSpaceMortonIndex);
		void TraverseChildren(IVisitor* visitor, uint32 linearSpaceMortonIndex);
	private:
		using ParentIndexArray = TArray<int32, TFixedAllocator<MaxSeparationLevel>>;

		QuadTree*        Tree             = nullptr;
		int32            SpaceMortonIndex = -1;
		ParentIndexArray Parents;
	};
	
public:
	QuadTree() = default;
	QuadTree(const FVector& begin, const FVector& end, int32 separateLevel = 4);
	~QuadTree() = default;

	/*!	初期化
		@param	begin			範囲開始(xy)
		@param	begin			範囲終了(xy)
		@param	separateLevel	分割レベル(1～8)
	*/
	void Initialize(const FVector& begin, const FVector& end, int32 separateLevel);

	/*! 有効な設定がされているか
	*/
	bool IsValid() const;

	/*!	点のモートンインデックスを計算
		@retval	範囲外の場合は-1を返す
	*/
	int32 CalcMortonIndex(const FVector& pos) const;

	/*!	線形空間でのモートンインデックスを計算
		@retval	範囲外の場合は-1を返す
	*/
	int32 CalcLinearSpaceMortonIndex(const FVector& pos) const;

	/*!	空間のモートンインデックスを計算
		@retval	範囲外の場合は-1を返す
	*/
	int32 CalcLinearSpaceIndex(const FVector& begin, const FVector& end) const;

	/*!	空間の範囲内に入っているか
	*/
	bool IsInRange(const FVector& pos) const;

	/*!	一部でも空間の範囲内に入っているか
	*/
	bool IsInRangePartially(const FVector& begin, const FVector& end) const;
	
	/*!	親空間と子空間のインデックスリストを返すトラバーサーを取得(幅優先)
	*/
	DepthTraverser GetDepthTraverser(const FVector& point);

	/*!	親空間と子空間のインデックスリストを返すトラバーサーを取得(幅優先)
	*/
	DepthTraverser GetDepthTraverser(const FVector& begin, const FVector& end);

	/*!	空間レベルを取得
	*/
	uint32 GetSpaceLevel(uint32 mortonIndex) const;
		
	/*!	空間配列サイズを取得
	*/
	static constexpr uint32 GetLinearSpaceSize(uint32 separationLevel)
	{
		uint32 size = 0;
		for(uint32 i = 0; i <= separationLevel; ++i) {
			size += (1 << (i * 2));
		}
		return size;
	}

	/*!	線形空間での親インデックスを取得
	*/
	static constexpr uint32 GetLinearSpaceParentIndex(uint32 currentIndex)
	{
		return (currentIndex - 1) / 4;
	}

	/*!	線形空間での子インデックスの開始インデックスを取得
	*/
	static constexpr uint32 GetLinearSpaceChildIndex(uint32 currentIndex)
	{
		return (currentIndex * 4) + 1;
	}

private:

	// 範囲内に入っているかどうか
	bool IsInRange(float point, float begin, float end) const;

	// クランプ
	bool Clamp(FVector& clampedBegin, FVector& clampedEnd, const FVector& begin, const FVector& end) const;

	// ビットの間隔を開ける
	uint64 WidenBit(uint32 num) const;

	// モートンインデックスを取得
	uint64 GetMortonIndex(uint32 xIndex, uint32 yIndex) const;

	// 2つのモートンインデックスの共有空間レベルを取得
	uint32 GetCommonLevel(uint64 mortonIndex1, uint64 mortonIndex2) const;

	// 軸に対するインデックスを取得
	uint32 GetAxisIndex(float point, float begin, float end) const;

	// 共有モートン番号を計算
	uint32 CalcCommonMortonIndex(uint32 beginMortonIndex, uint32 commonLevel) const;

	// 空間レベルを取得
	uint32 GetSpaceLevelByLinearSpaceIndex(uint32 linearSpaceMortonIndex) const;

	// 線形空間モートン番号に変換
	uint32 ConvertToLinearSpaceMortonIndex(uint32 level, uint32 mortonIndex) const;

private:
	int32   SeparateLevel = 0; //!< 分割レベル。2~n個分割する
	FVector BeginXY;           //!< 範囲開始座標
	FVector EndXY;             //!< 範囲終了座標
	
	
	template<std::size_t Size>
	using MortonOffsetArray = TArray<uint32, TFixedAllocator<Size>>;

	// 事前計算
	MortonOffsetArray<MaxSeparationLevel> LevelOffsets;      //!< 線形空間への変換オフセット
	int32                                 SeparationNum = 0; //!< 分割数
};
