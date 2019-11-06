FROM rwgrim/msys2-cross
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y scons g++-mingw-w64-x86-64 pkg-config python3-pefile && \
    update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists

RUN pacman-cross -S --noconfirm \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-SDL2 \
    mingw-w64-x86_64-SDL2_image \
    mingw-w64-x86_64-SDL2_mixer \
    mingw-w64-x86_64-SDL2_ttf \
    mingw-w64-x86_64-pango

COPY get_dlls.py /scripts/get_dlls.py

ENTRYPOINT mkdir /build && cd /build && scons -j `nproc` arch=x86-64 prefix=/windows/mingw64 gtkdir=/windows/mingw64 host=x86_64-w64-mingw32 -Y /wesnoth && cp /build/wesnoth.exe /output/ && cd /output && python3 /scripts/get_dlls.py
