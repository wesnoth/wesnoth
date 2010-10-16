#!/bin/sh
#
#  Check the format of PNGs
#
#  With --verbose switch, it reports every file and its non-RGBA format individually
#

rgb=0
rgba=0
gray=0
graya=0
index=0
indexa=0
other=0
filelist=""
verbose="no"

while [ "${1}" != "" ]; do
    if [ "${1}" = "--verbose" ] || [ "${1}" = "-v" ]; then
        verbose="yes"
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

report_file()
{
    if [ "$verbose" = "yes" ]; then
        echo "File $1 is in format $2"
    fi
}

for i in $filelist; do
    result=`pngcheck $i`
    if [ $? -ne 0 ]; then
        echo "Failure executing pngcheck. Exiting."
        exit 1
    fi
    if echo $result|grep 'RGB+alpha'>/dev/null
    then
        rgba=$(($rgba+1))
    elif echo $result|grep 'RGB'>/dev/null
    then
        rgb=$(($rgb+1))
        report_file $i 'RGB'
    elif echo $result|grep 'grayscale+alpha'>/dev/null
    then
        graya=$(($graya+1))
        report_file $i 'grayscale+alpha'
    elif echo $result|grep 'grayscale'>/dev/null
    then
        grey=$(($grey+1))
        report_file $i 'grayscale'
    elif echo $result|grep 'colormap+trns'>/dev/null
    then
        indexa=$(($indexa+1))
        report_file $i 'palette+trns (alpha)'
    elif echo $result|grep 'colormap'>/dev/null
    then
        index=$(($index+1))
        report_file $i 'palette'
    else
        other=$(($other+1))
        report_file $i "`echo $result | sed -e 's/^.*(//;s/).*$//;'`"
    echo $result
    fi
done
echo "RGBA: $rgba RGB: $rgb Gray+A: $graya Gray: $gray Index+A: $indexa Index: $index Other: $other"

