#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os

# リポジトリルート
Root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

# UE4プロジェクトルート
UE4ProjRoot = os.path.abspath(os.path.join(Root, 'bishrpg'))

# ツールディレクトリ
ToolDir = os.path.abspath(os.path.join(Root, 'tools', 'bin'))

# リソースディレクトリ
ResRoot = os.path.abspath(os.path.join(Root, '..', 'bishrpg_resources'))

# モデル変換csv
TempDir        = os.path.abspath(os.path.join(os.path.dirname(__file__), "model_build", 'temp'))
ConvertCsvPath = os.path.abspath(os.path.join(TempDir, 'convert_list.csv'))

