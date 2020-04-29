#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import sys
import optparse
import codecs

# プロジェクト変数
from .. import proj_def

# ninja/miscにパスを通す 
sys.path.append(os.path.join(os.path.dirname(__file__),'..','..','thirdparty','ninja','misc'))
from ninja_syntax import Writer

def main():	
	print("{}".format(os.path.basename(__file__)))
	current_dir = os.path.abspath(os.path.dirname(__file__))
	parser = optparse.OptionParser(description=u"Make a ninja config file.")
	parser.add_option("-o", "--out", default=os.path.join(current_dir, "ninja", "config.ninja"), help=u"ninja config path")

	options, args = parser.parse_args()

	with codecs.open(options.out, 'w', 'utf-8') as f:
		writer = Writer(f)

		writer.comment("設定定義")
		writer.newline()

		# リポジトリルート
		writer.comment("プロジェクトルート")
		writer.variable(key="proj_root", value=proj_def.Root)

		# UE4プロジェクトルート
		writer.comment("UE4プロジェクトルート")
		writer.variable(key="ue4_proj_root", value=proj_def.UE4ProjRoot)

		# ツールディレクトリ
		writer.comment("ツールディレクトリ")
		writer.variable(key="tool_dir", value=proj_def.ToolDir)

		# リソースディレクトリ
		writer.comment("リソースディレクトリ")
		writer.variable(key="res_root", value=proj_def.ResRoot)

		# モデル変換csv
		writer.comment("モデル変換csv")
		writer.variable(key="convert_csv", value=proj_def.ConvertCsvPath)

		# モデル変換用csv生成
		writer.comment("インポートcsv作成")
		writer.variable(key="import_csv_name", value="import_csv_maker.py")

		# モデル変換
		writer.comment("fbx生成")
		writer.variable(key="ply2fbx", value="ply2fbx_recursive.py")

	return 0

if __name__ == "__main__":
	exit(main())
