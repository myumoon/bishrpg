#!usr/bin/python
# -*- coding: utf-8 -*-
# 
# convert ply to fbx
# ply2fbx [in.ply] [out.fbx] [work_blender_path] [--hair|--hair_origin|--face|--accessory|--upper|--lower|--static_x10|--static_x1]
# --- options ---
# hair        import a hair skeletal mesh with offset position
# hair_origin import a hair skeletal mesh with origin position
# static_x10  import a static object and scale x10
# static_x1   import a static object and scale x1

import bpy
import bmesh
import os
import sys
import re
import json
import cProfile
import time
from optparse import OptionParser

startTime = time.time()

# ----------------------------------------------------------
# parse arguments 
# ----------------------------------------------------------
#optParser = OptionParser()
#optParser.add_option("-f", "--face", action="store_true", help="specify character's face parts")
#optParser.add_option("-r", "--hair", action="store_true", help="specify character's hair parts")
#optParser.add_option("-u", "--upper", action="store_true", help="specify character's upper parts")
#optParser.add_option("-l", "--lower", action="store_true", help="specify character's lower parts")
#optParser.add_option("-a", "--accessory", action="store_true", help="specify character's accessory parts")
#optParser.add_option("-w", "--work", action="store", default="none", help="save to work blender file")
#options, args = optParser.parse_args()
#if len(args) < 1:
#	optParser.print_help()
#	exit(0)

argStart = 7
"""
inFilePath  = sys.argv[argStart + 0].replace("\\", "/")
outFileDir = sys.argv[argStart + 1].replace("\\", "/")
workPath = sys.argv[argStart + 2].replace("\\", "/")
texSize = int(sys.argv[argStart + 3])
"""

inFilePath    = sys.argv[argStart + 0].replace("\\", "/")
meshType      = sys.argv[argStart + 1].lower()
fbxDestPath   = sys.argv[argStart + 2].replace("\\", "/")
texDestPath   = sys.argv[argStart + 3].replace("\\", "/")
workBlendPath = sys.argv[argStart + 4].replace("\\", "/")
texSize       = int(sys.argv[argStart + 5])

print("src -> " + inFilePath)
#print("dest -> " + outFileDir)


# ----------------------------------------------------------
# set descriotion
# ----------------------------------------------------------
srcpath = inFilePath


filedir = os.path.dirname(srcpath)
filebasename, fileext = os.path.splitext(os.path.basename(srcpath))

fileBaseName = ""
if not meshType == "onemesh":
	fileBaseName = re.sub(r'(.+)-[0-9]\.ply', r'\1', os.path.basename(srcpath))
else:
	fileBaseName = re.sub(r'(.+)\.ply', r'\1', os.path.basename(srcpath))

fbxDestDir    = os.path.dirname(fbxDestPath)
texDestDir    = os.path.dirname(texDestPath)
blendDestDir  = os.path.dirname(workBlendPath)
if not os.path.exists(fbxDestDir):
	os.makedirs(fbxDestDir)
if not os.path.exists(texDestDir):
	os.makedirs(texDestDir)
if not os.path.exists(blendDestDir):
	os.makedirs(blendDestDir)

def importPly():
	print("import " + srcpath)
	bpy.ops.import_mesh.ply(filepath=srcpath)

def convertSize():
	# ----------------------------------------------------------
	# convert size
	# ----------------------------------------------------------
	print("loaded")
	bpy.context.scene.objects.active = bpy.data.objects[filebasename]

	# バージョンアップでMagicaVoxcelのスケール値が0.1倍されていたのでここで10倍しておく
	bpy.context.object.scale[0] = 10
	bpy.context.object.scale[1] = 10
	bpy.context.object.scale[2] = 10
	bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

	if meshType == "hair":
		#bpy.data.objects[filebasename].location[2] = 35
		#bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)
		pass
	elif meshType == "accessory":
		bpy.data.objects[filebasename].location[2] = 98
		bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)

	# scaling
	if meshType == "static_x10":
		bpy.context.object.scale[0] = 10
		bpy.context.object.scale[1] = 10
		bpy.context.object.scale[2] = 10
		bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
	else: #if meshType == "hair" or meshType == "hair_origin" or meshType == "face" or meshType == "upper" or meshType == "lower" or meshType == "accessory":
		bpy.context.object.scale[0] = 2.5
		bpy.context.object.scale[1] = 2.5
		bpy.context.object.scale[2] = 2.5
		#bpy.data.objects[filebasename].scale[0] = 2.5
		#bpy.data.objects[filebasename].scale[1] = 2.5
		#bpy.data.objects[filebasename].scale[2] = 2.5
		bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

def reduction():
	# ----------------------------------------------------------
	# reduction
	# ----------------------------------------------------------
	#bpy.ops.object.editmode_toggle()
	bpy.ops.object.mode_set(mode="EDIT")
	bpy.ops.mesh.remove_doubles()
	#bpy.ops.object.editmode_toggle()

def makeTexture():
	# 頂点カラーのベイク準備
	# 
	# blender2.8からEeveeというレンダリングエンジンになったらしいが
	# まだベイク機能が搭載されていないようなのでエンジンを変更する
	#bpy.context.scene.render.engine = 'CYCLES'
	# マテリアル選択
	#bpy.context.object.active_material_index = 0
	#bpy.data.materials["Material"].node_tree.nodes["Vertex Color"].layer_name = "Col"

	#bpy.ops.object.editmode_toggle()
	bpy.ops.object.mode_set(mode="EDIT")

	# ----------------------------------------------------------
	# make texture
	# ----------------------------------------------------------
	#bpy.ops.object.editmode_toggle()
	# https://blender.stackexchange.com/questions/19310/vertex-color-bake-to-texture-causes-wrong-color-margin
	bpy.ops.uv.smart_project(island_margin=1.0)

	"""
	imagePath = destTexPath
	"""

	imageName = filebasename + "_tex.png"
	image = bpy.data.images.new(imageName, width=texSize, height=texSize)
	image.use_alpha = True
	image.alpha_mode = 'STRAIGHT'
	"""
	image.filepath_raw = imagePath
	"""
	image.filepath_raw = texDestPath

	image.file_format = 'PNG'
	image.save()

	# bake
	bpy.data.screens['UV Editing'].areas[1].spaces[0].image = image
	bpy.data.scenes["Scene"].render.bake_margin = 16
	bpy.data.scenes["Scene"].render.bake_type = 'VERTEX_COLORS'
	bpy.ops.object.bake_image()
	image.save()

def removeVertexColor():

	bpy.ops.object.mode_set(mode="EDIT")

	# remove vertex color
	bpy.ops.mesh.vertex_color_remove()

	#bpy.ops.object.editmode_toggle()
	#bpy.context.space_data.context = 'MODIFIER'

	# 頂点削減でUVが汚くなったので除去
	if False:
		# reduction vertex
		
		bpy.ops.object.modifier_add(type='DECIMATE')
		
		# COLLAPSEを使うと頂点数が変わらなくなったのでDISSOLVEのみにする
		if True:
			bpy.context.object.modifiers["Decimate"].decimate_type = 'DISSOLVE'
		else:
			if meshType == "hair" or meshType == "face" or meshType.startswith("static"):
				bpy.context.object.modifiers["Decimate"].decimate_type = 'DISSOLVE'
			else:
				bpy.context.object.modifiers["Decimate"].decimate_type = 'COLLAPSE'
				bpy.context.object.modifiers["Decimate"].ratio = 0.4

def addDecimate():
	bpy.ops.object.modifier_add(type='DECIMATE')
	bpy.context.object.modifiers["Decimate"].decimate_type = 'DISSOLVE'
	bpy.context.object.modifiers["Decimate"].angle_limit = 0.0872665

	# 適用しないと変換が重くなるので適用しておく
	#bpy.ops.object.editmode_toggle()
	bpy.ops.object.mode_set(mode="OBJECT")
	bpy.ops.object.modifier_apply(apply_as='DATA', modifier="Decimate")
	#bpy.ops.object.editmode_toggle()

def addSubSurface():
	bpy.ops.object.modifier_add(type='SUBSURF')
	bpy.context.object.modifiers["Subsurf"].subdivision_type = 'SIMPLE'
	bpy.context.object.modifiers["Subsurf"].show_only_control_edges = False
	bpy.context.object.modifiers["Subsurf"].render_levels = 1
	bpy.context.object.modifiers["Subsurf"].levels = 1

	# 適用しないと変換が重くなるので適用しておく
	#bpy.ops.object.editmode_toggle()
	bpy.ops.object.mode_set(mode="OBJECT")
	bpy.ops.object.modifier_apply(apply_as='DATA', modifier="Subsurf")
	#bpy.ops.object.editmode_toggle()

def addBebel():
	# bebel
	bpy.ops.object.modifier_add(type='BEVEL')
	bpy.context.object.modifiers["Bevel"].limit_method = 'ANGLE'
	bpy.context.object.modifiers["Bevel"].angle_limit = 1.0472
	#bpy.context.object.modifiers["Bevel"].width = 1
	bpy.context.object.modifiers["Bevel"].width = 0.5
	#bpy.context.object.modifiers["Bevel"].segments = 3
	bpy.context.object.modifiers["Bevel"].segments = 1
	bpy.context.object.modifiers["Bevel"].profile = 0.35

	# 適用しないと変換が重くなるので適用しておく
	#bpy.ops.object.editmode_toggle()
	bpy.ops.object.mode_set(mode="OBJECT")
	bpy.ops.object.modifier_apply(apply_as='DATA', modifier="Bevel")
	#bpy.ops.object.editmode_toggle()

def triangulate():
	bpy.ops.object.mode_set(mode="EDIT")
	bpy.ops.mesh.select_all(action="SELECT")
	bpy.ops.mesh.quads_convert_to_tris(quad_method='BEAUTY', ngon_method='BEAUTY')

def addAmature():
	startAddAmature = time.time()

	#bpy.ops.object.editmode_toggle()
	bpy.ops.object.mode_set(mode="OBJECT")

	if not meshType.startswith("static"):
		#bpy.ops.object.parent_drop(child=filebasename, parent="metarig", type='ARMATURE_AUTO')
		#bpy.ops.outliner.parent_drop(child=filebasename, parent="metarig", type='ARMATURE_AUTO')
		#bpy.data.objects[filebasename].parent = bpy.data.objects["metarig"]
		bpy.ops.object.select_all(action='DESELECT')
		bpy.data.objects[filebasename].select = True
		bpy.data.objects["metarig"].select = True
		bpy.context.scene.objects.active = bpy.data.objects["metarig"]
		bpy.ops.object.parent_set(type='ARMATURE_AUTO')

		obj = bpy.data.objects[filebasename]
		bm  = bmesh.new()
		bm.from_mesh(obj.data)

		if meshType == "face" or meshType == "hair":
			startFaceOrHair = time.time()

			#obj = bpy.data.objects[filebasename]

			# clear all weights
			#clearDeformWeights(bpy.data.objects[filebasename])
			
			if not obj:
				exit(0)
			bpy.ops.object.mode_set(mode='OBJECT')
			# get bone names in obj
			bnames = [b.name for b in obj.parent.pose.bones]

			# get vertex weights layer
			dflay = bm.verts.layers.deform.active
			if not dflay:
				exit(0)

			startDelVerts = time.time()
			# delete vertex weights
			for v in bm.verts:
				if v.select:
					for dvk, dvv in v[dflay].items():
						if obj.vertex_groups[dvk].name in bnames:
							del v[dflay][dvk]
			elapsedDelVerts = time.time() - startDelVerts
			print("Elapsed to delete {} vertixes : {}s".format(meshType, elapsedDelVerts))

			# apply
			bm.to_mesh(obj.data)
			bpy.ops.object.mode_set(mode='EDIT')
			# set weight 1.0 to a head mesh
			#invertWeights(bpy.data.objects[filebasename], "head")
			bpy.context.scene.objects.active = bpy.data.objects["metarig"]
			bpy.ops.object.mode_set(mode='POSE')
			bpy.context.object.pose.bones["head"].bone.select = True
			obj.select = True
			bpy.context.scene.objects.active = obj
			bpy.ops.object.mode_set(mode='WEIGHT_PAINT')
			bpy.ops.object.vertex_group_invert()

			elapsedFaceOrHair = time.time() - startFaceOrHair
			print("Elapsed to {} weight modification : {}s".format(meshType, elapsedFaceOrHair))
	
	elapsedAddAmature = time.time() - startAddAmature
	print("Elapsed addAmature : {}s verts({})".format(elapsedAddAmature, len(bm.verts)))

def export():
	# export
	#outpath = "J:/prog/0_myprogram/bishrpg_work/characters/lifeisbeautiful/" + filebasename + ".fbx"
	bpy.ops.export_scene.fbx(filepath=fbxDestPath, path_mode='ABSOLUTE')

def saveBlendFile():
	# save the blender file
	if os.path.exists(workBlendPath):
		os.remove(workBlendPath)
	bpy.ops.wm.save_as_mainfile(filepath=workBlendPath, check_existing=False)

def main():
	importPly()
	convertSize()
	
	reduction()

	makeTexture()
	removeVertexColor()
	
	addDecimate()
	#addSubSurface()
	addBebel()
	triangulate()

	addAmature()
	export()
	saveBlendFile()
	
#main()
cProfile.run("main()", filename=workBlendPath + ".prof")

elapsedTime = time.time() - startTime
print("Finished ply2fbx time={}s file({})".format(elapsedTime, workBlendPath))
