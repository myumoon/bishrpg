#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import sys
import optparse
import codecs
import csv

# ninja/miscにパスを通す 
sys.path.append(os.path.join(os.path.dirname(__file__),'..','..','thirdparty','ninja','misc'))
from ninja_syntax import Writer

def extractPartsName(assetPath):
	u"""
	アセットのファイルパスからパーツ情報を抽出
	"""
	filename = os.path.basename(assetPath)
	fileInfo = filename.split("_")
	return fileInfo[0] # ファイル名の先頭にパーツ情報が入っている

def makeDestFileName(assetPath):
	u"""
	アセットのパスからファイル名を生成
	"""
	filename = os.path.basename(assetPath)
	return os.path.splitext(filename)[0]

def main():	
	print("{}".format(os.path.basename(__file__)))
	current_dir = os.path.abspath(os.path.dirname(__file__))
	parser = optparse.OptionParser(description=u"Make a csv file to be imported.")
	parser.add_option("-o", "--out", help=u"destination csv file path")

	#options, args = parser.parse_args(["root/Content/Characters/Lower/Meshes/Lower_pl000_01.fbx", "root/Content/Characters/Lower/Textures/Lower_pl000_01.png", "root/Content/Characters/Accessory/Meshes/Accessory_pl000_01.fbx", "root/Content/Characters/Accessory/Textures/Accessory_pl000_01.png", "--out", "out.csv"])
	options, args = parser.parse_args()

	with codecs.open(options.out, 'w', 'utf-8') as f:
		csvWriter = csv.writer(f)
		for index in range(0, len(args), 2):
			fbxPath = args[index + 0]
			texPath = args[index + 1]
			csvWriter.writerow([fbxPath, texPath, extractPartsName(fbxPath), makeDestFileName(fbxPath)])
	return 0

if __name__ == "__main__":
	exit(main())
