#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#    woptipng - attempts to reduce PNGs in size using several other tools
#    Copyright (C) 2016  Matthias Krüger

#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2, or (at your option)
#    any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA


#  Please file bugs to https://github.com/matthiaskrgr/woptipng

from multiprocessing import Pool
import multiprocessing # cpu count
from PIL import Image as PIL # compare images
import enum
import subprocess # launch advdef, optipng, imagemagick
import os # os rename, niceness
import shutil # copy files
import argparse # argument parsing
import sys # sys.exit


parser = argparse.ArgumentParser()
parser.add_argument("inpath", help="file or path (recursively) to be searched for crushable pngs", metavar='path', nargs='+', type=str)
parser.add_argument("-d", "--debug", help="print debug information", action='store_true')
parser.add_argument("-t", "--threshold", help="size reduction below this percentage will be discarded, default: 10", metavar='n', nargs='?', default=10, type=float)
parser.add_argument("-j", "--jobs", help="max amount of jobs/threads. If unspecified, take number of cores found", metavar='n', nargs='?', default=multiprocessing.cpu_count(), type=int)
parser.add_argument("-n", "--nice", help="niceness of all threads (must be positive)", metavar='n', nargs="?", default=19, type=int)

args = parser.parse_args()

DEBUG = args.debug
INPATHS = args.inpath
THRESHOLD = args.threshold
MAX_THREADS = args.jobs
# program executables
EXEC_OPTIPNG = shutil.which("optipng")
EXEC_IMAGEMAGICK = shutil.which("convert")
EXEC_ADVDEF = shutil.which("advdef")

os.nice(args.nice) # set niceness

input_files=[]
bad_input_files=[]

print("Collecting files... ", end="")
for path in INPATHS: # iterate over arguments
    if (os.path.isfile(path)):   # inpath is a file
        if (path.endswith("png")):
            input_files.append(path)
        else: # not png?
            bad_input_files.append(path)
    elif (os.path.isdir(path)):  # inpath is a directory
        for root, directories, filenames in os.walk(path):
            for filename in filenames:
                if (filename.split('.')[-1] == "png"): # check for valid filetypes
                    input_files.append(os.path.join(root,filename)) # add to list
    else: # path does not exist
        bad_input_files.append(path)

bad_input_files.sort()
input_files.sort()

print(" done")
if (bad_input_files):
    print("WARNING: can't handle following files:' ")
    print(', '.join(bad_input_files) + "\n")

print("Compressing " + str(len(input_files)) + " pngs...")

def debugprint(arg):
    if (DEBUG):
        print(arg)

def images_identical(image1, image2):
    return PIL.open(image1).tobytes() == PIL.open(image2).tobytes()

def verify_images(source_img, new_img, transform):
    pixels_identical = images_identical(source_img, new_img) # image pixels' values remain unaltered, we want this
    image_got_smaller = os.path.getsize(source_img) > os.path.getsize(new_img)
    debugprint("size reduction: " + str(os.path.getsize(source_img) - os.path.getsize(new_img)))

    if (pixels_identical and image_got_smaller):
        os.rename(new_img, source_img) # move new image to old image // os.rename(src, dest)
    else: # we can't os.rename(image_after, image_before) because that would leave us with no source for the next transform
        shutil.copy(source_img, new_img) # override new image with old image // shutil.copy(src, dest)
        if not pixels_identical:
            debugprint(("Tool " + transform + " CHANGED THE PIXELS, REVERTING " + source_img))
        else:
            debugprint(("Tool " + transform + " made the file bigger, reverting " + source_img))

def run_imagemagick(image, tmpimage):
    shutil.copy(image, tmpimage)
    debugprint("imagemagick ")
    cmd = [ EXEC_IMAGEMAGICK,
            "-strip",
            "-define",
            "png:color-type=6",
            image,
            tmpimage
    ]
    subprocess.call(cmd)

def run_optipng(image, tmpimage):
    debugprint("optipng...")
    shutil.copy(image, tmpimage)
    cmd = [ EXEC_OPTIPNG,
            "-q",
            "-o5",
            "-nb",
            "-nc",
            "-np",
            tmpimage
    ]
    subprocess.call(cmd)

def run_advdef(image, tmpimage):
    debugprint("advdef")
    shutil.copy(image, tmpimage)
    compression_levels = [1, 2, 3, 4]

    for level in compression_levels:
        cmd = [
            EXEC_ADVDEF,
            "-z",
            "-" + str(level),
            tmpimage,
        ]
        subprocess.call(cmd, stdout=open(os.devnull, 'w')) # discard stdout

def check_progs():
    if (not EXEC_ADVDEF):
        print("ERROR: advdef binary not found!")
    if (not EXEC_IMAGEMAGICK):
        print("ERROR: imagemagick/convert binary not found!")
    if (not EXEC_OPTIPNG):
        print("ERROR: optipng not found!")

    if not (EXEC_ADVDEF and EXEC_IMAGEMAGICK and EXEC_OPTIPNG):
        sys.exit(1)

class ProcessingStatus(enum.Enum):
    UNCHANGED = 0
    OPTIMIZED = 1
    REVERTED_THRESHOLD = 2 # didn't grow, but was larger than the threshold
    REVERTED_GREW = 3

class ProcessingResult:
    def __init__(self, name, status, size_initial, size_after):
        self.name = name
        self.status = status
        self.size_initial = size_initial
        self.size_after = size_after

def optimize_image(image):
    size_initial = os.path.getsize(image)
    with open(image, 'rb') as f:
        initial_file_content = f.read()

    tmpimage  = image + ".tmp"

    run_imagemagick(image, tmpimage)
    verify_images(image, tmpimage, "imagemagick")

    run_optipng(image, tmpimage)
    verify_images(image, tmpimage, "optipng")

    run_advdef(image, tmpimage)
    verify_images(image, tmpimage, "advdef")

    size_after = os.path.getsize(image)
    size_delta = size_after - size_initial
    perc_delta = (size_delta/size_initial) *100

    if os.path.isfile(tmpimage): # clean up
        os.remove(tmpimage)

    summary_string = None
    status = None
    if size_after == size_initial:
        # probably already optimized with this script
        status = ProcessingStatus.UNCHANGED
    elif perc_delta*-1 < THRESHOLD:
        # changed size, but exceeds the threshold
        if size_after < size_initial:
            summary_string = "not replacing {image}, as the reduction was less than the threshold. Changed from {size_initial} to {size_after}, {size_delta}b, {perc_delta}%"
            status = ProcessingStatus.REVERTED_THRESHOLD
        else:
            # this should be unreachable, as the verify_images call above would have reverted the file
            summary_string = "file {image} grew in size from {size_initial} to {size_after}, {size_delta}b, {perc_delta}%"
            status = ProcessingStatus.REVERTED_GREW
    else:
        summary_string = "optimized  {image}  from {size_initial} to {size_after}, {size_delta}b, {perc_delta}%"
        status = ProcessingStatus.OPTIMIZED

    # If the file didn't shrink sufficiently, write back the original version
    if status != ProcessingStatus.OPTIMIZED:
        with open(image, 'wb') as f:
            f.write(initial_file_content)

    if summary_string:
        debugprint(summary_string.format(image=image, size_initial=size_initial, size_after=size_after, size_delta=size_delta, perc_delta=str(perc_delta)[0:6]))

    return ProcessingResult(image, status, size_initial, size_after)

check_progs() # all tools available? if not: exit

# do the crushing
p = Pool(MAX_THREADS)
result_list = p.map(optimize_image, set(input_files))

# obtain stats
size_before = 0
size_after = 0
files_optimized = 0
threshold_hit = False
for i in result_list:
    if i.status == ProcessingStatus.OPTIMIZED:
        size_before += i.size_initial
        size_after += i.size_after
        files_optimized += 1
    if i.status == ProcessingStatus.REVERTED_THRESHOLD:
        threshold_hit = True

# print stats
if (files_optimized):
    print("{files_optimized} of {files_processed} files optimized, {size_before} bytes reduced to {size_after} bytes; {size_diff} bytes, {percentage_delta}%".format(files_optimized = files_optimized, files_processed = len(result_list), size_before = size_before, size_after=size_after, size_diff = size_after - size_before, percentage_delta = str((size_after - size_before)/(size_before)*100)[0:6]))
else:
    print("Nothing optimized")

if threshold_hit:
    print("The following files could be reduced, but didn't meet the optimization threshold ({threshold}%), the --threshold option controls this".format(threshold=str(THRESHOLD)))
    for i in result_list:
        if i.status == ProcessingStatus.REVERTED_THRESHOLD:
            print("{percentage_delta}% {name}, {size_initial} bytes reduced to {size_after} bytes; {size_diff} bytes".format(
                name=i.name,
                size_initial=i.size_initial,
                size_after=i.size_after,
                size_diff=i.size_after - i.size_initial,
                percentage_delta = str((i.size_after - i.size_initial)/(i.size_initial)*100)[0:6]))
