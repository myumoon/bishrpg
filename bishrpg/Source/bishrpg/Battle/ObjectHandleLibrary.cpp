// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "ObjectHandleLibrary.h"
#include "BattleObjectHandle.h"

FBattleObjectHandle UObjectHandleLibrary::MakeObjectHandle(const FObjectHandleDesc& desc)
{
    return FBattleObjectHandle(desc.Index, desc.ObjectType, desc.PlayerGroup);
}

// プレイヤーかどうかを判定
bool UObjectHandleLibrary::IsPlayer(const FBattleObjectHandle& handle)
{
    return handle.IsManualPlayer();
}

// 相手プレイヤーかどうかを取得
bool UObjectHandleLibrary::IsOpponent(const FBattleObjectHandle& handle)
{
    return handle.IsOpponentPlayer();
}

// プレイヤーグループを取得
EPlayerGroup UObjectHandleLibrary::GetGroup(const FBattleObjectHandle& handle)
{
    return handle.GetGroup();
}

// プレイヤーグループインデックスを取得
int32 UObjectHandleLibrary::GetGroupIndex(const FBattleObjectHandle& handle)
{
    return handle.GetGroupIndex();
}

// オブジェクトインデックスを取得
int32 UObjectHandleLibrary::GetObjectIndex(const FBattleObjectHandle& handle)
{
    return handle.GetObjectIndex();
}
