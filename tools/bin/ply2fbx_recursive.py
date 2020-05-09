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


import os
import sys
import re
import glob
import subprocess
#import threading
import time
import shutil
from optparse import OptionParser
#from concurrent.futures import ThreadPoolExecutor
import proj_def
from model_build.scripts import ply_path

def execCommand(command):
	print("convert {}".format(command)) 
	subprocess.call(command)
	#subprocess.run(command)

def makeDummyFile(dummyPath, fileText=""):
	dummyDir = os.path.dirname(dummyPath)
	if not os.path.exists(dummyDir):
		os.makedirs(dummyDir)
	with open(dummyPath, "w") as f:
		f.write(fileText)

def convert(plyFile, destDir, workDir, texSize, threadPool, dummy=False):
	currentDir      = os.path.abspath(os.path.dirname(os.path.normpath(__file__))).replace("\\", "/")
	baseBlenderFile = currentDir + "/ply2fbx_base_character.blend"
	ply2fbxPath     = currentDir + "/ply2fbx.py"
	workPath        = workDir + "/" + os.path.basename(plyFile).split('.')[0] + ".blend"
	print("convert! : cur({}), blend({}), file({})".format(currentDir, baseBlenderFile, __file__))

	# plypath partsname fbxpath texpath workblendpath texsize
	destFbxPath     = os.path.join(destDir, ply_path.makeRelativeFbxContentsPath(plyFile))
	destTexPath     = os.path.join(destDir, ply_path.makeRelativeTexContentsPath(plyFile))
	partsType       = ply_path.getPartsName(plyFile)
	destBlendPath   = os.path.join(workDir, os.path.splitext(os.path.basename(plyFile))[0] + ".blend")

	if dummy:
		print("convert with dummy")
		makeDummyFile(destFbxPath, "Dummy fbx")
		makeDummyFile(destTexPath, "Dummy texture")
	else:
		print("copy .blend file : {} -> {}".format(baseBlenderFile, destBlendPath))
		if not os.path.exists(os.path.dirname(destBlendPath)):
			os.makedirs(os.path.dirname(destBlendPath))
		shutil.copy(baseBlenderFile, destBlendPath)
		print("start ply convertion :")
		print(" - src ply      : {}".format(plyFile.replace("\\", "/")))
		print(" - dest blend   : {}".format(destBlendPath))
		print(" - dest fbx     : {}".format(destFbxPath))
		print(" - dest texture : {}".format(destTexPath))
		print(" - parts        : {}".format(partsType))
		print(" - tex size     : {}".format(texSize))
		command     = ["blender.exe", destBlendPath, "-b", "-P", ply2fbxPath, "--", "", plyFile.replace("\\", "/"), partsType, destFbxPath, destTexPath, destBlendPath, str(texSize)]

		subprocess.run(command)
		#subprocess.run(command, stdout=subprocess.PIPE, subprocess.PIPE)
		#print(proc.stdout.decode("utf8"))
		#print(proc.stderr.decode("utf8"))
		#threadPool.submit(execCommand, command)

		#worker = threading.Thread(target=execCommand, args=(command,))
		#worker.start()

def convertRecursive(d, dest, work, texSize, threads=4, dummy=False):
	#workerThreads = []
	for f in glob.glob(d + "/**", recursive=True):
		print("f:{}".format(f))
		if os.path.isfile(f) and f.endswith(".ply"):
			print("conv:{}".format(f))
			workerThread = convert(f, dest, work, texSize, threads, dummy)
	#        workerThreads.append(workerThread)
	#for worker in workerThreads:
	#    worker.join()

def main():
	startTime = time.time()

	parser = OptionParser()
	#parser.add_option("source", nargs='+', help=u"source .ply file")
	parser.add_option("--destDir", default="./out", help=u"destination directory")
	parser.add_option("--workDir", default="./_work", help=u"destination of tempolary files directory")
	parser.add_option("--texSize", default=16, type=int, help="texture atlas size")
	parser.add_option("--threads", default=4, type=int, help="thread num")
	parser.add_option("--dummy", default=False, action="store_true", help="create empty fbx and texture to test this script.")

	options, args = parser.parse_args()
	print(args)

	#threadPool = ThreadPoolExecutor(max_workers=options.threads)

	for f in args:
		# ディレクトリ指定の場合は階層以下をすべて変換
		if os.path.isdir(f):
			print(u"recursive")
			convertRecursive(f, options.destDir, options.workDir, options.texSize, None, options.dummy)
		# ファイル指定は単体で変換
		elif os.path.isfile(f):
			print(u"file")
			convert(f, options.destDir, options.workDir, options.texSize, None, options.dummy)
    
	#threadPool.shutdown()

	elapsedTime = time.time() - startTime
	print("Done convert. (time={}s)".format(elapsedTime))

if __name__ == "__main__":
	exit(main())
