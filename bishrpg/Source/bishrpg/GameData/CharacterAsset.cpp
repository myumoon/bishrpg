﻿// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "CharacterAsset.h"



UTexture* UCharacterAssetUtil::LoadIcon(TSoftObjectPtr<UTexture> texture)
{
	return texture.LoadSynchronous();
}
