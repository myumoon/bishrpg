#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import sys
import optparse
import codecs
import csv

# ninja/miscにパスを通す 
sys.path.append(os.path.join(os.path.dirname(__file__),'..','..','..','thirdparty','ninja','misc'))
from ninja_syntax import Writer

def mappingDirectory(assetPath, resDestRoot):
	contentName    = os.path.splitext(os.path.basename(assetPath))[0]
	contentRelPath = os.path.dirname(os.path.relpath(assetPath, resDestRoot))
	contentPath    = os.path.join("/Game", contentRelPath, contentName)
	#print("mapping asset({}) \nroot({}) \nrel({}) \nname({})".format(assetPath, resDestRoot, contentRelPath, contentName))
	return (assetPath.replace(os.path.sep, "/"), contentPath.replace(os.path.sep, "/"))

def convertGamePathToContentPath(gameDir):
	return gameDir.replace("/Game/", "/Content/") + ".uasset"

def convertFbxPathToTexPath(fbxPath, texExt="png"):
	texPath = fbxPath.replace("Meshes", "Textures")
	texPath = texPath.replace(".fbx", "." + texExt)
	return texPath

def makeMaterialContentPath(fbxPath, resDestRoot):
	assetPath, contentPath = mappingDirectory(fbxPath, resDestRoot)
	return contentPath.replace("/Meshes/", "/Materials/")

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

def getLastModifiedTime(filepath):
	if not os.path.exists(filepath):
		return 0
	return os.path.getmtime(filepath)

def main():	
	print("{}".format(os.path.basename(__file__)))
	current_dir = os.path.abspath(os.path.dirname(__file__))
	parser = optparse.OptionParser(description=u"Make a csv file to be imported.")
	parser.add_option("-o", "--out", default="import_list.csv", help=u"destination csv file path")
	parser.add_option("-d", "--destrootdir", help=u"asset dest root directory")
	parser.add_option("-p", "--projroot", help=u"ue4 project root directory")

	#options, args = parser.parse_args(["root/Content/Characters/Lower/Meshes/Lower_pl000_01.fbx", "root/Content/Characters/Lower/Textures/Lower_pl000_01.png", "root/Content/Characters/Accessory/Meshes/Accessory_pl000_01.fbx", "root/Content/Characters/Accessory/Textures/Accessory_pl000_01.png", "--out", "out.csv"])
	options, args = parser.parse_args()

	with codecs.open(options.out, 'w', 'utf-8') as f:
		csvWriter = csv.writer(f)
		for inFbxPath in args:
			fbxPath, contentFbxPath = mappingDirectory(inFbxPath, options.destrootdir)
			checkContentPath        = os.path.join(options.projroot, convertGamePathToContentPath(contentFbxPath))

			# インポートされていないか更新があったものだけインポート対象にする
			if not os.path.exists(checkContentPath) or getLastModifiedTime(checkContentPath) < getLastModifiedTime(inFbxPath):
				inTexPath               = convertFbxPathToTexPath(inFbxPath)
				texPath, contentTexPath = mappingDirectory(inTexPath, options.destrootdir)
				contentMatPath          = makeMaterialContentPath(inFbxPath, options.destrootdir)
				csvWriter.writerow([fbxPath, contentFbxPath, texPath, contentTexPath, contentMatPath, extractPartsName(fbxPath), makeDestFileName(fbxPath)])
	return 0

if __name__ == "__main__":
	exit(main())
