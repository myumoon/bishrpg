// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BattleDataType.generated.h"



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
*/
UENUM(BlueprintType)
enum class EBattleSelectMethod : uint8 {
	None,
    
	// 前方
	Top1,          //!< 先頭1体目
    Top2,          //!< 先頭2体目(それ以下なら最後尾のやつ)
    Top3,          //!< 先頭3体目(それ以下なら最後尾のやつ)
    Top4,          //!< 先頭4体目(それ以下なら最後尾のやつ)
    Top5,          //!< 先頭5体目(それ以下なら最後尾のやつ)
    Top6,          //!< 先頭6体目(それ以下なら最後尾のやつ)
    
	// 目の前
	Ahead1,        //!< 1マス前(先頭じゃないときは不発)
	Ahead4,        //!< 4マス前
    
	// ステータス指定
    AttackTop1,    //!< 攻撃力トップ1
    DeffenceTop1,  //!< 防御力トップ1

	// 属性指定
    RockTop,       //!< Rock先頭
    RockBack,      //!< Rock後方
    SingTop,       //!< Sing先頭
    SingBack,      //!< Sing後方
    HurmorTop,     //!< Hurmor先頭
    HurmotBack,    //!< Hurmor後方
    
	// 指定セル
    Cell_0_Right,  //!< セル
    Cell_0_Center, //!< セル
    Cell_0_Left,   //!< セル
    Cell_1_Right,  //!< セル
    Cell_1_Center, //!< セル
    Cell_1_Left,   //!< セル
    Cell_2_Right,  //!< セル
    Cell_2_Center, //!< セル
    Cell_2_Left,   //!< セル
    Cell_3_Right,  //!< セル
    Cell_3_Center, //!< セル
    Cell_3_Left,   //!< セル
    
    // 複数選択
    // SelectRangeは無視される
	AllCells,      //!< 全マス
    
    // 指定回数ランダム
    Random1,       //!< ランダム
    Random2,       //!< ランダム
    Random3,       //!< ランダム
    Random4,       //!< ランダム
    Random5,       //!< ランダム
    Random6,       //!< ランダム
    Random7,       //!< ランダム
    Random8,       //!< ランダム
    Random9,       //!< ランダム
    Random10,      //!< ランダム
    Random11,      //!< ランダム
    Random12,      //!< ランダム
};

/*! 選択範囲
 */
UENUM(BlueprintType)
enum class EBattleSelectRange : uint8 {
    Single,        //!< 単体
    Col,           //!< 縦1列
    Row,           //!< 横1列
    Ahead1,        //!< 1マス前(先頭じゃないときは不発)
    Ahead4,        //!< 4マス前
    AroundPlus4,   //!< 選択マスと上下左右4マス
    AroundCross4,  //!< 選択マスとななめ4マス
    Around9,       //!< 選択マスと周囲8マス
    AndBack1,      //!< 後ろ1マス
};


