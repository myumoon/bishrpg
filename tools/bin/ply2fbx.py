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
from optparse import OptionParser

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
inFilePath  = sys.argv[argStart + 0].replace("\\", "/")
outFileDir = sys.argv[argStart + 1].replace("\\", "/")
workPath = sys.argv[argStart + 2].replace("\\", "/")
texSize = int(sys.argv[argStart + 3])

print("src -> " + inFilePath)
print("dest -> " + outFileDir)


# ----------------------------------------------------------
# set descriotion
# ----------------------------------------------------------
destDir = outFileDir
srcpath = inFilePath



#srcpath = "J:/prog/0_myprogram/bishrpg_work/characters/box/p005_lingling/school_lingling-0.ply"
#destpath = "J:/prog/0_myprogram/bishrpg_work/characters/box/p005_lingling/school_lingling-0.fbx"
#face = options.face
#hair = options.hair
#upper = options.upper
#lower = options.lower
#accessory = options.accessory
#face = False
#hair = True
#upper = False
#lower = False
#accessory = False

filedir = os.path.dirname(srcpath)
filebasename, fileext = os.path.splitext(os.path.basename(srcpath))
 
partsIndex = argStart + 4
if partsIndex < len(sys.argv):
	meshType = sys.argv[partsIndex].replace("-", "")
elif filebasename.endswith("-0"):
	meshType = "hair"
elif filebasename.endswith("-1"):
	meshType = "face"
elif filebasename.endswith("-2"):
	meshType = "upper"
elif filebasename.endswith("-3"):
	meshType = "lower"
elif filebasename.endswith("-4"):
	meshType = "accessory"
else:
	meshType = "onemesh"

meshTypeName = ""
if meshType == "hair_origin":
	meshTypeName = "Hair"
else:
	meshTypeName = meshType.capitalize()


print("basename:" + os.path.basename(srcpath))
fileBaseName = re.sub(r'(.+)(-[0-9])*\.ply', r'\1', os.path.basename(srcpath))
destFbxDir  = destDir + meshTypeName + "/Meshes/"
destFbxPath = destFbxDir + meshTypeName + "_" + fileBaseName + ".fbx"
destTexDir  = destDir + meshTypeName + "/Textures/"
destTexPath = destTexDir + meshTypeName + "_" + fileBaseName + ".png"

if not os.path.exists(destDir):
	os.mkdir(destDir)
if not os.path.exists(destDir + meshTypeName):
	os.mkdir(destDir + meshTypeName)
if not os.path.exists(destFbxDir):
	os.mkdir(destFbxDir)
if not os.path.exists(destTexDir):
	os.mkdir(destTexDir)

# ----------------------------------------------------------
# import
# ----------------------------------------------------------
print("import " + srcpath)
bpy.ops.import_mesh.ply(filepath=srcpath)


# ----------------------------------------------------------
# convert size
# ----------------------------------------------------------
print("loaded")
bpy.context.scene.objects.active = bpy.data.objects[filebasename]

if meshType == "hair":
	bpy.data.objects[filebasename].location[2] = 35
	bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)
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

# ----------------------------------------------------------
# reduction
# ----------------------------------------------------------
bpy.ops.object.editmode_toggle()
bpy.ops.mesh.remove_doubles()


# ----------------------------------------------------------
# make texture
# ----------------------------------------------------------
#bpy.ops.object.editmode_toggle()
# https://blender.stackexchange.com/questions/19310/vertex-color-bake-to-texture-causes-wrong-color-margin
bpy.ops.uv.smart_project(island_margin=0.1)

imagePath = destTexPath
imageName = filebasename + "_tex.png"
image = bpy.data.images.new(imageName, width=1024, height=1024)
image.use_alpha = True
image.alpha_mode = 'STRAIGHT'
image.filepath_raw = imagePath
image.file_format = 'PNG'
image.save()

# bake
bpy.data.screens['UV Editing'].areas[1].spaces[0].image = image
bpy.data.scenes["Scene"].render.bake_margin = 16
bpy.data.scenes["Scene"].render.bake_type = 'VERTEX_COLORS'
bpy.ops.object.bake_image()
image.save()

# remove vertex color
bpy.ops.mesh.vertex_color_remove()

# reduction vertex
bpy.ops.object.editmode_toggle()
#bpy.context.space_data.context = 'MODIFIER'
bpy.ops.object.modifier_add(type='DECIMATE')
if meshType == "hair" or meshType == "face" or meshType.startswith("static"):
	bpy.context.object.modifiers["Decimate"].decimate_type = 'DISSOLVE'
else:
	bpy.context.object.modifiers["Decimate"].decimate_type = 'COLLAPSE'
	bpy.context.object.modifiers["Decimate"].ratio = 0.4

#bpy.ops.object.modifier_apply(apply_as='DATA', modifier="Decimate")

# set parent
#bpy.context.area.type = 'OUTLINER'

if not meshType.startswith("static"):
	#bpy.ops.object.parent_drop(child=filebasename, parent="metarig", type='ARMATURE_AUTO')
	#bpy.ops.outliner.parent_drop(child=filebasename, parent="metarig", type='ARMATURE_AUTO')
	#bpy.data.objects[filebasename].parent = bpy.data.objects["metarig"]
	bpy.ops.object.select_all(action='DESELECT')
	bpy.data.objects[filebasename].select = True
	bpy.data.objects["metarig"].select = True
	bpy.context.scene.objects.active = bpy.data.objects["metarig"]
	bpy.ops.object.parent_set(type='ARMATURE_AUTO')

	if meshType == "face" or meshType == "hair":
		# clear all weights
		#clearDeformWeights(bpy.data.objects[filebasename])
		obj = bpy.data.objects[filebasename]
		if not obj:
			exit(0)
		bpy.ops.object.mode_set(mode='OBJECT')
		# get bone names in obj
		bnames = [b.name for b in obj.parent.pose.bones]
		bm = bmesh.new()
		bm.from_mesh(obj.data)
		# get vertex weights layer
		dflay = bm.verts.layers.deform.active
		if not dflay:
			exit(0)
		# delete vertex weights
		for v in bm.verts:
			if v.select:
				for dvk, dvv in v[dflay].items():
					if obj.vertex_groups[dvk].name in bnames:
						del v[dflay][dvk]
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
# export
#outpath = "J:/prog/0_myprogram/bishrpg_work/characters/lifeisbeautiful/" + filebasename + ".fbx"
bpy.ops.export_scene.fbx(filepath=destFbxPath, path_mode='ABSOLUTE')

if not os.path.exists(os.path.dirname(workPath)):
	os.mkdir(os.path.dirname(workPath))

# save the blender file
if os.path.exists(workPath):
	os.remove(workPath)
bpy.ops.wm.save_as_mainfile(filepath=workPath, check_existing=False)

