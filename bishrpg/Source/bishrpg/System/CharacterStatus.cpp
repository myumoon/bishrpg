﻿// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "CharacterStatus.h"



static UDataTable* s_charAssetTbl = nullptr;

FCharacterStatus::FCharacterStatus()
{
	Valid = false;
	//Id = "";
	Lv = 1;
	HpLv = 1;
	AttackLv = 1;
	DeffenceLv = 1;
	SpeedLv = 1;
	PartsFace = -1;
	PartsHair = -1;
	PartsUpper = -1;
	PartsLower = -1;
	PartsAccessory = -1;
	Onemesh = -1;

	// bug : これを実行するとUE4が落ちる・・・
	//Skills.AddUninitialized(static_cast<int32>(ESkillPos::Num));
}

