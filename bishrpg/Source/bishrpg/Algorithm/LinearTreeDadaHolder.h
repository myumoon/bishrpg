// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MortonIndex.h"
#include "QuadTree.h"
#include "bishrpg.h"

/**
 *	線形ツリーのデータホールダー
 * 
 * Algorithm使用するモートン順序アルゴリズム
 *   要求する関数
 *   - GetLinearSpaceSize()
 *   - CalcLinearSpaceMortonIndex(const FVector& pos)
 *   - GetDepthTraverser(const FVector& begin, const FVector& end)
 */
template<class T, class Algorithm = QuadTree>
class BISHRPG_API LinearTreeDadaHolder
{
public:
	using TElement          = T;
	using TElementContainer = TArray<TElement>;
	using TElementArray     = TArray<TElementContainer>;

	//! モートン番号とのペア
	struct FElementMortonPair {
		TElement     Element;
		FMortonIndex MortonIndex;
	};

public:
	LinearTreeDadaHolder(Algorithm& algorithm) : TreeAlgorithm(&algorithm)
	{
	}
	~LinearTreeDadaHolder() = default;

	/*!	初期化
		この関数はAlgorithmの初期化後に呼ばれる必要がある
	*/
	void Initialize()
	{
		const int32 size = TreeAlgorithm->GetLinearSpaceSize();
		TreeElements.AddDefaulted(size);
	}

	/*!	要素の追加
	*/
	FMortonIndex Add(const FVector& pos, const TElement& value)
	{
		int32 mortonIndex = TreeAlgorithm->CalcLinearSpaceMortonIndex(pos);

		// 範囲外のときはルート空間に所属させる
		if(!IsValidLinearTreeIndex(mortonIndex)) {
			GAME_WARNING_FMT("pos({0}, {1}, {2}) is outside of Morton space so added it to the root space.", pos.X, pos.Y, pos.Z);
			mortonIndex = 0;
		}
		TreeElements[mortonIndex].Add(value);

		return FMortonIndex { mortonIndex };
	}

	/*!	要素の削除
	*/
	bool Remove(const FMortonIndex& targetMortonIndex, const TElement& value)
	{
		if(!UMortonIndexFunctionLibrary::IsValid(targetMortonIndex)) {
			return false;
		}
		auto&       list         = TreeElements[targetMortonIndex.Index];
		const int32 removedCount = list.Remove(value);

		return (0 < removedCount);
	}

	/*!	指定要素を置換
	*/
	bool Replace(const TElement& from, const TElement& to, bool first = false)
	{
		bool replaced = false;

		for(int32 mortonIdx = 0; mortonIdx < TreeElements.Num(); ++mortonIdx) {
			auto& list = TreeElements[mortonIdx];
			for(int32 i = 0; i < list.Num(); ++i) {
				if(list[i] == from) {
					list[i] = to;
					if(first) {
						return true;
					}
					replaced = true;
				}
			}
		}
		return replaced;
	}

	/*!	座標から要素リストを取得
	*/
	bool Find(TElementContainer& emelents, FMortonIndex& mortonIndex, const FVector& pos) const
	{
		const int32 index = TreeAlgorithm->CalcLinearSpaceMortonIndex(pos);
		if(!IsValidLinearTreeIndex(index)) {
			return false;
		}
		mortonIndex.Index = index;
		emelents = TreeElements[index];
		return true;
	}

	/*!	範囲から要素リストを取得
	*/
	bool FindRange(TElementContainer& foundElements, TArray<FMortonIndex>& mortonIndexList, const FVector& begin, const FVector& end) const
	{
		auto traverser = TreeAlgorithm->GetDepthTraverser(begin, end);

		auto visitor = [this, &foundElements, &mortonIndexList](int32 linearSpaceMortonIndex) -> bool {
			// 範囲外ならスルー
			if(!IsValidLinearTreeIndex(linearSpaceMortonIndex)) {
				return false;
			}
			for(const T& element : TreeElements[linearSpaceMortonIndex]) {
				foundElements.Add(element);
				mortonIndexList.Add(FMortonIndex{linearSpaceMortonIndex});
			}
			return true;
		};
		traverser.Traverse(visitor);

		return true;
	}

	/*!	範囲から要素リストを取得
	*/
	bool FindRange(TElementContainer& foundElements, TArray<FMortonIndex>& mortonIndexList, const FVector& center, float radius) const
	{
		const FVector begin = { center.X - radius, center.Y - radius,  center.Z - radius };
		const FVector end   = { center.X + radius, center.Y + radius,  center.Z + radius };
		return FindRange(foundElements, mortonIndexList, begin, end);
	}

	/*!	指定モートン番号の要素リストを取得
	*/
	bool Get(TElementContainer& foundElement, const FMortonIndex& mortonIndex) const
	{
		if(!UMortonIndexFunctionLibrary::IsValid(mortonIndex)) {
			return false;
		}

		foundElement = TreeElements[mortonIndex.Index];

		return true;
	}

	/*!	指定モートン番号の要素を削除
	*/
	bool Clear(const FMortonIndex& mortonIndex)
	{
		if(!UMortonIndexFunctionLibrary::IsValid(mortonIndex)) {
			return false;
		}
		TreeElements[mortonIndex.Index].Reset();

		return true;
	}

	/*!	指定座標の要素を削除
	*/
	bool Clear(const FVector& pos)
	{
		const int32 index = TreeAlgorithm->CalcLinearSpaceMortonIndex(pos);
		if(!IsValidLinearTreeIndex(index)) {
			return false;
		}
		return Clear(FMortonIndex { index });
	}

	/*!	全要素をクリア
	*/
	void ClearAll()
	{
		for(auto& list : TreeElements) {
			list.Reset();
		}
	}

	/*!	全要素の個数を取得
	*/
	int32 GetCount() const
	{
		int32 count = 0;
		for(const auto& list : TreeElements) {
			count += list.Num();
		}
		return count;
	}

	/*!	要素の個数を取得
	*/
	int32 GetCount(const FMortonIndex& mortonIndex) const
	{
		if(!UMortonIndexFunctionLibrary::IsValid(mortonIndex)) {
			return false;
		}
		return TreeElements[mortonIndex.Index].Num();
	}

	/*!	有効なインデックスか
	*/
	bool IsValidLinearTreeIndex(int32 index) const
	{
		return (0 <= index) && (index < TreeElements.Num());
	}

private:
	Algorithm*      TreeAlgorithm = nullptr;
	TElementArray   TreeElements;
};
