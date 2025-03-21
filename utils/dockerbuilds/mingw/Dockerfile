FROM wesnoth/wesnoth:mingw-master
ENV DEBIAN_FRONTEND=noninteractive

COPY get_dlls.py /scripts/get_dlls.py

ENTRYPOINT cd /output && scons -j `nproc` arch=x86-64 prefix=/msys64/mingw64 gtkdir=/msys64/mingw64 host=x86_64-w64-mingw32 -Y /wesnoth force_color=true && python3 /scripts/get_dlls.py && rm -rf packaging && ln -sf /wesnoth/doc /wesnoth/packaging /wesnoth/data /wesnoth/fonts /wesnoth/images /wesnoth/sounds /wesnoth/README.md /wesnoth/copyright /wesnoth/COPYING /wesnoth/changelog.md /wesnoth/cwesnoth.cmd . && cp /usr/x86_64-w64-mingw32/bin/libssp-0.dll . && scons -Y /wesnoth windows-installer && rm packaging/windows/*.o
