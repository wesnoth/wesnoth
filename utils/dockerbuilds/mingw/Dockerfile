FROM wesnoth/wesnoth:mingw-master
ENV DEBIAN_FRONTEND=noninteractive

COPY get_dlls.py /scripts/get_dlls.py

ENTRYPOINT mkdir -p /build && cd /build && scons -j `nproc` arch=x86-64 prefix=/windows/mingw64 gtkdir=/windows/mingw64 host=x86_64-w64-mingw32 -Y /wesnoth && cp /build/wesnoth.exe /output/ && cp /build/wesnothd.exe /output/ && cd /output && python3 /scripts/get_dlls.py
