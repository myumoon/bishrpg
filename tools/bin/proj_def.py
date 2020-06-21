#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os

# UE4バージョン
UE4Version = "4.25"

# UE4エンジンディレクトリ
UE4EngineDir = "D:/Program Files/Epic Games/UE_{}/Engine/Binaries/Win64".format(UE4Version)

# リポジトリルート
Root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

# プロジェクト名
ProjectName = "bishrpg"

# UE4プロジェクトルート
UE4ProjRoot = os.path.abspath(os.path.join(Root, ProjectName))

# ツールディレクトリ
ToolDir = os.path.abspath(os.path.join(Root, "tools", "bin"))

# リソースディレクトリ
ResRoot = os.path.abspath(os.path.join(Root, "..", "bishrpg_resources"))

# fbx出力ディレクトリ
DestFbxDir = os.path.join(ResRoot, "models", "_out", "Content")

# 変換時の一時ファイルディレクトリ
TempDir = os.path.abspath(os.path.join(os.path.dirname(__file__), "model_build", "temp"))

# モデル変換csvファイル
ConvertCsvPath = os.path.abspath(os.path.join(TempDir, "convert_list.csv"))

