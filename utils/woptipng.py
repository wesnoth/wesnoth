#!/usr/bin/env python3

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
import subprocess # launch advdef, optipng, imagemagick
import os # os rename, niceness
import shutil # copy files
import argparse # argument parsing
import sys # sys.exit


parser = argparse.ArgumentParser()
parser.add_argument("inpath", help="file or path (recursively) to be searched for crushable pngss", metavar='path', nargs='+', type=str)
parser.add_argument("-d", "--debug", help="print debug information", action='store_true')
parser.add_argument("-t", "--threshold", help="size reduction below this percentage will be discarded, default: 10%", metavar='n', nargs='?', default=10, type=float)
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

# get sizes
file_list = []
for file_ in input_files:
    file_list.append([file_, os.path.getsize(file_), None])

print(" done")
if (bad_input_files):
    print("WARNING: can't handle following files:' ")
    print(', '.join(bad_input_files) + "\n")



print("Compressing " + str(len(file_list)) + " pngs...")

def debugprint(arg):
    if (DEBUG):
        print(arg)

def images_identical(image1, image2):
    return PIL.open(image1).tobytes() == PIL.open(image2).tobytes()

def verify_images(source_img, new_img, transform):
    no_change = images_identical(source_img, new_img) # image pixels values remain unaltered, we want this
    image_got_smaller = os.path.getsize(source_img) > os.path.getsize(new_img)
    debugprint("size reduction: " + str(os.path.getsize(source_img) - os.path.getsize(new_img)))

    if (no_change and image_got_smaller):
        os.rename(new_img, source_img) # move new image to old image // os.rename(src, dest)
    else: # we can't os.rename(image_after, image_before) because that would leave us with no source for the next transform
        shutil.copy(source_img, new_img) # override new image with old image // shutil.copy(src, dest)
        debugprint(("TRANSFORMATION unsuccessfull: + " + transform + ", REVERTING " + source_img))

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


def optimize_image(image):
    size_initial = os.path.getsize(image)
    with open(image, 'rb') as f:
        initial_file_content = f.read()


    size_initial = os.path.getsize(image)
    it=0
    size_after = 0
    size_before = os.path.getsize(image)
    while ((size_before > size_after) or (not it)):
        it+=1
        debugprint(("iteration " + str(it)))
        size_before = os.path.getsize(image)
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

    if (DEBUG and (size_initial < size_after)):
        debugprint("WARNING: " + str(image) + "got bigger !")
    if os.path.isfile(tmpimage): # clean up
        os.remove(tmpimage)

    if (os.path.getsize(image) > size_initial) or (perc_delta*-1 < THRESHOLD) : # got bigger, or exceeds threshold
        with open(image, 'wb') as f: # write back original file
            f.write(initial_file_content)
    else:
        print("optimized  {image}  from {size_initial} to {size_after}, {size_delta}b, {perc_delta}%".format(image=image, size_initial=size_initial, size_after=size_after, size_delta=size_delta, perc_delta=str(perc_delta)[0:6]))

check_progs() # all tools available? if not: exit

# do the crushing
p = Pool(MAX_THREADS)
p.map(optimize_image, set(input_files))

# update file_list
for index, file_ in enumerate(file_list):
    file_list[index][2] = os.path.getsize(file_[0]) # write new filesize into list

# obtain stats
size_before = 0
size_after = 0
files_optimized = 0
for i in file_list:
    if i[1] > i[2]: # file got smaller
        size_before += i[1]
        size_after += i[2]
        files_optimized += 1
# print stats
if (files_optimized):
    print("{files_optimized} of {files_processed} files optimized, {size_before} bytes reduced to {size_after} bytes; {size_diff} bytes, {percentage_delta}%".format(files_optimized = files_optimized, files_processed = len(file_list), size_before = size_before, size_after=size_after, size_diff = size_after - size_before, percentage_delta = str((size_after - size_before)/(size_before)*100)[0:6]))

    print("Optimization threshold was " + str(THRESHOLD) + "%")
else:
    print("Nothing optimized")
