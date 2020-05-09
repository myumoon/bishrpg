#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import unreal
import character_importer

def main():	
	print("import_character_models.py")
	fileDir = os.path.dirname(os.path.normpath(__file__))
	projDir = os.path.abspath(os.path.join(fileDir, "../../bishrpg/"))
	characterId = "p999"
	partsId     = "01"
	fbxPath = "D:\\prog\\0_myprogram\\bishrpg_resources\\models\\characters\\_out\\Lower\\Meshes\\Lower_{}_{}.fbx".format(characterId, partsId)
	texPath = "D:\\prog\\0_myprogram\\bishrpg_resources\\models\\characters\\_out\\Lower\\Textures\\Lower_{}_{}.png".format(characterId, partsId)
	parts   = "Lower"
	outName = "Lower_{}_{}".format(characterId, partsId)

	importer                = character_importer.CharacterImporter(projDir)
	createdMesh             = importer.importFbx(fbxPath, parts, outName)
	createdTexture          = importer.importTexture(texPath, parts, outName)
	createdMaterialInstance = importer.makeMaterialInstance(createdTexture, parts, outName)@
	return 0

if __name__ == "__main__":
	main()