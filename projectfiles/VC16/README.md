# Building with VS2019
 * Install [vcpkg](https://github.com/Microsoft/vcpkg).
 * Make vcpkg available to Visual Studio with `vcpkg integrate install` (may require admin rights).
 * Use vcpkg to install all dependencies:
```vcpkg install sdl2:x64-windows sdl2-image:x64-windows sdl2-image[libjpeg-turbo]:x64-windows sdl2-mixer[libvorbis,dynamic-load]:x64-windows sdl2-ttf:x64-windows bzip2:x64-windows zlib:x64-windows pango:x64-windows cairo:x64-windows fontconfig:x64-windows libvorbis:x64-windows libogg:x64-windows boost-filesystem:x64-windows boost-iostreams:x64-windows boost-locale[icu]:x64-windows boost-random:x64-windows boost-regex[icu]:x64-windows boost-asio:x64-windows boost-program-options:x64-windows boost-system:x64-windows boost-bimap:x64-windows boost-multi-array:x64-windows boost-logic:x64-windows boost-format:x64-windows boost-coroutine:x64-windows boost-context:x64-windows boost-circular-buffer:x64-windows boost-scope-exit:x64-windows```
  * The above dependencies were taken from github actions CI script.
  * Given the above will be compiling both the release and, as available, the debug versions of all those libraries and their dependencies, this may take an hour or more. The travis-ci script works around this somewhat by aliasing `make` to `make -j4` in order to force four threads to be used rather than just one thread.
