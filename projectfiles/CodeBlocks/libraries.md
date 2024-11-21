## Manually updating the external dependencies
We do our best to keep the build dependency repository up-to-date with the latest versions of the libraries
within, as well as synced with any build requirement changes. If you want to build with a different version
of a certain library, however, you can fetch the relevant files at the links below:

[**Boost:**](http://www.boost.org/users/download) Do note that you will need to build the necessary Boost
libraries yourself. See the [instructions]#updating-boost-libraries)
in the dependency repository for details.

[**SDL 2:**](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.8) You'll want the "GCC 32/64-bit" Development
Libraries.

[**SDL_Image:**](https://github.com/libsdl-org/SDL_image/releases) Again, you'll want the "GCC 32/64-bit"
Development Libraries.

[**SDL_Mixer:**](https://github.com/libsdl-org/SDL_mixer/releases) Again, you'll want the "GCC 32/64-bit"
Development Libraries.

[**SDL_ttf:**](https://github.com/libsdl-org/SDL_ttf/releases) Again, you'll want the "GCC 32/64-bit"
Development Libraries.

The other libraries require complicated compilation procedures too in-depth to document here.

## Updating Boost libraries

Download and unpack the source of the libraries zlib, libbzip2, boost (version 1.64 or 1.68 preferred

- **Bzip2** https://www.sourceware.org/bzip2/
- **Boost** http://www.boost.org/users/download/
- **Zlib** http://www.zlib.net/

Open `cmd`, go to the Boost directory and open bootstrap.bat (with the correct paths of the other two libraries):

Then run this command:
```
.\b2 -sZLIB_SOURCE=..\zlib-1.3.1 -sBZIP2_SOURCE=..\bzip2-1.0.8 -j2 --with-coroutine --with-date_time --with-filesystem --with-iostreams --with-locale --with-program_options --with-random --with-regex --with-system --with-thread --with-test --with-timer --toolset=gcc --layout=system variant=release address-model=64
```
and
```
.\b2 -sZLIB_SOURCE=..\zlib-1.3.1 -sBZIP2_SOURCE=..\bzip2-1.0.8 -j2 --with-test --toolset=gcc --layout=system link=shared variant=release address-model=64
```
Depending on your boost version, you may need to replace `..\` with the absolute paths to zlib and bzip.
If you have multiple versions of gcc, add `--toolset=gcc-X.Y.Z` with **X.Y.Z** being the target version number.

Separate the required subset of the Boost source:
Run this command for generate `bcp.exe`
```
.\b2 tools\bcp --toolset=gcc
```
Create `include` in same path what `boost_...` and run this command:
```
dist\bin\bcp.exe algorithm asio assign bimap core coroutine container date_time dynamic_bitset exception filesystem graph iostreams iterator locale math mpl multi_array multi_index program_options process ptr_container random range regex serialization system spirit test logic boost\nondet_random.hpp boost\fusion\include\define_struct.hpp boost\scope_exit.hpp boost/typeof/incr_registration_group.hpp ..\include
```

Replace the outdated files in 'cb/lib' with those from 'boost_.../stage/lib' and those in 'cb/include/boost' with  the ones in 'boost_.../boost'.
