// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BattleDataType.generated.h"

/*!	プレイヤーグループ
*/
UENUM(BlueprintType)
enum class EPlayerGroup : uint8 {
	One,     //!< 1P
	Two,     //!< 2P

	Max,
};
constexpr int32 MaxGroupNum = static_cast<int32>(EPlayerGroup::Max);

constexpr EPlayerGroup InvertGroup(EPlayerGroup group)
{
    return group == EPlayerGroup::One ? EPlayerGroup::Two : EPlayerGroup::One;
}
constexpr int32 ToIndex01(EPlayerGroup group)
{
    static_assert(static_cast<int32>(EPlayerGroup::Max) == 2, "EPlayerGroup size have to be 2");
    return static_cast<int32>(group);
}
constexpr bool IsPlayerOne(EPlayerGroup group)
{
    return (group == EPlayerGroup::One);
}

/*!	オブジェクトタイプ
*/
UENUM(BlueprintType)
enum class EObjectType : uint8 {
    None,          //!< なし
	Character,     //!< キャラ
	Item,          //!< アイテム

	Max,
};

/*! 行動パターン
*/
UENUM(BlueprintType)
enum class EBattleActionType : uint8 {
	None,
	Attack,    //!< 攻撃
	Skill,     //!< スキル
	Move,      //!< 移動
};

/*! 行動エフェクト
*/
UENUM(BlueprintType)
enum class EBattleActionEffect : uint8 {
	None,
	KnockBack,     //!< ノックバック
	Stan,          //!< スタン
	BufAttack,     //!< 攻撃力上昇
	BufDeffence,   //!< 防御力上昇
	DebufAttack,   //!< 攻撃力減少
	DebufDeffence, //!< 防御力減少
};

/*! 選択方法
    並び順を変えるときは FBattleParty::SelectTarget のテーブルも変えること！
*/
UENUM(BlueprintType)
enum class EBattleSelectMethod : uint8 {
	None,
    
    // --- 敵 ---
	// 前方
	E_Top1,          //!< 先頭1体目
    E_Top2,          //!< 先頭2体目(それ以下なら最後尾のやつ)
    E_Top3,          //!< 先頭3体目(それ以下なら最後尾のやつ)
    E_Top4,          //!< 先頭4体目(それ以下なら最後尾のやつ)
    E_Top5,          //!< 先頭5体目(それ以下なら最後尾のやつ)
    E_Top6,          //!< 先頭6体目(それ以下なら最後尾のやつ)
    
	// 目の前
    E_FacedTop1,     //!< 正面にいる敵
	E_Ahead1,        //!< 1マス前(先頭じゃないときは不発)
	E_Ahead4,        //!< 4マス前
    
	// ステータス指定
    E_AttackTop1,    //!< 攻撃力トップ1
    E_DeffenceTop1,  //!< 防御力トップ1

	// 属性指定
    E_RockTop,       //!< Rock先頭
    E_RockBack,      //!< Rock後方
    E_SingTop,       //!< Sing先頭
    E_SingBack,      //!< Sing後方
    E_HurmorTop,     //!< Hurmor先頭
    E_HurmorBack,    //!< Hurmor後方
    
	// 指定セル
    E_Cell_0_Faced,  //!< 正面のセル
    E_Cell_0_Right,  //!< セル
    E_Cell_0_Center, //!< セル
    E_Cell_0_Left,   //!< セル
    E_Cell_1_Right,  //!< セル
    E_Cell_1_Center, //!< セル
    E_Cell_1_Left,   //!< セル
    E_Cell_2_Right,  //!< セル
    E_Cell_2_Center, //!< セル
    E_Cell_2_Left,   //!< セル
    E_Cell_3_Right,  //!< セル
    E_Cell_3_Center, //!< セル
    E_Cell_3_Left,   //!< セル
    
    // 複数選択
    // SelectRangeは無視される
	E_AllCells,      //!< 全マス
    
    // 指定回数ランダム
    E_Random1,       //!< ランダム
    E_Random2,       //!< ランダム
    E_Random3,       //!< ランダム
    E_Random4,       //!< ランダム
    E_Random5,       //!< ランダム
    E_Random6,       //!< ランダム
    E_Random7,       //!< ランダム
    E_Random8,       //!< ランダム
    E_Random9,       //!< ランダム
    E_Random10,      //!< ランダム
    E_Random11,      //!< ランダム
    E_Random12,      //!< ランダム

    // --- 味方 ---
    P_Myself,        //!< 自身の位置
    P_Front1,        //!< 1マス前
    P_Top1,          //!< 最善
    P_Back1,          //!< 最善
    P_All,           //!< 全員

    Max,
};

/*! 選択範囲
    並び順を変えるときは FBattleParty::ExpandCell のテーブルも変えること！
 */
UENUM(BlueprintType)
enum class EBattleSelectRange : uint8 {
    Single,        //!< 単体
    Col,           //!< 縦1列
    Row,           //!< 横1列
    Side,          //!< 左右
    AroundPlus4,   //!< 選択マスと上下左右4マス
    AroundCross4,  //!< 選択マスとななめ4マス
    Around9,       //!< 選択マスと周囲8マス
    Back1,         //!< 後ろ1マス
    Back2,         //!< 後ろ2マス
    Back3,         //!< 後ろ3マス
    Back4,         //!< 後ろ4マス

    Max,
};


