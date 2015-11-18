#!/usr/bin/python

try:
    import sys
except ImportError, e:
    print("sys module not found! Please make sure you have Python 2.7 installed correctly.")
    exit(2)

try:
    import os
except ImportError, e:
    print("os module not found! Please make sure you have Python 2.7 installed correctly.")
    exit(2)

try:
    import math
except ImportError, e:
    print("math module not found! Please make sure you have Python 2.7 installed correctly.")
    exit(2)

try:
    import time
except ImportError, e:
    print("time module not found! Please make sure you have Python 2.7 installed correctly.")
    exit(2)

try:
    import json
except ImportError, e:
    print("json module not found! Please run 'make install' in this directory.")
    exit(2)

CMD_NONE = 0
CMD_HELP = 1
CMD_ATLAS_NO_CATEGORY = 2
CMD_ATLAS_FOLDER_CATEGORY = 3
CMD_ATLAS_DEFAULT = CMD_ATLAS_FOLDER_CATEGORY

supportedExtensions = set([".PNG", ".TGA", ".PPM"])

def Help():
    print("")
    print("Options:")
    print("")
    print("\tatlas <path/to/folder> <path/to/output>  ---  generates pktex-atlas JSON using default strategy")
    print("\tatlas-none <path/to/folder> <path/to/output> ---  generates pktex-atlas JSON with no category")
    print("\tatlas-folder <path/to/folder> <path/to/output> ---  generates pktex-atlas JSON with category as folders")
    print("\t--help or -h  ---  show this table")

def GetTexPaths(path):
    paths = []
    for root, subDirs, files in os.walk(path):
        for filename in files:
            filePath = os.path.join(root, filename)

            filePathAbs = os.path.abspath(filePath)

            extension = os.path.splitext(filePathAbs)[1]

            if extension.upper() in supportedExtensions:
                paths.append(filePathAbs)
    return paths

def AtlasNoCategory(filepath):
    jsonData = {}
    texList = GetTexPaths(filepath)
    for path in texList:
        jsonData[path] = "-1"
    return jsonData

def AtlasFolderCategory(filepath):
    lastCategory = -1
    folderToCategory = {}
    jsonData = {}
    texList = GetTexPaths(filepath)
    for path in texList:
        folder = os.path.dirname(path)
        if folder not in folderToCategory:
            lastCategory += 1
            folderToCategory[folder] = lastCategory
        jsonData[path] = str(folderToCategory[folder])
    return jsonData

def SaveJSON(data, path):
    with open(path, "w") as outfile:
        json.dump(data, outfile, indent=4, sort_keys=True)

def Main():
    cmd = CMD_NONE
    path = ""
    outputPath = "output"

    if len(sys.argv) == 2:
        if sys.argv[1] == "--help" or sys.argv[1] == "-h":
            cmd = CMD_HELP
    if len(sys.argv) == 4:
        if sys.argv[1] == "atlas":
            cmd = CMD_ATLAS_DEFAULT
            path = sys.argv[2]
            outputPath = sys.argv[3]
        if sys.argv[1] == "atlas-folder":
            cmd = CMD_ATLAS_FOLDER_CATEGORY
            path = sys.argv[2]
            outputPath = sys.argv[3]
        if sys.argv[1] == "atlas-none":
            cmd = CMD_ATLAS_NO_CATEGORY
            path = sys.argv[2]
            outputPath = sys.argv[3]

    if cmd == CMD_NONE:
        print("Please run: '" + sys.argv[0] + " --help' to see all commands")
        sys.exit(2)
    elif cmd == CMD_HELP:
        Help()
    elif cmd == CMD_ATLAS_NO_CATEGORY:
        atlasJson = AtlasNoCategory(path)
        SaveJSON(atlasJson, outputPath)
    elif cmd == CMD_ATLAS_FOLDER_CATEGORY:
        atlasJson = AtlasFolderCategory(path)
        SaveJSON(atlasJson, outputPath)

Main()
