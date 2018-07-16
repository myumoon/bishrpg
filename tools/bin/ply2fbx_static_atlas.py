#!usr/bin/python
# -*- coding: utf-8 -*-
# 
# convert ply to fbx
# ply2fbx [--static_x10|--static_x1] [out.fbx] [work_blender_path] [in.ply]
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
argStart = 7
meshType       = sys.argv[argStart + 0].replace("--", "")
inFilePathList = [path for path in sys.argv[argStart + 1:]]


for path in inFilePathList:
	print("src -> " + path)


# ----------------------------------------------------------
# set descriotion
# ----------------------------------------------------------
fixedBaseName = re.sub(r'(\w+)(\-[0-9]+\.\w+|\.\w+)', r'\1', os.path.basename(inFilePathList[0]))
srcDir    = os.path.dirname(inFilePathList[0]) + "/"
destdir   = srcDir + "/"
destpath  = srcDir + fixedBaseName + ".fbx"
blendpath = srcDir + fixedBaseName + ".blend"
importedObjects = []
importedObjectNames = []
srcFileDir  = os.path.dirname(inFilePathList[0])
texFileName = fixedBaseName + "_tex_atlas.png"

print("dest  : " + destpath)
print("blend : " + blendpath)
print("texFileName : " + texFileName)

# ----------------------------------------------------------
# import
# ----------------------------------------------------------
for srcPath in inFilePathList:
	filebasename, fileext = os.path.splitext(os.path.basename(srcPath))
	importedObjectNames.append(filebasename)

	# ----------------------------------------------------------
	# import
	# ----------------------------------------------------------
	print("import " + srcPath)
	bpy.ops.import_mesh.ply(filepath=srcPath)
	
	importedObjects.append(bpy.data.objects[filebasename])
	bpy.context.scene.objects.active = bpy.data.objects[filebasename]
	
	# scaling
	if meshType == "static_x10":
		bpy.context.object.scale[0] = 10
		bpy.context.object.scale[1] = 10
		bpy.context.object.scale[2] = 10
		bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
		
# ----------------------------------------------------------
# make material
# ----------------------------------------------------------
for obj in importedObjects:
	bpy.context.scene.objects.active = obj
	activeObject = bpy.context.active_object
	mat = bpy.data.materials.new(name="Material")
	activeObject.data.materials.append(mat)

# ----------------------------------------------------------
# merge
# ----------------------------------------------------------
mergedObjectList = []
mergedObjectNameList = []
mergedCheckList  = [False for i in range(len(importedObjects))]
checkLabel       = ""

while False in mergedCheckList:
	checkLabel = ""
	for obj in bpy.data.objects:
		obj.select = False
	for i in range(len(importedObjects)):
		if mergedCheckList[i] == False:
				label = re.sub(r'(\w+)[\-0-9]*', r'\1', importedObjectNames[i])
				if checkLabel == "":
					mergedObjectList.append(importedObjects[i])
					mergedObjectNameList.append(label)
					bpy.context.scene.objects.active = importedObjects[i]
					checkLabel = label
					print("base ", i, importedObjectNames[i], label)
				if checkLabel == label:
					mergedCheckList[i] = True
					importedObjects[i].select = True
					print(" add ", i, importedObjectNames[i], label)
	bpy.ops.object.join()
	print("checklist :", mergedCheckList)


# ----------------------------------------------------------
# reduction
# ----------------------------------------------------------
for obj in bpy.context.scene.objects:
	if obj.type == 'MESH':
		print(obj.name)
		bpy.context.scene.objects.active = obj
		bpy.ops.object.editmode_toggle()
		bpy.ops.mesh.select_all(action='SELECT')
		bpy.ops.mesh.remove_doubles()
		bpy.ops.object.editmode_toggle()

# ----------------------------------------------------------
# make texture
# ----------------------------------------------------------
bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
bpy.ops.scene.ms_add_lightmap_group()
bpy.context.scene.ms_lightmap_groups[0].autoUnwrapPrecision = 0.01

for obj in bpy.context.scene.objects:
	if obj.type == 'MESH':
		print("obj:", obj)
		bpy.context.scene.objects.active = obj
		obj.select = True
		bpy.ops.scene.ms_add_selected_to_group()
bpy.ops.object.ms_auto()

#bpy.ops.uv.smart_project(island_margin=0.1)
#bpy.ops.object.editmode_toggle()
imagePath = srcFileDir + "/" + texFileName
imageName = texFileName
image = bpy.data.images.new(imageName, width=1024, height=1024)
image.use_alpha = True
image.alpha_mode = 'STRAIGHT'
image.filepath_raw = imagePath
image.file_format = 'PNG'
image.save()

for obj in bpy.context.scene.objects:
	if obj.type == 'MESH':
		obj.select = False
for obj in bpy.context.scene.objects:
	if obj.type == 'MESH':
		bpy.context.scene.objects.active = obj
		bpy.ops.object.editmode_toggle()
		bpy.data.screens['UV Editing'].areas[1].spaces[0].image = image
		bpy.ops.object.editmode_toggle()

image.save()


# bake
bpy.data.scenes["Scene"].render.bake_margin = 16
bpy.data.scenes["Scene"].render.bake_type = 'VERTEX_COLORS'

for obj in bpy.context.scene.objects:
	if obj.type == 'MESH':
		obj.select = True
		bpy.context.scene.objects.active = obj

bpy.ops.object.bake_image()

image.save()

for obj in bpy.context.scene.objects:
	obj.select = False
	
#for obj in mergedObjectList:
for obj in bpy.context.scene.objects:
	if obj.type == 'MESH':
		bpy.context.scene.objects.active = obj
		
		# remove vertex color
		bpy.ops.mesh.vertex_color_remove()

		# reduction vertex
		bpy.ops.object.editmode_toggle()
		#bpy.context.space_data.context = 'MODIFIER'
		bpy.ops.object.modifier_add(type='DECIMATE')
		bpy.context.object.modifiers["Decimate"].decimate_type = 'DISSOLVE'

		#bpy.ops.object.modifier_apply(apply_as='DATA', modifier="Decimate")
		#bpy.ops.export_scene.fbx(filepath=destpath, path_mode='ABSOLUTE')
		objname = re.sub(r'(\w+)(\-[0-9]+)', r'\1', obj.name)

		obj.select = True
		bpy.ops.export_scene.fbx(filepath=destdir + objname + ".fbx", path_mode='ABSOLUTE', use_selection=True)
		obj.select = False

# ----------------------------------------------------------
# save
# ----------------------------------------------------------
# export
#outpath = "J:/prog/0_myprogram/bishrpg_work/characters/lifeisbeautiful/" + filebasename + ".fbx"
#bpy.ops.export_scene.fbx(filepath=destpath, path_mode='ABSOLUTE')

# save the blender file
if os.path.exists(blendpath):
	os.remove(blendpath)
bpy.ops.wm.save_mainfile(filepath=blendpath, check_existing=False)
