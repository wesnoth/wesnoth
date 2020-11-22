#!/usr/bin/bash
echo "python3: $(which python3)"
echo "sqlite3.exe: $(which sqlite3.exe)"
echo "cmd.exe: $(which cmd.exe)"
echo "MSBuild.exe: $(which MSBuild.exe)"
START_DIR="$PWD"
cd ".."
git clone --depth=1 https://github.com/microsoft/vcpkg.git vcpkg
cd vcpkg
cmd.exe //C bootstrap-vcpkg.bat
cmd.exe //C 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat' amd64 '&&' vcpkg integrate install
alias make="make -j2"
ls -Al
./vcpkg.exe install sdl2:x64-windows sdl2-image:x64-windows sdl2-image[libjpeg-turbo]:x64-windows sdl2-mixer:x64-windows sdl2-ttf:x64-windows bzip2:x64-windows zlib:x64-windows pango:x64-windows cairo:x64-windows fontconfig:x64-windows libvorbis:x64-windows libogg:x64-windows boost-filesystem:x64-windows boost-iostreams:x64-windows boost-locale:x64-windows boost-random:x64-windows boost-regex:x64-windows boost-asio:x64-windows boost-program-options:x64-windows boost-system:x64-windows boost-thread:x64-windows boost-bimap:x64-windows boost-multi-array:x64-windows boost-ptr-container:x64-windows boost-logic:x64-windows boost-format:x64-windows
rm -R downloads
rm -R buildtrees
