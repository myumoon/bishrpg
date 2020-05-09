#!usr/bin/python
# -*- coding: utf-8 -*-
# 

import os
import sys
import glob
import pstats
from optparse import OptionParser


def parse(file):
	print("--- stats {} ---".format(file))
	sts = pstats.Stats(file)
	sts.strip_dirs().sort_stats('cumulative').print_stats()

def parseRecursive(dir):
	for fileOrDir in glob.glob(dir + "/**", recursive=True):
		fileOrDir = os.path.normpath(fileOrDir)
		if os.path.isdir(fileOrDir):
			continue
		elif os.path.splitext(fileOrDir)[1] == ".prof":
			parse(fileOrDir)
		else:
			print("Skipped {}. Only .prof file.".format(fileOrDir))
	

def main():
	parser = OptionParser(description="Parse .prof files and show them.")
	#parser.add_option("-i", "--in", help="parse file")
	options, args = parser.parse_args()
	if len(args) == 0:
		optParser.print_help()
		return 0
	
	for fileOrDir in args:
		parseRecursive(fileOrDir)

	return 0

if __name__ == "__main__":
	exit(main())
