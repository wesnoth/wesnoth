import os
from time import time
from zlib import crc32

# #
# Functions
# #

# get the CRC of a file's contents
def getCRC32(file):
    src_file = open(file, "r", encoding="utf-8")
    file_contents = src_file.read()
    src_file.close()

    return crc32(file_contents.encode("utf8"))

# recursively get all files in all subdirectories
def getAllFiles(start):
    file_list = []

    for root, _, files in os.walk(start):
        for file in files:
            file_list.append(root+"/"+file)

    return file_list

# #
# Variables
# #
today = time()
yesterday = time() - (24*60*60)
mtime_file = "build/cmake_mtime_crc.txt"

# #
# Run
# #
file_list = getAllFiles("src")

# get the current CRC all relevant files in src/, and set their mtime to yesterday
crc_dict_curr = {}
for file in file_list:
    os.utime(file, (yesterday, yesterday))
    if (file.endswith(".cpp") or file.endswith(".hpp") or file.endswith(".tpp") or file.endswith(".c") or file.endswith(".h")) and "CMakeFiles" not in file:
        crc_dict_curr.update({file : getCRC32(file)})

# if there's an existing stored list of CRCs, read that in
if os.path.isfile(mtime_file):
    readfile = open(mtime_file, "r", encoding="utf-8")
    crc_dict_prev = {}

    for line in readfile:
        line_list = line.strip().split(":")
        crc_dict_prev.update({line_list[0] : int(line_list[1])})
    readfile.close()

# compare file CRCs between the set of CRCs from the previous run and the current run
# if the file has changed, or is brand new, set its mtime to today so that cmake will know to recompile it
    for key, value in crc_dict_curr.items():
        if key in crc_dict_prev:
            if value != crc_dict_prev[key]:
                os.utime(key, (today, today))
        else:
            os.utime(key, (today, today))

# write out the new set of file CRCs
writefile = open(mtime_file, "w")
for key, value in crc_dict_curr.items():
    writefile.write(key+":"+str(value)+"\n")
writefile.close()

print("Updated cmake mtime data!")
