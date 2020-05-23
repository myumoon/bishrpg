// Copyright © 2018 nekoatsume_atsuko. All rights reserved.


#include "MortonIndex.h"

bool UMortonIndexFunctionLibrary::IsValid(const FMortonIndex& mortonIndex)
{
	return (0 <= mortonIndex.Index);
}