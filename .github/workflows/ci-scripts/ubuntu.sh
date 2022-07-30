#!/bin/bash

echo "Using linux:"
echo "BRANCH: $BRANCH"
echo "IMAGE: $IMAGE"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXX_STD: $CXX_STD"
echo "CFG: $CFG"
echo "LTO: $LTO"

version=$(grep '#define VERSION' src/wesconfig.h | cut -d\" -f2)
echo "Found version: $version"

if [ "$IMAGE" == "steamrt" ]; then
		cd utils/dockerbuilds/
		./make_steam_build || exit 1
		tar -cf steambuild.tar steambuild
		mv steambuild.tar ~/steambuild-$version.tar
elif [ "$IMAGE" == "mingw" ]; then
		git archive --format=tar HEAD > wesnoth.tar
		tar rf wesnoth.tar src/modules/
		bzip2 -z wesnoth.tar
		mv wesnoth.tar.bz2 ~/wesnoth-$version.tar.bz2
		cd utils/dockerbuilds/
		./make_mingw_build || exit 1
		cd mingwbuild
		mv ./wesnoth*-win64.exe ~/wesnoth-$version-win64.exe
else
		# pull the pre-created image
		docker build -t wesnoth-repo:"$IMAGE"-"$BRANCH" -f - . <<-EOF
			FROM wesnoth/wesnoth:$IMAGE-$BRANCH
			COPY ./ /home/wesnoth-CI/
			WORKDIR /home/wesnoth-CI
		EOF

		[[ $NLS == only ]] || tty=1  # something in the update-po4a-manual step hangs when building the xml (po4a-translate) and --tty is used

		docker run ${tty+--tty} --cap-add=ALL --privileged \
				--env BRANCH --env IMAGE --env NLS --env TOOL --env CC --env CXX \
				--env CXX_STD --env CFG --env LTO --env CLICOLOR_FORCE \
				wesnoth-repo:"$IMAGE"-"$BRANCH" ./.github/workflows/ci-scripts/docker.sh
fi
