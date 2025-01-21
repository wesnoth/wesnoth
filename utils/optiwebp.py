#!/usr/bin/env python3

##
# This script uses cwebp to convert existing png/jpg images to webp, or reconvert existing webp images, to see if doing so meaningfully reduces their size.
##

import argparse
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import time

def signal_handler(signal, func):
    print("\nProcessing interrupted.")
    shutil.rmtree(options.tempdir)
    sys.exit(1)

signal.signal(signal.SIGINT, signal_handler)

args = argparse.ArgumentParser()

args.add_argument("--minpercent", default=90, help="The new file size must be reduced by to least this percentage compared to the original. Default 90%% (10%% reduction).")
args.add_argument("--nodryrun", action="store_true", help="Whether to replace the original file with the converted one if its size is reduced enough. Does not replace by default.")
args.add_argument("--quality", default="90", help="The quality to use for lossy compression. Defaults to 90.")
args.add_argument("--repo", default=os.getcwd(), help="The root of the Wesnoth repository. Defaults to the current directory.")
args.add_argument("--tempdir", default=os.path.join(os.getcwd(), "temp_webp"), help="The root of the Wesnoth repository. Defaults to 'temp_webp/' in the current directory.")

options = args.parse_args()

# test we're in a repo root
if not os.path.isfile(os.path.join(options.repo, "changelog.md")):
    print("Didn't find changelog.md so assuming this isn't the repo root, exiting.")
    sys.exit(1)

# test cwebp presence
if subprocess.run(["cwebp", "-version"], stdout=subprocess.DEVNULL).returncode != 0:
    print("cwebp executable not found in PATH, exiting.")
    sys.exit(1)

image_dirs = [
    os.path.join("data", "campaigns", "Dead_Water", "images", "maps"),
    os.path.join("data", "campaigns", "Dead_Water", "images", "portraits"),
    os.path.join("data", "campaigns", "Descent_Into_Darkness", "images", "maps"),
    os.path.join("data", "campaigns", "Descent_Into_Darkness", "images", "portraits"),
    os.path.join("data", "campaigns", "Descent_Into_Darkness", "images", "story"),
    os.path.join("data", "campaigns", "Eastern_Invasion", "images", "portraits"),
    os.path.join("data", "campaigns", "Eastern_Invasion", "images", "maps"),
    os.path.join("data", "campaigns", "Heir_To_The_Throne", "images", "portraits"),
    os.path.join("data", "campaigns", "Heir_To_The_Throne", "images", "maps"),
    os.path.join("data", "campaigns", "Heir_To_The_Throne", "images", "story"),
    os.path.join("data", "campaigns", "Legend_of_Wesmere", "images", "story"),
    os.path.join("data", "campaigns", "Legend_of_Wesmere", "images", "portraits"),
    os.path.join("data", "campaigns", "Legend_of_Wesmere", "images", "l10n"),
    os.path.join("data", "campaigns", "Liberty", "images", "portraits"),
    os.path.join("data", "campaigns", "Liberty", "images", "maps"),
    os.path.join("data", "campaigns", "Liberty", "images", "story"),
    os.path.join("data", "campaigns", "Northern_Rebirth", "images", "maps"),
    os.path.join("data", "campaigns", "Northern_Rebirth", "images", "portraits"),
    os.path.join("data", "campaigns", "Sceptre_of_Fire", "images", "maps"),
    os.path.join("data", "campaigns", "Sceptre_of_Fire", "images", "portraits"),
    os.path.join("data", "campaigns", "Secrets_of_the_Ancients", "images", "story"),
    os.path.join("data", "campaigns", "Secrets_of_the_Ancients", "images", "portraits"),
    os.path.join("data", "campaigns", "Son_Of_The_Black_Eye", "images", "maps"),
    os.path.join("data", "campaigns", "Son_Of_The_Black_Eye", "images", "portraits"),
    os.path.join("data", "campaigns", "The_Deceivers_Gambit", "images", "story"),
    os.path.join("data", "campaigns", "The_Deceivers_Gambit", "images", "portraits"),
    os.path.join("data", "campaigns", "The_Hammer_of_Thursagan", "images", "maps"),
    os.path.join("data", "campaigns", "The_Hammer_of_Thursagan", "images", "portraits"),
    os.path.join("data", "campaigns", "The_Rise_Of_Wesnoth", "images", "maps"),
    os.path.join("data", "campaigns", "The_Rise_Of_Wesnoth", "images", "portraits"),
    os.path.join("data", "campaigns", "The_Rise_Of_Wesnoth", "images", "story"),
    os.path.join("data", "campaigns", "The_South_Guard", "images", "portraits"),
    os.path.join("data", "campaigns", "The_South_Guard", "images", "maps"),
    os.path.join("data", "campaigns", "The_South_Guard", "images", "story"),
    os.path.join("data", "campaigns", "tutorial", "images", "portraits"),
    os.path.join("data", "campaigns", "Two_Brothers", "images", "maps"),
    os.path.join("data", "campaigns", "Two_Brothers", "images", "portraits"),
    os.path.join("data", "campaigns", "Two_Brothers", "images", "story"),
    os.path.join("data", "campaigns", "Under_the_Burning_Suns", "images", "portraits"),
    os.path.join("data", "campaigns", "Winds_of_Fate", "images", "portraits"),
    os.path.join("data", "campaigns", "Winds_of_Fate", "images", "maps"),
    os.path.join("data", "campaigns", "Winds_of_Fate", "images", "story"),
    os.path.join("data", "core", "images", "maps"),
    os.path.join("data", "core", "images", "story"),
    os.path.join("data", "core", "images", "portraits")
]

results_dict = {}
lossy_args = ["cwebp", "-mt", "-m", "6", "-q", options.quality, "-alpha_q", "100"]
lossless_args = ["cwebp", "-mt", "-z", "9", "-lossless", "-exact"]

os.makedirs(options.tempdir, exist_ok=True)

start = time.time()
count = 0

for image_dir in image_dirs:
    print("Processing "+image_dir)
    for image_root, _, files in os.walk(os.path.join(options.repo, image_dir)):
        for filename in files:
            count += 1
            if count % 100 == 0:
                print("Processing file {count}".format(count=count))

            filetype = Path(filename).suffix
            subprocess_args = []
            if filetype == ".png":
                subprocess_args = lossless_args
            elif filetype == ".webp":
                with open(os.path.join(image_root, filename), "rb") as img:
                    # discard first 12 bytes of WebP "magic number" and file size
                    img.read(12)
                    # read type of WebP file
                    # VP8L = lossless
                    # VP8 = lossy, so skip: lossy -> lossy is bad
                    # VP8X = can either, so assume lossless (better to try to go from lossy->lossless than the reverse)
                    webp_type = img.read(4)
                    if webp_type == b"VP8L" or webp_type == b"VP8X":
                        subprocess_args = lossless_args
                    elif webp_type == b"VP8 ":
                        continue
                    else:
                        print(filename, "is not a valid WebP file", file=sys.stderr)
                        continue
            elif filetype in (".jpg", ".jpeg"):
                subprocess_args = lossy_args
            else:
                continue

            initial_file = os.path.join(image_root, filename)
            webp_file = ""
            if filetype in (".jpg", ".png"):
                webp_file = os.path.join(options.tempdir, filename)[:-3]+"webp"
            elif filetype == ".jpeg":
                webp_file = os.path.join(options.tempdir, filename)[:-4]+"webp"
            else:
                webp_file = os.path.join(options.tempdir, filename)

            result = subprocess.run(subprocess_args + [initial_file, "-o", webp_file], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            if result.returncode != 0:
                print(initial_file+" failed conversion to webp.")
                continue

            initial_size = os.path.getsize(initial_file)
            converted_size = os.path.getsize(webp_file)
            percentage_change = round((converted_size/initial_size)*100, 2)
            absolute_change = initial_size - converted_size

            results_dict[initial_file] = {
                "type": filetype,
                "new_file": webp_file,
                "old_size": initial_size,
                "new_size": converted_size,
                "change_in_bytes": absolute_change,
                "change_in_percent": percentage_change
            }

            if options.nodryrun and percentage_change < options.minpercent:
                os.remove(initial_file)
                shutil.copy(webp_file, image_root)

            os.remove(webp_file)

initial_total_size = 0
final_total_size = 0

try:
    with open("conversion-good.tsv", "w") as good_f, open("conversion-bad.tsv", "w") as bad_f:
        good_f.write("filename\told_size\tnew_size\tchange_in_percent\tchange_in_bytes\n")
        bad_f.write("filename\told_size\tnew_size\tchange_in_percent\tchange_in_bytes\n")

        for filename, data in results_dict.items():
            line = "{filename}\t{old_size}\t{new_size}\t{change_in_percent}\t{change_in_bytes}\n".format(filename=filename, old_size=data["old_size"], new_size=data["new_size"], change_in_percent=data["change_in_percent"], change_in_bytes=data["change_in_bytes"])

            if data["change_in_percent"] < options.minpercent:
                initial_total_size = initial_total_size + data["old_size"]
                final_total_size = final_total_size + data["new_size"]

                good_f.write(line)
            else:
                bad_f.write(line)
except OSError:
    print("Error writing summary output files!")
    sys.exit(1)

total_percentage_change = round((final_total_size/initial_total_size)*100, 2)
duration = time.time() - start

hours_duration = int(duration / 3600)
minutes_duration = int((duration - (hours_duration * 3600)) / 60)
seconds_duration = int(duration % 60)

os.rmdir(options.tempdir)
print("Total size for converted images changed from {initial} to {final} ({percentage}%)".format(initial=initial_total_size, final=final_total_size, percentage=total_percentage_change))
print("Took {hours:02d}:{minutes:02d}:{seconds:02d} to process {count} files.".format(hours=hours_duration, minutes=minutes_duration, seconds=seconds_duration, count=len(results_dict)))
