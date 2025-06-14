#!/bin/bash -xe

SOURCES=(
https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz
https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.17.tar.gz
https://ftp.gnu.org/pub/gnu/gettext/gettext-0.21.1.tar.gz
https://sourceforge.net/projects/libpng/files/libpng16/1.6.39/libpng-1.6.39.tar.xz
https://sourceforge.net/projects/freetype/files/freetype2/2.13.0/freetype-2.13.0.tar.xz
https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.42/pcre2-10.42.tar.bz2
https://github.com/libffi/libffi/releases/download/v3.4.4/libffi-3.4.4.tar.gz
https://download.gnome.org/sources/glib/2.76/glib-2.76.1.tar.xz
https://www.cairographics.org/releases/pixman-0.42.2.tar.gz
https://github.com/libexpat/libexpat/releases/download/R_2_5_0/expat-2.5.0.tar.xz
https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.14.2.tar.xz
https://www.cairographics.org/releases/cairo-1.16.0.tar.xz
https://github.com/harfbuzz/harfbuzz/releases/download/7.1.0/harfbuzz-7.1.0.tar.xz
https://download.gnome.org/sources/pango/1.50/pango-1.50.14.tar.xz
https://github.com/libsdl-org/SDL/releases/download/release-2.30.12/SDL2-2.30.12.tar.gz
https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-1.4.0.tar.gz
https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.6.3.tar.gz
https://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.xz
https://ftp.osuosl.org/pub/xiph/releases/vorbis/libvorbis-1.3.7.tar.xz
https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.3/SDL2_mixer-2.6.3.tar.gz
https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.bz2
https://www.openssl.org/source/openssl-3.4.1.tar.gz
https://curl.se/download/curl-8.1.1.tar.xz
)
PACKAGES=()


ORIGIN=`pwd`
: ${BUILDDIR:=/tmp/android-build}
: ${PREFIXDIR:=/tmp/android-prefix}
: ${DOWNLOADDIR:=/tmp/android-dl}
: ${ARCHS:=x86_64 armeabi-v7a arm64-v8a x86}

export PREFIXDIR

./setup-toolchains.py

mkdir -p $BUILDDIR/src
pushd $BUILDDIR/src
for url in ${SOURCES[@]}
do
	wget -nc $url -P $DOWNLOADDIR
	archive=`basename $url`
	package=${archive%.*.*}
	if [ ! -d $package ]
	then
		tar -xf $DOWNLOADDIR/$archive
		if [ -f $ORIGIN/${package%-*}.patch ]
		then
			patch=$ORIGIN/${package%-*}.patch
			patch -d $package -p1 -i $patch
		fi
		if [ -f $ORIGIN/${package%-*}.autotools.patch ]
		then
			patch=$ORIGIN/${package%-*}.autotools.patch
			pushd $package
			patch -p1 -i $patch
			if [[ $package == *"SDL2"* ]]
			then
				./autogen.sh
				if [ -f android-project/app/jni/Android.mk ]
				then
					mkdir -p ../SDL2-ndk-build/jni
					cp android-project/app/jni/Android.mk ../SDL2-ndk-build/jni/
					cp android-project/app/jni/Application.mk ../SDL2-ndk-build/jni/
				fi
				ln -sf ../../$package ../SDL2-ndk-build/jni/${package%-*}
			else
				autoreconf
			fi
			popd
		fi
	fi

	PACKAGES+=($package)
done
popd

for prefix in $PREFIXDIR/*
do
	abi=`basename $prefix`
	if [[ ! " $ARCHS " =~ " $abi " ]] then
		continue
	fi
	rm -rf $BUILDDIR/$abi

	. $PREFIXDIR/$abi/android.env
	export PKG_CONFIG_PATH=$PREFIXDIR/$abi/lib/pkgconfig

	for package in ${PACKAGES[@]}
	do
		if [ -f ${package%-*}.config ]
		then
			extra_flags=`cat ${package%-*}.config`
		else
			extra_flags=
		fi
		if [[ $package == boost* ]]
		then
			if [ -f ${package%%_*}.config ]
			then
				extra_flags=`cat ${package%%_*}.config`
			fi
		fi

		host_arg="--host=$HOST"

		src_dir=$BUILDDIR/src/$package
		build_dir=$BUILDDIR/$abi/$package
		mkdir -p $build_dir
		if [ -f $src_dir/configure ]
		then
			pushd $build_dir
			$src_dir/configure $host_arg --prefix=$PREFIXDIR/$abi $extra_flags
			make -j`nproc`
			make install
			popd
			continue
		fi
		if [ -f $src_dir/Configure ] #openssl's Perl configure
		then
			pushd $src_dir
			if [ -f Makefile ]
			then
				make clean
			fi
			./Configure --prefix=$PREFIXDIR/$abi $extra_flags android-$ANDROID_ARCH -D__ANDROID_API__=$API
			make -j`nproc`
			make install
			popd
			continue
		fi
		if [ -f $src_dir/meson.build ]
		then
			meson setup --cross-file $PREFIXDIR/$abi/android.ini $build_dir $src_dir -Dprefix=$PREFIXDIR/$abi $extra_flags
			ninja -C $build_dir
			ninja -C $build_dir install
			continue
		fi
		if [ -f $src_dir/Jamroot ]
		then
			pushd $src_dir
			rm -rf ./bin.v2
			if [ ! -f ./b2 ]
			then
				./bootstrap.sh
			fi
			if [[ $ANDROID_ARCH == *"arm"* ]]
			then
				BCABI="aapcs"
				BOOSTARCH="arm"
			elif [[ $ANDROID_ARCH == *"risc"* ]]
			then
				BCABI="sysv"
				BOOSTARCH="riscv"
			else
				BCABI="sysv"
				BOOSTARCH="x86"
			fi
			./b2 --user-config=$PREFIXDIR/$abi/android.jam --prefix=$PREFIXDIR/$abi target-os=android architecture=$BOOSTARCH address-model=$BITNESS abi=$BCABI binary-format=elf install $extra_flags
			popd
			continue
		fi
		if [ -f $src_dir/Makefile ] # bzip uses plain Make
		then
			pushd $src_dir
			make clean
			make install CC="$CC -fPIC" AR="$AR" RANLIB="$RANLIB" PREFIX=$PREFIXDIR/$abi
			popd
			continue
		fi
	done
done


cd $BUILDDIR/src/SDL2-ndk-build
webpPath=($BUILDDIR/src/libwebp-*)
sdl_imagePath=($BUILDDIR/src/SDL2_image-*)
ln -sf $webpPath $sdl_imagePath/external/libwebp
$NDK/ndk-build SUPPORT_WEBP=true APP_ABI="$ARCHS"
for lib in libs/*/*.so
do
	instdir=$(basename $(dirname $lib))
	cp $lib $PREFIXDIR/$instdir/lib/
done
