#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import sys
import json
import unittest

def isOneMesh(plyPath):
	u"""
	ワンメッシュモデルかどうかを判定
	"""
	# ハイフンが無かったらワンメッシュモデル
	return plyPath[-6] != "-"

def getCharacterId(plyPath):
	u"""
	plyのキャラID部を取得

	ex : p000
	"""
	filename = os.path.basename(plyPath)
	return filename[0:4]

def getCharacterVersion(plyPath):
	u"""
	plyのキャラバージョン部を取得

	p002_01-2.ply の01部分
	"""
	filename = os.path.basename(plyPath)
	return filename[5:7]

def getPartsIndexStr(plyPath):
	u"""
	plyのファイル名からインデックス部を取得
	
	p005_03-X.plyのXの部分
	ワンメッシュは-1を返す
	"""
	if isOneMesh(plyPath):
		return "-1"
	return plyPath[-5]

def getPartsIndexInt(plyPath):
	u"""
	plyのファイル名からインデックス部を取得
	
	p005_03-0.plyの0の部分
	"""
	return int(getPartsIndexStr(plyPath))

def getCategoryShort(plyPath):
	u"""
	plyのファイル名からカテゴリ部を取得

	Returns :
		p  : player
		e  : enemy
		n  : npc
	"""
	id       = getCharacterId(plyPath)
	category = id[0]
	return category

def getCategoryLong(plyPath):
	longCategoryMap = { 
		"p" : "Player",
		"e" : "Enemy",
		"n" : "Npc",
	}
	shortCategory   = getCategoryShort(plyPath)
	longCategory    = longCategoryMap[shortCategory] if shortCategory in longCategoryMap else "Others"
	return longCategory

def getPartsName(plyPath):
	u""" plyのパスからパーツ名を取得
	"""
	filename  = os.path.basename(plyPath)
	indexfile = os.path.join(os.path.dirname(plyPath), "plyindex.json")
	if os.path.exists(indexfile):
		with open(indexfile) as f:
			indexJson  = json.load(f)
			partsIndex = getPartsIndexStr(plyPath)
			partsname  = indexJson[partsIndex]
			if partsname != "":
				return normalizePartsName(partsname)
	return getPartsNameDefault(getPartsIndexInt(plyPath))

def getPartsNameDefault(index):
	partsNameList = ["Lower", "Upper", "Face", "Hair", "Accessory", "OneModel"]
	assert index < len(partsNameList), u"getPartsNameDefault : index は {} 未満である必要があります : 引数 {}".format(len(partsNameList), index)
	return partsNameList[index]

def normalizePartsName(partsName):
	if partsName.lower() == "hut":
		return "Accessory"
	return partsName[0].upper() + partsName[1:].lower()

def makeContentsFileName(characterId, version, partsName, ext):
	u"""
	fbxのファイル名を生成
	"""
	if partsName.lower() == "onemodel":
		formatStr = "{}_{}_{}.{}"
	else:
		formatStr = "Parts{}_{}_{}.{}"
	return formatStr.format(partsName, characterId, version, ext)

def makeContentsInfo(plyPath):
	charId    = getCharacterId(plyPath)
	charVer   = getCharacterVersion(plyPath)
	partsName = getPartsName(plyPath)
	return (charId, charVer, partsName)

def makeRelativeContentPath(plyPath, resType, ext):
	u"""
	Parts/Lower/[resType]/PartsLower_pl000_01.[ext]
	OneModel/[resType]/[name].[ext]
	"""
	charId, charVer, partsName = makeContentsInfo(plyPath)
	fileName        = makeContentsFileName(charId, charVer, partsName, ext)
	if partsName.lower() == "onemodel":
		category    = getCategoryLong(plyPath)
		return os.path.join(partsName, category, resType, fileName)
	else:
		return os.path.join("Parts/", partsName, resType, fileName)

def makeRelativeFbxContentsPath(plyPath):
	u"""
	Parts/Lower/Textures/Lower_pl000_01.fbx
	"""
	return makeRelativeContentPath(plyPath, "Meshes", "fbx")

def makeRelativeTexContentsPath(plyPath):
	u"""
	Parts/Lower/Textures/Lower_pl000_01.png
	"""
	return makeRelativeContentPath(plyPath, "Textures", "png")

class TestPlyPath(unittest.TestCase):
	def test_plyindex(self):
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p005_lingling/03_tbs2/p005_03-0.ply", 0, "Accessory")
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p005_lingling/03_tbs2/p005_03-1.ply", 1, "Lower")
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p005_lingling/03_tbs2/p005_03-2.ply", 2, "Upper")
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p005_lingling/03_tbs2/p005_03-3.ply", 3, "Face")
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p005_lingling/03_tbs2/p005_03-4.ply", 4, "Hair")

	def test_default(self):
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p001_hug/01_otnk/p001_01-0.ply", 0, "Lower")
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p001_hug/01_otnk/p001_01-1.ply", 1, "Upper")
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p001_hug/01_otnk/p001_01-2.ply", 2, "Face")
		self.check("D:/prog/0_myprogram/bishrpg_resources/models/characters/p001_hug/01_otnk/p001_01-3.ply", 3, "Hair")

	def test_contents_fbx(self):
		self.assertEqual(makeRelativeFbxContentsPath("D:/prog/0_myprogram/bishrpg_resources/models/characters/p005_lingling/03_tbs2/p005_03-1.ply"), os.path.normpath("Lower/Meshes/Lower_p005_03.fbx"))
		self.assertEqual(makeRelativeFbxContentsPath("D:/prog/0_myprogram/bishrpg_resources/models/characters/p001_hug/01_otnk/p001_01-1.ply"), os.path.normpath("Upper/Meshes/Upper_p001_01.fbx"))

	def test_contents_tex(self):
		self.assertEqual(makeRelativeTexContentsPath("D:/prog/0_myprogram/bishrpg_resources/models/characters/p005_lingling/03_tbs2/p005_03-1.ply"), os.path.normpath("Lower/Textures/Lower_p005_03.png"))
		self.assertEqual(makeRelativeTexContentsPath("D:/prog/0_myprogram/bishrpg_resources/models/characters/p001_hug/01_otnk/p001_01-1.ply"), os.path.normpath("Upper/Textures/Upper_p001_01.png"))

	def check(self, path, index, partsName):
		self.assertEqual(getPartsIndexInt(path), index)
		self.assertEqual(getPartsName(path), partsName)

def main():
	unittest.main()
	return 0

if __name__ == "__main__":
	exit(main())
