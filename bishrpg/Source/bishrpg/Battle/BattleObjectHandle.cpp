// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "BattleObjectHandle.h"

#include "Battle/BattleSystem.h"
#include "bishrpg.h"

FBattleObjectHandle::FBattleObjectHandle(int32 objectIndex, EObjectType objectType, EPlayerGroup group)
{
	if(objectIndex < 0) {
		HandleValue = Invald;
		return;
	}

	HandleValue = 
		(objectIndex & static_cast<int32>(BitMask::Index)) << static_cast<int32>(BitField::Index) | 
		(static_cast<int32>(objectType) & static_cast<int32>(BitMask::ObjectType)) << static_cast<int32>(BitField::ObjectType) |
		(static_cast<int32>(group) & static_cast<int32>(BitMask::PlayerGroup)) << static_cast<int32>(BitField::PlayerGroup);
}

// 有効かどうか
bool FBattleObjectHandle::IsValid() const
{
	return (GetHandleValue() != Invald);
}

// 操作プレイヤーかどうかを取得
bool FBattleObjectHandle::IsManualPlayer() const
{
	return _IsPlayerObject(GetHandleValue());
}

// 的プレイヤーかどうかを取得
bool FBattleObjectHandle::IsOpponentPlayer() const
{
	return _IsOpponentObject(GetHandleValue());
}

// プレイヤーグループを取得
EPlayerGroup FBattleObjectHandle::GetGroup() const
{
	return static_cast<EPlayerGroup>(GetGroupIndex());
}

// プレイヤーグループをインデックスで取得
int32 FBattleObjectHandle::GetGroupIndex() const
{
	return _GetGroup(GetHandleValue());
}

// オブジェクトインデックスを取得
int32 FBattleObjectHandle::GetObjectIndex() const
{
	return _GetIndex(GetHandleValue());
}

