#!/usr/bin/python

import json
import sys
import os
import math
import time

from subprocess import call
from PIL import Image

supportedExtensions = set([".PNG", ".TGA", ".PPM"])

modeToBitsPerPixel = {'1': 1, 'L': 8, 'P': 8, 'RGB': 24, 'RGBA': 32,
                      'CMYK': 32, 'YCbCr': 24, 'I': 32, 'F': 32}
modeToBytesPerPixel = {'1': 1, 'L': 1, 'P': 1, 'RGB': 3, 'RGBA': 4,
                       'CMYK': 4, 'YCbCr': 3, 'I': 4, 'F': 4}

pwdStack = []

scriptPath = ""


def PushPWD(path):
    pwdStack.append(os.getcwd())
    os.chdir(path)


def PopPWD():
    os.chdir(pwdStack.pop())


def TranslatePath(path, configPath):
    if path.upper() == "@HERE":
        pathDir = os.path.dirname(configPath)
        if pathDir == "":
            return "."
        else:
            return pathDir
    else:
        return path


def mkdirFile(filename):
    folder = os.path.dirname(filename)
    if not os.path.exists(folder):
        try:
            original_umask = os.umask(0)
            os.makedirs(folder, 0777)
        finally:
            os.umask(original_umask)


def Byteify(input):
    if isinstance(input, dict):
        return {Byteify(key): Byteify(value) for key, value
                in input.iteritems()}
    elif isinstance(input, list):
        return [Byteify(element) for element in input]
    elif isinstance(input, unicode):
        return input.encode('utf-8')
    else:
        return input

suffixes = ['B', 'KB', 'MB', 'GB', 'TB', 'PB']


def PrettyBytes(nbytes):
    if nbytes < 0:
        return "-" + PrettyBytes(-nbytes)
    if nbytes == 0:
        return '0 B'
    i = 0
    while nbytes >= 1024 and i < len(suffixes)-1:
        nbytes /= 1024.
        i += 1
    f = ('%.2f' % nbytes).rstrip('0').rstrip('.')
    return '%s %s' % (f, suffixes[i])


def IntWithCommas(x):
    if type(x) not in [type(0), type(0L)]:
        raise TypeError("Parameter must be an integer.")
    if x < 0:
        return '-' + IntWithCommas(-x)
    result = ''
    while x >= 1000:
        x, r = divmod(x, 1000)
        result = ",%03d%s" % (r, result)
    return "%d%s" % (x, result)


def touch(fname, times=None):
    with open(fname, 'a'):
        os.utime(fname, times)


def Execute(configPath):
    print("Config file: " + configPath)
    with open(configPath) as configJsonFile:
        startTime = time.time()

        configJson = Byteify(json.load(configJsonFile))

        if "global" not in configJson:
            print("Error: No global field!")
            return 2

        if "dev-screen-width" not in configJson["global"]:
            print("Error: No global::dev-screen-width specified!")
            return 2
        developmentScreenWidth = configJson["global"]["dev-screen-width"]
        if developmentScreenWidth <= 0:
            print("Error: Invalid global::dev-screen-width value!")
            return 2

        if "dev-screen-height" not in configJson["global"]:
            print("Error: No global::dev-screen-height specified!")
            return 2
        developmentScreenHeight = configJson["global"]["dev-screen-height"]
        if developmentScreenHeight <= 0:
            print("Error: Invalid global::dev-screen-height value!")
            return 2

        if "targets" not in configJson:
            print("Error: no targets field!")
            return 2

        if "folders" not in configJson:
            print("Error: No folders to search for files")
            return 2

        if "result" not in configJson:
            print("Error: no result field!")
            return 2

        if "pwd" in configJson:
            PushPWD(TranslatePath(configJson["pwd"], configPath))

        for path in configJson["exec"]:
            Execute(path)

        # Get all supported files from folders
        filesData = {}
        for folder in configJson["folders"]:
            for root, subDirs, files in os.walk(os.path.join(os.getcwd(), folder)):
                for filename in files:
                    filePath = os.path.join(root, filename)

                    filePathAbs = os.path.abspath(filePath)

                    extension = os.path.splitext(filePathAbs)[1]

                    if extension.upper() in supportedExtensions:
                        image = Image.open(filePathAbs)
                        bpp = modeToBytesPerPixel[image.mode]
                        filesData[filePathAbs] = {"save-to": folder,
                                                  "real-width": image.size[0],
                                                  "real-height": image.size[1],
                                                  "bpp": bpp}

        # Update files with "tex-info" structure properties
        if "tex-info" in configJson["global"]:
            for folder in configJson["folders"]:
                for filePath, texInfo in configJson["global"]["tex-info"].iteritems():
                    filePath = folder + "/" + filePath
                    filePathAbs = os.path.abspath(filePath)
                    if filePathAbs in filesData:
                        filesData[filePathAbs].update(texInfo)
                    else:
                        print("Specified file '" + filePathAbs +
                              "' is either not found or unsupported")

        targetsData = {}
        cmds = []
        for target in configJson["targets"]:
            if "name" not in target:
                print("Error: target name field not specified")
                continue
            targetName = target["name"]
            if "width" not in target:
                print("Error: target '%s' does not have width specified"
                      % (targetName))
                continue
            targetWidth = target["width"]
            if "height" not in target:
                print("Error: target '%s' does not have height specified"
                      % (targetName))
                continue
            targetHeight = target["height"]
            if "cmds" not in target:
                print("Error: target '%s' does not have a cmd list specified"
                      % (targetName))
                continue

            targetsData[targetName] = {"pixel-count-after": 0,
                                       "pixel-count-before": 0,
                                       "GPU-memory-after": 0,
                                       "GPU-memory-before": 0,
                                       "disk-bytes-after": 0,
                                       "disk-bytes-before": 0}
            for filePath, fileData in filesData.iteritems():
                for func in target["cmds"]:
                    cmd = {}
                    cmd["func"] = func
                    cmd["path"] = filePath

                    cmd["real-width"] = fileData["real-width"]
                    cmd["real-height"] = fileData["real-height"]

                    if "dev-width" not in fileData:
                        cmd["dev-width"] = cmd["real-width"]
                    else:
                        cmd["dev-width"] = fileData["dev-width"]

                    if "dev-height" not in fileData:
                        cmd["dev-height"] = cmd["real-height"]
                    else:
                        cmd["dev-height"] = fileData["dev-height"]

                    cmd["new-width"] = math.ceil(1.0 * cmd["dev-width"] /
                                                 developmentScreenWidth *
                                                 targetWidth)
                    cmd["new-height"] = math.ceil(1.0 * cmd["dev-height"] /
                                                  developmentScreenHeight *
                                                  targetHeight)

                    if cmd["new-width"] > cmd["real-width"]:
                        cmd["new-width"] = cmd["real-width"]

                    if cmd["new-height"] > cmd["real-height"]:
                        cmd["new-height"] = cmd["real-height"]

                    nw = cmd["new-width"]
                    nh = cmd["new-height"]
                    na = nw * nh

                    rw = cmd["real-width"]
                    rh = cmd["real-height"]
                    ra = rw * rh

                    bpp = fileData["bpp"]

                    targetsData[targetName]["pixel-count-after"] += na
                    targetsData[targetName]["pixel-count-before"] += ra

                    targetsData[targetName]["GPU-memory-after"] += na * bpp
                    targetsData[targetName]["GPU-memory-before"] += ra * bpp

                    cmd["path-prefix"] = targetName
                    cmds.append(cmd)

        for cmd in cmds:
            pathPrefix = os.path.abspath(cmd["path-prefix"])
            whereTo = os.path.join(pathPrefix, os.path.relpath(cmd["path"]))
            mkdirFile(whereTo)
            print("Target: " + cmd["path-prefix"] + " -> " + cmd["func"] + ": "
                  + os.path.relpath(cmd["path"], scriptPath))
            if cmd["func"] == "resize":
                before = os.path.getsize(cmd["path"])
                targetsData[cmd["path-prefix"]]["disk-bytes-before"] += before
                touch(whereTo)
                call(["convert", cmd["path"], "-resize", "%dx%d!" %
                     (cmd["new-width"], cmd["new-height"]), whereTo])
                after = os.path.getsize(whereTo)
                targetsData[cmd["path-prefix"]]["disk-bytes-after"] += after
            else:
                print("Function '%s' not found. Skiping" % cmd["func"])

        initialDiskSize = 0
        finalDiskSize = 0
        for targetName, targetData in targetsData.iteritems():
            initialDiskSize += targetData["disk-bytes-before"]
            finalDiskSize += targetData["disk-bytes-after"]
            print("-------------------------------------------------------")
            print("Target: %s" % targetName)
            print("pixel-count-before: %d" % targetData["pixel-count-before"])
            print("pixel-count-after: %d" % targetData["pixel-count-after"])
            print("GPU-memory-before: %s (%s)" %
                  (IntWithCommas(int(targetData["GPU-memory-before"])),
                   PrettyBytes(targetData["GPU-memory-before"])))
            print("GPU-memory-after: %s (%s)" %
                  (IntWithCommas(int(targetData["GPU-memory-after"])),
                   PrettyBytes(targetData["GPU-memory-after"])))
            print("Saved memory: %s (%.2f%%)" %
                  (PrettyBytes(targetData["GPU-memory-before"] -
                   targetData["GPU-memory-after"]),
                   100.0 * (targetData["GPU-memory-before"] -
                   targetData["GPU-memory-after"]) /
                   targetData["GPU-memory-before"]))

            print("disk-bytes-before: %s (%s)" %
                  (IntWithCommas(int(targetData["disk-bytes-before"])),
                   PrettyBytes(targetData["disk-bytes-before"])))
            print("disk-bytes-after: %s (%s)" %
                  (IntWithCommas(int(targetData["disk-bytes-after"])),
                   PrettyBytes(targetData["disk-bytes-after"])))
            print("Saved disk memory: %s (%.2f%%)" %
                  (PrettyBytes(targetData["disk-bytes-before"] -
                   targetData["disk-bytes-after"]),
                   100.0 * (targetData["disk-bytes-before"] -
                   targetData["disk-bytes-after"]) /
                   targetData["disk-bytes-before"]))

        endTime = time.time()

        initialDiskSize /= len(targetsData)
        print("-------------------------------------------------------")
        print("initial-disk-size: %s" % (PrettyBytes(initialDiskSize)))
        print("final-disk-size: %s" % (PrettyBytes(finalDiskSize)))
        print("Elapsed time: %.2f s" % (endTime - startTime))

        if "pwd" in configJson:
            PopPWD()
        return 0
    print("Error: Can not open: %s" % (configPath))
    return 2


def Main():
    if len(sys.argv) != 2:
        print("Usage: " + sys.argv[0] + " <path/to/config.json>")
        sys.exit(2)

    global scriptPath
    scriptPath = os.getcwd()

    sys.exit(Execute(sys.argv[1]))

Main()