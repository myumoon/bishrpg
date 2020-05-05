#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import sys
import optparse
import codecs

# ninja/miscにパスを通す 
sys.path.append(os.path.join(os.path.dirname(__file__),'..','..','..','thirdparty','ninja','misc'))
from ninja_syntax import Writer

def main():	
	print("{}".format(os.path.basename(__file__)))
	current_dir = os.path.abspath(os.path.dirname(__file__))
	parser = optparse.OptionParser(description=u"Make a ninja rule file.")
	parser.add_option("-o", "--out", default=os.path.join(current_dir, "ninja", "rule.ninja"), help=u"ninja config path")
	parser.add_option("-c", "--config", default="config.ninja", help=u"ninja config path")

	options, args = parser.parse_args()

	if not os.path.exists(os.path.dirname(options.out)):
		os.makedirs(os.path.dirname(options.out))

	with codecs.open(options.out, 'w', 'utf-8') as f:
		writer = Writer(f)

		writer.comment("ルール定義")
		writer.newline()

		# configインクルード
		writer.include(options.config)
		writer.newline()

		# ---- ルール定義 ----

		# UE4インポート
		commandStr = '"${ue4_cmd}" ${ue4_proj_file} -run=CharacterModelImporter -csv=$in -stdout -UTF8Output'
		writer.rule(name=u"import_ue4", command=commandStr, description=u"UE4にインポート")
		writer.newline()

		# インポート用csv生成
		commandStr = "python $import_csv_name $in --out $out --destrootdir $res_dest --projroot $ue4_proj_root";
		writer.rule(name=u"make_import_csv", command=commandStr, description=u"UE4にインポートするファイル一覧csvを生成")
		writer.newline()

		# plyからfbxへの変換
		commandStr = "python $ply2fbx $in --destDir $res_dest/Characters --workDir $temp_dir --texSize 256"
		writer.rule(name=u"convert_ply", command=commandStr, description=u"plyフォルダをfbxに変換")
		writer.newline()

	return 0

if __name__ == "__main__":
	exit(main())
