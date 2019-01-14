#! /usr/bin/env python3

import os, sys, argparse, re, base64
from io import BytesIO

try:
    from PIL import Image
except ImportError:
    print("""Please install the Python Pillow Library to run this script.
You can download it from https://pypi.python.org/pypi/Pillow
On Debian and Ubuntu you can also type in a Terminal
sudo apt-get install python-pil""", file=sys.stderr)
    sys.exit(1)

suffix_re = re.compile(".*-(n|s|w|e|ne|nw|sw|se|[0-9]+)([-.]).*")
anim_re = re.compile(".*-(attack|defend|melee|ranged|magic|idle|die|dying|death|flying|leading|healing).*")
# the default mask is a png RGBA file encoded in base64
default_mask = b"""
iVBORw0KGgoAAAANSUhEUgAAAEgAAABICAYAAABV7bNHAAAA/UlEQVR42u3cWwoCMRBFQfe/6Tg/
gsg8b4bWxhLOAixEnSSdxxjjlpbX+KXuel8NcPIANQAKcGoDVAOU43QIUAFQhNMpQAVAAU6vABUA
BTi9AlQAFOD0ClABUIDTK0AFQAFOrwAVAAU4vQI0CzSP0z9AE0B/j/MKUAgE5y1AARCcjwBdBIKz
EqALQHA2AnQSCM5OgE4AwdnPp+cgX9C+gybyM+9/UJ5HDc9ieZY7rAflWXK1Jp1n28e+WJ6tZ3vz
eY6/OB+U5wieM4p5jgE7J51nFMGsRp5xKPNieUYyzazmGQs3N5/nagp3d+S5Hsf9QXmu6HKH2XeB
XBN40BNK3ENj+y1ozgAAAABJRU5ErkJggg=="""

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawTextHelpFormatter,
    description="Search png images not fitting in a hex",
    epilog="""return numbers of pixels out of the hex for each filename
(-1 if the image is not a standard 72x72 image and -f was not used)"""
    )
parser.add_argument("-m", "--mask",
                    action="store",
                    metavar="file",
                    help="""choose which image use as mask
(default is a mask embedded in the script)"""
                    )
parser.add_argument("-a", "--anim",
                    action="store_true",
                    help="""skip most animations images (containing:
-attack -defend -melee -ranged -magic -idle
-die -dying -death -healing -flying -leading)"""
                    )
parser.add_argument("-s", "--suffix",
                    action="store_true",
                    help="skip images with directional or numerical suffix"
                    )
parser.add_argument("-r", "--regex",
                    action="store",
                    metavar="REG",
                    help="""skip images matching the case-insensitive
regular expression REG (Python)"""
                    )
parser.add_argument("-f", "--format",
                    action="store_true",
                    help="skip images which are not in 72x72 format"
                    )
parser.add_argument("-q", "--quiet",
                    action="store_true",
                    help="only display results"
                    )
parser.add_argument("dirs",
                    action="store",
                    nargs="*",
                    help="directories to check",
                    default=os.getcwd()
                    )
args=parser.parse_args()

# get all the PNG images
images = []
for folder in args.dirs:
    for root, dirs, files in os.walk(folder):
        for filename in files:
            if filename.lower().endswith(".png"):
                images.append(os.path.join(root, filename))

# remove files matching the regexs
if args.anim:
    images = [elem for elem in images if not anim_re.match(elem)]
if args.suffix:
    images = [elem for elem in images if not suffix_re.match(elem)]
if args.regex:
    images = [elem for elem in images if not re.match(args.regex, elem)]

images.sort()

if not args.quiet:
    print("""Search 72x72 images not fitting in a hex
in directories: {}
Using alphamask image: {}
Skipping files matching regex: {}
Pixels out of hex : filename""".format(", ".join(args.dirs),
                                       args.mask if args.mask else "",
                                       args.regex if args.regex else ""))

# open the mask
if args.mask:
    try:
        mask = Image.open(args.mask)
    except OSError:
        print("cannot read mask file {}, exiting".format(args.mask), file=sys.stderr)
        sys.exit(1)
else:
    mask = Image.open(BytesIO(base64.b64decode(default_mask)))
mask_data = mask.getdata(3) # get alpha channel values

for fn in images:
    try:
        img = Image.open(fn)
    except OSError:
        print("cannot read file {}, skipping".format(fn), file=sys.stderr)
        continue

    # check if the image size is incorrect first
    if img.size != (72, 72):
        if args.format:
            continue
        else:
            px = -1
    else:
        px = 0

        comp = Image.alpha_composite(img, mask)

        # compare the alpha channel pixel by pixel
        # this was the method used by the Bash version
        comp_data = comp.getdata(3)
        for i, mask_alpha in enumerate(mask_data):
            if comp_data[i] != mask_alpha:
                px += 1

    if px != 0:
        print("{:d}\t: {}".format(px, fn))

sys.exit(0)
