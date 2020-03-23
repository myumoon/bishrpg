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
from optparse import OptionParser

def convert(plyFile, destDir, workDir, texSize):
    currentDir      = os.path.dirname(os.path.normpath(__file__)).replace("\\", "/")
    baseBlenderFile = currentDir + "/ply2fbx_base_character.blend"
    ply2fbxPath     = currentDir + "/ply2fbx.py"
    workPath        = workDir + "/" + os.path.basename(plyFile).split('.')[0] + ".blend"
    command         = ["blender.exe", baseBlenderFile, "-b", "-P", ply2fbxPath, "--", "", plyFile.replace("\\", "/"), destDir, workPath, str(texSize)]

    print("convert {}".format(command))
    #subprocess.call(command)
    subprocess.run(command)

def convertRecursive(d, dest, work, texSize):
    for f in glob.glob(d + "/**", recursive=True):
        print("f:{}".format(f))
        if os.path.isfile(f) and f.endswith(".ply"):
            print("conv:{}".format(f))
            convert(f, dest, work, texSize)

def main():
    parser = OptionParser()
    #parser.add_option("source", nargs='+', help=u"source .ply file")
    parser.add_option("--destDir", default="./out", help=u"destination directory")
    parser.add_option("--workDir", default="./_work", help=u"destination of tempolary files directory")
    parser.add_option("--texSize", default=16, type=int, help="texture atlas size")
    
    options, args = parser.parse_args()
    print(args)

    for f in args:
        # ディレクトリ指定の場合は階層以下をすべて変換
        if os.path.isdir(f):
            print(u"recursive")
            convertRecursive(f, options.destDir, options.workDir, options.texSize)
        # ファイル指定は単体で変換
        elif os.path.isfile(f):
            print(u"file")
            convert(f, options.destDir, options.workDir, options.texSize)

if __name__ == "__main__":
    exit(main())
