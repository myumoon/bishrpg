// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**	ゲームのファイルアクセス
 * 
 */
class BISHRPGED_API FileUtil
{
public:
	//! 指定ディレクトリ以下のファイルを全て取得
	static TArray<FString> GetFilesInDirectory(const FString& dir);

	//! ファイルコピー
	static bool CopyFile(const FString& src, const FString& destDir);

	//! フォルダを再帰的にコピー
	static bool CopyTree(const FString& srcDir, const FString& destDir, bool overrideExists = true);
};
