// Copyright © 2018 nekoatsume_atsuko. All rights reserved.
#pragma once

#include "CoreMinimal.h"

namespace Battle {

class Def {
public:
    static const int32 INVALID_CELL_NO = -1;
    static const int32 BOARD_ROW = 4;
    static const int32 BOARD_COL = 3;
    static const int32 COL_LEFT = 0;
    static const int32 COL_CENTER = 1;
    static const int32 COL_RIGHT = 2;
    static const int32 MAX_BOARD_CELLS = BOARD_COL * BOARD_ROW;
};

}
