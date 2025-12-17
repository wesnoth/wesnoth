#! /bin/bash

# Check number of arguments, print usage message
if (($# != 2)); then
	echo "Usage: ./create_patch.sh tag1 tag2"
	echo "This will checkout tag2, backup any critical files."
	exit 0
fi

git checkout $2

mkdir patch

# find out what changed between two given tags
for file in `git diff tags/$1..tags/$2 --name-only --diff-filter=AM`; do
	if [[ $file =~ ^(data|images|sounds|music|fonts|translations).* ]]; then
		install -Dv "../../$file" "patch/$file"
	fi
done

# list of files to be deleted during application of patch

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

git checkout HEAD@{1}
