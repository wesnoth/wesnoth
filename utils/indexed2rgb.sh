#!/bin/sh
#
#  Script to convert PNGs from indexed to RGB mode.
#
#  Run-time requirements: ImageMagick, pngcheck
#  If --optipng is used, also wesnoth-optipng's requirements
#

run_optipng="no"

while [ "${1}" != "" ]; do
    if [ "${1}" = "--help" ] || [ "${1}" = "-h" ]; then
        cat << EOF
Automatic PNG RGBA conversion script

Usage:
  `basename ${0}` [{--optipng|-o}] [{--help|-h}] {<file>|<dir>}...

Files given on command line, are converted if they are not already in RGB+alpha mode (PNG mode 6).
Directories will be recursively parsed for PNG files.

Switches:
  --help    / -h   Displays this help text.

  --optipng / -o   Runs wesnoth-optipng on the converted files afterwards.

This tool requires ImageMagick and pngcheck to be installed and available in PATH.
If the --optipng option is used, it also requires wesnoth-optipng to be present in the same directory.
EOF
        exit 0
    elif [ "${1}" = "--optipng" ] || [ "${1}" = "-o" ]; then
        run_optipng="yes"
        shift
    elif [ -d "${1}" ]; then
        filelist="${filelist} `find ${1} -iname "*.png"`"
        shift
    elif [ -f "${1}" ]; then
        filelist="${filelist} ${1}"
        shift
    else
        echo "Argument ${1} is not a known switch nor is it a directory or file. Exiting."
        exit 1
    fi
done

input_number=`echo $filelist|wc -w`
converted=""

if [ "$input_number" -eq "0" ]; then
    echo "No input files"
    exit 0
fi

echo -n "Converting $input_number files to RGB mode... "

for f in $filelist
do
    if !(pngcheck $f|grep -q 'RGB+alpha')
    then
        # -define png:color-type=6 is the PNG way to specify RGBA, it doesn't always work though
        # http://www.imagemagick.org/Usage/formats/#png_write
        convert -define png:color-type=6 $f $f.new

        if !(pngcheck $f.new|grep -q 'RGB+alpha')
        then
            # asking the PNG writer nicely didn't work, force it instead
            convert -type TrueColorMatte $f $f.new
        fi

        if (pngcheck $f.new|grep -q 'RGB+alpha')
        then
            mv -f $f.new $f
            converted="${converted} ${f}"
        else
            rm $f.new
            echo "Failed to convert $f"
        fi
    fi
done

converted_number=`echo $converted|wc -w`

echo "Done."
echo "Changed $converted_number out of $input_number files."

if [ "$converted_number" -eq "0" ];then
    true
elif [ "$run_optipng" = "yes" ];then
    echo "Running optipng"
    `dirname ${0}`/wesnoth-optipng $converted
else
    echo "Now would be a good time to run optipng"
fi
