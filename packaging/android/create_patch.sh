#! /bin/bash

# Check number of arguments, print usage message
if (($# != 2)); then
	echo "Usage: ./create_patch.sh tag1 tag2"
	exit 0
fi

mkdir patch

# find out what changed between two given tags, then copy those files
# from git into a temporary patch directory.
for file in `git diff tags/$1..tags/$2 --name-only --diff-filter=AM`; do
	if [[ $file =~ ^(data|images|sounds|music|fonts|translations).* ]]; then
		if [[ ! -d `dirname $file` ]]; then
			mkdir -pv "patch/`dirname $file`"
		fi
		
		git show $2:$file > "patch/$file"
	fi
done

# write list of files to be deleted during application of patch into delete.list
# put it inside patch. This file must be in patch.zip root.
touch patch/delete.list
for file in `git diff tags/$1..tags/$2 --name-only --diff-filter=D`; do
	if [[ $file =~ ^(data|images|sounds|music|fonts|translations).* ]]; then
		echo "$file" >> patch/delete.list
	fi
done

cd patch
zip -r ../patch.zip *
cd ..
rm -rf patch
