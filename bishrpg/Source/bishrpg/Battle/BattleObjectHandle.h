// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

//#include "Runtime/Core/Public/GenericPlatform/GenericPlatform.h"
#include "CoreMinimal.h"
#include "Battle/BattleDataType.h"

#include "BattleObjectHandle.generated.h"

class UBattleSystem;

/**
 *	オブジェクトハンドル
 */
USTRUCT( BlueprintType )
struct BISHRPG_API FBattleObjectHandle
{
	GENERATED_BODY()

	/*
		ビット列
		------------+-----------------+
		|    bit    |      desc       |
		------------+-----------------+
		|  1- 4 bit | Character Index |
		|  5- 8 bit | Object Type     |
		|  9-15 bit | [Empty]         |
		|    16 bit | EPlayerGroup    |
		|           |   0:Player      |
		|           |   1:Opponent    |
		| 17-32     | [Reserve]       |
		------------+-----------------+
	*/
	enum class BitField : uint8 {
		Index        = 0,
		ObjectType   = 4,
		PlayerGroup  = 15,
	};
	enum class BitMask : uint8 {
		Index       = 0x000f,
		ObjectType  = 0x000f,
		PlayerGroup = 0x0001,
	};
	static constexpr int32 Mask(int32 id, BitField field, BitMask mask)
	{
		return (id >> static_cast<int32>(field) & static_cast<int32>(mask));
	}
	static constexpr int32 _GetIndex(int32 id)
	{
		return Mask(id, BitField::Index, BitMask::Index);
	}
	static constexpr int32 _GetObjectType(int32 id)
	{
		return Mask(id, BitField::ObjectType, BitMask::ObjectType);
	}
	static constexpr int32 _GetGroup(int32 id)
	{
		return Mask(id, BitField::PlayerGroup, BitMask::PlayerGroup);
	}
	static constexpr bool _IsPlayerObject(int32 id)
	{
		return (Mask(id, BitField::PlayerGroup, BitMask::PlayerGroup) == static_cast<int32>(EPlayerGroup::One));
	}
	static constexpr bool _IsOpponentObject(int32 id)
	{
		return (Mask(id, BitField::PlayerGroup, BitMask::PlayerGroup) == static_cast<int32>(EPlayerGroup::Two));
	}

public:
	//! 無効な値
	static constexpr int32 Invald = -1;

public:
	FBattleObjectHandle() = default;
	FBattleObjectHandle(const FBattleObjectHandle& handle) = default;
	FBattleObjectHandle(int32 objectIndex, EObjectType objectType, EPlayerGroup group);
	~FBattleObjectHandle() = default;

	FBattleObjectHandle& operator=(const FBattleObjectHandle& handle) = default;
	bool operator==(const FBattleObjectHandle& rhs) const
	{
		// 無効なハンドルのときは中身を見ずに判定
		if(!IsValid() && !rhs.IsValid()) {
			return true;
		}
		return (HandleValue == rhs.HandleValue);
	}
	bool operator!=(const FBattleObjectHandle& rhs) const
	{
		// 無効なハンドルのときは中身を見ずに判定
		if(!IsValid() && !rhs.IsValid()) {
			return false;
		}
		return (HandleValue != rhs.HandleValue);
	}

	/*!	有効かどうか
	*/
	//UFUNCTION(BlueprintCallable, Category = "Battle")
	bool IsValid() const;

	/*!	通しセル番号を取得
	 * @retval 0～12
	 */
	//int32 GetCellIndex() const;

	/*!	通しキャラ番号を取得
	*	@retval 0～キャラ数
	*/
	//int32 GetCharacterIndex() const;

	/*!	操作プレイヤーかどうかを取得
	*/
	bool IsManualPlayer() const;

	/*!	相手プレイヤーかどうかを取得
	*/
	bool IsOpponentPlayer() const;

	/*!	プレイヤーグループを取得
	*/
	EPlayerGroup GetGroup() const;

	/*!	プレイヤーグループインデックスを取得
	*/
	int32 GetGroupIndex() const;

	/*!	オブジェクトインデックスを取得
	*/
	int32 GetObjectIndex() const;

	/*!	ハンドルのハッシュ値を取得
	*/
	uint32 CalcHash() const;
	
private:
	int32 GetHandleValue() const { return HandleValue; }

private:
	UBattleSystem*	BattleSystem	= nullptr;
	int32			HandleValue		= Invald;
};

