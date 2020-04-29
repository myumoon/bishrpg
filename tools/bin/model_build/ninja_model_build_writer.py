#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import sys
import optparse
import codecs
import glob


import ply_path

sys.path.append(os.path.join(os.path.dirname(__file__),'..'))
import proj_def

sys.path.append(os.path.join(os.path.dirname(__file__),'..','..','thirdparty','ninja','misc'))
from ninja_syntax import Writer

def main():	
	print("{}".format(os.path.basename(__file__)))
	current_dir = os.path.abspath(os.path.dirname(__file__))
	parser = optparse.OptionParser(description=u"Make a ninja build file.")
	parser.add_option("-o", "--out", default=os.path.join(current_dir, "ninja", "build.ninja"), help=u"ninja config path")

	args, options = parser.parse_args()

	with codecs.open(args.out, 'w', 'utf-8') as f:
		writer = Writer(f)

		writer.comment("ビルド設定")
		writer.newline()
		
		writer.comment("plyをfbxにビルド")
		fbxList = []
		for path in glob.glob(os.path.join(proj_def.ResRoot, "models", "characters") + "/**/**.ply", recursive=True):
			fbxPath    = ply_path.makeRelativeFbxContentsPath(path)
			#texPath = ply_path.makeRelativeTexContentsPath(path)
			ue4FbxPath = os.path.join("$ue4_proj_root", "Content", "Characters", fbxPath)
			inputPath  = os.path.join("$res_root", os.path.relpath(path, proj_def.ResRoot))
			writer.build(outputs=[ue4FbxPath], rule="ply2fbx", inputs=[inputPath], implicit=None)
			fbxList.append(ue4FbxPath)

		writer.comment("インポート用csvを作成")
		writer.build(outputs=["$convert_csv"], rule="make_import_csv", inputs=fbxList, implicit=None)

		writer.comment("UE4にインポート")
		writer.build(outputs=None, rule="import_ue4", inputs="$convert_csv", implicit=None)
	return 0

if __name__ == "__main__":
	exit(main())
